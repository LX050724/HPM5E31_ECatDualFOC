#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "app/MotorClass.h"
#include "app/current_calibration/current_calibration.h"
#include "app/electrical_angle_calibration/electrical_angle_calibration.h"
#include "board.h"
#include "foc/foc_core.h"
#include "hardware/adc_init.h"
#include "hpm_clock_drv.h"
#include "hpm_common.h"
#include "hpm_gpio_drv.h"
#include "hpm_ioc_regs.h"
#include "hpm_iomux.h"
#include "hpm_mbx_drv.h"
#include "hpm_misc.h"
#include "hpm_soc.h"
#include "hpm_trgm_drv.h"
#include "hpm_trgmmux_src.h"
#include "hpm_uart_drv.h"
#include "pinmux.h"
#include "project_config.h"
#include "stdbool.h"
#include "hardware/pwmv2.h"
#include "hardware/encoder/mt6835_spi_abz.h"



#define MAIN_DEBUG(fmt, ...) DLOG("MAIN", fmt, ##__VA_ARGS__)

static void motor0_get_uvw_current(MotorClass_t *motor, foc_uvw_current_t *uvw_current);
static uint16_t motor0_get_raw_angle(MotorClass_t *motor);
static void motor0_set_pwm(MotorClass_t *motor, const foc_pwm_t *pwm);
static void motor0_enable_pwm(MotorClass_t *motor, bool en);

ATTR_PLACE_AT_FAST_RAM_BSS MotorClass_t motor0;
ATTR_PLACE_AT_FAST_RAM_BSS volatile int vofa_write_ptr;

#if SPEED_FILTER_MODE == SPEED_FILTER_IIR
/**
 * @brief IIR滤波器系数，切比雪夫II型@100Khz低通滤波器，通带频率100Hz/200Hz
 */
const float FLITER_NUM[][3] = {{0.3588240147, 0, 0},
                               {1, -1.999825358, 1},
                               {0.1754118353, 0, 0},
                               {1, -1.999542952, 1},
                               {0.004925392102, 0, 0},
                               {1, 1, 0},
                               {1, 0, 0}};
const float FLITER_DEN[][3] = {{1, 0, 0}, {1, -1.996026397, 0.9960891008},
                               {1, 0, 0}, {1, -1.986816645, 0.9868968725},
                               {1, 0, 0}, {1, -0.9901492, 0},
                               {1, 0, 0}};
#endif

static void adc_callback(ADC16_Type *adc, uint32_t flag);

int main(void)
{
    board_init();

    uart_config_t uart_config;
    init_uart_pins(HPM_UART2);
    clock_add_to_group(clock_uart2, 0);

    uart_default_config(HPM_UART2, &uart_config);
    uart_config.src_freq_in_hz = clock_get_frequency(clock_uart2);
    uart_config.baudrate = 115200;
    uart_init(HPM_UART2, &uart_config);

    uint8_t udata[] = {0x5a, 0x80, 0, 0x00, 0x00};

    // udata[2] = 5;
    // for (int i = 1; i < sizeof(udata) - 1; i++)
    //     udata[sizeof(udata) - 1] ^= udata[i];

    // CPU_Delay(300);
    // uart_send_data(HPM_UART2, udata, sizeof(udata));

    init_py32_pins();
    gpio_set_pin_output_with_initial(BOARD_RGB_PYRST_GPIO_CTRL, BOARD_RGB_PYRST_GPIO_INDEX,
                                         BOARD_RGB_PYRST_GPIO_PIN, 0);
    clock_cpu_delay_ms(1);
    gpio_write_pin(BOARD_RGB_PYRST_GPIO_CTRL, BOARD_RGB_PYRST_GPIO_INDEX,
                                         BOARD_RGB_PYRST_GPIO_PIN, 1);

    
    init_trgm_extern_pins();
    board_init_encoder_gpio_pins(BOARD_DRIVER_A_INDEX);
    board_encoderA_powerup(false);
    clock_cpu_delay_ms(1000);
    board_encoderA_powerup(true);
    clock_cpu_delay_ms(1000);
    mt6835_spi_abz_init(BOARD_ENCODER_A_SPI, 500000, BOARD_ENCODER_A_QEI);
    
    
    udata[2] = 3;
    for (int i = 1; i < sizeof(udata) - 1; i++)
        udata[sizeof(udata) - 1] ^= udata[i];
    CPU_Delay(300);
    uart_send_data(HPM_UART2, udata, sizeof(udata));



//     motor0.speed_pll.pi.output_limit = 2000;

//     motor0.speed_pll.pi.kp = 0.05f;
//     motor0.speed_pll.pi.output_limit = 200;

//     motor0.angle_pid.kp = 0.08f;
//     motor0.angle_pid.ki = 0.002f;
//     motor0.angle_pid.integral_limit = 300;
//     motor0.angle_pid.output_limit = 1000;

//     motor0.speed_pid.kp = 0.01f;
//     motor0.speed_pid.ki = 0.01f;
//     motor0.speed_pid.integral_limit = 5;
//     motor0.speed_pid.output_limit = 10;

//     motor0.current_iq_pid.kp = 0.4f;
//     motor0.current_iq_pid.ki = 0.06f;
//     motor0.current_iq_pid.integral_limit = 6;
//     motor0.current_id_pid.kp = 1.6f;
//     motor0.current_id_pid.ki = 0.03f;
//     motor0.current_id_pid.integral_limit = 3;

//     motor0.get_uvw_current_cb = motor0_get_uvw_current;
//     motor0.get_raw_angle_cb = motor0_get_raw_angle;
//     motor0.set_pwm_cb = motor0_set_pwm;
//     motor0.enable_pwm = motor0_enable_pwm;
// #if SPEED_FILTER_MODE == SPEED_FILTER_IIR
//     IIRFilterInit(&motor0.speed_filter, 3, FLITER_NUM, FLITER_DEN);
// #endif
//     Motor_Init(&motor0);

    /* 按键通过TRGM1输出0连接到PWM0内部故障输入1 */
    // HPM_IOC->PAD[IOC_PAD_PA27].FUNC_CTL = IOC_PA27_FUNC_CTL_TRGM1_P_07;
    // HPM_IOC->PAD[IOC_PAD_PA27].PAD_CTL = IOC_PAD_PAD_CTL_PE_SET(1) | IOC_PAD_PAD_CTL_PS_SET(1);
    // trgm_input_filter_t filter = {};
    // filter.mode = trgm_filter_mode_rapid_change;
    // filter.sync = true;
    // filter.invert = false;
    // filter.filter_length = TRGM_FILTCFG_FILTLEN_MASK;
    // trgm_input_filter_config(HPM_TRGM1, HPM_TRGM1_FILTER_SRC_TRGM1_IN7, &filter);
    // trgm_connect(HPM_TRGM1, HPM_TRGM1_INPUT_SRC_TRGM1_P7, HPM_TRGM1_OUTPUT_SRC_TRGM1_OUTX0, trgm_output_same_as_input,
    //              true);
    // trgm_connect(HPM_TRGM0, HPM_TRGM0_INPUT_SRC_TRGM1_OUTX0, HPM_TRGM0_OUTPUT_SRC_PWM0_FAULTI1,
    //              trgm_output_same_as_input, false);

    /* 连接DRV故障线到PWM0内部故障输入0并取反 */
    // trgm_connect(HPM_TRGM0, HPM_TRGM0_INPUT_SRC_TRGM0_P6, HPM_TRGM0_OUTPUT_SRC_PWM0_FAULTI0, trgm_output_same_as_input,
    //              true);

    /* PWM0采样时刻通过TRGM0输出0连接到PA25 */
    // HPM_IOC->PAD[IOC_PAD_PA25].FUNC_CTL = IOC_PA25_FUNC_CTL_TRGM1_P_05;
    // HPM_IOC->PAD[IOC_PAD_PA25].PAD_CTL = IOC_PAD_PAD_CTL_SPD_SET(3) | IOC_PAD_PAD_CTL_DS_SET(7);
    // trgm_connect(HPM_TRGM0, HPM_TRGM0_INPUT_SRC_PWM0_CH15REF, HPM_TRGM0_OUTPUT_SRC_TRGM0_OUTX0,
    //              trgm_output_same_as_input, false);
    // trgm_connect(HPM_TRGM1, HPM_TRGM1_INPUT_SRC_TRGM0_OUTX0, HPM_TRGM1_OUTPUT_SRC_TRGM1_P5,
    //              trgm_output_pulse_at_input_rising_edge, false);
    // trgm_enable_io_output(HPM_TRGM1, 1 << 5);
    
    adc_init();
    
    /* PWM初始化 */
    board_init_driverEN_pins(BOARD_DRIVER_A_INDEX);
    board_set_driverA_enable(true);

    init_pwm_pins(HPM_PWM0);
    init_pwm_fault_pins();
    pwm_init(HPM_PWM0, 0, 1e6);

    while (1) {
        printf("%d\n", mt6835_spi_read_angle_status(BOARD_ENCODER_A_SPI, NULL, NULL));
        CPU_Delay(1000);
    }
    /* ADC初始化 */
    // adc_init();
    // adc_enable_irq(1);
    // adc_enable_it();

    /* 连接PWMCH8、ADCX_PTRGI0A */
    // trgm_connect(HPM_TRGM0, HPM_TRGM0_INPUT_SRC_PWM0_CH8REF, HPM_TRGM0_OUTPUT_SRC_ADCX_PTRGI0A,
    //              trgm_output_same_as_input, false);

    /* adc中值校准程序 */
    // current_calibration(&motor0);
    // drv832x_calibration(false);

#if ENCODER_TYPE == ENCODER_MT6701
    mt6701_spi_init();
#endif

    // motor0.encoder.ang_offset = 0x81b6;
    // motor0.encoder.pole_pairs = -7;
    // if (electrical_angle_calibration(&motor0) == 0)
    // { 
    //     motor0.qd_current_exp.iq = 5;
    //     motor0.qd_voltage_exp.iq = 0.8;
    //     Motor_SetMode(&motor0, CURRENT_MODE);
    //     adc_enable_irq(2);
    //     adc_enable_it();
    //     adc_set_callback(adc_callback);
    // }

    while (1)
    {
        // if (motor0.mode == SVPWM_OPEN_LOOP_MODE)
        // {
        //     motor0.angle_exp += 100;
        //     clock_cpu_delay_us(200);
        // }
    }
    return 0;
}

static int count = 0;
static void adc_callback(ADC16_Type *adc, uint32_t flag)
{
    Motor_RunFoc(&motor0);

    // just_float_data *pdata = &core_comm_ctl.vofa_buf[vofa_write_ptr];
    // pdata->data[0] = motor0.qd_current.iq;
    // pdata->data[1] = motor0.qd_current_exp.iq;
    // pdata->data[2] = motor0.qd_current.id;
    // pdata->data[3] = motor0.qd_current_exp.id;
    // pdata->data[4] = motor0.speed;
    // pdata->data[5] = motor0.speed_exp;
    // pdata->data[6] = motor0.uvw_current.iu;
    // pdata->data[7] = motor0.uvw_current.iv;
    // pdata->data[8] = motor0.uvw_current.iw;
    // pdata->data[9] = motor0.raw_angle;
    // pdata->data[10] = motor0.bus_voltage;
    // pdata->data[11] = motor0.power;
    // pdata->data[12] = count;

    // if (++count > 1000)
    //     count = 0;
}


// static void motor0_get_uvw_current(MotorClass_t *motor, foc_uvw_current_t *uvw_current)
// {
//     uint16_t cal[3];
//     adc_get_trigger0a_raw(cal);
//     current_get_cal(&motor->current_cal, cal, uvw_current);
// }

// static uint16_t motor0_get_raw_angle(MotorClass_t *motor)
// {
//     return encoder_get_rawAngle();
// }

// static void motor0_set_pwm(MotorClass_t *motor, const foc_pwm_t *pwm)
// {
//     pwm_setvalue(pwm);
// }

// static void motor0_enable_pwm(MotorClass_t *motor, bool en)
// {
//     en ? pwm_enable_all_output() : pwm_disable_all_output();
// }