/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _BOARD_H_
#define _BOARD_H_

#include "clock_config.h"
#include "fsl_gpio.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
	/*! @brief The board name */
	#define FRDM_MCXN947	0
	#define MCX_N9XX_EVK	1
	#define BOARD_NAME		FRDM_MCXN947

    /*! @brief The UART to use for debug messages. */
    #define BOARD_DEBUG_UART_TYPE       kSerialPort_Uart
    #define BOARD_DEBUG_UART_BASEADDR   (uint32_t) LPUART4
    #define BOARD_DEBUG_UART_INSTANCE   4U
    #define BOARD_DEBUG_UART_CLK_FREQ   12000000U
    #define BOARD_DEBUG_UART_CLK_ATTACH kFRO12M_to_FLEXCOMM4
    #define BOARD_DEBUG_UART_RST        kFC4_RST_SHIFT_RSTn
    #define BOARD_DEBUG_UART_CLKSRC     kCLOCK_FlexComm4
    #define BOARD_UART_IRQ_HANDLER      LP_FLEXCOMM4_IRQHandler
    #define BOARD_UART_IRQ              LP_FLEXCOMM4_IRQn

#ifndef BOARD_NAME
	#error BOARD_NAME must be defined as a valid board, see board.h
#endif
#if (BOARD_NAME == MCX_N9XX_EVK)
    #ifndef BOARD_LED_RED_GPIO
    #define BOARD_LED_RED_GPIO GPIO3
    #endif
    #ifndef BOARD_LED_RED_GPIO_PIN
    #define BOARD_LED_RED_GPIO_PIN 4U
    #endif

    #define BOARD_WOKEN_PORT_CLOCK SYSCON_AHBCLKCTRL0_PORT3_MASK
    #define BOARD_WOKEN_GPIO_CLOCK SYSCON_AHBCLKCTRL0_GPIO3_MASK
    #define BOARD_WOKEN_PORT       PORT3
    #define BOARD_WOKEN_GPIO       BOARD_LED_RED_GPIO
    #define BOARD_WOKEN_GPIO_PIN   BOARD_LED_RED_GPIO_PIN

    /* Board led color mapping */
    #define LOGIC_LED_ON  1U
    #define LOGIC_LED_OFF 0U
#elif (BOARD_NAME == FRDM_MCXN947)
    #ifndef BOARD_LED_RED_GPIO
    #define BOARD_LED_RED_GPIO GPIO0
    #endif
    #ifndef BOARD_LED_RED_GPIO_PIN
    #define BOARD_LED_RED_GPIO_PIN 10U
    #endif

    #define BOARD_WOKEN_PORT_CLOCK SYSCON_AHBCLKCTRL0_PORT0_MASK
    #define BOARD_WOKEN_GPIO_CLOCK SYSCON_AHBCLKCTRL0_GPIO0_MASK
    #define BOARD_WOKEN_PORT       PORT0
    #define BOARD_WOKEN_GPIO       BOARD_LED_RED_GPIO
    #define BOARD_WOKEN_GPIO_PIN   BOARD_LED_RED_GPIO_PIN

    /* Board led color mapping */
    #define LOGIC_LED_ON  0U
    #define LOGIC_LED_OFF 1U
#else
	#error BOARD_NAME must be defined as a valid board, see board.h
#endif /* BOARD_NAME */

    #ifndef BOARD_LED_BLUE_GPIO
    #define BOARD_LED_BLUE_GPIO GPIO3
    #endif
    #ifndef BOARD_LED_BLUE_GPIO_PIN
    #define BOARD_LED_BLUE_GPIO_PIN 3U
    #endif

    #ifndef BOARD_LED_GREEN_GPIO
    #define BOARD_LED_GREEN_GPIO GPIO3
    #endif
    #ifndef BOARD_LED_GREEN_GPIO_PIN
    #define BOARD_LED_GREEN_GPIO_PIN 2U
    #endif

    #ifndef BOARD_SW2_GPIO
    #define BOARD_SW2_GPIO GPIO0
    #endif
    #ifndef BOARD_SW2_GPIO_PIN
    #define BOARD_SW2_GPIO_PIN 29U
    #endif
    #define BOARD_SW2_NAME        "SW2"
    #define BOARD_SW2_IRQ         GPIO00_IRQn
    #define BOARD_SW2_IRQ_HANDLER GPIO00_IRQHandler

    #ifndef BOARD_SW3_GPIO
    #define BOARD_SW3_GPIO GPIO0
    #endif
    #ifndef BOARD_SW3_GPIO_PIN
    #define BOARD_SW3_GPIO_PIN 6U
    #endif
    #define BOARD_SW3_NAME        "SW3"
    #define BOARD_SW3_IRQ         GPIO00_IRQn
    #define BOARD_SW3_IRQ_HANDLER GPIO00_IRQHandler

    #ifndef BOARD_SW4_GPIO
    #define BOARD_SW4_GPIO GPIO1
    #endif
    #ifndef BOARD_SW4_GPIO_PIN
    #define BOARD_SW4_GPIO_PIN 3U
    #endif
    #define BOARD_SW4_NAME        "SW4"
    #define BOARD_SW4_IRQ         GPIO10_IRQn
    #define BOARD_SW4_IRQ_HANDLER GPIO10_IRQHandler

#ifndef BOARD_DEBUG_UART_BAUDRATE
#define BOARD_DEBUG_UART_BAUDRATE 115200U
#endif /* BOARD_DEBUG_UART_BAUDRATE */

#define LED_RED_INIT(output)                                           \
    GPIO_PinWrite(BOARD_LED_RED_GPIO, BOARD_LED_RED_GPIO_PIN, output); \
    BOARD_LED_RED_GPIO->PDDR |= (1U << BOARD_LED_RED_GPIO_PIN)                         /*!< Enable target LED_RED */
#define LED_RED_ON()  GPIO_PortSet(BOARD_LED_RED_GPIO, 1U << BOARD_LED_RED_GPIO_PIN)   /*!< Turn on target LED_RED */
#define LED_RED_OFF() GPIO_PortClear(BOARD_LED_RED_GPIO, 1U << BOARD_LED_RED_GPIO_PIN) /*!< Turn off target LED_RED */
#define LED_RED_TOGGLE() \
    GPIO_PortToggle(BOARD_LED_RED_GPIO, 1U << BOARD_LED_RED_GPIO_PIN) /*!< Toggle on target LED_RED */

#define LED_BLUE_INIT(output)                                            \
    GPIO_PinWrite(BOARD_LED_BLUE_GPIO, BOARD_LED_BLUE_GPIO_PIN, output); \
    BOARD_LED_BLUE_GPIO->PDDR |= (1U << BOARD_LED_BLUE_GPIO_PIN)                       /*!< Enable target LED_BLUE */
#define LED_BLUE_ON() GPIO_PortSet(BOARD_LED_BLUE_GPIO, 1U << BOARD_LED_BLUE_GPIO_PIN) /*!< Turn on target LED_BLUE */
#define LED_BLUE_OFF() \
    GPIO_PortClear(BOARD_LED_BLUE_GPIO, 1U << BOARD_LED_BLUE_GPIO_PIN) /*!< Turn off target LED_BLUE */
#define LED_BLUE_TOGGLE() \
    GPIO_PortToggle(BOARD_LED_BLUE_GPIO, 1U << BOARD_LED_BLUE_GPIO_PIN) /*!< Toggle on target LED_BLUE */

#define LED_GREEN_INIT(output)                                             \
    GPIO_PinWrite(BOARD_LED_GREEN_GPIO, BOARD_LED_GREEN_GPIO_PIN, output); \
    BOARD_LED_GREEN_GPIO->PDDR |= (1U << BOARD_LED_GREEN_GPIO_PIN) /*!< Enable target LED_GREEN */
#define LED_GREEN_ON() \
    GPIO_PortSet(BOARD_LED_GREEN_GPIO, 1U << BOARD_LED_GREEN_GPIO_PIN) /*!< Turn on target LED_GREEN */
#define LED_GREEN_OFF() \
    GPIO_PortClear(BOARD_LED_GREEN_GPIO, 1U << BOARD_LED_GREEN_GPIO_PIN) /*!< Turn off target LED_GREEN */
#define LED_GREEN_TOGGLE() \
    GPIO_PortToggle(BOARD_LED_GREEN_GPIO, 1U << BOARD_LED_GREEN_GPIO_PIN) /*!< Toggle on target LED_GREEN */

#define BOARD_WOKEN_PIN_INIT   LED_RED_INIT

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/*******************************************************************************
 * API
 ******************************************************************************/

void BOARD_InitDebugConsole(void);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* _BOARD_H_ */
