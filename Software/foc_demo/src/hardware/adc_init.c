
#include "adc_init.h"
#include "board.h"
#include "hpm_adc.h"
#include "hpm_adc16_regs.h"
#include "hpm_soc.h"
#include "project_config.h"
#include <stdbool.h>
#include <stdint.h>

static adc_callback_t __isr_callback;

typedef struct
{
    uint32_t value : 16;
    uint32_t : 4;
    uint32_t adc_ch : 4;
    uint32_t trig_ch : 5;
    uint32_t seq_num : 2;
    uint32_t flag : 1;
} AdcDMA_t;

static adc_type adc0;
static adc_type adc1;
static DMA_ATTR AdcDMA_t adc0_dma_buf[48];
static DMA_ATTR AdcDMA_t adc1_dma_buf[48];

void init_trigger_cfg(void)
{
}

void adc_init()
{
    adc_config_t cfg;
    adc_channel_config_t ch_cfg;
    cfg.module = ADCX_MODULE_ADC16;

    adc0.module = adc_module_adc16;
    adc0.adc_base.adc16 = HPM_ADC0;
    adc1.module = adc_module_adc16;
    adc1.adc_base.adc16 = HPM_ADC1;

    board_init_adc16_pins();

    board_init_adc_clock(HPM_ADC0, true);
    board_init_adc_clock(HPM_ADC1, true);
    hpm_adc_init_default_config(&cfg);

    cfg.config.adc16.res = adc16_res_16_bits;
    cfg.config.adc16.conv_mode = adc16_conv_mode_preemption;
    cfg.config.adc16.adc_clk_div = adc16_clock_divider_4;
    cfg.config.adc16.sel_sync_ahb = true;
    cfg.config.adc16.adc_ahb_en = true;

    cfg.adc_base.adc16 = HPM_ADC0;
    hpm_adc_init(&cfg);
    cfg.adc_base.adc16 = HPM_ADC1;
    hpm_adc_init(&cfg);

    ch_cfg.module = ADCX_MODULE_ADC16;
    hpm_adc_init_channel_default_config(&ch_cfg);

    const uint32_t ADC_Channels[] = {
        BOARD_MOTOR0_CUR_A_ADC_CH,   BOARD_MOTOR0_CUR_B_ADC_CH,   BOARD_MOTOR0_CUR_C_ADC_CH,
        BOARD_MOTOR1_CUR_A_ADC_CH,   BOARD_MOTOR1_CUR_B_ADC_CH,   BOARD_MOTOR1_CUR_C_ADC_CH,
        BOARD_MOTOR0_CUR_BUS_ADC_CH, BOARD_MOTOR1_CUR_BUS_ADC_CH, BOARD_BUS_VOLIATE_ADC_CH,
    };

    /* 初始化ADC通道 */
    for (int i = 0; i < sizeof(ADC_Channels) / sizeof(ADC_Channels[0]); i++)
    {
        adc16_channel_config_t adc_channel_config = {};
        adc16_pmt_config_t pmt_config = {};

        adc_channel_config.sample_cycle_shift = 0;
        adc_channel_config.sample_cycle = 10;
        adc_channel_config.ch = ADC_Channels[i];
        
        pmt_config.trig_ch = i;
        pmt_config.adc_ch[0] = ADC_Channels[i];

        adc16_init_channel(HPM_ADC0, &adc_channel_config);
        adc16_init_channel(HPM_ADC1, &adc_channel_config);
        adc16_set_pmt_config(HPM_ADC0,&pmt_config);
        adc16_set_pmt_config(HPM_ADC1,&pmt_config);
        adc16_set_pmt_queue_enable(HPM_ADC0, pmt_config.trig_ch, true);
        adc16_set_pmt_queue_enable(HPM_ADC1, pmt_config.trig_ch, true);
    }

    hpm_adc_init_pmt_dma(&adc0, core_local_mem_to_sys_address(HPM_CORE0, (uint32_t)adc0_dma_buf));
    hpm_adc_init_pmt_dma(&adc1, core_local_mem_to_sys_address(HPM_CORE0, (uint32_t)adc1_dma_buf));

    /* NTC序列采集 */
    // adc16_set_nonblocking_read(BOARD_NTC1_ADC_BASE);
    // adc16_enable_busywait(BOARD_NTC1_ADC_BASE);
}

uint16_t adc_oneshot_read(ADC16_Type *ptr, uint8_t ch)
{
    uint16_t result;
    if (adc16_get_oneshot_result(ptr, ch, &result) == status_success)
    {
        if (adc16_is_nonblocking_mode(ptr))
        {
            adc16_get_oneshot_result(ptr, ch, &result);
        }
    }
    return result;
}

uint16_t adc_get_ntc1_raw()
{
    // return adc_oneshot_read(BOARD_NTC1_ADC_BASE, BOARD_NTC1_ADC_CH);
}

uint16_t adc_get_ntc2_raw()
{
    // return adc_oneshot_read(BOARD_NTC2_ADC_BASE, BOARD_NTC2_ADC_CH);
}

void adc_set_callback(adc_callback_t cb)
{
    __isr_callback = cb;
}

void adc_get_trigger0a_raw(uint16_t data[3])
{
    // data[0] = adc0_dma_buf[0].value;
    // data[1] = adc2_dma_buf[0].value;
    // data[2] = adc1_dma_buf[0].value;
}

void adc_get_trigger0b_raw(uint16_t data[4])
{
    // data[0] = adc0_dma_buf[4].value;
    // data[1] = adc1_dma_buf[4].value;
    // data[2] = adc2_dma_buf[4].value;
    // data[3] = adc1_dma_buf[5].value;
}

/**
 * @brief 中断服务函数
 */
static SDK_DECLARE_EXT_ISR_M(IRQn_ADC0, __isr_adc0_fun);
static void __isr_adc0_fun(void)
{
    uint32_t status = hpm_adc_get_status_flags(&adc0);

    if ((status & BOARD_BLDC_ADC_TRIG_FLAG) != 0)
    {
        hpm_adc_clear_status_flags(&adc0, BOARD_BLDC_ADC_TRIG_FLAG);
        if (__isr_callback)
            __isr_callback(HPM_ADC0, BOARD_BLDC_ADC_TRIG_FLAG);
    }
}

static SDK_DECLARE_EXT_ISR_M(IRQn_ADC1, __isr_adc1_fun);
static void __isr_adc1_fun(void)
{
    uint32_t status = hpm_adc_get_status_flags(&adc1);

    if ((status & BOARD_BLDC_ADC_TRIG_FLAG) != 0)
    {
        hpm_adc_clear_status_flags(&adc1, BOARD_BLDC_ADC_TRIG_FLAG);
        if (__isr_callback)
            __isr_callback(HPM_ADC1, BOARD_BLDC_ADC_TRIG_FLAG);
    }
}
