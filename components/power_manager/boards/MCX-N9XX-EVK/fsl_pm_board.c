/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <setjmp.h>

#include "fsl_pm_core.h"
#include "fsl_pm_board.h"
#include "fsl_pm_board_config.h"

#include "fsl_cmc.h"
#include "fsl_spc.h"
#include "fsl_wuu.h"
#include "fsl_clock.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Macros
 ******************************************************************************/
#define WUU_PIN_FLAG_MASK WUU_PF_WUF7_MASK

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
typedef struct _resource_recode resource_recode_t;

static void EnterLowPowerMode(uint8_t stateIndex, pm_resc_mask_t *pSoftRescMask, pm_resc_group_t *pSysRescGroup);
static void CleanExitLowPowerMode(void);
static void SetSRAMOperateMode  (uint8_t operateMode, resource_recode_t *pResourceRecode);
static void SetAnalogOperateMode(uint8_t operateMode, resource_recode_t *pResourceRecode);
void EnableResources(pm_resc_mask_t *pSoftRescMask, pm_resc_group_t *pSysRescGroup);

#if (defined(FSL_PM_SUPPORT_WAKEUP_SOURCE_MANAGER) && FSL_PM_SUPPORT_WAKEUP_SOURCE_MANAGER)
    static status_t ManageWakeupSource(pm_wakeup_source_t *ws, bool enable);
    static bool IsWakeupSource(pm_wakeup_source_t *ws);
#endif /* FSL_PM_SUPPORT_WAKEUP_SOURCE_MANAGER */

/*******************************************************************************
 * Variables
 ******************************************************************************/
AT_ALWAYS_ON_DATA_INIT(cmc_power_domain_config_t g_mainWakePDConfig) = {
    .clock_mode = kCMC_GateAllSystemClocksEnterLowPowerMode,
    /* we chose to put the main and wake domain in retention (deep sleep) by default if no constraints are set */
    .main_domain = kCMC_DeepSleepMode,
    .wake_domain = kCMC_DeepSleepMode,
};

/*!
 *  |----------------|-------------|------------|------------|------------|------------|------------|
 *  |                |             |            |       Power Down        |    Deep    |            |
 *  | Module         |    Sleep    | Deep Sleep | WAKE DSlp. | WAKE PDown | Power Down |    VBAT    |
 *  |----------------|-------------|------------|------------|------------|------------|------------|
 *  | CM33           |    Static   |   Static   |   Static   |   Static   |     Off    |    Off     |
 *  | Bus/System Clk |     On      |   Static   |   Static   |   Static   |     Off    |    Off     |
 *  | VDD_CORE_MAIN  |     On      |     LP     |   Static   |   Static   |     Off    |    Off     |
 *  | VDD_CORE_WAKE  |     On      |     LP     |     LP     |   Static   |     Off    |    Off     |
 *  | RAMA SRAM      | On/Stat/Off | Static/Off | Static/Off | Static/Off | Static/Off | Static/Off |
 *  | System SRAMs   |    On/Off   |   Static   | Static/Off | Static/Off |     Off    |    Off     |
 *  | Flash          |    On/LP    |     LP     |    Off     |    Off     |     Off    |    Off     |
 *  | DCDC/LDO_CORE  |    On/LP    |   On/LP    |   On/LP    |   On/LP    |     Off    |    Off     |
 *  | LDO/VDD_SYS    |    On/LP    |   On/LP    |   On/LP    |   On/LP    |    On/LP   |    Off     |
 *  | FRO_144M       |  Optional   |    Off     |    Off     |    Off     |     Off    |    Off     |
 *  | FRO_12M        |  Optional   |  Optional  |  Optional  |    Off     |     Off    |    Off     |
 *  | FRO_16K        |  Optional   |  Optional  |  Optional  |  Optional  |  Optional  |  Optional  |
 *  | OSC_RTC        |  Optional   |  Optional  |  Optional  |  Optional  |  Optional  |  Optional  |
 *  | OSC_SYS        |  Optional   |    Off     |    Off     |    Off     |     Off    |    Off     |
 *  | PLL0/1         |  Optional   |    Off     |    Off     |    Off     |     Off    |    Off     |
 *  | ADC            |  Optional   |    Off     |    Off     |    Off     |     Off    |    Off     |
 *  | CMP0/1         |  Optional   |  Optional  |  Optional  |  Optional  |  Optional  |    Off     |
 *  | CMP2           |  Optional   |  Optional  |  Optional  |    Off     |     Off    |    Off     |
 *  | DAC0/1         |  Optional   |  Optional  |   Static   |   Static   |     Off    |    Off     |
 *  | DAC2           |  Optional   |  Optional  |    Off     |    Off     |     Off    |    Off     |
 *  | OPAMP0/1/2     |  Optional   |  Optional  |    Off     |    Off     |     Off    |    Off     |
 *  | VREF           |  Optional   |  Optional  |   Static   |   Static   |     Off    |    Off     |
 *  | SINC Filter    |  Optional   |  Optional  |   Static   |   Static   |     Off    |    Off     |
 *  | USB 3V Detect  |  Optional   |  Optional  |  Optional  |  Optional  |     Off    |    Off     |
 *  | CORE Bandgap   |  On/LP/Off  |  On/LP/Off |  On/LP/Off |  On/LP/Off |     Off    |    Off     |
 *  | CORE GlitchDet |  Optional   |  Optional  |  Optional  |  Optional  |     Off    |    Off     |
 *  | VDD_CORE_HVD   |  Optional   |  Optional  |  Optional  |  Optional  |     Off    |    Off     |
 *  | VDD_CORE_LVD   |  Optional   |  Optional  |  Optional  |  Optional  |     Off    |    Off     |
 *  | VDD_SYS_HVD    |  Optional   |  Optional  |  Optional  |  Optional  |  Optional  |    Off     |
 *  | VDD_SYS_LVD    |  Optional   |  Optional  |  Optional  |  Optional  |  Optional  |    Off     |
 *  | VDD_IO_LVD     |  Optional   |  Optional  |  Optional  |  Optional  |     Off    |    Off     |
 *  | VDD_IO_HVD     |  Optional   |  Optional  |  Optional  |  Optional  |     Off    |    Off     |
 *  | VDD_IO_Detect  |  Optional   |  Optional  |  Optional  |  Optional  |     Off    |    Off     |
 *  | VBAT Bandgap   |  Optional   |  Optional  |  Optional  |  Optional  |  Optional  |  Optional  |
 *  |----------------|-------------|------------|------------|------------|------------|------------|
 */

// ToDo Update all Exit Latencies to match datasheet

const pm_device_option_t g_devicePMOption = {
    .states =
        {
            /* Sleep */
            {
                .exitLatency = 14U, /* 14 us */
                .fixConstraintsMask =
                    {
                        .rescMask[0U] = ~(PM_MASK_RESC_LOWEST_SLEEP0),
                        .rescMask[1U] = ~(PM_MASK_RESC_LOWEST_SLEEP1),
                    },
                .varConstraintsMask =
                    {
                        .rescMask[0U] = PM_MASK_RESC_LOWEST_SLEEP0 & PM_MASK_RESC_NOT_VAR0,
                        .rescMask[1U] = PM_MASK_RESC_LOWEST_SLEEP1 & PM_MASK_RESC_NOT_VAR1,
                    },
            },
            /* Deep Sleep */
            {
                .exitLatency = 14U, /* 14 us */
                .fixConstraintsMask =
                    {
                        .rescMask[0U] = ~(PM_MASK_RESC_LOWEST_DEEP_SLEEP0),
                        .rescMask[1U] = ~(PM_MASK_RESC_LOWEST_DEEP_SLEEP1),
                    },
                .varConstraintsMask =
                    {
                        .rescMask[0U] = PM_MASK_RESC_LOWEST_DEEP_SLEEP0 & PM_MASK_RESC_NOT_VAR0,
                        .rescMask[1U] = PM_MASK_RESC_LOWEST_DEEP_SLEEP1 & PM_MASK_RESC_NOT_VAR1,
                    },
            },
            /* Power Down with CORE_WAKE in Deep Sleep*/
            {
                .exitLatency = 600U, /* 600 us */
                .fixConstraintsMask =
                    {
                        .rescMask[0U] = ~(PM_MASK_RESC_LOWEST_PDDS0),
                        .rescMask[1U] = ~(PM_MASK_RESC_LOWEST_PDDS1),
                    },
                .varConstraintsMask =
                    {
                        .rescMask[0U] = PM_MASK_RESC_LOWEST_PDDS0 & PM_MASK_RESC_NOT_VAR0,
                        .rescMask[1U] = PM_MASK_RESC_LOWEST_PDDS1 & PM_MASK_RESC_NOT_VAR1,
                    },
            },
            /* Power Down with CORE_WAKE in Power Down*/
            {
                .exitLatency = 600U, /* 600 us */
                .fixConstraintsMask =
                    {
                        .rescMask[0U] = ~(PM_MASK_RESC_LOWEST_PDPD0),
                        .rescMask[1U] = ~(PM_MASK_RESC_LOWEST_PDPD1),
                    },
                .varConstraintsMask =
                    {
                        .rescMask[0U] = PM_MASK_RESC_LOWEST_PDPD0 & PM_MASK_RESC_NOT_VAR0,
                        .rescMask[1U] = PM_MASK_RESC_LOWEST_PDPD1 & PM_MASK_RESC_NOT_VAR1,
                    },
            },
            /* Deep Power Down */
            {
                .fixConstraintsMask =
                    {
                        .rescMask[0U] = ~(PM_MASK_RESC_LOWEST_DPD0),
                        .rescMask[1U] = ~(PM_MASK_RESC_LOWEST_DPD1),
                    },
                .varConstraintsMask =
                    {
                        .rescMask[0U] = PM_MASK_RESC_LOWEST_DPD0 & PM_MASK_RESC_NOT_VAR0,
                        .rescMask[1U] = PM_MASK_RESC_LOWEST_DPD1 & PM_MASK_RESC_NOT_VAR1,
                    },
            },
            /* VBAT */
            {
                .fixConstraintsMask =
                    {
                        .rescMask[0U] = ~(PM_MASK_RESC_LOWEST_VBAT0),
                        .rescMask[1U] = ~(PM_MASK_RESC_LOWEST_VBAT1),
                    },
                .varConstraintsMask =
                    {
                        .rescMask[0U] = PM_MASK_RESC_LOWEST_VBAT0 & PM_MASK_RESC_NOT_VAR0,
                        .rescMask[1U] = PM_MASK_RESC_LOWEST_VBAT1 & PM_MASK_RESC_NOT_VAR1,
                    },
            },
        },
    .stateCount = PM_LP_STATE_COUNT,
    .enter      = EnterLowPowerMode,
    .clean      = CleanExitLowPowerMode,

#if (defined(FSL_PM_SUPPORT_WAKEUP_SOURCE_MANAGER) && FSL_PM_SUPPORT_WAKEUP_SOURCE_MANAGER)
    .manageWakeupSource = ManageWakeupSource,
    .isWakeupSource     = IsWakeupSource,
#endif /* FSL_PM_SUPPORT_WAKEUP_SOURCE_MANAGER */
};

struct _resource_recode
{
    uint8_t currentOperateMode : 4U;
    void (*resourceConfigFunc)(uint8_t operateMode, resource_recode_t *pResourceRecode);
};

// Todo Remove all references to this placeholder, and remove the placeholder
static void PlaceHolderOperateMode(uint8_t operateMode, resource_recode_t *pResourceRecode)
{
}

/* Struct below stored in Always-On memory, to retain in all power modes*/
AT_ALWAYS_ON_DATA_INIT(resource_recode_t resourceDB[PM_CONSTRAINT_COUNT]) = {
    [kResc_SRAM_RAMA0_8K]   = {0U, SetSRAMOperateMode},
    [kResc_SRAM_RAMA1_8K]   = {0U, SetSRAMOperateMode},
    [kResc_SRAM_RAMA2_8K]   = {0U, SetSRAMOperateMode},
    [kResc_SRAM_RAMA3_8K]   = {0U, SetSRAMOperateMode},
    [kResc_SRAM_RAMX0_32K]  = {0U, SetSRAMOperateMode},
    [kResc_SRAM_RAMX1_32K]  = {0U, SetSRAMOperateMode},
    [kResc_SRAM_RAMX2_32K]  = {0U, SetSRAMOperateMode},
    [kResc_SRAM_RAMB0_32K]  = {0U, SetSRAMOperateMode},
    [kResc_SRAM_RAMC0_32K]  = {0U, SetSRAMOperateMode},
    [kResc_SRAM_RAMC1_32K]  = {0U, SetSRAMOperateMode},
    [kResc_SRAM_RAMD0_32K]  = {0U, SetSRAMOperateMode},
    [kResc_SRAM_RAMD1_32K]  = {0U, SetSRAMOperateMode},
    [kResc_SRAM_RAME0_32K]  = {0U, SetSRAMOperateMode},
    [kResc_SRAM_RAME1_32K]  = {0U, SetSRAMOperateMode},
    [kResc_SRAM_RAMF0_32K]  = {0U, SetSRAMOperateMode},
    [kResc_SRAM_RAMF1_32K]  = {0U, SetSRAMOperateMode},
    [kResc_SRAM_RAMG01_32K] = {0U, SetSRAMOperateMode},
    [kResc_SRAM_RAMG23_32K] = {0U, SetSRAMOperateMode},
    [kResc_SRAM_RAMH01_32K] = {0U, SetSRAMOperateMode},
    [kResc_Flash]           = {0U, PlaceHolderOperateMode},
    [kResc_DCDC_CORE]       = {0U, PlaceHolderOperateMode},
    [kResc_LDO_CORE]        = {0U, PlaceHolderOperateMode},
    [kResc_LDO_SYS]         = {0U, PlaceHolderOperateMode},
    [kResc_FRO_144M]        = {0U, PlaceHolderOperateMode},
    [kResc_FRO_12M]         = {0U, PlaceHolderOperateMode},
    [kResc_FRO_16K]         = {0U, PlaceHolderOperateMode},
    [kResc_OSC_RTC]         = {0U, PlaceHolderOperateMode},
    [kResc_OSC_SYS]         = {0U, PlaceHolderOperateMode},
    [kResc_PLL0]            = {0U, PlaceHolderOperateMode},
    [kResc_CMP0]            = {0U, SetAnalogOperateMode},
    [kResc_CMP1]            = {0U, SetAnalogOperateMode},
    [kResc_CMP2]            = {0U, SetAnalogOperateMode},
    [kResc_DAC0]            = {0U, SetAnalogOperateMode},
    [kResc_DAC1]            = {0U, SetAnalogOperateMode},
    [kResc_DAC2]            = {0U, SetAnalogOperateMode},
    [kResc_OPAMP0]          = {0U, SetAnalogOperateMode},
    [kResc_OPAMP1]          = {0U, SetAnalogOperateMode},
    [kResc_OPAMP2]          = {0U, SetAnalogOperateMode},
    [kResc_VREF]            = {0U, SetAnalogOperateMode},
    [kResc_USB3V_DET]       = {0U, SetAnalogOperateMode},
    [kResc_ADC]             = {0U, PlaceHolderOperateMode},
    [kResc_SINC]            = {0U, PlaceHolderOperateMode},
    [kResc_BG_CORE]         = {0U, PlaceHolderOperateMode},
    [kResc_BG_VBAT]         = {0U, PlaceHolderOperateMode},
    [kResc_GDET]            = {0U, PlaceHolderOperateMode},
    [kResc_HVD_CORE]        = {0U, PlaceHolderOperateMode},
    [kResc_HVD_SYS]         = {0U, PlaceHolderOperateMode},
    [kResc_HVD_IO]          = {0U, PlaceHolderOperateMode},
    [kResc_LVD_CORE]        = {0U, PlaceHolderOperateMode},
    [kResc_LVD_SYS]         = {0U, PlaceHolderOperateMode},
    [kResc_LVD_IO]          = {0U, PlaceHolderOperateMode},
    [kResc_IO_Det]          = {0U, PlaceHolderOperateMode},
};

/*******************************************************************************
 * Code
 ******************************************************************************/

static uint8_t FindOperateMode(uint32_t rescIndex, pm_resc_group_t *pSysRescGroup)
{
    assert(pSysRescGroup);
    uint8_t u8Tmp = PM_RESC_GROUP(pSysRescGroup, rescIndex);

    u8Tmp |= (u8Tmp >> 1U);
    u8Tmp |= (u8Tmp >> 2U);

    return ((u8Tmp + 1U) >> 1U);
}

void EnableResources(pm_resc_mask_t *pSoftRescMask, pm_resc_group_t *pSysRescGroup)
{
    assert(pSoftRescMask);
    assert(pSysRescGroup);

    resc_name_t i;
    uint8_t opMode = PM_RESOURCE_OFF;

    for (i = 0UL; i < PM_CONSTRAINT_COUNT; i++)
    {
        if(resourceDB[i].resourceConfigFunc != NULL)
        {
            opMode = FindOperateMode(i, pSysRescGroup);
            if (PM_RESC_MASK(pSoftRescMask, i) == 0)
            {
                opMode = PM_RESOURCE_OFF;
            }

            if (opMode != resourceDB[i].currentOperateMode)
            {
                resourceDB[i].resourceConfigFunc(opMode, &resourceDB[i]);
            }
        }
    }
}

static void EnterLowPowerMode(uint8_t stateIndex, pm_resc_mask_t *pSoftRescMask, pm_resc_group_t *pSysRescGroup)
{
    assert(pSoftRescMask);
    assert(pSysRescGroup);

    switch (stateIndex)
    {
        case PM_LP_STATE_SLEEP:
        {
            g_mainWakePDConfig.clock_mode    = kCMC_CoreClockGated;
            g_mainWakePDConfig.main_domain   = kCMC_ActiveOrSleepMode;
            g_mainWakePDConfig.wake_domain   = kCMC_ActiveOrSleepMode;
            break;
        }
        case PM_LP_STATE_DEEP_SLEEP:
        {
            g_mainWakePDConfig.clock_mode    = kCMC_GateAllSystemClocksEnterLowPowerMode;
            g_mainWakePDConfig.main_domain   = kCMC_DeepSleepMode;
            g_mainWakePDConfig.wake_domain   = kCMC_DeepSleepMode;
            break;
        }
        case PM_LP_STATE_POWER_DOWN_WAKE_DS:
        {
            g_mainWakePDConfig.clock_mode    = kCMC_GateAllSystemClocksEnterLowPowerMode;
            g_mainWakePDConfig.main_domain   = kCMC_PowerDownMode;
            g_mainWakePDConfig.wake_domain   = kCMC_DeepSleepMode;
            break;
        }
        case PM_LP_STATE_POWER_DOWN_WAKE_PD:
        {
            g_mainWakePDConfig.clock_mode    = kCMC_GateAllSystemClocksEnterLowPowerMode;
            g_mainWakePDConfig.main_domain   = kCMC_PowerDownMode;
            g_mainWakePDConfig.wake_domain   = kCMC_PowerDownMode;
            break;
        }
        case PM_LP_STATE_DEEP_POWER_DOWN:
        case PM_LP_STATE_VBAT:
        {
            g_mainWakePDConfig.clock_mode    = kCMC_GateAllSystemClocksEnterLowPowerMode;
            g_mainWakePDConfig.main_domain   = kCMC_DeepPowerDown;
            g_mainWakePDConfig.wake_domain   = kCMC_DeepPowerDown;
            break;
        }
        default:
        {
            assert(0);
            break;
        }
    }

    EnableResources(pSoftRescMask, pSysRescGroup);
    CMC_EnterLowPowerMode(CMC0, &g_mainWakePDConfig);
}

static void CleanExitLowPowerMode(void)
{
    if ((SPC_CheckPowerDomainLowPowerRequest(SPC0, kSPC_PowerDomain0) == true) &&
        (SPC_GetPowerDomainLowPowerMode(SPC0, kSPC_PowerDomain0) >= kSPC_DeepPowerDownWithSysClockOff))
    {
        /* We need to release IO isolation when exiting from Power Down mode
         * This is done here after all peripherals have been reinitialized, to
         * avoid any glitch on IOs */
        SPC_ClearPeriphIOIsolationFlag(SPC0);
    }

    /* Clear SPC LP request status for next low power phase
     * The clean up API has been added because the modules registered to notification
     * could need to read some status registers
     * so we don't want a specific module to handle the clear of those status
     * as it could lead to some modules missing the status information
     * (depending on the notification order).
     * So this API is meant to clear everything needed for the platform once
     * every module got notified. */
    SPC_ClearPowerDomainLowPowerRequestFlag(SPC0, kSPC_PowerDomain0);
    SPC_ClearPowerDomainLowPowerRequestFlag(SPC0, kSPC_PowerDomain1);
}

static void SetSRAMOperateMode(uint8_t operateMode, resource_recode_t *pResourceRecode)
{
    assert(pResourceRecode);

    uint8_t sramId;
    uint32_t sramret;

    /* Get the SRAM resc number */
    sramId = pResourceRecode - &(resourceDB[0]);
    assert(sramId >= kResc_SRAM_RAMA0_8K);
    assert(sramId <= kResc_SRAM_RAMH01_32K);
    // ToDo add support for dedicated peripheral SRAMs

    if(sramId <= kResc_SRAM_RAMA3_8K)
    {
        //ToDo manage RAMA resources
    } else
    {
        /* Calculate the bit shift value for SRAMDIS/SRAMRET registers */
        sramId -= kResc_SRAM_RAMX0_32K;

        sramret = CMC0->SRAMRET[0];

        switch (operateMode)
        {
            case PM_RESOURCE_FULL_ON:   /* RETAINED */
            {
                CMC_PowerOnSRAMLowPowerOnly(CMC0, (1U << sramId) | ~sramret);
                break;
            }

            case PM_RESOURCE_OFF:       /* Not retained below Deep Sleep mode */
            {
                CMC_PowerOffSRAMLowPowerOnly_to_add(CMC0, (1U << sramId) | sramret);
                break;
            }

            default:
            {
                assert(0U);
                break;
            }
        }
    }

    pResourceRecode->currentOperateMode = operateMode;
}

static void SetAnalogOperateMode(uint8_t operateMode, resource_recode_t *pResourceRecode)
{
    assert(pResourceRecode);

    uint8_t analogId;
    uint32_t mask;

    /* Get the SRAM resc number */
    analogId = pResourceRecode - &(resourceDB[0]);
    assert(analogId >= kResc_VREF);
    assert(analogId <= kResc_CMP2);

    switch (analogId)
    {
        case kResc_VREF:
            mask = kSPC_controlVref;
            break;

        case kResc_USB3V_DET:
            mask = kSPC_controlUsb3vDet;
            break;

        case kResc_DAC0:
            mask = kSPC_controlDac0;
            break;

        case kResc_DAC1:
            mask = kSPC_controlDac1;
            break;

        case kResc_DAC2:
            mask = kSPC_controlDac2;
            break;

        case kResc_OPAMP0:
            mask = kSPC_controlOpamp0;
            break;

        case kResc_OPAMP1:
            mask = kSPC_controlOpamp1;
            break;

        case kResc_OPAMP2:
            mask = kSPC_controlOpamp2;
            break;

        case kResc_CMP0:
            mask = (kSPC_controlCmp0 | kSPC_controlCmp0Dac);
            break;

        case kResc_CMP1:
            mask = (kSPC_controlCmp1 | kSPC_controlCmp1Dac);
            break;

        case kResc_CMP2:
            mask = (kSPC_controlCmp2 | kSPC_controlCmp2Dac);
            break;

        default:
            assert(0U);
            break;
    }

    if(operateMode == PM_RESOURCE_FULL_ON)
    {
        SPC_EnableLowPowerModeAnalogModules(SPC0, mask);
    } else
    {
        SPC_DisableLowPowerModeAnalogModules(SPC0, mask);
    }

    pResourceRecode->currentOperateMode = operateMode;
}

#if (defined(FSL_PM_SUPPORT_WAKEUP_SOURCE_MANAGER) && FSL_PM_SUPPORT_WAKEUP_SOURCE_MANAGER)
static status_t ManageWakeupSource(pm_wakeup_source_t *ws, bool enable)
{
    assert(ws);

    uint32_t inputType;
    uint32_t inputId;
    uint32_t irqn;
    uint32_t misc;

    PM_DECODE_WAKEUP_SOURCE_ID(ws->wsId);

    if (inputType == kWup_inputType_extPin)
    {
        wuu_external_wakeup_pin_config_t pinConfig;

        pinConfig.edge  = enable ? (wuu_external_pin_edge_detection_t)misc : kWUU_ExternalPinDisable;
        pinConfig.event = kWUU_ExternalPinInterrupt;
        pinConfig.mode  = kWUU_ExternalPinActiveDSPD;

        WUU_SetExternalWakeUpPinsConfig(WUU0, inputId, &pinConfig);

        if (enable)
        {
            EnableIRQ((IRQn_Type)irqn);
            EnableIRQ(WUU_IRQn);
        }
        else
        {
            DisableIRQ((IRQn_Type)irqn);
            DisableIRQ(WUU_IRQn);
        }
    }
    else
    {
        if (enable)
        {
            /* Wakeup source is internal module. */
            EnableIRQ(WUU_IRQn);
            WUU_SetInternalWakeUpModulesConfig(WUU0, inputId, kWUU_InternalModuleInterrupt);
            EnableIRQ((IRQn_Type)irqn);
        }
        else
        {
            WUU0->ME &= ~(1UL << inputId);
            DisableIRQ((IRQn_Type)irqn);
            DisableIRQ(WUU_IRQn);
        }
    }

    return kStatus_Success;
}

static bool IsWakeupSource(pm_wakeup_source_t *ws)
{
    uint32_t inputType;
    uint32_t inputId;
    uint32_t irqn;
    uint32_t misc;
    uint32_t wuuPf;
    uint32_t mask;
    bool ret = false;

    assert(ws != NULL);

    PM_DECODE_WAKEUP_SOURCE_ID(ws->wsId);

    /* Wakeup source is external pin. */
    if (inputType == kWup_inputType_extPin)
    {
        wuuPf = WUU_GetExternalWakeUpPinsFlag(WUU0);
        mask  = (1U << inputId) & WUU_PIN_FLAG_MASK;

        if ((wuuPf * mask) != 0U)
        {
            /* This wake up source triggered the last wake up */
            ret = true;
        }
    }

    (void)irqn;
    (void)misc;

    return ret;
}
#endif /* FSL_PM_SUPPORT_WAKEUP_SOURCE_MANAGER*/

/* Reported in MCUX-65866 to keep reserved bits cleared */
void CMC_PowerOffSRAMAllMode_to_add(CMC_Type *base, uint32_t mask)
{
    uint32_t reserved_bits = 0xC0FF8000U;

    CMC_PowerOffSRAMAllMode(base, mask & ~reserved_bits); /* reserved bits must remain 0 */
}

/* Reported in MCUX-65866 to keep reserved bits cleared */
void CMC_PowerOffSRAMLowPowerOnly_to_add(CMC_Type *base, uint32_t mask)
{
    uint32_t reserved_bits = 0xC0FF8000U;

    CMC_PowerOffSRAMLowPowerOnly(base, mask & ~reserved_bits); /* reserved bits must remain 0 */
}
