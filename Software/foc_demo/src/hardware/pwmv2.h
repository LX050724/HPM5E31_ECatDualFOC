#pragma once

#include "foc/foc_core.h"
#include "hpm_pwmv2_drv.h"

#ifdef __cplusplus
extern "C" {
#endif

int pwm_init(PWMV2_Type *pwm, uint32_t adc_trigger_cmp, float dead_time_ns);
void pwm_synt_init();
void pwm_synt_start();
void pwm_synt_stop();

void pwm_setvalue(PWMV2_Type *pwm, const foc_pwm_t *par);
void pwm_set_adc_trigger_cmp0(PWMV2_Type *pwm, uint32_t adc_trigger_cmp);
void pwm_set_adc_trigger_cmp1(PWMV2_Type *pwm, uint32_t adc_trigger_cmp);
void pwm_enable_all_output(PWMV2_Type *pwm);
void pwm_disable_all_output(PWMV2_Type *pwm);

#ifdef __cplusplus
};
#endif
