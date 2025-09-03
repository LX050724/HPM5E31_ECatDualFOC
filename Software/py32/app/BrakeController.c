#include "BrakeController.h"
#include "uart_dict.h"
#include <stdint.h>

#define MAX_FAULT_TICKS 100

bool BrakeController_CheckCurrent(BrakeController_t *self)
{
    if (self->state == BRAKE_ST_HOLD)
    {
        if ((*self->p_adc_value - 2048) < uart_dict[self->index + UD_BRAKE1_HOLD_CURRENT])
        {
            if (self->cur_fault_tick < MAX_FAULT_TICKS)
            {
                self->cur_fault_tick++;
            }
            else
            {
                return false;
            }
        }
        else
        {
            self->cur_fault_tick = 0;
        }
    }
    else if (self->state == BRAKE_ST_UNLOCKING)
    {
        if ((*self->p_adc_value - 2048) < uart_dict[self->index + UD_BRAKE1_ON_CURRENT])
        {
            if (self->cur_fault_tick < MAX_FAULT_TICKS)
            {
                self->cur_fault_tick++;
            }
            else
            {
                return false;
            }
        }
        else
        {
            self->cur_fault_tick = 0;
        }
    }
    else
    {
        self->cur_fault_tick = 0;
    }
    return true;
}

void BrakeController(BrakeController_t *self, BrakeEvent_t event)
{
    if (event == BREAK_EV_TICK && !BrakeController_CheckCurrent(self))
    {
        event = BREAK_EV_FAULT;
    }

    if (event == BREAK_EV_FAULT)
    {
        if (self->state != BRAKE_ST_IDLE)
        {
            self->brake_set_pwm_cb(false, 0);
            self->brake_write_int_pin(false);
            self->state = BRAKE_ST_IDLE;
        }
        return;
    }

    switch (self->state)
    {
    case BRAKE_ST_IDLE:
        if (event == BREAK_EV_ENABLE)
        {
            self->brake_set_pwm_cb(true, uart_dict[self->index + UD_BRAKE1_ON_DUTY]);
            self->delay_tick = uart_dict[self->index + UD_BRAKE1_ON_TIME];
            self->state = BRAKE_ST_UNLOCKING;
        }
        // else
        // {
        //     self->brake_set_pwm_cb(false, 0);
        //     self->brake_write_int_pin(false);
        // }
        break;
    case BRAKE_ST_UNLOCKING:
        if (event == BREAK_EV_DISABLE)
        {
            self->state = BRAKE_ST_LOCKING;
            self->delay_tick = uart_dict[self->index + UD_BRAKE1_OFF_TIME];
            self->brake_write_int_pin(false);
            self->brake_set_pwm_cb(false, 0);
        }
        else if (self->delay_tick-- == 0)
        {
            self->brake_set_pwm_cb(true, uart_dict[self->index + UD_BRAKE1_HOLD_DUTY]);
            self->brake_write_int_pin(true);
            self->state = BRAKE_ST_HOLD;
        }
        break;
    case BRAKE_ST_HOLD:
        if (event == BREAK_EV_DISABLE)
        {
            self->state = BRAKE_ST_LOCKING;
            self->delay_tick = uart_dict[self->index + UD_BRAKE1_OFF_TIME];
            self->brake_write_int_pin(false);
            self->brake_set_pwm_cb(false, 0);
        }
        break;
    case BRAKE_ST_LOCKING:
        if (event == BREAK_EV_ENABLE)
        {
            self->brake_set_pwm_cb(true, uart_dict[self->index + UD_BRAKE1_ON_DUTY]);
            self->delay_tick = uart_dict[self->index + UD_BRAKE1_ON_TIME];
            self->state = BRAKE_ST_UNLOCKING;
        }
        else if (self->delay_tick-- == 0)
        {
            self->brake_set_pwm_cb(false, 0);
            self->brake_write_int_pin(false);
            self->state = BRAKE_ST_IDLE;
        }
        break;
    }
}
