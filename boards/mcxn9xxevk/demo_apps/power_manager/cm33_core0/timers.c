/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "timers.h"

#include "fsl_lptmr.h"
#include "fsl_clock.h"
#include "fsl_pm_core.h"

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

AT_ALWAYS_ON_DATA(pm_wakeup_source_t g_lptmr0WakeupSource);

/*******************************************************************************
 * Code
 ******************************************************************************/

void LPTMR0_IRQHandler(void)
{
    PM_TriggerWakeSourceService(&g_lptmr0WakeupSource);
}

void APP_StartLptmr(uint64_t timeOutTickes)
{
    const lptmr_config_t DEMO_LPTMR_config = {.timerMode            = kLPTMR_TimerModeTimeCounter,
                                              .pinSelect            = kLPTMR_PinSelectInput_0,
                                              .pinPolarity          = kLPTMR_PinPolarityActiveHigh,
                                              .enableFreeRunning    = false,
                                              .bypassPrescaler      = true,
                                              .prescalerClockSource = kLPTMR_PrescalerClock_1,  /* clk_16k[1] */
                                              .value                = kLPTMR_Prescale_Glitch_0};

    CLOCK_SetupClk16KClocking(kCLOCK_Clk16KToVsys);     /* Enable clk_16k[1] for LPTMR0 */
    LPTMR_Init(APP_LPTMR, &DEMO_LPTMR_config);
    LPTMR_SetTimerPeriod(APP_LPTMR, (uint32_t)timeOutTickes);
    LPTMR_EnableInterrupts(APP_LPTMR, kLPTMR_TimerInterruptEnable);
    LPTMR_StartTimer(APP_LPTMR);
}

void APP_StopLptmr(void)
{
    LPTMR_StopTimer(APP_LPTMR);
}

void APP_Lptmr0WakeupService(void)
{
    if (kLPTMR_TimerInterruptEnable & LPTMR_GetEnabledInterrupts(APP_LPTMR))
    {
        LPTMR_DisableInterrupts(APP_LPTMR, kLPTMR_TimerInterruptEnable);
        LPTMR_ClearStatusFlags(APP_LPTMR, kLPTMR_TimerCompareFlag);
        LPTMR_StopTimer(APP_LPTMR);
    }
}
