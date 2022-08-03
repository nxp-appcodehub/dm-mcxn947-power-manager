/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _TIMERS_H_
#define _TIMERS_H_

#include "fsl_common.h"
#include "fsl_pm_core.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define APP_DEFAULT_PM_DURATION  2     /* seconds in low-power mode */
#define APP_PM_TIMER_FREQ       16384   /* PM timer ticks per second */
#define APP_LPTMR   LPTMR0

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
extern AT_ALWAYS_ON_DATA(pm_wakeup_source_t g_lptmr0WakeupSource);

void APP_Lptmr0WakeupService(void);
void APP_StartLptmr(uint64_t timeOutTickes);
void APP_StopLptmr(void);

#endif //_TIMERS_H_
