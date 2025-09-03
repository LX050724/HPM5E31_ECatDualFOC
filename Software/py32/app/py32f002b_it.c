/**
 ******************************************************************************
 * @file    py32f002b_it.c
 * @author  MCU Application Team
 * @brief   Interrupt Service Routines.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2023 Puya Semiconductor Co.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by Puya under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2016 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "py32f002b_ll_adc.h"
#include "py32f002b_ll_exti.h"
#include "py32f002b_ll_usart.h"
#include "py32f002bx5.h"
#include "uart_dict.h"
#include <stdint.h>
/* Private includes ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private user code ---------------------------------------------------------*/
/* External variables --------------------------------------------------------*/

/******************************************************************************/
/*          Cortex-M0+ Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
 * @brief This function handles Non maskable interrupt.
 */
void NMI_Handler(void)
{
}

/**
 * @brief This function handles Hard fault interrupt.
 */
void HardFault_Handler(void)
{
    while (1)
    {
    }
}

/**
 * @brief This function handles System service call via SWI instruction.
 */
void SVC_Handler(void)
{
}

/**
 * @brief This function handles Pendable request for system service.
 */
void PendSV_Handler(void)
{
}

/**
 * @brief This function handles System tick timer.
 */
void SysTick_Handler(void)
{
    IncTick();
}

/******************************************************************************/
/* PY32F002B Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file.                                          */
/******************************************************************************/
void EXTI4_15_IRQHandler(void)
{
    /* Check if EXTI line 5 is asserted */
    // if (LL_EXTI_IsActiveFlag(LL_EXTI_LINE_5))
    {
        /* Clear the EXTI line 5 pending bit */
        LL_EXTI_ClearFlag(LL_EXTI_LINE_5);
        EmergencyStop();
        /* Add your code here to handle the interrupt */
    }
}

void USART1_IRQHandler(void)
{
    if (LL_USART_IsEnabledIT_RXNE(USART1) && LL_USART_IsActiveFlag_RXNE(USART1))
    {
        uart_rxfifo_push(LL_USART_ReceiveData8(USART1));
    }

    if (LL_USART_IsEnabledIT_IDLE(USART1) && LL_USART_IsActiveFlag_IDLE(USART1))
    {
        (void)(LL_USART_ReceiveData8(USART1)); // clear IDLE
        uart_rxfifo_rxidle();
    }

    if (LL_USART_IsEnabledIT_TXE(USART1) && LL_USART_IsActiveFlag_TXE(USART1))
    {
        uart_transmit_next_byte();
    }
}

void ADC_COMP_IRQHandler(void)
{
    LL_ADC_ClearFlag_EOS(ADC1);

    ADC_IRQ_Callback(LL_ADC_REG_ReadConversionData12(ADC1));
}

/************************ (C) COPYRIGHT Puya *****END OF FILE******************/
