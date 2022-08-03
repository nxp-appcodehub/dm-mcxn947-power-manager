/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_pm_core.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* helper macros for SRAM constraints */
#define PM_RESC_SRAM_HALF_NUM 8U
#define PM_RESC_SRAM_HALF(mode) PM_ENCODE_RESC(mode, kResc_SRAM_RAMX0_32K),     \
                                PM_ENCODE_RESC(mode, kResc_SRAM_RAMX1_32K),     \
                                PM_ENCODE_RESC(mode, kResc_SRAM_RAMX2_32K),     \
                                PM_ENCODE_RESC(mode, kResc_SRAM_RAMB0_32K),     \
                                PM_ENCODE_RESC(mode, kResc_SRAM_RAMC0_32K),     \
                                PM_ENCODE_RESC(mode, kResc_SRAM_RAMC1_32K),     \
                                PM_ENCODE_RESC(mode, kResc_SRAM_RAMD0_32K),     \
                                PM_ENCODE_RESC(mode, kResc_SRAM_RAMD1_32K)
#define ENABLED_HALF_SRAM (kCMC_RAMX0 | kCMC_RAMX1 | kCMC_RAMX2 | kCMC_RAMB |   \
                           kCMC_RAMC0 | kCMC_RAMC1 | kCMC_RAMD0 | kCMC_RAMD1)

#define PM_RESC_SRAM_ALL_NUM 7U + PM_RESC_SRAM_HALF_NUM
#define PM_RESC_SRAM_ALL(mode) PM_RESC_SRAM_HALF(mode),                         \
                                PM_ENCODE_RESC(mode, kResc_SRAM_RAME0_32K),     \
                                PM_ENCODE_RESC(mode, kResc_SRAM_RAME1_32K),     \
                                PM_ENCODE_RESC(mode, kResc_SRAM_RAMF0_32K),     \
                                PM_ENCODE_RESC(mode, kResc_SRAM_RAMF1_32K),     \
                                PM_ENCODE_RESC(mode, kResc_SRAM_RAMG01_32K),    \
                                PM_ENCODE_RESC(mode, kResc_SRAM_RAMG23_32K),    \
                                PM_ENCODE_RESC(mode, kResc_SRAM_RAMH01_32K)
#define ENABLED_ALL_SRAM (kCMC_RAME0 | kCMC_RAME1 | kCMC_RAMF0 | kCMC_RAMF1 |      \
                          kCMC_RAMG0_RAMG1 | kCMC_RAMG2_RAMG3 | kCMC_RAMH0_RAMH1 | \
                          ENABLED_HALF_SRAM)

#define PM_RELEASE_ALL_SRAM_CONSTRAINTS                                                                             \
    PM_ReleaseConstraints(PM_LP_STATE_NO_CONSTRAINT, PM_RESC_SRAM_ALL_NUM, PM_RESC_SRAM_ALL(PM_RESOURCE_FULL_ON));

#define DURATION_SECONDS(x) (x * APP_PM_TIMER_FREQ)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

void EnableResources(pm_resc_mask_t *pSoftRescMask, pm_resc_group_t *pSysRescGroup);

/* Calls to this API are not required in the application, because the Power
 * Manager framework will call this API entering a power mode.  However, this
 * application forces the update after menu selections to show the register 
 * changes.
 */
#define FORCE_RESOURCE_UPDATE(handle) EnableResources(&(handle.resConstraintMask), &(handle.sysRescGroup))