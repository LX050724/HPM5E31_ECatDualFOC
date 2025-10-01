/*
 * Copyright (c) 2025 HPMicro
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

/*
 * Note:
 *   PY and PZ IOs: if any SOC pin function needs to be routed to these IOs,
 *  besides of IOC, PIOC/BIOC needs to be configured SOC_GPIO_X_xx, so that
 *  expected SoC function can be enabled on these IOs.
 *
 */
#include "board.h"
#include "hpm_gpio_drv.h"
#include "hpm_gpio_regs.h"
#include "hpm_ioc_regs.h"
#include "hpm_iomux.h"
#include <stdint.h>
#include <stdio.h>
//
#include "pinmux.h"

void init_uart_pins(UART_Type *ptr)
{
    if (ptr == HPM_UART0)
    {
        HPM_IOC->PAD[IOC_PAD_PA00].FUNC_CTL = IOC_PA00_FUNC_CTL_UART0_TXD;
        HPM_IOC->PAD[IOC_PAD_PA01].FUNC_CTL = IOC_PA01_FUNC_CTL_UART0_RXD;
    }
    else if (ptr == HPM_UART2)
    {
        HPM_IOC->PAD[IOC_PAD_PC08].FUNC_CTL = IOC_PC08_FUNC_CTL_UART2_TXD;
        HPM_IOC->PAD[IOC_PAD_PC09].FUNC_CTL = IOC_PC09_FUNC_CTL_UART2_RXD;
    }
    else if (ptr == HPM_UART3)
    {
        HPM_IOC->PAD[IOC_PAD_PC15].FUNC_CTL = IOC_PC15_FUNC_CTL_UART3_TXD;
        HPM_IOC->PAD[IOC_PAD_PC14].FUNC_CTL = IOC_PC14_FUNC_CTL_UART3_RXD;
        HPM_IOC->PAD[IOC_PAD_PC13].FUNC_CTL = IOC_PC13_FUNC_CTL_UART3_DE;
    }
    else if (ptr == HPM_UART4)
    {
        HPM_IOC->PAD[IOC_PAD_PA16].FUNC_CTL = IOC_PA16_FUNC_CTL_UART4_TXD;
        HPM_IOC->PAD[IOC_PAD_PA17].FUNC_CTL = IOC_PA17_FUNC_CTL_UART4_RXD;
        HPM_IOC->PAD[IOC_PAD_PA18].FUNC_CTL = IOC_PA18_FUNC_CTL_UART4_DE;
    }
    else if (ptr == HPM_UART6)
    {
        HPM_IOC->PAD[IOC_PAD_PA24].FUNC_CTL = IOC_PA24_FUNC_CTL_UART6_TXD;
        HPM_IOC->PAD[IOC_PAD_PA25].FUNC_CTL = IOC_PA25_FUNC_CTL_UART6_RXD;
        HPM_IOC->PAD[IOC_PAD_PA26].FUNC_CTL = IOC_PA26_FUNC_CTL_UART6_DE;
    }
}

void init_rgb_gpio_pins()
{
    HPM_IOC->PAD[IOC_PAD_PC02].FUNC_CTL = IOC_PC02_FUNC_CTL_GPIO_C_02; /* G */
    HPM_IOC->PAD[IOC_PAD_PC03].FUNC_CTL = IOC_PC03_FUNC_CTL_GPIO_C_03; /* R */
    HPM_IOC->PAD[IOC_PAD_PC18].FUNC_CTL = IOC_PC18_FUNC_CTL_GPIO_C_18; /* B */
}

void init_encoder_gpio_pins(uint8_t index)
{
    if (index == 0)
    {
        HPM_IOC->PAD[IOC_PAD_PA22].FUNC_CTL = IOC_PA19_FUNC_CTL_GPIO_A_19;
        HPM_IOC->PAD[IOC_PAD_PA19].FUNC_CTL = IOC_PA22_FUNC_CTL_GPIO_A_22;
        HPM_IOC->PAD[IOC_PAD_PA20].FUNC_CTL = IOC_PA20_FUNC_CTL_GPIO_A_20;
        HPM_IOC->PAD[IOC_PAD_PA21].FUNC_CTL = IOC_PA21_FUNC_CTL_GPIO_A_21;
    }
    else if (index == 1)
    {
        HPM_IOC->PAD[IOC_PAD_PD10].FUNC_CTL = IOC_PD10_FUNC_CTL_GPIO_D_10;
        HPM_IOC->PAD[IOC_PAD_PD11].FUNC_CTL = IOC_PD11_FUNC_CTL_GPIO_D_11;
        HPM_IOC->PAD[IOC_PAD_PD12].FUNC_CTL = IOC_PD12_FUNC_CTL_GPIO_D_12;
        HPM_IOC->PAD[IOC_PAD_PD13].FUNC_CTL = IOC_PD13_FUNC_CTL_GPIO_D_13;
    }
}

void init_spi_pins(SPI_Type *ptr)
{
    if (ptr == HPM_SPI0)
    {
        HPM_IOC->PAD[IOC_PAD_PC06].FUNC_CTL = IOC_PC06_FUNC_CTL_SPI0_MISO;
        HPM_IOC->PAD[IOC_PAD_PC07].FUNC_CTL = IOC_PC07_FUNC_CTL_SPI0_MOSI;
        HPM_IOC->PAD[IOC_PAD_PC04].FUNC_CTL = IOC_PC04_FUNC_CTL_SPI0_SCLK | IOC_PAD_FUNC_CTL_LOOP_BACK_MASK;
        HPM_IOC->PAD[IOC_PAD_PC05].FUNC_CTL = IOC_PC05_FUNC_CTL_SPI0_CS_0;
        HPM_IOC->PAD[IOC_PAD_PC05].PAD_CTL = IOC_PAD_PAD_CTL_DS_SET(8) | IOC_PAD_PAD_CTL_PE_SET(1) |
                                             IOC_PAD_PAD_CTL_PS_SET(1) | IOC_PAD_PAD_CTL_PRS_SET(1);
    }
    else if (ptr == HPM_SPI2)
    {
        HPM_IOC->PAD[IOC_PAD_PC30].FUNC_CTL = IOC_PC30_FUNC_CTL_SPI2_DAT2;
        HPM_IOC->PAD[IOC_PAD_PC31].FUNC_CTL = IOC_PC31_FUNC_CTL_SPI2_DAT3;
        HPM_IOC->PAD[IOC_PAD_PC28].FUNC_CTL = IOC_PC28_FUNC_CTL_SPI2_MISO;
        HPM_IOC->PAD[IOC_PAD_PC29].FUNC_CTL = IOC_PC29_FUNC_CTL_SPI2_MOSI;
        HPM_IOC->PAD[IOC_PAD_PC26].FUNC_CTL = IOC_PC26_FUNC_CTL_SPI2_SCLK | IOC_PAD_FUNC_CTL_LOOP_BACK_MASK;
        HPM_IOC->PAD[IOC_PAD_PC27].FUNC_CTL = IOC_PC27_FUNC_CTL_SPI2_CS_0;

        HPM_IOC->PAD[IOC_PAD_PC30].PAD_CTL = IOC_PAD_PAD_CTL_SR_MASK | IOC_PAD_PAD_CTL_SPD_SET(3);
        HPM_IOC->PAD[IOC_PAD_PC31].PAD_CTL = IOC_PAD_PAD_CTL_SR_MASK | IOC_PAD_PAD_CTL_SPD_SET(3);
        HPM_IOC->PAD[IOC_PAD_PC28].PAD_CTL = IOC_PAD_PAD_CTL_SR_MASK | IOC_PAD_PAD_CTL_SPD_SET(3);
        HPM_IOC->PAD[IOC_PAD_PC29].PAD_CTL = IOC_PAD_PAD_CTL_SR_MASK | IOC_PAD_PAD_CTL_SPD_SET(3);
        HPM_IOC->PAD[IOC_PAD_PC26].PAD_CTL = IOC_PAD_PAD_CTL_SR_MASK | IOC_PAD_PAD_CTL_SPD_SET(3);
        HPM_IOC->PAD[IOC_PAD_PC27].PAD_CTL = IOC_PAD_PAD_CTL_SR_MASK | IOC_PAD_PAD_CTL_SPD_SET(3) |
                                             IOC_PAD_PAD_CTL_PE_SET(1) | IOC_PAD_PAD_CTL_PS_SET(1) |
                                             IOC_PAD_PAD_CTL_PRS_SET(1);
    }
    else if (ptr == HPM_SPI3)
    {
        HPM_IOC->PAD[IOC_PAD_PC22].FUNC_CTL = IOC_PC22_FUNC_CTL_SPI3_MISO;
        HPM_IOC->PAD[IOC_PAD_PC23].FUNC_CTL = IOC_PC23_FUNC_CTL_SPI3_MOSI;
        HPM_IOC->PAD[IOC_PAD_PC20].FUNC_CTL = IOC_PC20_FUNC_CTL_SPI3_SCLK | IOC_PAD_FUNC_CTL_LOOP_BACK_MASK;
        HPM_IOC->PAD[IOC_PAD_PC21].FUNC_CTL = IOC_PC21_FUNC_CTL_SPI3_CS_0;
        HPM_IOC->PAD[IOC_PAD_PC21].PAD_CTL =
            IOC_PAD_PAD_CTL_PE_SET(1) | IOC_PAD_PAD_CTL_PS_SET(1) | IOC_PAD_PAD_CTL_PRS_SET(1);
    }
}

void init_gptmr_pins(GPTMR_Type *ptr)
{
    trgm_output_t trgm0_io_config = {0};
    if (ptr == HPM_GPTMR0)
    {
        trgm0_io_config.invert = 0;
        trgm0_io_config.type = trgm_output_same_as_input;

        HPM_IOC->PAD[IOC_PAD_PC00].FUNC_CTL = IOC_PC00_FUNC_CTL_TRGM_P_00;
        trgm0_io_config.input = HPM_TRGM0_INPUT_SRC_TRGM0_P00;
        trgm_output_config(HPM_TRGM0, HPM_TRGM0_OUTPUT_SRC_GPTMR0_CAPT_2, &trgm0_io_config);

        HPM_IOC->PAD[IOC_PAD_PD07].FUNC_CTL = IOC_PD07_FUNC_CTL_TRGM_P_07;
        trgm_enable_io_output(HPM_TRGM0, 1 << 7);
        trgm0_io_config.input = HPM_TRGM0_INPUT_SRC_GPTMR0_OUT2;
        trgm_output_config(HPM_TRGM0, HPM_TRGM0_OUTPUT_SRC_TRGM0_P07, &trgm0_io_config);

        HPM_IOC->PAD[IOC_PAD_PD15].FUNC_CTL = IOC_PD15_FUNC_CTL_TRGM_P_15;
        trgm_enable_io_output(HPM_TRGM0, 1 << 15);
        trgm0_io_config.input = HPM_TRGM0_INPUT_SRC_GPTMR0_OUT3;
        trgm_output_config(HPM_TRGM0, HPM_TRGM0_OUTPUT_SRC_TRGM0_P15, &trgm0_io_config);
    }
    else if (ptr == HPM_GPTMR1)
    {
        HPM_IOC->PAD[IOC_PAD_PC03].FUNC_CTL = IOC_PC03_FUNC_CTL_TRGM_P_03;
        trgm_enable_io_output(HPM_TRGM0, 1 << 3);
        trgm0_io_config.input = HPM_TRGM0_INPUT_SRC_GPTMR1_OUT2;
        trgm_output_config(HPM_TRGM0, HPM_TRGM0_OUTPUT_SRC_TRGM0_P03, &trgm0_io_config);
    }
}

void init_butn_pins(void)
{
    /* configure pad setting: pull enable and pull up, schmitt trigger enable */
    /* enable schmitt trigger to eliminate jitter of pin used as button */

    /* Button */
}

void init_pwm_fault_pins(void)
{
    HPM_IOC->PAD[IOC_PAD_PC10].FUNC_CTL = IOC_PC10_FUNC_CTL_TRGM_P_10;
}

void init_driverEN_pins(uint8_t index)
{
    if (index == BOARD_DRIVER_A_INDEX)
    {
        HPM_IOC->PAD[IOC_PAD_PF06].FUNC_CTL = IOC_PF06_FUNC_CTL_GPIO_F_06;
    }
    else if (index == BOARD_DRIVER_B_INDEX)
    {
        HPM_IOC->PAD[IOC_PAD_PE00].FUNC_CTL = IOC_PE00_FUNC_CTL_GPIO_E_00;
    }
}

void init_pwm_pins(PWMV2_Type *ptr)
{
    if (ptr == HPM_PWM0)
    {
        HPM_IOC->PAD[IOC_PAD_PF00].FUNC_CTL = IOC_PF00_FUNC_CTL_PWM0_P_0;
        HPM_IOC->PAD[IOC_PAD_PF01].FUNC_CTL = IOC_PF01_FUNC_CTL_PWM0_P_1;
        HPM_IOC->PAD[IOC_PAD_PF02].FUNC_CTL = IOC_PF02_FUNC_CTL_PWM0_P_2;
        HPM_IOC->PAD[IOC_PAD_PF03].FUNC_CTL = IOC_PF03_FUNC_CTL_PWM0_P_3;
        HPM_IOC->PAD[IOC_PAD_PF04].FUNC_CTL = IOC_PF04_FUNC_CTL_PWM0_P_4;
        HPM_IOC->PAD[IOC_PAD_PF05].FUNC_CTL = IOC_PF05_FUNC_CTL_PWM0_P_5;
    }
    else if (ptr == HPM_PWM1)
    {
        HPM_IOC->PAD[IOC_PAD_PF08].FUNC_CTL = IOC_PF08_FUNC_CTL_PWM1_P_0;
        HPM_IOC->PAD[IOC_PAD_PF09].FUNC_CTL = IOC_PF09_FUNC_CTL_PWM1_P_1;
        HPM_IOC->PAD[IOC_PAD_PF10].FUNC_CTL = IOC_PF10_FUNC_CTL_PWM1_P_2;
        HPM_IOC->PAD[IOC_PAD_PF11].FUNC_CTL = IOC_PF11_FUNC_CTL_PWM1_P_3;
        HPM_IOC->PAD[IOC_PAD_PF12].FUNC_CTL = IOC_PF12_FUNC_CTL_PWM1_P_4;
        HPM_IOC->PAD[IOC_PAD_PF13].FUNC_CTL = IOC_PF13_FUNC_CTL_PWM1_P_5;
    }
}

void init_usb_pins(USB_Type *ptr)
{
    if (ptr == HPM_USB0)
    {
    }
}

void init_clk_obs_pins(void)
{
    /* HPM_IOC->PAD[IOC_PAD_PB02].FUNC_CTL = IOC_PB02_FUNC_CTL_SYSCTL_CLK_OBS_0; */
}

void init_qeo_pins(QEOV2_Type *ptr)
{
    if (ptr == HPM_QEO1)
    {
        HPM_IOC->PAD[IOC_PAD_PC12].FUNC_CTL = IOC_PC12_FUNC_CTL_QEO1_Z;
        HPM_IOC->PAD[IOC_PAD_PC13].FUNC_CTL = IOC_PC13_FUNC_CTL_QEO1_A;
        HPM_IOC->PAD[IOC_PAD_PC14].FUNC_CTL = IOC_PC14_FUNC_CTL_QEO1_B;
    }
}

void init_qeiv2_abz_pins(QEIV2_Type *ptr)
{
    if (ptr == HPM_QEI0)
    {
        HPM_IOC->PAD[IOC_PAD_PD07].FUNC_CTL = IOC_PD07_FUNC_CTL_QEI0_A;
        HPM_IOC->PAD[IOC_PAD_PD06].FUNC_CTL = IOC_PD06_FUNC_CTL_QEI0_B;
        HPM_IOC->PAD[IOC_PAD_PD05].FUNC_CTL = IOC_PD05_FUNC_CTL_QEI0_Z;
    }
    else if (ptr == HPM_QEI1)
    {
        HPM_IOC->PAD[IOC_PAD_PD30].FUNC_CTL = IOC_PD30_FUNC_CTL_QEI1_A;
        HPM_IOC->PAD[IOC_PAD_PD31].FUNC_CTL = IOC_PD31_FUNC_CTL_QEI1_B;
        HPM_IOC->PAD[IOC_PAD_PD29].FUNC_CTL = IOC_PD29_FUNC_CTL_QEI1_Z;
    }
}

void init_enet_pins(ENET_Type *ptr)
{
    if (ptr == HPM_ENET0)
    {
        HPM_IOC->PAD[IOC_PAD_PA23].FUNC_CTL = IOC_PA23_FUNC_CTL_ETH0_RXER;
        HPM_IOC->PAD[IOC_PAD_PB06].FUNC_CTL = IOC_PB06_FUNC_CTL_ETH0_TXCK;
        HPM_IOC->PAD[IOC_PAD_PB11].FUNC_CTL = IOC_PB11_FUNC_CTL_ETH0_TXEN;
        HPM_IOC->PAD[IOC_PAD_PB01].FUNC_CTL = IOC_PB01_FUNC_CTL_ETH0_RXD_0;
        HPM_IOC->PAD[IOC_PAD_PB02].FUNC_CTL = IOC_PB02_FUNC_CTL_ETH0_RXD_1;
        HPM_IOC->PAD[IOC_PAD_PB03].FUNC_CTL = IOC_PB03_FUNC_CTL_ETH0_RXD_2;
        HPM_IOC->PAD[IOC_PAD_PB04].FUNC_CTL = IOC_PB04_FUNC_CTL_ETH0_RXD_3;
        HPM_IOC->PAD[IOC_PAD_PB07].FUNC_CTL = IOC_PB07_FUNC_CTL_ETH0_TXD_0;
        HPM_IOC->PAD[IOC_PAD_PB08].FUNC_CTL = IOC_PB08_FUNC_CTL_ETH0_TXD_1;
        HPM_IOC->PAD[IOC_PAD_PB09].FUNC_CTL = IOC_PB09_FUNC_CTL_ETH0_TXD_2;
        HPM_IOC->PAD[IOC_PAD_PB10].FUNC_CTL = IOC_PB10_FUNC_CTL_ETH0_TXD_3;
        HPM_IOC->PAD[IOC_PAD_PB05].FUNC_CTL = IOC_PB05_FUNC_CTL_ETH0_RXCK;
        HPM_IOC->PAD[IOC_PAD_PB00].FUNC_CTL = IOC_PB00_FUNC_CTL_ETH0_RXDV;
        HPM_IOC->PAD[IOC_PAD_PA31].FUNC_CTL = IOC_PA31_FUNC_CTL_ETH0_MDC;
        HPM_IOC->PAD[IOC_PAD_PA30].FUNC_CTL = IOC_PA30_FUNC_CTL_ETH0_MDIO;
        HPM_IOC->PAD[IOC_PAD_PA09].FUNC_CTL = IOC_PA09_FUNC_CTL_ESC0_REFCK;
    }
}

void init_adc16_pins(void)
{
    HPM_IOC->PAD[IOC_PAD_PF31].FUNC_CTL = IOC_PAD_FUNC_CTL_ANALOG_MASK;
    HPM_IOC->PAD[IOC_PAD_PF28].FUNC_CTL = IOC_PAD_FUNC_CTL_ANALOG_MASK;
    HPM_IOC->PAD[IOC_PAD_PF20].FUNC_CTL = IOC_PAD_FUNC_CTL_ANALOG_MASK;
    HPM_IOC->PAD[IOC_PAD_PF26].FUNC_CTL = IOC_PAD_FUNC_CTL_ANALOG_MASK;
    HPM_IOC->PAD[IOC_PAD_PF16].FUNC_CTL = IOC_PAD_FUNC_CTL_ANALOG_MASK;
    HPM_IOC->PAD[IOC_PAD_PF17].FUNC_CTL = IOC_PAD_FUNC_CTL_ANALOG_MASK;
    HPM_IOC->PAD[IOC_PAD_PF22].FUNC_CTL = IOC_PAD_FUNC_CTL_ANALOG_MASK;
    HPM_IOC->PAD[IOC_PAD_PF18].FUNC_CTL = IOC_PAD_FUNC_CTL_ANALOG_MASK;
    HPM_IOC->PAD[IOC_PAD_PF24].FUNC_CTL = IOC_PAD_FUNC_CTL_ANALOG_MASK;
}

void init_can_pins(MCAN_Type *ptr)
{
    if (ptr == HPM_MCAN0)
    {
        HPM_IOC->PAD[IOC_PAD_PC01].FUNC_CTL = IOC_PC01_FUNC_CTL_MCAN0_RXD;
        HPM_IOC->PAD[IOC_PAD_PC00].FUNC_CTL = IOC_PC00_FUNC_CTL_MCAN0_TXD;
    }
    else if (ptr == HPM_MCAN3)
    {
        HPM_IOC->PAD[IOC_PAD_PD15].FUNC_CTL = IOC_PD15_FUNC_CTL_MCAN3_TXD;
        HPM_IOC->PAD[IOC_PAD_PD14].FUNC_CTL = IOC_PD14_FUNC_CTL_MCAN3_RXD;
    }
}

void init_esc_pins(void)
{
    HPM_IOC->PAD[IOC_PAD_PA09].FUNC_CTL = IOC_PA09_FUNC_CTL_ESC0_REFCK;
    HPM_IOC->PAD[IOC_PAD_PA30].FUNC_CTL = IOC_PA30_FUNC_CTL_ESC0_MDIO;
    HPM_IOC->PAD[IOC_PAD_PA31].FUNC_CTL = IOC_PA31_FUNC_CTL_ESC0_MDC;

    /* ESC needs to configure these pins for specific functions, see ESC IOCFG registers */
    HPM_IOC->PAD[IOC_PAD_PB24].FUNC_CTL = IOC_PB24_FUNC_CTL_GPIO_B_24;  /* GPIO to reset PHY */
    HPM_IOC->PAD[IOC_PAD_PB25].FUNC_CTL = IOC_PB25_FUNC_CTL_ESC0_CTR_5; /* NMII_LINK0 function */
    HPM_IOC->PAD[IOC_PAD_PB25].PAD_CTL =
        IOC_PAD_PAD_CTL_PE_SET(1) | IOC_PAD_PAD_CTL_PS_SET(1);          /* Internally pull up to avoid suspension */
    HPM_IOC->PAD[IOC_PAD_PE03].FUNC_CTL = IOC_PE03_FUNC_CTL_ESC0_CTR_1; /* NMII_LINK2 function */
    HPM_IOC->PAD[IOC_PAD_PE03].PAD_CTL =
        IOC_PAD_PAD_CTL_PE_SET(1) | IOC_PAD_PAD_CTL_PS_SET(1); /* Internally pull up to avoid suspension */

    HPM_IOC->PAD[IOC_PAD_PB30].FUNC_CTL = IOC_PB30_FUNC_CTL_ESC0_CTR_7;
    HPM_IOC->PAD[IOC_PAD_PB31].FUNC_CTL = IOC_PB31_FUNC_CTL_ESC0_CTR_8;

    /* ESC port0 */
    HPM_IOC->PAD[IOC_PAD_PB00].FUNC_CTL = IOC_PB00_FUNC_CTL_ESC0_P0_RXDV;
    HPM_IOC->PAD[IOC_PAD_PB05].FUNC_CTL = IOC_PB05_FUNC_CTL_ESC0_P0_RXCK;
    HPM_IOC->PAD[IOC_PAD_PA23].FUNC_CTL = IOC_PA23_FUNC_CTL_ESC0_P0_RXER;
    HPM_IOC->PAD[IOC_PAD_PB06].FUNC_CTL = IOC_PB06_FUNC_CTL_ESC0_P0_TXCK;
    HPM_IOC->PAD[IOC_PAD_PB11].FUNC_CTL = IOC_PB11_FUNC_CTL_ESC0_P0_TXEN;
    HPM_IOC->PAD[IOC_PAD_PB01].FUNC_CTL = IOC_PB01_FUNC_CTL_ESC0_P0_RXD_0;
    HPM_IOC->PAD[IOC_PAD_PB02].FUNC_CTL = IOC_PB02_FUNC_CTL_ESC0_P0_RXD_1;
    HPM_IOC->PAD[IOC_PAD_PB03].FUNC_CTL = IOC_PB03_FUNC_CTL_ESC0_P0_RXD_2;
    HPM_IOC->PAD[IOC_PAD_PB04].FUNC_CTL = IOC_PB04_FUNC_CTL_ESC0_P0_RXD_3;
    HPM_IOC->PAD[IOC_PAD_PB07].FUNC_CTL = IOC_PB07_FUNC_CTL_ESC0_P0_TXD_0;
    HPM_IOC->PAD[IOC_PAD_PB08].FUNC_CTL = IOC_PB08_FUNC_CTL_ESC0_P0_TXD_1;
    HPM_IOC->PAD[IOC_PAD_PB09].FUNC_CTL = IOC_PB09_FUNC_CTL_ESC0_P0_TXD_2;
    HPM_IOC->PAD[IOC_PAD_PB10].FUNC_CTL = IOC_PB10_FUNC_CTL_ESC0_P0_TXD_3;

    /* ESC port2 */
    HPM_IOC->PAD[IOC_PAD_PD21].FUNC_CTL = IOC_PD21_FUNC_CTL_ESC0_P2_RXCK;
    HPM_IOC->PAD[IOC_PAD_PD16].FUNC_CTL = IOC_PD16_FUNC_CTL_ESC0_P2_RXDV;
    HPM_IOC->PAD[IOC_PAD_PD22].FUNC_CTL = IOC_PD22_FUNC_CTL_ESC0_P2_RXER;
    HPM_IOC->PAD[IOC_PAD_PD23].FUNC_CTL = IOC_PD23_FUNC_CTL_ESC0_P2_TXCK;
    HPM_IOC->PAD[IOC_PAD_PD28].FUNC_CTL = IOC_PD28_FUNC_CTL_ESC0_P2_TXEN;
    HPM_IOC->PAD[IOC_PAD_PD17].FUNC_CTL = IOC_PD17_FUNC_CTL_ESC0_P2_RXD_0;
    HPM_IOC->PAD[IOC_PAD_PD18].FUNC_CTL = IOC_PD18_FUNC_CTL_ESC0_P2_RXD_1;
    HPM_IOC->PAD[IOC_PAD_PD19].FUNC_CTL = IOC_PD19_FUNC_CTL_ESC0_P2_RXD_2;
    HPM_IOC->PAD[IOC_PAD_PD20].FUNC_CTL = IOC_PD20_FUNC_CTL_ESC0_P2_RXD_3;
    HPM_IOC->PAD[IOC_PAD_PD24].FUNC_CTL = IOC_PD24_FUNC_CTL_ESC0_P2_TXD_0;
    HPM_IOC->PAD[IOC_PAD_PD25].FUNC_CTL = IOC_PD25_FUNC_CTL_ESC0_P2_TXD_1;
    HPM_IOC->PAD[IOC_PAD_PD26].FUNC_CTL = IOC_PD26_FUNC_CTL_ESC0_P2_TXD_2;
    HPM_IOC->PAD[IOC_PAD_PD27].FUNC_CTL = IOC_PD27_FUNC_CTL_ESC0_P2_TXD_3;
}

/* ESC input/output demo pins */
void init_esc_in_out_pin(void)
{
    HPM_IOC->PAD[IOC_PAD_PD06].FUNC_CTL = IOC_PD06_FUNC_CTL_GPIO_D_06;
    HPM_IOC->PAD[IOC_PAD_PD12].FUNC_CTL = IOC_PD12_FUNC_CTL_GPIO_D_12;

    HPM_IOC->PAD[IOC_PAD_PC23].FUNC_CTL = IOC_PC23_FUNC_CTL_GPIO_C_23;
    HPM_IOC->PAD[IOC_PAD_PC24].FUNC_CTL = IOC_PC24_FUNC_CTL_GPIO_C_24;
}

void init_eui_pins(EUI_Type *ptr)
{
    if (ptr == HPM_EUI1)
    {
        HPM_IOC->PAD[IOC_PAD_PB26].FUNC_CTL = IOC_PB26_FUNC_CTL_EUI1_CK;
        HPM_IOC->PAD[IOC_PAD_PB27].FUNC_CTL = IOC_PB27_FUNC_CTL_EUI1_SH;
        HPM_IOC->PAD[IOC_PAD_PB28].FUNC_CTL = IOC_PB28_FUNC_CTL_EUI1_DI;
        HPM_IOC->PAD[IOC_PAD_PB29].FUNC_CTL = IOC_PB29_FUNC_CTL_EUI1_DO;
    }
}

void init_gptmr_channel_pin(GPTMR_Type *ptr, uint32_t channel, bool as_comp)
{
    trgm_output_t trgm0_io_config = {0};
    if (ptr == HPM_GPTMR0)
    {
        if (as_comp == true)
        {
            if (channel == 2)
            {
                HPM_IOC->PAD[IOC_PAD_PD07].FUNC_CTL = IOC_PD07_FUNC_CTL_TRGM_P_07;
                trgm_enable_io_output(HPM_TRGM0, 1 << 7);
                trgm0_io_config.input = HPM_TRGM0_INPUT_SRC_GPTMR0_OUT2;
                trgm_output_config(HPM_TRGM0, HPM_TRGM0_OUTPUT_SRC_TRGM0_P07, &trgm0_io_config);
            }
            else if (channel == 3)
            {
                HPM_IOC->PAD[IOC_PAD_PD15].FUNC_CTL = IOC_PD15_FUNC_CTL_TRGM_P_15;
                trgm_enable_io_output(HPM_TRGM0, 1 << 15);
                trgm0_io_config.input = HPM_TRGM0_INPUT_SRC_GPTMR0_OUT3;
                trgm_output_config(HPM_TRGM0, HPM_TRGM0_OUTPUT_SRC_TRGM0_P15, &trgm0_io_config);
            }
            else
            {
                ;
            }
        }
        else
        {
            if (channel == 2)
            {
                HPM_IOC->PAD[IOC_PAD_PC00].FUNC_CTL = IOC_PC00_FUNC_CTL_TRGM_P_00;
                trgm0_io_config.invert = 0;
                trgm0_io_config.type = trgm_output_same_as_input;
                trgm0_io_config.input = HPM_TRGM0_INPUT_SRC_TRGM0_P00;
                trgm_output_config(HPM_TRGM0, HPM_TRGM0_OUTPUT_SRC_GPTMR0_CAPT_2, &trgm0_io_config);
            }
            else if (channel == 3)
            {
                HPM_IOC->PAD[IOC_PAD_PC08].FUNC_CTL = IOC_PC08_FUNC_CTL_TRGM_P_08;
                trgm0_io_config.invert = 0;
                trgm0_io_config.type = trgm_output_same_as_input;
                trgm0_io_config.input = HPM_TRGM0_INPUT_SRC_TRGM0_P08;
                trgm_output_config(HPM_TRGM0, HPM_TRGM0_OUTPUT_SRC_GPTMR0_CAPT_3, &trgm0_io_config);
            }
            else
            {
                ;
            }
        }
    }
    else if (ptr == HPM_GPTMR1)
    {
        if (as_comp == true)
        {
            if (channel == 2)
            {
                HPM_IOC->PAD[IOC_PAD_PC03].FUNC_CTL = IOC_PC03_FUNC_CTL_TRGM_P_03;
                trgm_enable_io_output(HPM_TRGM0, 1 << 3);
                trgm0_io_config.input = HPM_TRGM0_INPUT_SRC_GPTMR1_OUT2;
                trgm_output_config(HPM_TRGM0, HPM_TRGM0_OUTPUT_SRC_TRGM0_P03, &trgm0_io_config);
            }
        }
        else
        {
            if (channel == 2)
            {
                HPM_IOC->PAD[IOC_PAD_PC00].FUNC_CTL = IOC_PC00_FUNC_CTL_TRGM_P_00;
                trgm0_io_config.invert = 0;
                trgm0_io_config.type = trgm_output_same_as_input;
                trgm0_io_config.input = HPM_TRGM0_INPUT_SRC_TRGM0_P00;
                trgm_output_config(HPM_TRGM0, HPM_TRGM0_OUTPUT_SRC_GPTMR1_CAPT_2, &trgm0_io_config);
            }
            else if (channel == 3)
            {
                HPM_IOC->PAD[IOC_PAD_PC08].FUNC_CTL = IOC_PC08_FUNC_CTL_TRGM_P_08;
                trgm0_io_config.invert = 0;
                trgm0_io_config.type = trgm_output_same_as_input;
                trgm0_io_config.input = HPM_TRGM0_INPUT_SRC_TRGM0_P08;
                trgm_output_config(HPM_TRGM0, HPM_TRGM0_OUTPUT_SRC_GPTMR1_CAPT_3, &trgm0_io_config);
            }
            else
            {
                ;
            }
        }
    }
}


void init_trgm_extern_pins()
{
    HPM_IOC->PAD[IOC_PAD_PE04].FUNC_CTL = IOC_PE04_FUNC_CTL_TRGM_P_04;
    HPM_IOC->PAD[IOC_PAD_PE04].PAD_CTL = IOC_PAD_PAD_CTL_SR_SET(1) | IOC_PAD_PAD_CTL_SPD_SET(11) | IOC_PAD_PAD_CTL_DS_SET(3);
    HPM_IOC->PAD[IOC_PAD_PE05].FUNC_CTL = IOC_PE05_FUNC_CTL_TRGM_P_05;
    HPM_IOC->PAD[IOC_PAD_PE05].PAD_CTL = IOC_PAD_PAD_CTL_SR_SET(1) | IOC_PAD_PAD_CTL_SPD_SET(11) | IOC_PAD_PAD_CTL_DS_SET(3);
    
}

void init_py32_pins()
{
    HPM_IOC->PAD[IOC_PAD_PC11].FUNC_CTL = IOC_PC11_FUNC_CTL_GPIO_C_11;
    HPM_IOC->PAD[IOC_PAD_PC11].PAD_CTL = IOC_PAD_PAD_CTL_OD_SET(1) | IOC_PAD_PAD_CTL_PE_SET(1) | IOC_PAD_PAD_CTL_PRS_SET(3);
    HPM_IOC->PAD[IOC_PAD_PC12].FUNC_CTL = IOC_PC12_FUNC_CTL_GPIO_C_12;
    HPM_IOC->PAD[IOC_PAD_PC12].PAD_CTL = IOC_PAD_PAD_CTL_OD_SET(1) | IOC_PAD_PAD_CTL_PE_SET(1) | IOC_PAD_PAD_CTL_PRS_SET(3);
    HPM_IOC->PAD[IOC_PAD_PF29].FUNC_CTL = IOC_PF29_FUNC_CTL_GPIO_F_29;
    HPM_IOC->PAD[IOC_PAD_PF29].PAD_CTL = IOC_PAD_PAD_CTL_OD_SET(1) | IOC_PAD_PAD_CTL_PE_SET(1) | IOC_PAD_PAD_CTL_PRS_SET(3);
}
