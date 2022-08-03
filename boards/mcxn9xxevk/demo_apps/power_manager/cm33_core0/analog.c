/*
 * Copyright 2016-2017, 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "analog.h"

#include "fsl_spc.h"
#include "fsl_clock.h"
#include "fsl_dac.h"
#include "fsl_dac14.h"
#include "fsl_lpcmp.h"
#include "fsl_opamp.h"
#include "fsl_vref.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * brief Get if analog module is already active
 *
 * param mod Module mask, of type _spc_analog_module_control
 */
bool App_GetAnalogActive(uint32_t mod)
{
    if(SPC_GetActiveModeEnabledAnalogModules(SPC0) & mod)
        return true;
    else
        return false;
}

void App_Enable_CMP(LPCMP_Type *base)
{
    lpcmp_config_t mLpcmpConfigStruct;
    lpcmp_dac_config_t mLpcmpDacConfigStruct;
    clock_div_name_t div;
    clock_attach_id_t clk_attach_id;
    clock_attach_id_t clk_detach_id;
    uint32_t active_cfg1_mask;

    if(base == CMP0)
    {
        div                 = kCLOCK_DivCmp0FClk;
        clk_attach_id       = kFRO12M_to_CMP0F;
        clk_detach_id       = kNONE_to_CMP0F;
        active_cfg1_mask    = kSPC_controlCmp0 | kSPC_controlCmp0Dac;
    } else if(base == CMP1)
    {
        div                 = kCLOCK_DivCmp1FClk;
        clk_attach_id       = kFRO12M_to_CMP1F;
        clk_detach_id       = kNONE_to_CMP1F;
        active_cfg1_mask    = kSPC_controlCmp1 | kSPC_controlCmp1Dac;
    } else if(base == CMP2)
    {
        div                 = kCLOCK_DivCmp2FClk;
        clk_attach_id       = kFRO12M_to_CMP2F;
        clk_detach_id       = kNONE_to_CMP2F;
        active_cfg1_mask    = kSPC_controlCmp2 | kSPC_controlCmp2Dac;
    } else
    {
        assert(0);
    }

    if(App_GetAnalogActive(active_cfg1_mask))
    {
        /* CMP already active */
        return;
    }

    /* attach clock to CMP */
    CLOCK_SetClkDiv(div, 1u);
    CLOCK_AttachClk(clk_attach_id);

    if(base == CMP2)
    {
    	/* This is a workaround for MCUX-65867 */
        /* Enable AHB clock to CMP registers */
    #if !(defined(FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL) && FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL)
        CLOCK_EnableClock(kCLOCK_Cmp2);
    #endif /* FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL */
    }

    /* enable analog module */
    SPC_EnableActiveModeAnalogModules(SPC0, active_cfg1_mask);

    /* Configure the CMP */
    LPCMP_GetDefaultConfig(&mLpcmpConfigStruct);
    mLpcmpConfigStruct.enableStopMode = true;
    LPCMP_Init(base, &mLpcmpConfigStruct);

    /* Configure the internal DAC to output half of reference voltage. */
    mLpcmpDacConfigStruct.enableLowPowerMode     = true;
    mLpcmpDacConfigStruct.referenceVoltageSource = kLPCMP_VrefSourceVin1;
    mLpcmpDacConfigStruct.DACValue =
        ((LPCMP_DCR_DAC_DATA_MASK >> LPCMP_DCR_DAC_DATA_SHIFT) >> 1U); /* Half of reference voltage. */
    LPCMP_SetDACConfig(base, &mLpcmpDacConfigStruct);

    if(base == CMP2)
    {
        /* Disable AHB clock to CMP registers */
    #if !(defined(FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL) && FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL)
        CLOCK_DisableClock(kCLOCK_Cmp2);
    #endif /* FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL */
    }

    /* detach clock to DAC2, and halt divider */
    CLOCK_SetClkDiv(div, 0u);
    CLOCK_AttachClk(clk_detach_id);
}

void App_Disable_CMP(LPCMP_Type *base)
{
    clock_div_name_t div;
    clock_attach_id_t clk_attach_id;
    clock_attach_id_t clk_detach_id;
    uint32_t active_cfg1_mask;

    if(base == CMP0)
    {
        div                 = kCLOCK_DivCmp0FClk;
        clk_attach_id       = kFRO12M_to_CMP0F;
        clk_detach_id       = kNONE_to_CMP0F;
        active_cfg1_mask    = kSPC_controlCmp0 | kSPC_controlCmp0Dac;
    } else if(base == CMP1)
    {
        div                 = kCLOCK_DivCmp1FClk;
        clk_attach_id       = kFRO12M_to_CMP1F;
        clk_detach_id       = kNONE_to_CMP1F;
        active_cfg1_mask    = kSPC_controlCmp1 | kSPC_controlCmp1Dac;
    } else if(base == CMP2)
    {
        div                 = kCLOCK_DivCmp2FClk;
        clk_attach_id       = kFRO12M_to_CMP2F;
        clk_detach_id       = kNONE_to_CMP2F;
        active_cfg1_mask    = kSPC_controlCmp2 | kSPC_controlCmp2Dac;
    } else
    {
        assert(0);
    }

    if(!App_GetAnalogActive(active_cfg1_mask))
    {
        /* CMP already disabled */
        return;
    }

    if(base == CMP2)
    {
        /* Enable AHB clock to CMP registers */
    #if !(defined(FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL) && FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL)
        CLOCK_EnableClock(kCLOCK_Cmp2);
    #endif /* FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL */
    }

    /* attach clock to CMP */
    CLOCK_SetClkDiv(div, 1u);
    CLOCK_AttachClk(clk_attach_id);

    LPCMP_Deinit(base);

    /* disable analog module */
    SPC_DisableActiveModeAnalogModules(SPC0, active_cfg1_mask);

    if(base == CMP2)
    {
        /* Disable AHB clock to CMP registers */
    #if !(defined(FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL) && FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL)
        CLOCK_DisableClock(kCLOCK_Cmp2);
    #endif /* FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL */
    }

    /* detach clock to DAC, and halt divider */
    CLOCK_SetClkDiv(div, 0u);
    CLOCK_AttachClk(clk_detach_id);
}

void App_Enable_DAC(LPDAC_Type *base)
{
    dac_config_t dacConfigStruct;
    clock_div_name_t div;
    clock_attach_id_t clk_attach_id;
    clock_attach_id_t clk_detach_id;
    clock_ip_name_t clk_ip;
    uint32_t active_cfg1_mask;

    if(base == DAC0)
    {
        div                 = kCLOCK_DivDac0Clk;
        clk_attach_id       = kFRO12M_to_DAC0;
        clk_detach_id       = kNONE_to_DAC0;
        clk_ip              = kCLOCK_Dac0;
        active_cfg1_mask    = kSPC_controlDac0;
    } else if(base == DAC1)
    {
        div                 = kCLOCK_DivDac1Clk;
        clk_attach_id       = kFRO12M_to_DAC1;
        clk_detach_id       = kNONE_to_DAC1;
        clk_ip              = kCLOCK_Dac1;
        active_cfg1_mask    = kSPC_controlDac1;
    } else
    {
        assert(0);
    }

    if(App_GetAnalogActive(active_cfg1_mask))
    {
        /* DAC already active */
        return;
    }

    /* attach clock to DAC */
    CLOCK_SetClkDiv(div, 1u);
    CLOCK_AttachClk(clk_attach_id);

    /* enable analog module */
    SPC_EnableActiveModeAnalogModules(SPC0, active_cfg1_mask);

    /* Configure the DAC. */
    DAC_GetDefaultConfig(&dacConfigStruct);
    dacConfigStruct.referenceVoltageSource = kDAC_ReferenceVoltageSourceAlt1;
    DAC_Init(base, &dacConfigStruct);
    DAC_Enable(base, true); /* Enable the logic and output. */

    /* Disable AHB clock to DAC registers */
#if !(defined(FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL) && FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL)
    CLOCK_DisableClock(clk_ip);
#endif /* FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL */
    
    /* detach clock to DAC2, and halt divider */
    CLOCK_SetClkDiv(div, 0u);
    CLOCK_AttachClk(clk_detach_id);
}

void App_Disable_DAC(LPDAC_Type *base)
{
    clock_div_name_t div;
    clock_attach_id_t clk_attach_id;
    clock_attach_id_t clk_detach_id;
    clock_ip_name_t clk_ip;
    uint32_t active_cfg1_mask;

    if(base == DAC0)
    {
        div                 = kCLOCK_DivDac0Clk;
        clk_attach_id       = kFRO12M_to_DAC0;
        clk_detach_id       = kNONE_to_DAC0;
        clk_ip              = kCLOCK_Dac0;
        active_cfg1_mask    = kSPC_controlDac0;
    } else if(base == DAC1)
    {
        div                 = kCLOCK_DivDac1Clk;
        clk_attach_id       = kFRO12M_to_DAC1;
        clk_detach_id       = kNONE_to_DAC1;
        clk_ip              = kCLOCK_Dac1;
        active_cfg1_mask    = kSPC_controlDac1;
    } else
    {
        assert(0);
    }

    if(!App_GetAnalogActive(active_cfg1_mask))
    {
        /* DAC already disabled */
        return;
    }

    /* Enable AHB clock to DAC registers */
#if !(defined(FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL) && FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL)
    CLOCK_EnableClock(clk_ip);
#endif /* FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL */

    /* attach clock to DAC */
    CLOCK_SetClkDiv(div, 1u);
    CLOCK_AttachClk(clk_attach_id);

    DAC_Deinit(base);

    /* disable analog module */
    SPC_DisableActiveModeAnalogModules(SPC0, active_cfg1_mask);

    /* detach clock to DAC, and halt divider */
    CLOCK_SetClkDiv(div, 0u);
    CLOCK_AttachClk(clk_detach_id);
}

void App_Enable_DAC14(void)
{
    dac14_config_t dac14ConfigStruct;

    if(App_GetAnalogActive(kSPC_controlDac2))
    {
        /* DAC2 already active */
        return;
    }

    /* attach clock to DAC2 */
    CLOCK_SetClkDiv(kCLOCK_DivDac2Clk, 1u);
    CLOCK_AttachClk(kFRO12M_to_DAC2);

    /* enable analog module */
    SPC_EnableActiveModeAnalogModules(SPC0, kSPC_controlDac2);

    /* Configure the DAC14. */
    DAC14_GetDefaultConfig(&dac14ConfigStruct);
    dac14ConfigStruct.enableOpampBuffer = true;
    dac14ConfigStruct.enableDAC         = true;
    DAC14_Init(DAC2, &dac14ConfigStruct);

    /* Disable AHB clock to DAC2 registers */
#if !(defined(FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL) && FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL)
    CLOCK_DisableClock(kCLOCK_Dac2);
#endif /* FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL */
    
    /* detach clock to DAC2, and halt divider */
    CLOCK_SetClkDiv(kCLOCK_DivDac2Clk, 0u);
    CLOCK_AttachClk(kNONE_to_DAC2);
}

void App_Disable_DAC14(void)
{
    if(!App_GetAnalogActive(kSPC_controlDac2))
    {
        /* DAC2 already disabled */
        return;
    }

    /* Enable AHB clock to DAC2 registers */
#if !(defined(FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL) && FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL)
    CLOCK_EnableClock(kCLOCK_Dac2);
#endif /* FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL */

    /* attach clock to DAC2 */
    CLOCK_SetClkDiv(kCLOCK_DivDac2Clk, 1u);
    CLOCK_AttachClk(kFRO12M_to_DAC2);

    DAC14_Deinit(DAC2);

    /* disable analog module */
    SPC_DisableActiveModeAnalogModules(SPC0, kSPC_controlDac2);

    /* Disable AHB clock to DAC2 registers */
#if !(defined(FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL) && FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL)
    CLOCK_DisableClock(kCLOCK_Dac2);
#endif /* FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL */

    /* detach clock to DAC2, and halt divider */
    CLOCK_SetClkDiv(kCLOCK_DivDac2Clk, 0u);
    CLOCK_AttachClk(kNONE_to_DAC2);
}

void App_Enable_OpAmp(OPAMP_Type *base)
{
    opamp_config_t config;
    clock_ip_name_t clk_ip;
    uint32_t active_cfg1_mask;

    if(base == OPAMP0)
    {
        clk_ip              = kCLOCK_Opamp0;
        active_cfg1_mask    = kSPC_controlOpamp0;
    } else if(base == OPAMP1)
    {
        clk_ip              = kCLOCK_Opamp1;
        active_cfg1_mask    = kSPC_controlOpamp1;
    } else if(base == OPAMP2)
    {
        clk_ip              = kCLOCK_Opamp2;
        active_cfg1_mask    = kSPC_controlOpamp2;
    } else
    {
        assert(0);
    }

    if(App_GetAnalogActive(active_cfg1_mask))
    {
        /* OpAmp already active */
        return;
    }

    /* enable analog module */
    SPC_EnableActiveModeAnalogModules(SPC0, active_cfg1_mask);

    /* Configure the OpAmp. */
    OPAMP_GetDefaultConfig(&config);
    config.posGain = kOPAMP_PosGainNonInvertDisableBuffer2X;
    config.negGain = kOPAMP_NegGainInvert1X;
    config.enable  = true;
    OPAMP_Init(base, &config);

    /* Disable AHB clock to OpAmp registers */
#if !(defined(FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL) && FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL)
    CLOCK_DisableClock(clk_ip);
#endif /* FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL */
}

void App_Disable_OpAmp(OPAMP_Type *base)
{
    clock_ip_name_t clk_ip;
    uint32_t active_cfg1_mask;

    if(base == OPAMP0)
    {
        clk_ip              = kCLOCK_Opamp0;
        active_cfg1_mask    = kSPC_controlOpamp0;
    } else if(base == OPAMP1)
    {
        clk_ip              = kCLOCK_Opamp1;
        active_cfg1_mask    = kSPC_controlOpamp1;
    } else if(base == OPAMP2)
    {
        clk_ip              = kCLOCK_Opamp2;
        active_cfg1_mask    = kSPC_controlOpamp2;
    } else
    {
        assert(0);
    }

    if(!App_GetAnalogActive(active_cfg1_mask))
    {
        /* OpAmp already disabled */
        return;
    }

    /* Enable AHB clock to OpAmp registers */
#if !(defined(FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL) && FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL)
    CLOCK_EnableClock(clk_ip);
#endif /* FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL */

    OPAMP_Deinit(base);

    /* disable analog module */
    SPC_DisableActiveModeAnalogModules(SPC0, active_cfg1_mask);
}

void App_Enable_VREF(void)
{
    vref_config_t vrefConfig;

    if(App_GetAnalogActive(kSPC_controlVref))
    {
        /* VREF already active */
        return;
    }

    /* enable analog module */
    SPC_EnableActiveModeAnalogModules(SPC0, kSPC_controlVref);

    /* Configure the VREF */
    VREF_GetDefaultConfig(&vrefConfig);
    vrefConfig.bufferMode = kVREF_ModeLowPowerBuffer;
    VREF_Init(VREF0, &vrefConfig);

    /* Disable AHB clock to VREF registers */
#if !(defined(FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL) && FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL)
    CLOCK_DisableClock(kCLOCK_Vref);
#endif /* FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL */
}

void App_Disable_VREF(void)
{
    if(!App_GetAnalogActive(kSPC_controlVref))
    {
        /* VREF already disabled */
        return;
    }

    VREF_Deinit(VREF0);

    /* disable analog module */
    SPC_DisableActiveModeAnalogModules(SPC0, kSPC_controlVref);
}

