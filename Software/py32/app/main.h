#ifndef __MAIN_H
#define __MAIN_H

#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/


void IncTick();
void EmergencyStop(void);
void ADC_IRQ_Callback(uint16_t value);

void APP_ErrorHandler(void);

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT Puya *****END OF FILE******************/
