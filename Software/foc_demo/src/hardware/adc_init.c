
#include "adc_init.h"
#include "board.h"
#include "foc/foc_core.h"
#include "hardware/trgm.h"
#include "hpm_adc.h"
#include "hpm_adc16_regs.h"
#include "hpm_soc.h"
#include "hpm_trgmmux_src.h"
#include "project_config.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

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

static DMA_ATTR AdcDMA_t adc_dma_buf[48];

typedef struct
{
    ADC16_Type *adc;
    uint8_t channles[4];
    uint8_t trig_ch;
} ADC_Channels_t;

ADC_Channels_t ADC_Channels[] = {
    // MOTOR0
    {
        .adc = HPM_ADC0,
        .channles =
            {
                BOARD_MOTOR0_CUR_A_ADC_CH,
                BOARD_MOTOR0_CUR_BUS_ADC_CH,
                0xff,
                0xff,
            },
        .trig_ch = ADC16_CONFIG_TRG0A,
    },
    {
        .adc = HPM_ADC0,
        .channles =
            {
                BOARD_MOTOR0_CUR_B_ADC_CH,
                BOARD_MOTOR0_CUR_BUS_ADC_CH,
                0xff,
                0xff,
            },
        .trig_ch = ADC16_CONFIG_TRG1A,
    },
    {
        .adc = HPM_ADC1,
        .channles =
            {
                BOARD_MOTOR0_CUR_B_ADC_CH,
                BOARD_BUS_VOLIATE_ADC_CH,
                0xff,
                0xff,
            },
        .trig_ch = ADC16_CONFIG_TRG2A,
    },
    {
        .adc = HPM_ADC1,
        .channles =
            {
                BOARD_MOTOR0_CUR_C_ADC_CH,
                BOARD_BUS_VOLIATE_ADC_CH,
                0xff,
                0xff,
            },
        .trig_ch = ADC16_CONFIG_TRG3A,
    },
    // MOTOR1
    {
        .adc = HPM_ADC0,
        .channles =
            {
                BOARD_MOTOR1_CUR_A_ADC_CH,
                BOARD_MOTOR1_CUR_BUS_ADC_CH,
                0xff,
                0xff,
            },
        .trig_ch = ADC16_CONFIG_TRG0B,
    },
    {
        .adc = HPM_ADC0,
        .channles =
            {
                BOARD_MOTOR1_CUR_B_ADC_CH,
                BOARD_MOTOR1_CUR_BUS_ADC_CH,
                0xff,
                0xff,
            },
        .trig_ch = ADC16_CONFIG_TRG1B,
    },
    {
        .adc = HPM_ADC1,
        .channles =
            {
                BOARD_MOTOR1_CUR_B_ADC_CH,
                BOARD_BUS_VOLIATE_ADC_CH,
                0xff,
                0xff,
            },
        .trig_ch = ADC16_CONFIG_TRG2B,
    },
    {
        .adc = HPM_ADC1,
        .channles =
            {
                BOARD_MOTOR1_CUR_C_ADC_CH,
                BOARD_BUS_VOLIATE_ADC_CH,
                0xff,
                0xff,
            },
        .trig_ch = ADC16_CONFIG_TRG3B,
    },
    // MOTOR0 CALI
    {
        .adc = HPM_ADC0,
        .channles =
            {
                BOARD_MOTOR0_CUR_A_ADC_CH,
                BOARD_MOTOR0_CUR_B_ADC_CH,
                BOARD_MOTOR0_CUR_C_ADC_CH,
                BOARD_MOTOR0_CUR_BUS_ADC_CH,
            },
        .trig_ch = ADC16_CONFIG_TRG0C,
    },
    // MOTOR1 CALI
    {
        .adc = HPM_ADC0,
        .channles =
            {
                BOARD_MOTOR1_CUR_A_ADC_CH,
                BOARD_MOTOR1_CUR_B_ADC_CH,
                BOARD_MOTOR1_CUR_C_ADC_CH,
                BOARD_MOTOR1_CUR_BUS_ADC_CH,
            },
        .trig_ch = ADC16_CONFIG_TRG1C,
    },
};

void adc_init()
{
    adc16_config_t cfg;
    memset(adc_dma_buf, 0, sizeof(adc_dma_buf));

    board_init_adc16_pins();

    board_init_adc_clock(HPM_ADC0, true);
    board_init_adc_clock(HPM_ADC1, true);
    adc16_get_default_config(&cfg);

    cfg.res = adc16_res_16_bits;
    cfg.conv_mode = adc16_conv_mode_preemption;
    cfg.adc_clk_div = adc16_clock_divider_4;
    cfg.sel_sync_ahb = true;
    cfg.adc_ahb_en = true;
    adc16_init(HPM_ADC0, &cfg);
    adc16_init(HPM_ADC1, &cfg);

    /* 初始化ADC通道 */
    for (int i = 0; i < sizeof(ADC_Channels) / sizeof(ADC_Channels[0]); i++)
    {
        adc16_channel_config_t adc_channel_config = {};
        adc16_pmt_config_t pmt_config = {};

        adc_channel_config.sample_cycle_shift = 0;
        adc_channel_config.sample_cycle = 10;
        adc_channel_config.ch = ADC_Channels[i].channles[0];
        adc16_init_channel(ADC_Channels[i].adc, &adc_channel_config);

        adc_channel_config.ch = ADC_Channels[i].channles[1];
        adc16_init_channel(ADC_Channels[i].adc, &adc_channel_config);

        pmt_config.trig_len = 2;
        pmt_config.trig_ch = ADC_Channels[i].trig_ch;
        pmt_config.adc_ch[0] = ADC_Channels[i].channles[0];
        pmt_config.adc_ch[1] = ADC_Channels[i].channles[1];

        if (ADC_Channels[i].channles[2] != 0xFF)
        {
            adc_channel_config.ch = ADC_Channels[i].channles[2];
            adc16_init_channel(ADC_Channels[i].adc, &adc_channel_config);
            pmt_config.trig_len = 3;
            pmt_config.adc_ch[2] = ADC_Channels[i].channles[2];
        }
        
        if (ADC_Channels[i].channles[3] != 0xFF)
        {
            adc_channel_config.ch = ADC_Channels[i].channles[3];
            adc16_init_channel(ADC_Channels[i].adc, &adc_channel_config);
            pmt_config.trig_len = 4;
            pmt_config.adc_ch[3] = ADC_Channels[i].channles[3];
        }

        pmt_config.inten[pmt_config.trig_len - 1] = (ADC_Channels[i].adc == HPM_ADC0);
        adc16_set_pmt_config(ADC_Channels[i].adc, &pmt_config);
        adc16_set_pmt_queue_enable(ADC_Channels[i].adc, pmt_config.trig_ch, true);
    }

    trgm_output_t trgm_output_cfg = {
        .input = HPM_TRGM0_INPUT_SRC_VSS,
        .invert = false,
        .type = trgm_output_pulse_at_input_rising_edge,
    };

    trgm_output_config(HPM_TRGM0, HPM_TRGM0_OUTPUT_SRC_ADCX_PTRGI0A, &trgm_output_cfg);
    trgm_output_config(HPM_TRGM0, HPM_TRGM0_OUTPUT_SRC_ADCX_PTRGI1A, &trgm_output_cfg);
    trgm_output_config(HPM_TRGM0, HPM_TRGM0_OUTPUT_SRC_ADCX_PTRGI2A, &trgm_output_cfg);
    trgm_output_config(HPM_TRGM0, HPM_TRGM0_OUTPUT_SRC_ADCX_PTRGI3A, &trgm_output_cfg);
    trgm_output_config(HPM_TRGM0, HPM_TRGM0_OUTPUT_SRC_ADCX_PTRGI0B, &trgm_output_cfg);
    trgm_output_config(HPM_TRGM0, HPM_TRGM0_OUTPUT_SRC_ADCX_PTRGI1B, &trgm_output_cfg);
    trgm_output_config(HPM_TRGM0, HPM_TRGM0_OUTPUT_SRC_ADCX_PTRGI2B, &trgm_output_cfg);
    trgm_output_config(HPM_TRGM0, HPM_TRGM0_OUTPUT_SRC_ADCX_PTRGI3B, &trgm_output_cfg);
    trgm_output_config(HPM_TRGM0, HPM_TRGM0_OUTPUT_SRC_ADCX_PTRGI0C, &trgm_output_cfg);
    trgm_output_config(HPM_TRGM0, HPM_TRGM0_OUTPUT_SRC_ADCX_PTRGI1C, &trgm_output_cfg);

    adc16_init_pmt_dma(HPM_ADC0, core_local_mem_to_sys_address(HPM_CORE0, (uint32_t)adc_dma_buf));
    adc16_init_pmt_dma(HPM_ADC1, core_local_mem_to_sys_address(HPM_CORE0, (uint32_t)adc_dma_buf));
}

void adc_driverA_set_trigger_sequence(uint8_t triggrt_sig, AdcTriggerSequence_t seqence)
{
    switch (seqence)
    {
    case ADC_TRIGGGER_SEQ_UV:
        trgm_output_update_source(HPM_TRGM0, HPM_TRGM0_OUTPUT_SRC_ADCX_PTRGI0A, triggrt_sig);
        trgm_output_update_source(HPM_TRGM0, HPM_TRGM0_OUTPUT_SRC_ADCX_PTRGI1A, HPM_TRGM0_INPUT_SRC_VSS);
        trgm_output_update_source(HPM_TRGM0, HPM_TRGM0_OUTPUT_SRC_ADCX_PTRGI2A, triggrt_sig);
        trgm_output_update_source(HPM_TRGM0, HPM_TRGM0_OUTPUT_SRC_ADCX_PTRGI3A, HPM_TRGM0_INPUT_SRC_VSS);
        trgm_output_update_source(HPM_TRGM0, HPM_TRGM0_OUTPUT_SRC_ADCX_PTRGI0C, HPM_TRGM0_INPUT_SRC_VSS);
        break;
    case ADC_TRIGGGER_SEQ_VW:
        trgm_output_update_source(HPM_TRGM0, HPM_TRGM0_OUTPUT_SRC_ADCX_PTRGI0A, HPM_TRGM0_INPUT_SRC_VSS);
        trgm_output_update_source(HPM_TRGM0, HPM_TRGM0_OUTPUT_SRC_ADCX_PTRGI1A, triggrt_sig);
        trgm_output_update_source(HPM_TRGM0, HPM_TRGM0_OUTPUT_SRC_ADCX_PTRGI2A, HPM_TRGM0_INPUT_SRC_VSS);
        trgm_output_update_source(HPM_TRGM0, HPM_TRGM0_OUTPUT_SRC_ADCX_PTRGI3A, triggrt_sig);
        trgm_output_update_source(HPM_TRGM0, HPM_TRGM0_OUTPUT_SRC_ADCX_PTRGI0C, HPM_TRGM0_INPUT_SRC_VSS);
        break;
    case ADC_TRIGGGER_SEQ_WU:
        trgm_output_update_source(HPM_TRGM0, HPM_TRGM0_OUTPUT_SRC_ADCX_PTRGI0A, triggrt_sig);
        trgm_output_update_source(HPM_TRGM0, HPM_TRGM0_OUTPUT_SRC_ADCX_PTRGI1A, HPM_TRGM0_INPUT_SRC_VSS);
        trgm_output_update_source(HPM_TRGM0, HPM_TRGM0_OUTPUT_SRC_ADCX_PTRGI2A, HPM_TRGM0_INPUT_SRC_VSS);
        trgm_output_update_source(HPM_TRGM0, HPM_TRGM0_OUTPUT_SRC_ADCX_PTRGI3A, triggrt_sig);
        trgm_output_update_source(HPM_TRGM0, HPM_TRGM0_OUTPUT_SRC_ADCX_PTRGI0C, HPM_TRGM0_INPUT_SRC_VSS);
        break;
    case ADC_TRIGGGER_SEQ_CALI:
        trgm_output_update_source(HPM_TRGM0, HPM_TRGM0_OUTPUT_SRC_ADCX_PTRGI0A, HPM_TRGM0_INPUT_SRC_VSS);
        trgm_output_update_source(HPM_TRGM0, HPM_TRGM0_OUTPUT_SRC_ADCX_PTRGI1A, HPM_TRGM0_INPUT_SRC_VSS);
        trgm_output_update_source(HPM_TRGM0, HPM_TRGM0_OUTPUT_SRC_ADCX_PTRGI2A, HPM_TRGM0_INPUT_SRC_VSS);
        trgm_output_update_source(HPM_TRGM0, HPM_TRGM0_OUTPUT_SRC_ADCX_PTRGI3A, HPM_TRGM0_INPUT_SRC_VSS);
        trgm_output_update_source(HPM_TRGM0, HPM_TRGM0_OUTPUT_SRC_ADCX_PTRGI0C, triggrt_sig);
        break;
    }
}

void adc_driverB_set_trigger_sequence(uint8_t triggrt_sig, AdcTriggerSequence_t seqence)
{
    trgm_output_t trgm_output_cfg = {
        .input = triggrt_sig,
        .invert = false,
        .type = trgm_output_pulse_at_input_rising_edge,
    };

    switch (seqence)
    {
            case ADC_TRIGGGER_SEQ_UV:
        trgm_output_update_source(HPM_TRGM0, HPM_TRGM0_OUTPUT_SRC_ADCX_PTRGI0B, triggrt_sig);
        trgm_output_update_source(HPM_TRGM0, HPM_TRGM0_OUTPUT_SRC_ADCX_PTRGI1B, HPM_TRGM0_INPUT_SRC_VSS);
        trgm_output_update_source(HPM_TRGM0, HPM_TRGM0_OUTPUT_SRC_ADCX_PTRGI2B, triggrt_sig);
        trgm_output_update_source(HPM_TRGM0, HPM_TRGM0_OUTPUT_SRC_ADCX_PTRGI3B, HPM_TRGM0_INPUT_SRC_VSS);
        trgm_output_update_source(HPM_TRGM0, HPM_TRGM0_OUTPUT_SRC_ADCX_PTRGI1C, HPM_TRGM0_INPUT_SRC_VSS);
        break;
    case ADC_TRIGGGER_SEQ_VW:
        trgm_output_update_source(HPM_TRGM0, HPM_TRGM0_OUTPUT_SRC_ADCX_PTRGI0B, HPM_TRGM0_INPUT_SRC_VSS);
        trgm_output_update_source(HPM_TRGM0, HPM_TRGM0_OUTPUT_SRC_ADCX_PTRGI1B, triggrt_sig);
        trgm_output_update_source(HPM_TRGM0, HPM_TRGM0_OUTPUT_SRC_ADCX_PTRGI2B, HPM_TRGM0_INPUT_SRC_VSS);
        trgm_output_update_source(HPM_TRGM0, HPM_TRGM0_OUTPUT_SRC_ADCX_PTRGI3B, triggrt_sig);
        trgm_output_update_source(HPM_TRGM0, HPM_TRGM0_OUTPUT_SRC_ADCX_PTRGI1C, HPM_TRGM0_INPUT_SRC_VSS);
        break;
    case ADC_TRIGGGER_SEQ_WU:
        trgm_output_update_source(HPM_TRGM0, HPM_TRGM0_OUTPUT_SRC_ADCX_PTRGI0B, triggrt_sig);
        trgm_output_update_source(HPM_TRGM0, HPM_TRGM0_OUTPUT_SRC_ADCX_PTRGI1B, HPM_TRGM0_INPUT_SRC_VSS);
        trgm_output_update_source(HPM_TRGM0, HPM_TRGM0_OUTPUT_SRC_ADCX_PTRGI2B, HPM_TRGM0_INPUT_SRC_VSS);
        trgm_output_update_source(HPM_TRGM0, HPM_TRGM0_OUTPUT_SRC_ADCX_PTRGI3B, triggrt_sig);
        trgm_output_update_source(HPM_TRGM0, HPM_TRGM0_OUTPUT_SRC_ADCX_PTRGI1C, HPM_TRGM0_INPUT_SRC_VSS);
        break;
    case ADC_TRIGGGER_SEQ_CALI:
        trgm_output_update_source(HPM_TRGM0, HPM_TRGM0_OUTPUT_SRC_ADCX_PTRGI0B, HPM_TRGM0_INPUT_SRC_VSS);
        trgm_output_update_source(HPM_TRGM0, HPM_TRGM0_OUTPUT_SRC_ADCX_PTRGI1B, HPM_TRGM0_INPUT_SRC_VSS);
        trgm_output_update_source(HPM_TRGM0, HPM_TRGM0_OUTPUT_SRC_ADCX_PTRGI2B, HPM_TRGM0_INPUT_SRC_VSS);
        trgm_output_update_source(HPM_TRGM0, HPM_TRGM0_OUTPUT_SRC_ADCX_PTRGI3B, HPM_TRGM0_INPUT_SRC_VSS);
        trgm_output_update_source(HPM_TRGM0, HPM_TRGM0_OUTPUT_SRC_ADCX_PTRGI1C, triggrt_sig);
        break;
    }
}

void adc_driverA_get_value(AdcTriggerSequence_t seqence, AdcResult_t *result)
{
    switch (seqence)
    {
    case ADC_TRIGGGER_SEQ_UV:
        result->iu = adc_dma_buf[ADC16_CONFIG_TRG0A * 4].value;
        result->iv = adc_dma_buf[ADC16_CONFIG_TRG2A * 4].value;
        result->iw = 0;
        result->ibus = adc_dma_buf[ADC16_CONFIG_TRG0A * 4 + 1].value;
        result->vbus = adc_dma_buf[ADC16_CONFIG_TRG2A * 4 + 1].value;
        break;
    case ADC_TRIGGGER_SEQ_VW:
        result->iu = 0;
        result->iv = adc_dma_buf[ADC16_CONFIG_TRG1A * 4].value;
        result->iw = adc_dma_buf[ADC16_CONFIG_TRG3A * 4].value;
        result->ibus = adc_dma_buf[ADC16_CONFIG_TRG1A * 4 + 1].value;
        result->vbus = adc_dma_buf[ADC16_CONFIG_TRG3A * 4 + 1].value;
        break;
    case ADC_TRIGGGER_SEQ_WU:
        result->iu = adc_dma_buf[ADC16_CONFIG_TRG0A * 4].value;
        result->iv = 0;
        result->iw = adc_dma_buf[ADC16_CONFIG_TRG3A * 4].value;
        result->ibus = adc_dma_buf[ADC16_CONFIG_TRG0A * 4 + 1].value;
        result->vbus = adc_dma_buf[ADC16_CONFIG_TRG3A * 4 + 1].value;
        break;
    case ADC_TRIGGGER_SEQ_CALI:
        result->iu = adc_dma_buf[ADC16_CONFIG_TRG0C * 4].value;
        result->iv = adc_dma_buf[ADC16_CONFIG_TRG0C * 4 + 1].value;
        result->iw = adc_dma_buf[ADC16_CONFIG_TRG0C * 4 + 2].value;
        result->ibus = adc_dma_buf[ADC16_CONFIG_TRG0C * 4 + 3].value;
        result->vbus = 0;
        break;
    }
}

void adc_driverB_get_value(AdcTriggerSequence_t seqence, AdcResult_t *result)
{

    switch (seqence)
    {
    case ADC_TRIGGGER_SEQ_UV:
        result->iu = adc_dma_buf[ADC16_CONFIG_TRG0B * 4].value;
        result->iv = adc_dma_buf[ADC16_CONFIG_TRG2B * 4].value;
        result->iw = 0;
        result->ibus = adc_dma_buf[ADC16_CONFIG_TRG0B * 4 + 1].value;
        result->vbus = adc_dma_buf[ADC16_CONFIG_TRG2B * 4 + 1].value;
        break;
    case ADC_TRIGGGER_SEQ_VW:
        result->iu = 0;
        result->iv = adc_dma_buf[ADC16_CONFIG_TRG1B * 4].value;
        result->iw = adc_dma_buf[ADC16_CONFIG_TRG3B * 4].value;
        result->ibus = adc_dma_buf[ADC16_CONFIG_TRG1B * 4 + 1].value;
        result->vbus = adc_dma_buf[ADC16_CONFIG_TRG3B * 4 + 1].value;
        break;
    case ADC_TRIGGGER_SEQ_WU:
        result->iu = adc_dma_buf[ADC16_CONFIG_TRG0B * 4].value;
        result->iv = 0;
        result->iw = adc_dma_buf[ADC16_CONFIG_TRG3B * 4].value;
        result->ibus = adc_dma_buf[ADC16_CONFIG_TRG0B * 4 + 1].value;
        result->vbus = adc_dma_buf[ADC16_CONFIG_TRG3B * 4 + 1].value;
    case ADC_TRIGGGER_SEQ_CALI:
        result->iu = adc_dma_buf[ADC16_CONFIG_TRG1C * 4].value;
        result->iv = adc_dma_buf[ADC16_CONFIG_TRG1C * 4 + 1].value;
        result->iw = adc_dma_buf[ADC16_CONFIG_TRG1C * 4 + 2].value;
        result->ibus = adc_dma_buf[ADC16_CONFIG_TRG1C * 4 + 3].value;
        result->vbus = 0;
        break;
    }
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
    uint32_t status = adc16_get_status_flags(HPM_ADC0);

    if ((status & BOARD_BLDC_ADC_TRIG_FLAG) != 0)
    {
        adc16_clear_status_flags(HPM_ADC0, BOARD_BLDC_ADC_TRIG_FLAG);
        if (__isr_callback)
            __isr_callback(HPM_ADC0, BOARD_BLDC_ADC_TRIG_FLAG);
    }
}

static SDK_DECLARE_EXT_ISR_M(IRQn_ADC1, __isr_adc1_fun);
static void __isr_adc1_fun(void)
{
    uint32_t status = adc16_get_status_flags(HPM_ADC1);

    if ((status & BOARD_BLDC_ADC_TRIG_FLAG) != 0)
    {
        adc16_clear_status_flags(HPM_ADC1, BOARD_BLDC_ADC_TRIG_FLAG);
        if (__isr_callback)
            __isr_callback(HPM_ADC1, BOARD_BLDC_ADC_TRIG_FLAG);
    }
}
