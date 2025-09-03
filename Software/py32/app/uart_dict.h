#pragma once

#include <stdint.h>
#ifdef __cplusplus
extern "C"
{
#endif

typedef enum
{
    // SDO
    UD_BRAKE1_CTRL,         // 控制字1
    UD_BRAKE1_ON_DUTY,      // 开启占空比
    UD_BRAKE1_ON_TIME,      // 开启时间
    UD_BRAKE1_ON_CURRENT,   // 开启电流
    UD_BRAKE1_HOLD_DUTY,    // 维持占空比
    UD_BRAKE1_HOLD_CURRENT, // 维持电流
    UD_BRAKE1_OFF_TIME,     // 关闭时间

    UD_BRAKE2_CTRL,         // 控制字2
    UD_BRAKE2_ON_DUTY,      // 开启占空比
    UD_BRAKE2_ON_TIME,      // 开启时间
    UD_BRAKE2_ON_CURRENT,   // 开启电流
    UD_BRAKE2_HOLD_DUTY,    // 维持占空比
    UD_BRAKE2_HOLD_CURRENT, // 维持电流
    UD_BRAKE2_OFF_TIME,     // 关闭时间

    UD_MOTOR1_NTP_ADC_UP_THRESHOLD,
    UD_MOTOR1_NTP_ADC_DOWN_THRESHOLD,
    UD_MOTOR2_NTP_ADC_UP_THRESHOLD,
    UD_MOTOR2_NTP_ADC_DOWN_THRESHOLD,
    
    // PDO
    UD_STATUS,              // 状态
    UD_BRAKE1_CUR_CURRENT,  // 当前抱闸电流
    UD_BRAKE2_CUR_CURRENT,  // 当前抱闸电流
    UD_MOTOR1_TEMP_ADC,     // 电机1 温度读数
    UD_MOTOR2_TEMP_ADC,     // 电机2 温度读数
    UD_MCU_TEMP_ADC,        // MCU温度读数
} UART_DICT_ENUM;

#define UD_STATUS_FLAG_BRAKE1_OPENED 0x0001
#define UD_STATUS_FLAG_BRAKE2_OPENED 0x0002

#define UD_BRAKE_CTRL_FLAG_INIT_DONE 0x0001
#define UD_BRAKE_CTRL_FLAG_ENABLE 0x0002
#define UD_BRAKE_CTRL_FLAG_DISABLE 0x0004

extern uint16_t uart_dict[32];

void uart_rxfifo_push(uint8_t data);
void uart_transmit_next_byte();
void uart_rxfifo_rxidle();
void uart_recv_frame(uint8_t *data, uint8_t len);

uint32_t uart_get_edit_flag();
int uart_send_pdo(uint8_t start_index, uint8_t num);

#define UART_DICT_HAS_FLAG(flag, index) ((flag) & (1 << (index)))

#ifdef __cplusplus
}
#endif