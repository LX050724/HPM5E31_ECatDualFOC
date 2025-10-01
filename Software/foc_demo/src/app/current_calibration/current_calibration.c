#include "current_calibration.h"
#include "hardware/adc_init.h"
#include "hpm_adc16_drv.h"
#include "project_config.h"
#include <stdbool.h>
#include <stdint.h>

#define CURRCAL_DEBUG(fmt, ...) DLOG("CC", fmt, ##__VA_ARGS__)

static volatile int calibration_status;
static int calibration[4];
static uint32_t calibration_count;
static MotorClass_t *gpMotor;

static void __adc_cb(ADC16_Type *adc, uint32_t flag)
{
    if (calibration_status == 1)
    {
        calibration[0] = 0;
        calibration[1] = 0;
        calibration[2] = 0;
        calibration[3] = 0;
        calibration_status = 2;
        return;
    }

    if (calibration_status == 2)
    {
        if (calibration_count < ADC_CALIBRATION_TIMES)
        {
            gpMotor->get_analog_cb(gpMotor);
            calibration[0] += gpMotor->adc_value.iu;
            calibration[1] += gpMotor->adc_value.iv;
            calibration[2] += gpMotor->adc_value.iw;
            calibration[3] += gpMotor->adc_value.ibus;
            calibration_count += 1;
        }
        else
        {
            calibration[0] /= ADC_CALIBRATION_TIMES;
            calibration[1] /= ADC_CALIBRATION_TIMES;
            calibration[2] /= ADC_CALIBRATION_TIMES;
            calibration[3] /= ADC_CALIBRATION_TIMES;
            current_set_calibration(&gpMotor->current_cal, calibration);
            calibration_status = 3;
        }
    }
}

int current_calibration(MotorClass_t *motor)
{
    motor->enable_pwm(motor, false);

    gpMotor = motor;

    calibration_count = 0;
    calibration_status = 1;
    motor->adc_seq = ADC_TRIGGGER_SEQ_CALI;
    motor->set_adc_seq_cb(motor, ADC_TRIGGGER_SEQ_CALI);
    adc_set_callback(__adc_cb);
    adc_enable_irq(1);
    adc_enable_it();

    while (calibration_status != 3)
    {
    }

    adc_disable_it();
    adc_disable_irq();
    adc_set_callback(NULL);
    motor->adc_seq = ADC_TRIGGGER_SEQ_UV;
    motor->set_adc_seq_cb(motor, ADC_TRIGGGER_SEQ_UV);
    gpMotor = NULL;

    CURRCAL_DEBUG("current calibration done");
    CURRCAL_DEBUG("  A offset %dmV", (int)(adc_voltage(calibration[0]) * 1000));
    CURRCAL_DEBUG("  B offset %dmV", (int)(adc_voltage(calibration[1]) * 1000));
    CURRCAL_DEBUG("  C offset %dmV", (int)(adc_voltage(calibration[2]) * 1000));
    CURRCAL_DEBUG("BUS offset %dmV", (int)(adc_voltage(calibration[3]) * 1000));
    return 0;
}