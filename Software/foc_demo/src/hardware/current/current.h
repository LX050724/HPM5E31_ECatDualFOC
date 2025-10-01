#pragma once

#include "foc/foc_core.h"
#include "hardware/adc_init.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    int adc_calibration_a;
    int adc_calibration_b;
    int adc_calibration_c;
    int adc_calibration_bus;
#if ADC_ENABLE_FILTER == 1
    int last_adc_raw_a;
    int last_adc_raw_b;
    int last_adc_raw_c;
    int last_adc_raw_bus;
#endif
} CurrentCal_t;

static inline void current_get_cal(CurrentCal_t *self, AdcResult_t *result, AdcTriggerSequence_t seq,
                                   foc_uvw_current_t *cur)
{
#if ADC_ENABLE_FILTER == 1
    cur->iu = adc_voltage(((result->iu + self->last_adc_raw_a) >> 1) - self->adc_calibration_a) / CURRENT_COE;
    cur->iv = adc_voltage(((result->iv + self->last_adc_raw_b) >> 1) - self->adc_calibration_b) / CURRENT_COE;
    cur->iw = adc_voltage(((result->iw + self->last_adc_raw_c) >> 1) - self->adc_calibration_c) / CURRENT_COE;
    self->last_adc_raw_a = result->iu;
    self->last_adc_raw_b = result->iv;
    self->last_adc_raw_c = result->iw;
#else
    cur->iu = adc_voltage(result->iu - self->adc_calibration_a) / CURRENT_COE;
    cur->iv = adc_voltage(result->iv - self->adc_calibration_b) / CURRENT_COE;
    cur->iw = adc_voltage(result->iw - self->adc_calibration_c) / CURRENT_COE;
    switch (seq)
    {
    case ADC_TRIGGGER_SEQ_UV:
        cur->iw = -cur->iu - cur->iv;
        break;
    case ADC_TRIGGGER_SEQ_VW:
        cur->iu = -cur->iv - cur->iw;
        break;
    case ADC_TRIGGGER_SEQ_WU:
        cur->iv = -cur->iw - cur->iu;
        break;
    case ADC_TRIGGGER_SEQ_CALI:
        break;
    }
#endif
}

static inline void current_set_calibration(CurrentCal_t *self, int value[4])
{
    self->adc_calibration_a = value[0];
    self->adc_calibration_b = value[1];
    self->adc_calibration_c = value[2];
    self->adc_calibration_bus = value[3];
#if ADC_ENABLE_FILTER == 1
    self->last_adc_raw_a = value[0];
    self->last_adc_raw_b = value[1];
    self->last_adc_raw_c = value[2];
    self->last_adc_raw_bus = value[3];
#endif
}

static inline void current_init(CurrentCal_t *self)
{
    self->adc_calibration_a = (65536 >> ADC_IGNORE_BIT) / 2;
    self->adc_calibration_b = (65536 >> ADC_IGNORE_BIT) / 2;
    self->adc_calibration_c = (65536 >> ADC_IGNORE_BIT) / 2;
#if ADC_ENABLE_FILTER == 1
    self->last_adc_raw_a = (65536 >> ADC_IGNORE_BIT) / 2;
    self->last_adc_raw_b = (65536 >> ADC_IGNORE_BIT) / 2;
    self->last_adc_raw_c = (65536 >> ADC_IGNORE_BIT) / 2;
#endif
}

#ifdef __cplusplus
}
#endif