
#include "mt6835_spi_abz.h"
#include "board.h"
#include "hpm_clock_drv.h"
#include "hpm_common.h"
#include "hpm_crc_drv.h"
#include "hpm_crc_regs.h"
#include "hpm_qeiv2_drv.h"
#include "hpm_soc.h"
#include "hpm_spi_drv.h"
#include "hpm_spi_regs.h"
#include "pinmux.h"
#include "project_config.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static CRC_Type *mt_crc;
static uint32_t mt_ctc_ch;

int mt6835_enable_crc(CRC_Type *crc, uint32_t index)
{
    mt_crc = crc;
    mt_ctc_ch = index;
    clock_add_to_group(clock_crc0, 0);

    return 0;
}

int mt6835_spi_abz_init(SPI_Type *spi, uint32_t baud, QEIV2_Type *qei)
{
    init_spi_pins(spi);
    init_qeiv2_abz_pins(qei);
    board_init_qeiv2_clock(qei);

    mt6835_enable_crc(HPM_CRC, CRC_CHN_0);

    uint32_t spi_clock = board_init_spi_clock(spi);

    spi_timing_config_t timing_config = {0};
    timing_config.master_config.cs2sclk = spi_cs2sclk_half_sclk_1;
    timing_config.master_config.csht = spi_csht_half_sclk_1;
    timing_config.master_config.clk_src_freq_in_hz = spi_clock;
    timing_config.master_config.sclk_freq_in_hz = baud;
    if (spi_master_timing_init(spi, &timing_config) != status_success)
    {
        printf("SPI master timming init failed\n");
        while (1)
        {
        }
    }

    spi_format_config_t format_config = {0};
    spi_master_get_default_format_config(&format_config);
    format_config.common_config.data_len_in_bits = 8;
    format_config.common_config.lsb = false;
    format_config.common_config.mode = spi_master_mode;
    format_config.common_config.cpha = spi_sclk_sampling_even_clk_edges;
    format_config.common_config.cpol = spi_sclk_high_idle;
    spi_format_init(spi, &format_config);

    mt6835_set_abz_res(spi, 16384);

    qeiv2_mode_config_t mode_config = {0};
    /*  mode config */
    mode_config.work_mode = qeiv2_work_mode_abz;
    mode_config.spd_tmr_content_sel = qeiv2_spd_tmr_as_spd_tm;
    mode_config.z_count_inc_mode = qeiv2_z_count_inc_on_z_input_assert;
    mode_config.phcnt_max = 16384 * 4;
    mode_config.z_cali_enable = true;
    mode_config.z_cali_ignore_ab = false;
    mode_config.phcnt_idx = 0;
    qeiv2_config_mode(qei, &mode_config);
    qeiv2_set_z_phase(qei, 0);   /* setting z phase init value */
    qeiv2_set_phase_cnt(qei, 0); /* setting phase cnt init value */

    /*  cmp config */
    // phcnt_cmp_config.phcnt_cmp_value = ENCODER_MAX / 2;
    // phcnt_cmp_config.ignore_rotate_dir = true;
    // phcnt_cmp_config.ignore_zcmp = true;
    // qeiv2_config_phcnt_cmp_match_condition(qei, &phcnt_cmp_config);
    // qeiv2_enable_load_read_trigger_event(qei, QEIV2_EVENT_POSITION_COMPARE_FLAG_MASK);



    return 0;
}

hpm_stat_t mt6835_spi_read_angle_status(SPI_Type *spi, uint32_t *angle, uint8_t *status)
{
    hpm_stat_t stat;
    uint8_t cmd[2] = {0b1010 << 4 | 0, 3};
    uint8_t data[4] = {};
    // spi_set_transfer_mode(spi, spi_trans_write_read);
    // spi_set_write_data_count(spi, 2);
    // spi_set_read_data_count(spi, 4);
    // spi->CMD = SPI_CMD_CMD_SET(0xff);
    // stat = spi_write_read_data(spi, 1, cmd, 2, data, 4);

    static spi_control_config_t config = {
        .common_config.cs_index = spi_cs_0,
        .common_config.data_phase_fmt = spi_single_io_mode,
        .common_config.rx_dma_enable = false,
        .common_config.tx_dma_enable = false,
        .common_config.dummy_cnt = spi_dummy_count_2,
        .common_config.trans_mode = spi_trans_write_read,
        .master_config.addr_enable = false,
        .master_config.addr_phase_fmt = spi_address_phase_format_single_io_mode,
        .master_config.cmd_enable = false,
        .master_config.token_enable = false,
        .master_config.token_value = 0,
    };

    stat = spi_transfer(spi, &config, NULL, NULL, cmd, 2, data, 4);

    if (stat != status_success)
        return stat;

    if (mt_crc)
    {
        crc_channel_config_t cfg;
        cfg.preset = crc_preset_crc8;
        cfg.in_byte_order = crc_in_byte_order_msb;
        crc_setup_channel_config(mt_crc, mt_ctc_ch, &cfg);
        crc_calc_block_bytes(mt_crc, mt_ctc_ch, data, 3);
        uint8_t crc = crc_get_result(mt_crc, mt_ctc_ch);
        if (crc != data[3])
        {
            return status_fail;
        }
    }

    if (angle)
        *angle = data[0] << 13 | data[1] << 5 | data[2] >> 3;
    if (status)
        *status = data[2] & 0x07;
    return status_success;
}

hpm_stat_t mt6835_spi_read_reg(SPI_Type *spi, uint16_t addr, uint8_t *data)
{
    uint8_t cmd[2] = {0b0011 << 4 | ((addr >> 8) & 0xff), addr & 0xff};

    static spi_control_config_t config = {
        .common_config.cs_index = spi_cs_0,
        .common_config.data_phase_fmt = spi_single_io_mode,
        .common_config.rx_dma_enable = false,
        .common_config.tx_dma_enable = false,
        .common_config.dummy_cnt = spi_dummy_count_2,
        .common_config.trans_mode = spi_trans_write_read,
        .master_config.addr_enable = false,
        .master_config.addr_phase_fmt = spi_address_phase_format_single_io_mode,
        .master_config.cmd_enable = false,
        .master_config.token_enable = false,
        .master_config.token_value = 0,
    };

    return spi_transfer(spi, &config, NULL, NULL, cmd, 2, data, 1);
}

hpm_stat_t mt6835_spi_write_reg(SPI_Type *spi, uint16_t addr, uint8_t data)
{
    uint8_t cmd[3] = {0b0110 << 4 | ((addr >> 8) & 0xf), addr & 0xff, data};
    static spi_control_config_t config = {
        .common_config.cs_index = spi_cs_0,
        .common_config.data_phase_fmt = spi_single_io_mode,
        .common_config.rx_dma_enable = false,
        .common_config.tx_dma_enable = false,
        .common_config.dummy_cnt = spi_dummy_count_2,
        .common_config.trans_mode = spi_trans_write_only,
        .master_config.addr_enable = false,
        .master_config.addr_phase_fmt = spi_address_phase_format_single_io_mode,
        .master_config.cmd_enable = false,
        .master_config.token_enable = false,
        .master_config.token_value = 0,
    };

    return spi_transfer(spi, &config, NULL, NULL, cmd, 3, NULL, 0);
}

static hpm_stat_t mt6835_spi_special_cmd(SPI_Type *spi, uint8_t cmd)
{
    hpm_stat_t stat;
    uint8_t tx_data[2] = {cmd << 4};
    uint8_t data = 0;
    spi_set_transfer_mode(spi, spi_trans_write_read);
    spi_set_write_data_count(spi, 2);
    spi_set_read_data_count(spi, 1);
    spi->CMD = SPI_CMD_CMD_SET(0xff);
    stat = spi_write_read_data(spi, 1, tx_data, 2, &data, 1);
    if (stat != status_success)
        return stat;
    if (data != 0x55)
        return status_fail;
    return status_success;
}

hpm_stat_t mt6835_spi_program_eeprom(SPI_Type *spi)
{
    return mt6835_spi_special_cmd(spi, 0b1100);
}

hpm_stat_t mt6835_spi_auto_zero(SPI_Type *spi)
{
    return mt6835_spi_special_cmd(spi, 0b0101);
}

hpm_stat_t mt6835_spi_read_userid(SPI_Type *spi, uint8_t *userid)
{
    return mt6835_spi_read_reg(spi, 1, userid);
}

hpm_stat_t mt6835_set_abz_res(SPI_Type *spi, uint16_t abz_res)
{
    hpm_stat_t ret;
    uint8_t reg7, reg8;
    abz_res = (abz_res - 1) & 0x3fff;

    ret = mt6835_spi_read_reg(spi, 7, &reg7);
    ret = mt6835_spi_read_reg(spi, 8, &reg8);
    if (ret != status_success)
        return ret;
    printf("%02x %02x\r\n", reg7, reg8);
    reg7 = abz_res >> 6;
    reg8 &= 0x03;
    reg8 |= abz_res << 2;
    printf("%02x %02x\r\n", reg7, reg8);
    ret = mt6835_spi_write_reg(spi, 7, reg7);
    if (ret != status_success)
        return ret;
    ret = mt6835_spi_write_reg(spi, 8, reg8);
    return ret;
}

hpm_stat_t mt6835_spi_dump_regs(SPI_Type *spi)
{
    for (int i = 0; i < 0xd2; i++)
    {
        uint8_t data;
        mt6835_spi_read_reg(spi, i, &data);
        printf("[%02x] %02x\r\n", i, data);
    }
    return status_success;
}