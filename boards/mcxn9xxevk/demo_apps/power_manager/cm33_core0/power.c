/*
 * Copyright (c) 2013 - 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017, 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "power.h"

#include "board.h"
#include "fsl_debug_console.h"
#include "fsl_lpuart.h"
#include "fsl_port.h"

#include "fsl_clock.h"
#include "fsl_cmc.h"
#include "fsl_spc.h"
#include "fsl_vbat.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* Debug console RX pin: PORT1_8 MUX: 2 */
#define DEBUG_CONSOLE_RX_PORT   PORT1
#define DEBUG_CONSOLE_RX_GPIO   GPIO1
#define DEBUG_CONSOLE_RX_PIN    8U
#define DEBUG_CONSOLE_RX_PINMUX kPORT_MuxAlt2
/* Debug console TX pin: PORT1_9 MUX: 2 */
#define DEBUG_CONSOLE_TX_PORT   PORT1
#define DEBUG_CONSOLE_TX_GPIO   GPIO1
#define DEBUG_CONSOLE_TX_PIN    9U
#define DEBUG_CONSOLE_TX_PINMUX kPORT_MuxAlt2

/*******************************************************************************
 * Variables
 ******************************************************************************/

const char powerStrings [6][16]=
{
    "Active         \0",
    "Sleep          \0",
    "Deep Sleep     \0",
    "Power Down     \0",
    "Deep Power Down\0",
    "VBAT           \0"
};

const char clockStrings [5][19]=
{
    "FRO LF  @ 12 MHz\0",
    "FRO HF  @ 48 MHz\0",
    "PLL     @100 MHz\0",
    "FRO HF  @144 MHz\0",
    "PLL     @150 MHz\0"
};

const char vddCoreSrcStrings [3][8]=
{
    "DCDC\0",
    "LDO \0",
    "PMIC\0"
};

const char vddCoreLvlStrings [4][19]=
{
    "\0",
    "VDD_CORE = 1.0V\0",
    "VDD_CORE = 1.1V\0",
    "VDD_CORE = 1.2V\0"
};

const char vddCoreDriveStrings [5][33] = 
{
    "DCDC Low Drive        = 15mA \0",
    "DCDC Pulse Refresh    = 45mA",
    "DCDC Normal Drive     = 100mA\0",
    "LDO_CORE Low Drive    = TBD  \0",
    "LDO_CORE Normal Drive = TBD  \0"
};

const char vddSysDriveStrings [2][33] =
{
   "LDO_CORE Low Drive    = 2mA  \0",
   "LDO_CORE Normal Drive = 40mA \0"
};

const char systemSRAMStrings [5][34] = 
{
    "  0kB System SRAM               \0",
    " 32kB System SRAM (RAMX0)       \0",
    " 64kB System SRAM (RAMX0, RAMX1)\0",
    "256kB System SRAM (~half)       \0",
    "480kB System SRAM (ALL)         \0"
};

const char resetStrings[32][9] = 
{
    "DPD,    \0",
    "POR,    \0",
    "        \0",
    "LVD-HVD,\0",
    "Warm,   \0",
    "Fatal,  \0",
    "        \0",
    "        \0",
    "PIN,    \0",
    "DAP,    \0",
    "Timeout,\0",
    "LPACK,  \0",
    "SCG,    \0",
    "WWDT0,  \0",
    "SW,     \0",
    "LOCKUP, \0",
    "CPU1,   \0",
    "        \0",
    "        \0",
    "        \0",
    "        \0",
    "        \0",
    "        \0",
    "        \0",
    "VBAT,   \0",
    "WWDT1,  \0",
    "CDOG0,  \0",
    "CDOG1,  \0",
    "JTAG,   \0",
    "        \0",
    "SECVIO, \0",
    "Tamper, \0"
};

/* Strings to print the resc */
const char* resc_strings[PM_CONSTRAINT_COUNT] = {
    [kResc_BUS_SYS_Clk  ]   = "Bus/System Clocks",
    [kResc_CORE_WAKE    ]   = "CORE_WAKE Domain",
    [kResc_SRAM_RAMA0_8K]   = "RAMA0 SRAM",
    [kResc_SRAM_RAMA1_8K]   = "RAMA1 SRAM",
    [kResc_SRAM_RAMA2_8K]   = "RAMA2 SRAM",
    [kResc_SRAM_RAMA3_8K]   = "RAMA3 SRAM",
    [kResc_SRAM_RAMX0_32K]  = "RAMX0 SRAM",
    [kResc_SRAM_RAMX1_32K]  = "RAMX1 SRAM",
    [kResc_SRAM_RAMX2_32K]  = "RAMX2 SRAM",
    [kResc_SRAM_RAMB0_32K]  = "RAMB0 SRAM",
    [kResc_SRAM_RAMC0_32K]  = "RAMC0 SRAM",
    [kResc_SRAM_RAMC1_32K]  = "RAMC1 SRAM",
    [kResc_SRAM_RAMD0_32K]  = "RAMD0 SRAM",
    [kResc_SRAM_RAMD1_32K]  = "RAMD1 SRAM",
    [kResc_SRAM_RAME0_32K]  = "RAME0 SRAM",
    [kResc_SRAM_RAME1_32K]  = "RAME1 SRAM",
    [kResc_SRAM_RAMF0_32K]  = "RAMF0 SRAM",
    [kResc_SRAM_RAMF1_32K]  = "RAMF1 SRAM",
    [kResc_SRAM_RAMG01_32K] = "RAMG0/1 SRAM",
    [kResc_SRAM_RAMG23_32K] = "RAMG2/3 SRAM",
    [kResc_SRAM_RAMH01_32K] = "RAMH0/1 SRAM",
    [kResc_Flash]           = "Flash Memory",
    [kResc_DCDC_CORE]       = "DCDC_CORE Regulator",
    [kResc_LDO_CORE]        = "LDO_CORE Regulator",
    [kResc_LDO_SYS]         = "LDO_SYS Regulator",
    [kResc_FRO_144M]        = "Internal 144 MHz Oscillator",
    [kResc_FRO_12M]         = "Internal  12 MHz Oscillator",
    [kResc_FRO_16K]         = "Internal  16 kHz Oscillator",
    [kResc_OSC_RTC]         = "32 kHz Crystal Oscillator",
    [kResc_OSC_SYS]         = "High-Frequency Crystal Oscillator",
    [kResc_PLL0]            = "Auxillary APLL",
    [kResc_PLL1]            = "System SPLL",
    [kResc_ADC]             = "ADC peripheral",
    [kResc_CMP0]            = "CMP0 peripheral",
    [kResc_CMP1]            = "CMP1 peripheral",
    [kResc_CMP2]            = "CMP2 peripheral",
    [kResc_DAC0]            = "DAC0 peripheral",
    [kResc_DAC1]            = "DAC1 peripheral",
    [kResc_DAC2]            = "DAC2 peripheral",
    [kResc_OPAMP0]          = "OPAMP0 peripheral",
    [kResc_OPAMP1]          = "OPAMP1 peripheral",
    [kResc_OPAMP2]          = "OPAMP2 peripheral",
    [kResc_VREF]            = "VREF peripheral",
    [kResc_SINC]            = "SINC Filter peripheral",
    [kResc_USB3V_DET]       = "USB 3V Detect",
    [kResc_BG_CORE]         = "VDD_CORE Bandgap",
    [kResc_BG_VBAT]         = "VBAT Bandgap",
    [kResc_GDET]            = "VDD_CORE Glitch Detect",
    [kResc_HVD_CORE]        = "VDD_CORE HVD",
    [kResc_HVD_SYS]         = "VDD_SYS HVD",
    [kResc_HVD_IO]          = "VDD IO HVD",
    [kResc_LVD_CORE]        = "VDD_CORE LVD",
    [kResc_LVD_SYS]         = "VDD_SYS LVD",
    [kResc_LVD_IO]          = "VDD IO LVD",
    [kResc_IO_Det]          = "VDD IO voltage detect",
};

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
uint8_t getUserSelection (uint8_t maxOption);

/*******************************************************************************
 * Code
 ******************************************************************************/
void APP_DeinitDebugConsole(void)
{   
    while (!(kLPUART_TransmissionCompleteFlag & LPUART_GetStatusFlags((LPUART_Type *)BOARD_DEBUG_UART_BASEADDR)));
    DbgConsole_Deinit();
    DEBUG_CONSOLE_RX_PORT->PCR[DEBUG_CONSOLE_RX_PIN] &= ~PORT_PCR_IBE_MASK;  /* Disable pin input buffer */
    PORT_SetPinMux(DEBUG_CONSOLE_RX_PORT, DEBUG_CONSOLE_RX_PIN, kPORT_PinDisabledOrAnalog);
    PORT_SetPinMux(DEBUG_CONSOLE_TX_PORT, DEBUG_CONSOLE_TX_PIN, kPORT_PinDisabledOrAnalog);
}
/*******************************************************************************/
void APP_InitDebugConsole(void)
{
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom4Clk, 1u);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);   
    BOARD_InitDebugConsole();
    DEBUG_CONSOLE_RX_PORT->PCR[DEBUG_CONSOLE_RX_PIN] |= PORT_PCR_IBE_MASK;  /* Enable pin input buffer */
    PORT_SetPinMux(DEBUG_CONSOLE_RX_PORT, DEBUG_CONSOLE_RX_PIN, kPORT_MuxAlt2);
    PORT_SetPinMux(DEBUG_CONSOLE_TX_PORT, DEBUG_CONSOLE_TX_PIN, kPORT_MuxAlt2);
}
/*******************************************************************************/
/*!
 * @brief Callback notification from PM_EnterLowPower()
 */
status_t APP_PowerSwitchNotification(pm_event_type_t eventType, uint8_t powerState, void *data)
{
    switch(eventType)
    {
        case kPM_EventEnteringSleep:
            APP_DeinitDebugConsole();

            /* Disable FRO_12M */
            SCG0->SIRCCSR &= ~SCG_SIRCCSR_LK_MASK;
            SCG0->SIRCCSR &= ~(SCG_SIRCCSR_SIRC_CLK_PERIPH_EN_MASK | SCG_SIRCCSR_SIRCSTEN_MASK);
            SCG0->SIRCCSR |= SCG_SIRCCSR_LK_MASK;

            /* Gate clocks */
            SCG0->FIRCCSR &= ~(SCG_FIRCCSR_FIRC_SCLK_PERIPH_EN_MASK | SCG_FIRCCSR_FIRC_FCLK_PERIPH_EN_MASK);
            CLOCK_DisableClock(kCLOCK_Scg);

            /* Signal MCU is entering low-power mode */
            GPIO_PinWrite(BOARD_WOKEN_GPIO, BOARD_WOKEN_GPIO_PIN, LOGIC_LED_OFF);
            break;

        case kPM_EventExitingSleep:
            /* Signal MCU has woken and in Active mode */
            GPIO_PinWrite(BOARD_WOKEN_GPIO, BOARD_WOKEN_GPIO_PIN, LOGIC_LED_ON);

            /* Ungate clocks */
            CLOCK_EnableClock(kCLOCK_Scg);
            SCG0->FIRCCSR |= (SCG_FIRCCSR_FIRC_SCLK_PERIPH_EN_MASK | SCG_FIRCCSR_FIRC_FCLK_PERIPH_EN_MASK);

            /*TurnON FRO_12M */
            SCG0->SIRCCSR &= ~SCG_SIRCCSR_LK_MASK;
            SCG0->SIRCCSR |= SCG_SIRCCSR_SIRC_CLK_PERIPH_EN_MASK;
            while( (SCG0->SIRCCSR & SCG_SIRCCSR_SIRCVLD_MASK) != SCG_SIRCCSR_SIRCVLD_MASK);
            SCG0->SIRCCSR |= SCG_SIRCCSR_LK_MASK;

            APP_InitDebugConsole();
            break;

        default:
            assert(0);
            break;
    }

    return kStatus_PMSuccess;
}
/*******************************************************************************/
void APP_PrintPowerModeToEnter(uint64_t duration)
{
    pm_deepest_state_results_t results;
    uint8_t mode_str;

    PM_findDeepestState(duration, &results);

    /* Convert to mode string to print */
    switch(results.deepestState)
    {
        case PM_LP_STATE_SLEEP:
        case PM_LP_STATE_DEEP_SLEEP:
            mode_str = results.deepestState + 1;
            break;

        case PM_LP_STATE_POWER_DOWN_WAKE_DS:
        case PM_LP_STATE_POWER_DOWN_WAKE_PD:
            mode_str = kAPP_PowerDown;
            break;

        case PM_LP_STATE_DEEP_POWER_DOWN:
        case PM_LP_STATE_VBAT:
            mode_str = results.deepestState;
            break;

        default:
            PRINTF("Error: PM_findDeepestState() deepestState unknown state %d\r\n", results.deepestState);
            return;
    }

    PRINTF("Calculated power mode is %s", powerStrings[mode_str]);

    /* print more if in Power Down mode */
    if(mode_str == kAPP_PowerDown)
    {
        PRINTF(" with WAKE domain in ");
        if(results.deepestState == PM_LP_STATE_POWER_DOWN_WAKE_DS)
        {
            PRINTF("Deep Sleep");
        } else
        {
            PRINTF("Power Down");
        }
    }

    switch(results.reason)
    {
        case kPM_reason_deepest:
            PRINTF(", which is deepest mode for this MCU.\r\n");
            break;

        case kPM_reason_latency:
            PRINTF(", because duration was too short for deeper power modes.\r\n");
            break;

        case kPM_reason_mode_constraint:
            PRINTF(", because a power-mode constraint limits to this mode.\r\n");
            break;

        case kPM_reason_resc:
            PRINTF(", because resc %s is set, and limits to this mode.\r\n", 
                        resc_strings[results.resc_num]);
            break;

        default:
            PRINTF("\r\nERROR: results.reason returned from PM_findDeepestState() ");
            PRINTF("is not handled.\r\n");
            break;
    }
}


/*******************************************************************************/
app_reset_src_t APP_GetResetSource(void)
{
    uint32_t resetSource;
    uint32_t i=0;
    
    resetSource = CMC_GetStickySystemResetStatus(CMC0);
    CMC_ClearStickySystemResetStatus(CMC0, resetSource);

    PRINTF("\r\n\nSticky RESET sources: \r\n");
    for (i=0; i<32; i++)
    {
        if (resetSource & (uint32_t)(1<<i) )
        {
            PRINTF("%s", resetStrings[i]);
        }
    }
    PRINTF ("\r\n");

    if ((resetSource & CMC_SSRS_WAKEUP_MASK) == CMC_SSRS_WAKEUP_MASK)
    {
        return kAPP_Wakeup_Reset;
    }
    else
    {
        if ((resetSource & CMC_SSRS_POR_MASK) == CMC_SSRS_POR_MASK)
        {
            return kAPP_POR_Reset;
        }
        else
        {
            return kAPP_Warm_Reset;
        }
    }
}

/*-----------------------------------------------------------------------------*/

app_power_mode_t APP_GetPowerModefromUser (void)
{
    uint8_t userOption = kAPP_Active;
    uint32_t i;

    PRINTF("\r\n\r\nSelect a Power Mode:\r\n");
    for (i=kAPP_Active; i <= kAPP_DeepPowerDown; i ++)
    {
        PRINTF("\t%d. %s \r\n", i, powerStrings[i]);
    }

    userOption = getUserSelection(kAPP_DeepPowerDown);
    return ( (app_power_mode_t)(userOption));
}
/*******************************************************************************/
app_clk_src_t APP_GetMainClockfromUser (void)
{
    uint8_t userOption = kAPP_FRO12M;
    uint32_t i;

    PRINTF("Select a CORE0 Clock Source:\r\n");
    for (i=kAPP_FRO12M; i <= kAPP_PLL150M; i ++)
    {
        PRINTF("%d. %s \r\n", i, clockStrings[i]);
    }

    userOption = getUserSelection( kAPP_PLL150M);
    return ( (app_clk_src_t)(userOption));
}
/*******************************************************************************/
app_clk_src_t APP_GetEnabledClockfromUser (app_power_mode_t powerMode)
{
    bool waitValidOption = true;
    uint8_t ch;
    uint8_t maxOption = kAPP_PLL150M;
    app_clk_src_t enabledClocks = 0;
    uint32_t i;

    switch (powerMode)
    {
    case kAPP_PowerDown:
        maxOption = kAPP_FRO12M;
        break;
    
    case kAPP_DeepPowerDown:
        //maxOption = kAPP_OSCRTC;
    
    default:
        maxOption = kAPP_PLL150M;
        break;
    }

    PRINTF("Select what clock sources stay enabled during %s \r\n", powerStrings[powerMode]);   
    PRINTF("Type 'e' to enable the clock or 'd' to disabled it \r\n");
    for (i=kAPP_FRO12M; i <= maxOption; i ++)
    {
        waitValidOption = true;
        PRINTF("%s \r\n", clockStrings[i]);
        do 
        {
            ch = GETCHAR();
            if ('e' == ch || 'E' == ch)
            {
                enabledClocks += (1<<i);
                waitValidOption = false;
            }

            if ('d' == ch || 'D' == ch)
            {
                waitValidOption = false;
            }
            
        }while(waitValidOption);         
    }

    PRINTF("%s", powerStrings[powerMode]);
    PRINTF("ENABLED CLOCKS = \r\n");
    for (i=kAPP_FRO12M; i <= maxOption; i ++)
    {
        if (enabledClocks & (1<<i))
        PRINTF("\t%s\r\n", clockStrings[i]);     
    }
    PRINTF("--------------------------------------------------------------------------------\r\n\r\n");

    return enabledClocks;
}
/*******************************************************************************/
app_vdd_core_src_t APP_GetVddCoreSrcfromUser (void)
{
    uint8_t userOption = kAPP_DCDC;
    uint32_t i;

    PRINTF("Select the VDD_CORE voltage source: \r\n");
    for (i=kAPP_DCDC; i <= kAPP_PMIC; i ++)
    {
        PRINTF("%d. %s \r\n", i, vddCoreSrcStrings[i]);
    }

    userOption = getUserSelection( kAPP_PMIC);
    return ( (app_vdd_core_src_t)(userOption));
}
/*******************************************************************************/
app_vdd_core_lvl_t APP_GetVddCoreLvlfromUser (void)
{
    uint8_t userOption = kAPP_1p00V;
    uint32_t i;

    PRINTF("Select the VDD_CORE voltage level: \r\n");
    for (i=kAPP_1p00V; i <= kAPP_1p20V; i ++)
    {
        PRINTF("%d. %s \r\n", i, vddCoreLvlStrings[i]);
    }

    userOption = getUserSelection( kAPP_1p20V);
    return ( (app_vdd_core_lvl_t)(userOption));
}
/*******************************************************************************/
app_vdd_sys_ds_t APP_GetVddCoreDriveStregth (app_vdd_core_src_t vddSrc)
{
    bool waitValidOption = true;
    uint8_t ch;
    app_vdd_sys_ds_t vddCoreDS;

    if (vddSrc == kAPP_DCDC)
    {
        PRINTF ("0. %s \r\n", vddCoreDriveStrings[0]);
        PRINTF ("1. %s \r\n", vddCoreDriveStrings[1]);
        do 
        {
            ch = GETCHAR();
            switch (ch)
            {
            case '0':
                vddCoreDS = kAPP_DCDC_15mA;
                waitValidOption = false;
                break;
            
            case '1':
                vddCoreDS = kAPP_DCDC_100mA;
                waitValidOption = false;
                break;

            default:
                waitValidOption = true;
                break;
            }
        }while (waitValidOption);
    }

    if (vddSrc == kAPP_LDO)
    {
        PRINTF ("0. %s \r\n", vddCoreDriveStrings[2]);
        PRINTF ("1. %s \r\n", vddCoreDriveStrings[3]);
        do 
        {
            ch = GETCHAR();
            switch (ch)
            {
            case '0':
                vddCoreDS = kAPP_LDO_LowDS;
                waitValidOption = false;
                break;
            
            case '1':
                vddCoreDS = kAPP_LDO_NormalDS;
                waitValidOption = false;
                break;

            default:
                waitValidOption = true;
                break;
            }
        }while (waitValidOption);
    }

    return vddCoreDS;
}
/*******************************************************************************/
status_t APP_SetRegulatorsVoltage (app_voltage_t volt, app_power_mode_t powerMode)
{
    status_t status;
    spc_active_mode_regulators_config_t   activeRegulators;
    spc_lowpower_mode_regulators_config_t lpRegulators;
        
    if( kAPP_Active ==  powerMode || kAPP_Sleep == powerMode)
    {
        activeRegulators.bandgapMode = kSPC_BandgapEnabledBufferDisabled;
        activeRegulators.lpBuff = false;
        activeRegulators.DCDCOption.DCDCVoltage = volt.VDDCoreLvl;
        activeRegulators.DCDCOption.DCDCDriveStrength = volt.VDDCoreDS;
        activeRegulators.CoreLDOOption.CoreLDOVoltage = volt.VDDCoreLvl;
        activeRegulators.CoreLDOOption.CoreLDODriveStrength = volt.VDDCoreDS;
        activeRegulators.SysLDOOption.SysLDOVoltage = volt.VDDSysLvl;
        activeRegulators.SysLDOOption.SysLDODriveStrength = volt.VDDSysDS;
        status = SPC_SetActiveModeRegulatorsConfig(SPC0, &activeRegulators);
    }
    else
    {
        lpRegulators.bandgapMode = kSPC_BandgapEnabledBufferDisabled;
        lpRegulators.CoreIVS = true;
        lpRegulators.lpIREF = false;        //ToDo enabled when CMP0/1/2, and FRO12M in PowerDown  
        lpRegulators.DCDCOption.DCDCDriveStrength = volt.VDDCoreDS;
        lpRegulators.CoreLDOOption.CoreLDOVoltage = volt.VDDCoreLvl;
        lpRegulators.CoreLDOOption.CoreLDODriveStrength = volt.VDDCoreDS;
        lpRegulators.SysLDOOption.SysLDODriveStrength = kAPP_LDO_LowDS;
        status = SPC_SetLowPowerModeRegulatorsConfig(SPC0, &lpRegulators);
    }

    if (kStatus_Success != status)
    {
        PRINTF("\r\n!!!ERROR setting regulator voltage \r\n");
    }
    while (SPC_GetBusyStatusFlag(SPC0));


    
    return status;
}

void APP_CheckValidVoltageConfig(app_power_config_t *pwr)
{
    bool validVoltage = true;

    /* 1. Check if this is a valid voltage -- speed configuraiton */
    switch (pwr->voltageConfig.VDDCoreLvl)
    {
    case kAPP_1p00V:
        if (pwr->clockConfig.mainClock > kAPP_FRO48M)
        {
            validVoltage = false;
        }
        break;
    
    case kAPP_1p10V:
        if (pwr->clockConfig.mainClock > kAPP_PLL100M)
        {
            validVoltage = false;
        }
        break;

    default:
        break;
    }

    /* For incorrect voltage-frequency, force a valid VDD_CORE level*/
    if (false == validVoltage)
    {
        PRINTF ("%s is NOT an valid option for %s ", vddCoreLvlStrings[pwr->voltageConfig.VDDCoreLvl], clockStrings[pwr->clockConfig.mainClock]);

        if (pwr->clockConfig.mainClock >= kAPP_FRO144M)
        {
            pwr->voltageConfig.VDDCoreLvl = kAPP_1p20V;
        }

        if (pwr->clockConfig.mainClock == kAPP_PLL100M)
        {
            pwr->voltageConfig.VDDCoreLvl = kAPP_1p10V;
        }

        if (pwr->clockConfig.mainClock <= kAPP_FRO48M)
        {
            pwr->voltageConfig.VDDCoreLvl = kAPP_1p00V;
        }

        PRINTF("    %s new level\r\n\r\n", vddCoreLvlStrings[pwr->voltageConfig.VDDCoreLvl]);
    }

    /* 2. If running at 48MHz or higher, foce a higher drive strength*/
    if(pwr->clockConfig.mainClock >= kAPP_FRO48M)
    {       
        if (kAPP_DCDC == pwr->voltageConfig.VDDCoreSrc)
        {
            pwr->voltageConfig.VDDCoreDS = kAPP_DCDC_100mA;
        }
        
        if (kAPP_LDO == pwr->voltageConfig.VDDCoreSrc)
        {
            pwr->voltageConfig.VDDCoreDS = kAPP_LDO_NormalDS;
        }
    }
}

void APP_FindLowestVDDCoreVoltage(app_power_config_t *pwr)
{

    if (pwr->clockConfig.mainClock <= kAPP_FRO48M && pwr->voltageConfig.VDDCoreLvl != kAPP_1p00V)
    {
        pwr->voltageConfig.VDDCoreLvl = kAPP_1p00V;
        PRINTF("    %s lower voltage\r\n", vddCoreLvlStrings[kAPP_1p00V]);
    }

    if (pwr->clockConfig.mainClock > kAPP_FRO48M && pwr->clockConfig.mainClock <= kAPP_PLL100M && pwr->voltageConfig.VDDCoreLvl != kAPP_1p10V)
    {
        pwr->voltageConfig.VDDCoreLvl = kAPP_1p10V;
        PRINTF("    %s lower voltage\r\n", vddCoreLvlStrings[kAPP_1p10V]);
    }

    if (pwr->clockConfig.mainClock > kAPP_PLL100M && pwr->clockConfig.mainClock <= kAPP_PLL150M && pwr->voltageConfig.VDDCoreLvl != kAPP_1p20V)
    {
        pwr->voltageConfig.VDDCoreLvl = kAPP_1p20V;
        PRINTF("    %s lower voltage\r\n", vddCoreLvlStrings[kAPP_1p20V]);
    }
    
}

app_sram_size_t APP_GetSRAMfromUser (void)
{
    uint8_t userOption = kAPP_SRAM_0kB;
    uint32_t i;

    for (i=kAPP_SRAM_0kB; i <= kAPP_SRAM_480kB; i ++)
    {
        PRINTF("\t%d.  %s \r\n", i, systemSRAMStrings[i]);
    }
    userOption = getUserSelection(kAPP_SRAM_480kB);

    return ( (app_sram_size_t)(userOption)); 
}
/*******************************************************************************/
/*!
 * @brief Enables clocks to enabled SRAMs, disables clocks to disabled SRAMs
 *
 * @param enabledRAMs SRAMs that need clocks, of type _cmc_system_sram_arrays
 */
void APP_ConfigureSRAMclks(uint32_t enabledRAMs)
{
    if(enabledRAMs & kCMC_RAMB)
    {
        CLOCK_EnableClock(kCLOCK_Sram1);
    } else
    {
        CLOCK_DisableClock(kCLOCK_Sram1);
    }

    if(enabledRAMs & (kCMC_RAMC0 | kCMC_RAMC1))
    {
        CLOCK_EnableClock(kCLOCK_Sram2);
    } else
    {
        CLOCK_DisableClock(kCLOCK_Sram2);
    }

    if(enabledRAMs & (kCMC_RAMD0 | kCMC_RAMD1))
    {
        CLOCK_EnableClock(kCLOCK_Sram3);
    } else
    {
        CLOCK_DisableClock(kCLOCK_Sram3);
    }

    if(enabledRAMs & (kCMC_RAME0 | kCMC_RAME1))
    {
        CLOCK_EnableClock(kCLOCK_Sram4);
    } else
    {
        CLOCK_DisableClock(kCLOCK_Sram4);
    }

    if(enabledRAMs & (kCMC_RAMF0 | kCMC_RAMF1))
    {
        CLOCK_EnableClock(kCLOCK_Sram5);
    } else
    {
        CLOCK_DisableClock(kCLOCK_Sram5);
    }

    if(enabledRAMs & (kCMC_RAMG0_RAMG1 | kCMC_RAMG2_RAMG3))
    {
        CLOCK_EnableClock(kCLOCK_Sram6);
    } else
    {
        CLOCK_DisableClock(kCLOCK_Sram6);
    }

    if(enabledRAMs & kCMC_RAMH0_RAMH1)
    {
        CLOCK_EnableClock(kCLOCK_Sram7);
    } else
    {
        CLOCK_DisableClock(kCLOCK_Sram7);
    }
}
/*!
 * @brief Optimizes power configuration for the application
 */
void APP_OptimizePower(void)
{
    status_t status;
    spc_lowpower_mode_regulators_config_t regulators_config;

#if (BOARD_NAME == FRDM_MCXN947)
    /* FRDM board has external pull-up on PORT0_6/ISPMODE_N.
     * This causes leakage on VDD_MCU supply if this pin has internal pull-up enabled.
     * EVK board does not have external pull-up */
    CLOCK_EnableClock(kCLOCK_Port0);
    PORT0->PCR[6] &= ~(PORT_PCR_PE_MASK);  /* Disable internal pull-up */
    CLOCK_DisableClock(kCLOCK_Port0);
#endif /* FRDM_MCXN947 */

    CMC_SetPowerModeProtection(CMC0, kCMC_AllowAllLowPowerModes);

    /* Set hardware config to match resource constraint defaults before menus print their registers */
    CMC_PowerOffSRAMAllMode_to_add     (CMC0, ~APP_INIT_SRAMS_ENABLED);
    CMC_PowerOffSRAMLowPowerOnly_to_add(CMC0, ~APP_INIT_SRAMS_ENABLED);

    /* Disable Debugger in low-power modes*/
    CMC_EnableDebugOperation(CMC0, false);

    /* Disable unused clocks */
    CLOCK_DisableClock(kCLOCK_Rom);
    CLOCK_DisableClock(kCLOCK_PkcRam);
    CLOCK_DisableClock(kCLOCK_Gdet);
    CLOCK_DisableClock(kCLOCK_Pkc);
    CLOCK_DisableClock(kCLOCK_Css);

    /* Enable clocks to enabled System SRAMs */
    APP_ConfigureSRAMclks(APP_INIT_SRAM_CLOCKS);

    /* Disable USB3V_Detect */
    SPC_DisableActiveModeAnalogModules  (SPC0, kSPC_controlUsb3vDet);
    SPC_DisableLowPowerModeAnalogModules(SPC0, kSPC_controlUsb3vDet);

    /* ToDo the remaining configurations should be managed by PM or app.
     * Until then, the registers are optimized here */

    /* Configure Regulators */
    regulators_config.lpIREF = false;
    regulators_config.lpBuff = false;
    regulators_config.CoreIVS = true;
    regulators_config.bandgapMode = kSPC_BandgapDisabled;
    regulators_config.DCDCOption.DCDCVoltage             = kSPC_DCDC_MidVoltage;
    regulators_config.DCDCOption.DCDCDriveStrength       = kSPC_DCDC_LowDriveStrength;
    regulators_config.SysLDOOption.SysLDODriveStrength   = kSPC_SysLDO_LowDriveStrength;
    regulators_config.CoreLDOOption.CoreLDOVoltage       = kSPC_CoreLDO_MidDriveVoltage;
    regulators_config.CoreLDOOption.CoreLDODriveStrength = kSPC_CoreLDO_LowDriveStrength;
    status = SPC_SetLowPowerModeRegulatorsConfig(SPC0, &regulators_config);
    assert(status == kStatus_Success);

    /* the SRAM regulator should be set based on RAMA rescs.
     * Enable the VBAT SRAM regulator and bandgap */
    if(kStatus_Success != VBAT_EnableBandgap(VBAT0, true))
    {
        assert(0);
    }

    if(kStatus_Success != VBAT_EnableBackupSRAMRegulator(VBAT0, true))
    {
        assert(0);
    }

    /*Disable Flash access while in low-power modes */
    CMC_ConfigFlashMode(CMC0, false, true, false);
}

/*******************************************************************************/
uint8_t getUserSelection (uint8_t maxOption)
{
    uint8_t temp = maxOption;
    
    do 
    {    
        temp = GETCHAR();
        temp -= '0';   
    }while(temp > maxOption);
    PRINTF("--------------------------------------------------------------------------------\r\n\r\n");

    return (temp);
}

bool PM_GetRescEnabled(resc_name_t resc)
{
    uint8_t index;
    uint32_t resc_bit_mask;
    pm_resc_mask_t resc_mask;

    assert(resc < kResc_Max_Num);

    resc_mask = PM_GetResourceConstraintsMask();

    index = resc / 32U;
    resc_bit_mask = resc % 32U;

    if(resc_mask.rescMask[index] & (1 << resc_bit_mask))
        return true;
    else
        return false;
}

void PM_ToggleConstraint(resc_name_t resc)
{
    if(PM_GetRescEnabled(resc))
    {
        PM_ReleaseConstraints(PM_LP_STATE_NO_CONSTRAINT, 1U, 
                              PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, resc));
    } else
    {
        PM_SetConstraints(PM_LP_STATE_NO_CONSTRAINT, 1U, 
                          PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, resc));
    }
}
