#pragma once
#include "hpm_adc.h"
#include "hpm_adc16_drv.h"
#include "hpm_interrupt.h"
#include "hpm_soc.h"
#include "project_config.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*adc_callback_t)(ADC16_Type *, uint32_t);

typedef enum
{
    ADC_TRIGGGER_SEQ_UV,
    ADC_TRIGGGER_SEQ_VW,
    ADC_TRIGGGER_SEQ_WU,
    ADC_TRIGGGER_SEQ_CALI,
} AdcTriggerSequence_t;

typedef struct
{
    uint16_t iu;
    uint16_t iv;
    uint16_t iw;
    uint16_t ibus;
    uint16_t vbus;
} AdcResult_t;

void adc_init();

void adc_set_callback(adc_callback_t cb);

void adc_driverA_set_trigger_sequence(uint8_t triggrt_sig, AdcTriggerSequence_t seqence);
void adc_driverA_get_value(AdcTriggerSequence_t seqence, AdcResult_t *result);
void adc_driverB_set_trigger_sequence(uint8_t triggrt_sig, AdcTriggerSequence_t seqence);
void adc_driverB_get_value(AdcTriggerSequence_t seqence, AdcResult_t *result);

static inline float adc_voltage(int raw)
{
    return (raw >> ADC_IGNORE_BIT) * 3.0f / (65536 >> ADC_IGNORE_BIT);
}

static inline void adc_enable_irq(uint32_t pri)
{
    intc_m_enable_irq_with_priority(IRQn_ADC0, pri);
}

static inline void adc_disable_irq()
{
    intc_m_disable_irq(IRQn_ADC0);
}

static inline void adc_enable_it()
{
    adc16_enable_interrupts(HPM_ADC0, adc16_event_trig_complete);
}

static inline void adc_disable_it()
{
    adc16_disable_interrupts(HPM_ADC0, adc16_event_trig_complete);
}

uint16_t adc_get_ntc1_raw();
uint16_t adc_get_ntc2_raw();

/**
 * @brief current A, B, C
 *
 * @param data
 */
void adc_get_trigger0a_raw(uint16_t data[3]);

/**
 * @brief volate A, B, C, BUS
 *
 * @param data
 */
void adc_get_trigger0b_raw(uint16_t data[4]);

#ifdef __cplusplus
}
#endif