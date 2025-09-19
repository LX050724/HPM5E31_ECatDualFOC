#pragma once

#include "hpm_pwmv2_drv.h"

#ifdef __cplusplus
extern "C" {
#endif

int pwm_init(PWMV2_Type *pwm, uint32_t adc_trigger_cmp, uint32_t dead_time_ps);
void pwm_synt_init();
void pwm_synt_start();
void pwm_synt_stop();


#ifdef __cplusplus
};
#endif
