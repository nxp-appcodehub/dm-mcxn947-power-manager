/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "power_manager.h"
#include "analog.h"
#include "power.h"
#include "timers.h"
#include "fsl_debug_console.h"
#include "board.h"

#include "fsl_pm_board.h"

#include "fsl_cmc.h"

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

extern AT_ALWAYS_ON_DATA(pm_handle_t g_pmHndle);

/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief Placeholder for custom use-case.  See lab guide for details.
 *        This function needs to be completed before it will build
 */
void APP_LabCustomUseCase(void)
{
#if 1   /* Code enabled for lab, see lab guide */
    uint8_t ch;
    uint32_t duration = 2;

    /* Enable the System SRAM arrays used by application */
    CMC_PowerOnSRAMAllMode(CMC0, ENABLED_ALL_SRAM);

    /* Enable peripherals in Active mode */
    App_Enable_CMP(CMP0);
    App_Enable_CMP(CMP1);
    App_Enable_CMP(CMP2);
    
    /* Set the PM resource constraints.
     * Note: this could be done with a single function call,
     *       but uses multiple calls for clarity */
    PM_SetConstraints(PM_LP_STATE_NO_CONSTRAINT, PM_RESC_SRAM_ALL_NUM, 
                                PM_RESC_SRAM_ALL(PM_RESOURCE_FULL_ON));
    PM_SetConstraints(PM_LP_STATE_NO_CONSTRAINT, 1U, PM_RESC_CMP0_ON);
    PM_SetConstraints(PM_LP_STATE_NO_CONSTRAINT, 1U, PM_RESC_CMP1_ON);
    PM_SetConstraints(PM_LP_STATE_NO_CONSTRAINT, 1U, PM_RESC_CMP2_ON);

    /* Print configuration before entering power mode */
    FORCE_RESOURCE_UPDATE(g_pmHndle);
    APP_PrintPowerModeToEnter(DURATION_SECONDS(duration));
    PRINTF("SRAMDIS     =0x%x\r\n", CMC0->SRAMDIS[0]);
    PRINTF("SRAMRET     =0x%x\r\n", CMC0->SRAMRET[0]);
    PRINTF("ACTIVE_CFG1 =0x%x\r\n", SPC0->ACTIVE_CFG1);
    PRINTF("LP_CFG1     =0x%x\r\n", SPC0->LP_CFG1);

    while(1)
    {
        PRINTF("Press any key to enter low-power mode\r\n");
        do
        {
            ch = GETCHAR();
        } while(ch == 0xFFU); /* addresses issue after wake-up when 1st char is 0xFF */

        /* Use PM component to enter low-power mode */
        PM_EnterLowPower(DURATION_SECONDS(duration));

        PRINTF("Woke from low-power mode\n\r"); 
    }
#endif
}
