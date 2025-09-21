#include "pwmv2.h"
#include "board.h"
#include "foc/foc_core.h"
#include "hpm_clock_drv.h"
#include "hpm_pwmv2_drv.h"
#include "hpm_pwmv2_regs.h"
#include "hpm_soc.h"
#include "hpm_synt_drv.h"
#include "hpm_synt_regs.h"
#include "hpm_trgm_drv.h"
#include "hpm_trgmmux_src.h"
#include "project_config.h"
#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>

#define PWM_SHADOW_VAL_RELOAD PWMV2_SHADOW_VAL_0
#define PWM_SHADOW_VAL_UA PWMV2_SHADOW_VAL_1
#define PWM_SHADOW_VAL_UB PWMV2_SHADOW_VAL_2
#define PWM_SHADOW_VAL_VA PWMV2_SHADOW_VAL_3
#define PWM_SHADOW_VAL_VB PWMV2_SHADOW_VAL_4
#define PWM_SHADOW_VAL_WA PWMV2_SHADOW_VAL_5
#define PWM_SHADOW_VAL_WB PWMV2_SHADOW_VAL_6

#define PWM_ADC_CMP0 PWMV2_CMP_16
#define PWM_ADC_CMP1 PWMV2_CMP_17
#define PWM_SHADOW_VAL_ADC_CMP0 PWMV2_SHADOW_VAL_7
#define PWM_SHADOW_VAL_ADC_CMP1 PWMV2_SHADOW_VAL_8

static void pwmv2_init_reload_cmp_reg(PWMV2_Type *pwm, pwm_counter_t counter, uint8_t cmp_index, uint8_t shadow_index)
{
    pwmv2_set_shadow_val(pwm, shadow_index, 0, 0, false);
    pwmv2_set_shadow_val(pwm, shadow_index + 1, 0, 0, false);

    /* 选择计数器重载值 */
    pwmv2_counter_select_data_offset_from_shadow_value(pwm, counter, PWM_SHADOW_VAL_RELOAD);
    /* 关闭burst发波 */
    pwmv2_counter_burst_disable(pwm, counter);
    /* 设置加载时刻 */
    pwmv2_reload_select_input_trigger(pwm, counter, 0);
    pwmv2_set_reload_update_time(pwm, counter, pwm_reload_update_on_trigger);

    /* 设置比较值 */
    pwmv2_select_cmp_source(pwm, cmp_index + 0, cmp_value_from_shadow_val, shadow_index);
    pwmv2_select_cmp_source(pwm, cmp_index + 1, cmp_value_from_shadow_val, shadow_index + 1);
}

static void pwmv2_init_channel_config(PWMV2_Type *pwm, pwm_channel_t chn, float dead_time_ns)
{
    /* 计算死区时间 */
    uint32_t clock_freq = clock_get_frequency(clock_pwm0);
    uint32_t dead_time = (dead_time_ns * 256) / (1e9 / clock_freq);
    if (dead_time < 0x200)
        dead_time = 0x200;

    /* 设置死区时间 */
    pwm->PWM[chn].DEAD_AREA = PWMV2_PWM_DEAD_AREA_DEAD_AREA_SET(dead_time);
    pwm->PWM[chn + 1].DEAD_AREA = PWMV2_PWM_DEAD_AREA_DEAD_AREA_SET(dead_time);
    // pwmv2_set_dead_area(pwm, chn + 1, dead_time);

    /* 关闭4比较模式 */
    pwmv2_disable_four_cmp(pwm, chn);

    /* 使能PWM互补模式 */
    pwmv2_enable_pair_mode(pwm, chn);
    pwmv2_enable_pair_mode(pwm, chn + 1);

    /* 使能PWM通道输出 */
    pwmv2_channel_enable_output(pwm, chn);
    pwmv2_channel_enable_output(pwm, chn + 1);
}

void pwm_fault_async(PWMV2_Type *pwm)
{
    pwmv2_async_fault_source_config_t fault_cfg;
    fault_cfg.async_signal_from_pad_index = BOARD_APP_PWM_FAULT_PIN;
    fault_cfg.fault_async_pad_level = pad_fault_active_low;

    for (pwm_channel_t i = pwm_channel_0; i <= pwm_channel_5; i++)
    {
        pwmv2_config_async_fault_source(pwm, i, &fault_cfg);
        pwmv2_set_fault_mode(pwm, i, pwm_fault_output_0);
        pwmv2_set_fault_recovery_time(pwm, i, pwm_fault_recovery_on_fault_clear);
        pwmv2_enable_async_fault(pwm, i);
    }
}

int pwm_init(PWMV2_Type *pwm, uint32_t adc_trigger_cmp, float dead_time_ns)
{
    /* 复位PWM */
    pwmv2_deinit(pwm);

    /* 配置异步故障保护信号 */
    pwm_fault_async(pwm);

    /* 解锁PWM影子寄存器 */
    pwmv2_shadow_register_unlock(pwm);
    pwmv2_set_shadow_val(pwm, PWM_SHADOW_VAL_RELOAD, PWM_RELOAD, 0, false);

    /* PWM第一组重载值和比较值 */
    pwmv2_init_reload_cmp_reg(pwm, pwm_counter_0, PWMV2_CMP_0, PWMV2_SHADOW_VAL_1);
    /* PWM第二组重载值和比较值 */
    pwmv2_init_reload_cmp_reg(pwm, pwm_counter_1, PWMV2_CMP_4, PWMV2_SHADOW_VAL_3);
    /* PWM第三组重载值和比较值 */
    pwmv2_init_reload_cmp_reg(pwm, pwm_counter_2, PWMV2_CMP_8, PWMV2_SHADOW_VAL_5);

    /* 初始化影子寄存器 */
    pwmv2_set_shadow_val(pwm, PWM_SHADOW_VAL_ADC_CMP0, adc_trigger_cmp, 0, false);
    pwmv2_set_shadow_val(pwm, PWM_SHADOW_VAL_ADC_CMP1, adc_trigger_cmp, 0, false);
    /* 选择计数器0 */
    pwmv2_cmp_select_counter(pwm, PWM_ADC_CMP0, pwm_counter_0);
    pwmv2_cmp_select_counter(pwm, PWM_ADC_CMP1, pwm_counter_0);
    /* 绑定影子寄存器 */
    pwmv2_select_cmp_source(pwm, PWM_ADC_CMP0, cmp_value_from_shadow_val, PWM_SHADOW_VAL_ADC_CMP0);
    pwmv2_select_cmp_source(pwm, PWM_ADC_CMP1, cmp_value_from_shadow_val, PWM_SHADOW_VAL_ADC_CMP1);
    /* 设置重载时刻 */
    pwmv2_cmp_update_trig_time(pwm, PWM_ADC_CMP0, pwm_shadow_register_update_on_reload);
    pwmv2_cmp_update_trig_time(pwm, PWM_ADC_CMP1, pwm_shadow_register_update_on_reload);
    /* 绑定触发输出通道 */
    pwmv2_set_trigout_cmp_index(pwm, PWMV2_TRIGGER_CFG_0, PWM_ADC_CMP0);
    pwmv2_set_trigout_cmp_index(pwm, PWMV2_TRIGGER_CFG_1, PWM_ADC_CMP1);

    pwmv2_shadow_register_lock(pwm);

    pwmv2_init_channel_config(pwm, pwm_channel_0, dead_time_ns);
    pwmv2_init_channel_config(pwm, pwm_channel_2, dead_time_ns);
    pwmv2_init_channel_config(pwm, pwm_channel_4, dead_time_ns);

    pwmv2_enable_counter(pwm, pwm_counter_0);
    pwmv2_enable_counter(pwm, pwm_counter_1);
    pwmv2_enable_counter(pwm, pwm_counter_2);
#ifdef HRPWM_ENABLE
    pwmv2_enable_hrpwm(pwm);
#endif
    pwmv2_start_pwm_output(pwm, pwm_counter_0);
    pwmv2_start_pwm_output(pwm, pwm_counter_1);
    pwmv2_start_pwm_output(pwm, pwm_counter_2);

    pwm_synt_init();
    pwm_synt_start();
    return 0;
}

void pwm_synt_init()
{
    synt_reset_counter(HPM_SYNT);
    synt_set_reload(HPM_SYNT, PWM_RELOAD);
    synt_set_comparator(HPM_SYNT, SYNT_CMP_0, PWM_RELOAD / 2);

    trgm_output_t trgm_config;
    trgm_config.input = HPM_TRGM0_INPUT_SRC_SYNT_CH00;
    trgm_config.type = trgm_output_pulse_at_input_rising_edge;
    trgm_config.invert = false;

    trgm_output_config(HPM_TRGM0, HPM_TRGM0_OUTPUT_SRC_PWM0_TRIG_IN_0, &trgm_config);
    trgm_output_config(HPM_TRGM0, HPM_TRGM0_OUTPUT_SRC_PWM1_TRIG_IN_0, &trgm_config);
    trgm_output_config(HPM_TRGM0, HPM_TRGM0_OUTPUT_SRC_TRGM0_P04, &trgm_config);

    trgm_config.input = HPM_TRGM0_INPUT_SRC_PWM0_TRGO_0;
    trgm_output_config(HPM_TRGM0, HPM_TRGM0_OUTPUT_SRC_TRGM0_P05, &trgm_config);

    trgm_enable_io_output(HPM_TRGM0, 1 << 4 | 1 << 5);
}

void pwm_synt_start()
{
    synt_enable_counter(HPM_SYNT, true);
}

void pwm_synt_stop()
{
    synt_enable_counter(HPM_SYNT, false);
    synt_reset_counter(HPM_SYNT);
}

void pwm_set_adc_trigger_cmp0(PWMV2_Type *pwm, uint32_t adc_trigger_cmp)
{
    pwmv2_set_shadow_val(pwm, PWMV2_SHADOW_VAL_16, adc_trigger_cmp, 0, false);
}

void pwm_set_adc_trigger_cmp1(PWMV2_Type *pwm, uint32_t adc_trigger_cmp)
{
    pwmv2_set_shadow_val(pwm, PWMV2_SHADOW_VAL_17, adc_trigger_cmp, 0, false);
}

void pwm_setvalue(PWMV2_Type *pwm, const foc_pwm_t *par)
{
    pwmv2_shadow_register_unlock(pwm);

    if (par->pwm_u >= PWM_MAX)
    {
        pwmv2_set_shadow_val(pwm, PWM_SHADOW_VAL_UA, 1, 0, false);
        pwmv2_set_shadow_val(pwm, PWM_SHADOW_VAL_UB, PWM_RELOAD + 2, 0, false);
    }
    else if (par->pwm_u == 0)
    {
        pwmv2_set_shadow_val(pwm, PWM_SHADOW_VAL_UA, 2, 0, false);
        pwmv2_set_shadow_val(pwm, PWM_SHADOW_VAL_UB, 1, 0, false);
    }
    {
        uint32_t pwm_half = par->pwm_u >> 1;
        pwm->SHADOW_VAL[PWM_SHADOW_VAL_UA] = PWMV2_SHADOW_VAL_VALUE_SET(PWM_MAX / 2 - pwm_half);
        pwm->SHADOW_VAL[PWM_SHADOW_VAL_UB] = PWMV2_SHADOW_VAL_VALUE_SET(PWM_MAX / 2 + pwm_half);
    }

    if (par->pwm_v >= PWM_MAX)
    {
        pwmv2_set_shadow_val(pwm, PWM_SHADOW_VAL_VA, 1, 0, false);
        pwmv2_set_shadow_val(pwm, PWM_SHADOW_VAL_VB, PWM_RELOAD + 2, 0, false);
    }
    else if (par->pwm_v == 0)
    {
        pwmv2_set_shadow_val(pwm, PWM_SHADOW_VAL_VA, 2, 0, false);
        pwmv2_set_shadow_val(pwm, PWM_SHADOW_VAL_VB, 1, 0, false);
    }
    else
    {
        uint32_t pwm_half = par->pwm_v >> 1;
        pwm->SHADOW_VAL[PWM_SHADOW_VAL_VA] = PWMV2_SHADOW_VAL_VALUE_SET(PWM_MAX / 2 - pwm_half);
        pwm->SHADOW_VAL[PWM_SHADOW_VAL_VB] = PWMV2_SHADOW_VAL_VALUE_SET(PWM_MAX / 2 + pwm_half);
    }

    if (par->pwm_w >= PWM_MAX)
    {
        pwmv2_set_shadow_val(pwm, PWM_SHADOW_VAL_WA, 1, 0, false);
        pwmv2_set_shadow_val(pwm, PWM_SHADOW_VAL_WB, PWM_RELOAD + 2, 0, false);
    }
    else if (par->pwm_w == 0)
    {
        pwmv2_set_shadow_val(pwm, PWM_SHADOW_VAL_WA, 2, 0, false);
        pwmv2_set_shadow_val(pwm, PWM_SHADOW_VAL_WB, 1, 0, false);
    }
    else
    {
        uint32_t pwm_half = par->pwm_w >> 1;
        pwm->SHADOW_VAL[PWM_SHADOW_VAL_WA] = PWMV2_SHADOW_VAL_VALUE_SET(PWM_MAX / 2 - pwm_half);
        pwm->SHADOW_VAL[PWM_SHADOW_VAL_WB] = PWMV2_SHADOW_VAL_VALUE_SET(PWM_MAX / 2 + pwm_half);
    }
    pwmv2_shadow_register_lock(pwm);
}

void pwm_enable_all_output(PWMV2_Type *pwm)
{
    pwm->GLB_CTRL &= ~(PWMV2_GLB_CTRL_SW_FORCE_SET(0x3f));
}

void pwm_disable_all_output(PWMV2_Type *pwm)
{
    pwm->GLB_CTRL |= PWMV2_GLB_CTRL_SW_FORCE_SET(0x3f);
}