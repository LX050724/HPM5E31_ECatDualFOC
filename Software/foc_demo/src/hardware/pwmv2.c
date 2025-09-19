#include "pwmv2.h"
#include "board.h"
#include "hpm_pwmv2_drv.h"
#include "hpm_soc.h"
#include "hpm_synt_drv.h"
#include "hpm_synt_regs.h"
#include "hpm_trgm_drv.h"
#include "hpm_trgmmux_src.h"
#include "project_config.h"
#include <stdbool.h>
#include <stdint.h>

#define DEAD_TIME_NS(t) (MIN((uint32_t)((t) / (1e12 / AHB_CLOCK) * 256 / 2), 0x200)) // 计算t ns的死区时间值

static void pwmv2_init_reload_cmp_reg(PWMV2_Type *pwm, pwm_counter_t counter, uint8_t index)
{
    /* 设置影子寄存器 */
    pwmv2_set_shadow_val(pwm, index + 0, PWM_RELOAD, 0, false);
    pwmv2_set_shadow_val(pwm, index + 1, PWM_RELOAD / 3, 0, false);
    pwmv2_set_shadow_val(pwm, index + 2, PWM_RELOAD / 3 * 2, 0, false);

    /* 选择计数器重载值 */
    pwmv2_counter_select_data_offset_from_shadow_value(pwm, counter, index + 0);
    /* 关闭burst发波 */
    pwmv2_counter_burst_disable(pwm, counter);
    /* 设置加载时刻 */
    pwmv2_reload_select_input_trigger(pwm, counter, 0);
    pwmv2_set_reload_update_time(pwm, counter, pwm_reload_update_on_trigger);

    /* 设置比较值 */
    pwmv2_select_cmp_source(pwm, index + 0, cmp_value_from_shadow_val, index + 1);
    pwmv2_select_cmp_source(pwm, index + 1, cmp_value_from_shadow_val, index + 2);
}

static void pwmv2_init_channel_config(PWMV2_Type *pwm, pwm_channel_t chn, uint32_t dead_time_ns)
{
    /* 设置死区时间 */
    pwmv2_set_dead_area(pwm, chn, DEAD_TIME_NS(dead_time_ns));
    pwmv2_set_dead_area(pwm, chn + 1, DEAD_TIME_NS(dead_time_ns));

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

int pwm_init(PWMV2_Type *pwm, uint32_t adc_trigger_cmp, uint32_t dead_time_ns)
{
    dead_time_ns = 50000;
    /* 复位PWM */
    pwmv2_deinit(pwm);

    /* 配置异步故障保护信号 */
    // pwm_fault_async(pwm);

    /* 解锁PWM影子寄存器 */
    pwmv2_shadow_register_unlock(pwm);

    /* PWM第一组重载值和比较值 */
    pwmv2_init_reload_cmp_reg(pwm, pwm_counter_0, PWMV2_SHADOW_VAL_0);
    /* PWM第二组重载值和比较值 */
    pwmv2_init_reload_cmp_reg(pwm, pwm_counter_1, PWMV2_SHADOW_VAL_4);
    /* PWM第三组重载值和比较值 */
    pwmv2_init_reload_cmp_reg(pwm, pwm_counter_2, PWMV2_SHADOW_VAL_8);

    pwmv2_shadow_register_lock(pwm);

    pwmv2_init_channel_config(pwm, pwm_channel_0, dead_time_ns);
    pwmv2_init_channel_config(pwm, pwm_channel_2, dead_time_ns);
    pwmv2_init_channel_config(pwm, pwm_channel_4, dead_time_ns);

    pwmv2_enable_counter(pwm, pwm_counter_0);
    pwmv2_enable_counter(pwm, pwm_counter_1);
    pwmv2_enable_counter(pwm, pwm_counter_2);
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