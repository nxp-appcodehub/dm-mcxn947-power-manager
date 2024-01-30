/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_pm_board.h"

#ifndef _FSL_PM_BOARD_CONFIG_H_
#define _FSL_PM_BOARD_CONFIG_H_

#define FSL_PM_SUPPORT_NOTIFICATION          (1U)
#define FSL_PM_SUPPORT_WAKEUP_SOURCE_MANAGER (1U)
#define FSL_PM_SUPPORT_LP_TIMER_CONTROLLER   (1U)

#define PM_CONSTRAINT_COUNT (kResc_Max_Num)
#define PM_LP_STATE_COUNT   (6U)

#define PM_RESC_GROUP_ARRAY_SIZE (PM_CONSTRAINT_COUNT /  8 + 1)
#define PM_RESC_MASK_ARRAY_SIZE  (PM_CONSTRAINT_COUNT / 32 + 1)

/*!
 * inputType: 4 bit width. Used to distinguish WUU input source type, 0 for external pins, 1 for internal modules.
 * inputId:   4 bit width. The id of WUU input.
 * irqn:      8 bit width. The irq number of wuu input.
 * misc:      Misc usage, if input type is external pin, this field is used to store edge detection type.
 *            01b -- External input pin enabled with rising edge detection.
 *            10b -- External input pin enabled with falling edge detection.
 *            11b -- External input pin enabled with any change detection.
 */
#define PM_ENCODE_WAKEUP_SOURCE_ID(inputType, inputId, irqn, misc)                        \
    (((inputType)&0xFUL) | (((inputId) << 4UL) & 0xF0UL) | (((irqn) << 8UL) & 0xFF00UL) | \
     ((misc << 16UL) & 0xFFFF0000UL))

#define PM_DECODE_WAKEUP_SOURCE_ID(wsId)  \
    inputType = ((wsId)&0xFUL);           \
    inputId   = ((wsId)&0xF0UL) >> 4UL;   \
    irqn      = ((wsId)&0xFF00UL) >> 8UL; \
    misc      = ((wsId)&0xFFFF0000UL) >> 16UL

#endif /* _FSL_PM_BOARD_CONFIG_H_ */
