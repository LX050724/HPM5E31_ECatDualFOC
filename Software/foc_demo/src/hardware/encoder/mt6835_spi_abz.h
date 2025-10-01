#pragma once
#include "hpm_qeiv2_drv.h"
#include "hpm_spi_drv.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MT6835_POS_MAX (0x200000)

int mt6835_spi_abz_init(SPI_Type *spi, uint32_t baud, QEIV2_Type *qei);
uint32_t mt6835_spi_read_angle_status(SPI_Type *spi, uint32_t *angle, uint8_t *status);
int mt6835_enable_crc(CRC_Type *crc, uint32_t index);
hpm_stat_t mt6835_spi_read_reg(SPI_Type *spi, uint16_t addr, uint8_t *data);
hpm_stat_t mt6835_spi_write_reg(SPI_Type *spi, uint16_t addr, uint8_t data);
hpm_stat_t mt6835_spi_program_eeprom(SPI_Type *spi);
hpm_stat_t mt6835_spi_auto_zero(SPI_Type *spi);
hpm_stat_t mt6835_set_abz_res(SPI_Type *spi, uint16_t abz_res);
hpm_stat_t mt6835_spi_dump_regs(SPI_Type *spi);
#ifdef __cplusplus
}
#endif
