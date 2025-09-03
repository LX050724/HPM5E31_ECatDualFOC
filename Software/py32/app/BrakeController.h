#pragma once

#include <stdint.h>
#ifdef __cplusplus
extern "C"
{
#endif


typedef enum {
    BREAK_EV_TICK,
    BREAK_EV_ENABLE,
    BREAK_EV_DISABLE,
    BREAK_EV_FAULT,
} BrakeEvent_t;

typedef enum {
    BRAKE_ST_IDLE,
    BRAKE_ST_UNLOCKING,
    BRAKE_ST_HOLD,
    BRAKE_ST_LOCKING,
} BrakeStatus_t;

typedef struct
{
    uint8_t index;
    uint16_t delay_tick;
    uint16_t cur_fault_tick;
    BrakeStatus_t state;
    uint16_t *p_adc_value;
    void (*brake_set_pwm_cb)(bool, uint16_t);
    void (*brake_write_int_pin)(bool);
} BrakeController_t;

void BrakeController(BrakeController_t *self, BrakeEvent_t event);

#ifdef __cplusplus
}
#endif
