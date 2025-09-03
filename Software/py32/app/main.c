#include "main.h"
#include "BrakeController.h"
#include "py32f002b_ll_adc.h"
#include "py32f002b_ll_bus.h"
#include "py32f002b_ll_exti.h"
#include "py32f002b_ll_gpio.h"
#include "py32f002b_ll_rcc.h"
#include "py32f002b_ll_system.h"
#include "py32f002b_ll_tim.h"
#include "py32f002b_ll_usart.h"
#include "py32f002b_ll_utils.h"
#include "py32f002bx5.h"
#include "system_py32f0xx.h"
#include "uart_dict.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef RTT_ENABLE
#include "SEGGER_RTT.h"
#endif

static void SystemClockConfig(void);
void PWM_Init(void);
void GPIO_Init(void);
void USART_Init(void);
void ADC_Init(void);
void EXTI_Init(void);

void brake1_enable(bool enable, uint16_t duty);
void brake2_enable(bool enable, uint16_t duty);
void brake1_write_int_pin(bool state);
void brake2_write_int_pin(bool state);

BrakeController_t brake_controller[2];

volatile uint32_t delay_tick = 0;

static const struct
{
    uint32_t channel;
    uint32_t sampling_time;
} adc_ConvRank[] = {
    {LL_ADC_CHANNEL_7, LL_ADC_SAMPLINGTIME_13CYCLES_5},           //
    {LL_ADC_CHANNEL_0, LL_ADC_SAMPLINGTIME_13CYCLES_5},           //
    {LL_ADC_CHANNEL_1, LL_ADC_SAMPLINGTIME_71CYCLES_5},           //
    {LL_ADC_CHANNEL_2, LL_ADC_SAMPLINGTIME_71CYCLES_5},           //
    {LL_ADC_CHANNEL_TEMPSENSOR, LL_ADC_SAMPLINGTIME_239CYCLES_5}, //
};

uint8_t adc_rank_index = 0;

uint16_t adc_value[sizeof(adc_ConvRank) / sizeof(adc_ConvRank[0])] = {0};

void IncTick()
{
    if (delay_tick)
        delay_tick--;

    BrakeController(&brake_controller[0], BREAK_EV_TICK);
    BrakeController(&brake_controller[1], BREAK_EV_TICK);
}

#define CURRENT_TO_ADC(current) ((uint16_t)((current) * 4096 / 3.3f))

/**
 * @brief  Main program.
 * @retval int
 */
int main(void)
{
    SystemClockConfig();
    LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_SYSCFG);
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);
    LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOA);
    LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOB);
    LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOC);

    GPIO_Init();
    EXTI_Init();
    USART_Init();
    ADC_Init();
    PWM_Init();

    brake_controller[0].brake_set_pwm_cb = brake1_enable;
    brake_controller[0].brake_write_int_pin = brake1_write_int_pin;
    brake_controller[0].index = UD_BRAKE1_CTRL;
    brake_controller[0].p_adc_value = &adc_value[0];

    brake_controller[1].brake_set_pwm_cb = brake2_enable;
    brake_controller[1].brake_write_int_pin = brake2_write_int_pin;
    brake_controller[1].index = UD_BRAKE2_CTRL;
    brake_controller[1].p_adc_value = &adc_value[1];

    uart_dict[UD_BRAKE1_CTRL] = UD_BRAKE_CTRL_FLAG_INIT_DONE;
    uart_dict[UD_BRAKE1_ON_DUTY] = 150;
    uart_dict[UD_BRAKE1_ON_TIME] = 200;
    uart_dict[UD_BRAKE1_ON_CURRENT] = CURRENT_TO_ADC(0.3f);
    uart_dict[UD_BRAKE1_HOLD_DUTY] = 100;
    uart_dict[UD_BRAKE1_HOLD_CURRENT] = CURRENT_TO_ADC(0.2f);
    uart_dict[UD_BRAKE1_OFF_TIME] = 100;

    uart_dict[UD_BRAKE2_CTRL] = UD_BRAKE_CTRL_FLAG_INIT_DONE;
    uart_dict[UD_BRAKE2_ON_DUTY] = 150;
    uart_dict[UD_BRAKE2_ON_TIME] = 200;
    uart_dict[UD_BRAKE2_ON_CURRENT] = CURRENT_TO_ADC(0.3f);
    uart_dict[UD_BRAKE2_HOLD_DUTY] = 100;
    uart_dict[UD_BRAKE2_HOLD_CURRENT] = CURRENT_TO_ADC(0.2f);
    uart_dict[UD_BRAKE2_OFF_TIME] = 100;

    delay_tick = 50;

    while (1)
    {
        uint32_t flag = uart_get_edit_flag();

        if (UART_DICT_HAS_FLAG(flag, UD_BRAKE1_CTRL))
        {
            if (uart_dict[UD_BRAKE1_CTRL] & UD_BRAKE_CTRL_FLAG_INIT_DONE)
            {
                if (uart_dict[UD_BRAKE1_CTRL] & UD_BRAKE_CTRL_FLAG_ENABLE)
                    BrakeController(&brake_controller[0], BREAK_EV_ENABLE);
                if (uart_dict[UD_BRAKE1_CTRL] & UD_BRAKE_CTRL_FLAG_DISABLE)
                    BrakeController(&brake_controller[0], BREAK_EV_DISABLE);
                CLEAR_BIT(uart_dict[UD_BRAKE1_CTRL], UD_BRAKE_CTRL_FLAG_ENABLE | UD_BRAKE_CTRL_FLAG_DISABLE);
            }
        }

        if (UART_DICT_HAS_FLAG(flag, UD_BRAKE2_CTRL))
        {
            if (uart_dict[UD_BRAKE2_CTRL] & UD_BRAKE_CTRL_FLAG_INIT_DONE)
            {
                if (uart_dict[UD_BRAKE2_CTRL] & UD_BRAKE_CTRL_FLAG_ENABLE)
                    BrakeController(&brake_controller[1], BREAK_EV_ENABLE);
                if (uart_dict[UD_BRAKE2_CTRL] & UD_BRAKE_CTRL_FLAG_DISABLE)
                    BrakeController(&brake_controller[1], BREAK_EV_DISABLE);
                CLEAR_BIT(uart_dict[UD_BRAKE2_CTRL], UD_BRAKE_CTRL_FLAG_ENABLE | UD_BRAKE_CTRL_FLAG_DISABLE);
            }
        }

        if (delay_tick == 0)
        {
            delay_tick = 50;
            uart_dict[UD_BRAKE1_CUR_CURRENT] = adc_value[0];
            uart_dict[UD_BRAKE2_CUR_CURRENT] = adc_value[1];
            uart_dict[UD_MOTOR1_TEMP_ADC] = adc_value[2];
            uart_dict[UD_MOTOR2_TEMP_ADC] = adc_value[3];
            uart_dict[UD_MCU_TEMP_ADC] = adc_value[4];
            uart_send_pdo(UD_STATUS, 6);
        }
    }
}

void EmergencyStop(void)
{
    BrakeController(&brake_controller[0], BREAK_EV_FAULT);
    BrakeController(&brake_controller[1], BREAK_EV_FAULT);
}

void brake1_enable(bool enable, uint16_t duty)
{
    if (enable)
    {
        LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_7);
        LL_TIM_OC_SetCompareCH2(TIM1, duty);
    }
    else
    {
        LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_7);
        LL_TIM_OC_SetCompareCH2(TIM1, 0);
    }
}

void brake2_enable(bool enable, uint16_t duty)
{
    if (enable)
    {
        LL_GPIO_SetOutputPin(GPIOC, LL_GPIO_PIN_1);
        LL_TIM_OC_SetCompareCH1(TIM1, duty);
    }
    else
    {
        LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_1);
        LL_TIM_OC_SetCompareCH1(TIM1, 0);
    }
}

void brake1_write_int_pin(bool state)
{
    if (state)
    {
        LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_3);
        SET_BIT(uart_dict[UD_STATUS], UD_STATUS_FLAG_BRAKE1_OPENED);
    }
    else
    {
        LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_3);
        CLEAR_BIT(uart_dict[UD_STATUS], UD_STATUS_FLAG_BRAKE1_OPENED);
    }
}

void brake2_write_int_pin(bool state)
{
    if (state)
    {
        LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_2);
        SET_BIT(uart_dict[UD_STATUS], UD_STATUS_FLAG_BRAKE2_OPENED);
    }
    else
    {
        LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_2);
        CLEAR_BIT(uart_dict[UD_STATUS], UD_STATUS_FLAG_BRAKE2_OPENED);
    }
}

void ADC_IRQ_Callback(uint16_t value)
{
    adc_value[adc_rank_index] = value;
    adc_rank_index++;
    if (adc_rank_index >= sizeof(adc_ConvRank) / sizeof(adc_ConvRank[0]))
        adc_rank_index = 0;

    LL_ADC_REG_StopConversion(ADC1);
    LL_ADC_REG_SetSequencerChannels(ADC1, adc_ConvRank[adc_rank_index].channel);
    LL_ADC_SetSamplingTimeCommonChannels(ADC1, adc_ConvRank[adc_rank_index].sampling_time);
    LL_ADC_Enable(ADC1);
    LL_ADC_REG_StartConversion(ADC1);
}

void GPIO_Init(void)
{
    LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_7, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinOutputType(GPIOB, LL_GPIO_PIN_7, LL_GPIO_OUTPUT_PUSHPULL);
    LL_GPIO_SetPinPull(GPIOB, LL_GPIO_PIN_7, LL_GPIO_PULL_NO);
    LL_GPIO_SetPinSpeed(GPIOB, LL_GPIO_PIN_7, LL_GPIO_SPEED_FREQ_HIGH);

    LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_7, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinOutputType(GPIOB, LL_GPIO_PIN_7, LL_GPIO_OUTPUT_PUSHPULL);
    LL_GPIO_SetPinPull(GPIOB, LL_GPIO_PIN_7, LL_GPIO_PULL_NO);
    LL_GPIO_SetPinSpeed(GPIOB, LL_GPIO_PIN_7, LL_GPIO_SPEED_FREQ_HIGH);
}

void EXTI_Init()
{
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    LL_EXTI_InitTypeDef EXTI_InitStruct = {0};

    GPIO_InitStruct.Pin = LL_GPIO_PIN_5;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    LL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    LL_EXTI_SetEXTISource(LL_EXTI_CONFIG_PORTA, LL_EXTI_CONFIG_LINE5);
    LL_SYSCFG_EnableGPIOFilter(LL_SYSCFG_GPIO_PORTA, LL_GPIO_PIN_5);

    EXTI_InitStruct.Line = LL_EXTI_LINE_5;
    EXTI_InitStruct.LineCommand = ENABLE;
    EXTI_InitStruct.Mode = LL_EXTI_MODE_IT;
    EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_FALLING;
    LL_EXTI_Init(&EXTI_InitStruct);

    NVIC_SetPriority(EXTI4_15_IRQn, 0);
    NVIC_EnableIRQ(EXTI4_15_IRQn);
}

void USART_Init(void)
{
    LL_USART_InitTypeDef USART_InitStruct = {0};
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_USART1);

    GPIO_InitStruct.Pin = LL_GPIO_PIN_4;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
    GPIO_InitStruct.Alternate = LL_GPIO_AF1_USART1;
    LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = LL_GPIO_PIN_5;
    GPIO_InitStruct.Alternate = LL_GPIO_AF1_USART1;
    LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    USART_InitStruct.BaudRate = 115200;
    USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_8B;
    USART_InitStruct.StopBits = LL_USART_STOPBITS_1;
    USART_InitStruct.Parity = LL_USART_PARITY_NONE;
    USART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX_RX;
    USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
    USART_InitStruct.OverSampling = LL_USART_OVERSAMPLING_16;
    LL_USART_Init(USART1, &USART_InitStruct);

    LL_USART_ConfigAsyncMode(USART1);
    LL_USART_Enable(USART1);

    NVIC_SetPriority(USART1_IRQn, 1);
    NVIC_EnableIRQ(USART1_IRQn);
}

void PWM_Init(void)
{
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

    LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_TIM1);

    GPIO_InitStruct.Pin = LL_GPIO_PIN_0 | LL_GPIO_PIN_1;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = LL_GPIO_AF2_TIM1;
    LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    LL_TIM_SetCounterMode(TIM1, LL_TIM_COUNTERMODE_CENTER_UP);
    LL_TIM_SetAutoReload(TIM1, 300 - 1);
    LL_TIM_SetPrescaler(TIM1, 0);
    LL_TIM_SetClockDivision(TIM1, LL_TIM_CLOCKDIVISION_DIV1);
    LL_TIM_SetRepetitionCounter(TIM1, 80 - 1);
    LL_TIM_EnableARRPreload(TIM1);

    LL_TIM_OC_SetMode(TIM1, LL_TIM_CHANNEL_CH1, LL_TIM_OCMODE_PWM1);
    LL_TIM_OC_SetPolarity(TIM1, LL_TIM_CHANNEL_CH1, LL_TIM_OCPOLARITY_HIGH);
    LL_TIM_OC_SetCompareCH1(TIM1, 0);
    LL_TIM_OC_EnablePreload(TIM1, LL_TIM_CHANNEL_CH1);

    LL_TIM_OC_SetMode(TIM1, LL_TIM_CHANNEL_CH2, LL_TIM_OCMODE_PWM1);
    LL_TIM_OC_SetPolarity(TIM1, LL_TIM_CHANNEL_CH2, LL_TIM_OCPOLARITY_HIGH);
    LL_TIM_OC_SetCompareCH2(TIM1, 0);
    LL_TIM_OC_EnablePreload(TIM1, LL_TIM_CHANNEL_CH2);

    LL_TIM_SetTriggerOutput(TIM1, LL_TIM_TRGO_UPDATE);

    LL_TIM_EnableAllOutputs(TIM1);
    LL_TIM_EnableCounter(TIM1);
    LL_TIM_CC_EnableChannel(TIM1, LL_TIM_CHANNEL_CH1 | LL_TIM_CHANNEL_CH2);
}

void ADC_Init(void)
{
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    LL_ADC_InitTypeDef ADC_InitStruct = {0};
    LL_ADC_REG_InitTypeDef ADC_REG_InitStruct = {0};

    LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_ADC1);

    GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;

    GPIO_InitStruct.Pin = LL_GPIO_PIN_3 | LL_GPIO_PIN_4 | LL_GPIO_PIN_6 | LL_GPIO_PIN_7;
    LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = LL_GPIO_PIN_0 | LL_GPIO_PIN_1;
    LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    ADC_InitStruct.Clock = LL_ADC_CLOCK_SYNC_PCLK_DIV4;
    ADC_InitStruct.Resolution = LL_ADC_RESOLUTION_12B;
    ADC_InitStruct.DataAlignment = LL_ADC_DATA_ALIGN_RIGHT;
    ADC_InitStruct.LowPowerMode = LL_ADC_LP_MODE_NONE;
    LL_ADC_Init(ADC1, &ADC_InitStruct);

    ADC_REG_InitStruct.TriggerSource = LL_ADC_REG_TRIG_EXT_TIM1_TRGO;
    ADC_REG_InitStruct.SequencerDiscont = LL_ADC_REG_SEQ_DISCONT_1RANK;
    ADC_REG_InitStruct.ContinuousMode = LL_ADC_REG_CONV_SINGLE;
    ADC_REG_InitStruct.Overrun = LL_ADC_REG_OVR_DATA_OVERWRITTEN;
    LL_ADC_REG_Init(ADC1, &ADC_REG_InitStruct);
    LL_ADC_REG_SetTriggerEdge(ADC1, LL_ADC_REG_TRIG_EXT_RISING);

    LL_ADC_SetCommonPathInternalCh(__LL_ADC_COMMON_INSTANCE(ADC1), LL_ADC_PATH_INTERNAL_TEMPSENSOR);
    LL_ADC_REG_SetSequencerChannels(ADC1, adc_ConvRank[0].channel);

    volatile uint32_t wait_loop_index = ((LL_ADC_DELAY_TEMPSENSOR_STAB_US * (SystemCoreClock / (100000 * 2))) / 10);
    while (wait_loop_index != 0)
    {
        wait_loop_index--;
    }

    /* Enable EOC IT */
    LL_ADC_EnableIT_EOS(ADC1);

    NVIC_SetPriority(ADC_COMP_IRQn, 3);
    NVIC_EnableIRQ(ADC_COMP_IRQn);

    LL_ADC_StartCalibration(ADC1);
    while (LL_ADC_IsCalibrationOnGoing(ADC1))
    {
    }

    delay_tick = 2;
    while (delay_tick == 0)
        ;

    LL_ADC_Enable(ADC1);

    delay_tick = 2;
    while (delay_tick == 0)
        ;

    LL_ADC_REG_StartConversion(ADC1);
}

static void SystemClockConfig(void)
{
    /* Enable HSI */
    LL_RCC_HSI_Enable();
    while (LL_RCC_HSI_IsReady() != 1)
    {
    }

    /* Set AHB divider: HCLK = SYSCLK */
    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);

    /* HSISYS used as SYSCLK clock source  */
    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_HSISYS);
    while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_HSISYS)
    {
    }

    /* Set APB1 divider */
    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
    LL_Init1msTick(24000000);
    SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;

    NVIC_SetPriority(SysTick_IRQn, 15);
    NVIC_EnableIRQ(SysTick_IRQn);

    /* Update CMSIS variable (which can be updated also through SystemCoreClockUpdate function) */
    LL_SetSystemCoreClock(24000000);
}

/**
 * @brief  Error executing function.
 * @param  None
 * @retval None
 */
void APP_ErrorHandler(void)
{
    while (1)
    {
    }
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
    /* Users can add their own printing information as needed,
       for example: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
    /* Infinite loop */
    while (1)
    {
    }
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT Puya *****END OF FILE******************/
