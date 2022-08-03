/*
 * Copyright (c) 2013 - 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017, 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"

#include "fsl_clock.h"
#include "fsl_pm_core.h"
#include "fsl_pm_board.h"
#include "fsl_cmc.h"
#include "fsl_spc.h"
#include "fsl_port.h"
#include "fsl_lpuart.h"

#include "power_manager.h"
#include "power.h"
#include "timers.h"
#include "analog.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

typedef enum _menu_status
{
    kMenu_Main,
    kMenu_Power,
    kMenu_Clocks,
    kMenu_Voltage,
    kMenu_SRAM,
    kMenu_Analog,
    kMenu_Analog_LP,
    kMenu_Analog_Act,
    kMenu_Back,
    kMenu_Done
} menu_status_t;

/* Initial HW settings for application */
#define APP_DEFAULT_RESCS       1U, PM_RESC_FRO_16K_ON
#define APP_INIT_SRAMS_ENABLED  kCMC_LPCAC	/* All System SRAMs disabled except LPCAC */

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
AT_ALWAYS_ON_DATA(pm_handle_t g_pmHndle);
AT_ALWAYS_ON_DATA(pm_notify_element_t g_notify_element);
AT_ALWAYS_ON_DATA(uint32_t g_pmDuration) = APP_DEFAULT_PM_DURATION;

void menu_main(app_power_config_t *pwr);

menu_status_t menu_power     (app_power_config_t *pwr);
menu_status_t menu_clocks    (app_power_config_t *pwr);
menu_status_t menu_voltage   (app_power_config_t *pwr);
menu_status_t menu_sram      (app_power_config_t *pwr);
menu_status_t menu_analog    (app_power_config_t *pwr);
menu_status_t menu_analog_LP (app_power_config_t *pwr);
menu_status_t menu_analog_Act(app_power_config_t *pwr);

void APP_GetDefaultPowerConfiguration (app_power_config_t *pwr);
void APP_ActiveLoop (app_power_config_t *pwr);
void APP_InitMainClock(app_clk_src_t mainClk);
void APP_ActiveModeDisablePeripherals (app_power_config_t *pwr);
void APP_InitSystemSRAM (app_power_config_t *pwr);
void APP_InitWakeupSource(void);
void APP_LabCustomUseCase(void);
void print_enabled(bool status);


menu_status_t   nextMenu;
app_power_config_t  pwrConfig;
/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief Main function
 */
/* ToDo list of tasks still needed
 *      Tasks after ready for hands-on training
 *          - Manage AHBCLKCTRL0 clocks to SRAMs with SRAM rescs.
 *          - DVFS and Active mode features
 *          - See if app will fit in less RAMA
 *          - add these updates to the SDK drivers fsl_spc/fsl_cmc:
 *              - SCG_LDO, Powers OSC_SYS, PLL0 and PLL1 from the SYSTEM domain when LDOCSR[LDOEN] = 1
 *              - LP_IREF, used by CMP0/1/2, and FRO_12M, disabled by LP_CFG[LP_IREFEN]
 *              - LPBUFF_EN, CMP Bandgap Buffer Enable, Enables the buffer-stored reference voltage to CMP.
 */
int main(void)
{
    app_reset_src_t resetSrc;
    status_t status;

    BOARD_EVK_InitPins();
    BOARD_InitDebugConsole();

    APP_OptimizePower();

    /* Check if waking from Deep Power Down mode */
    resetSrc = APP_GetResetSource();
    if (kAPP_Wakeup_Reset == resetSrc)
    {
        PRINTF("\r\n----------------- Woke from Deep Power Down ---------------\r\n");
        FORCE_RESOURCE_UPDATE(g_pmHndle);
        
        /* Need to reinitialize timer hardware after reset */
        status = PM_DisableWakeupSource(&g_lptmr0WakeupSource);
        assert(status == kStatus_PMSuccess);
        status = PM_EnableWakeupSource(&g_lptmr0WakeupSource);
        assert(status == kStatus_PMSuccess);
    }
    else
    {
        PRINTF("\r\n#######    MCX Nx4x Power Manager component demo    #######\r\n");
        PRINTF("\r\n----------------------- Normal Boot -----------------------\r\n");

        PM_CreateHandle(&g_pmHndle);
        PM_EnablePowerManager(true);

        /* Initialize PM callback notification */
        g_notify_element.notifyCallback = APP_PowerSwitchNotification;
        status = PM_RegisterNotify(kPM_NotifyGroup0, &g_notify_element);
        assert(status == kStatus_PMSuccess);

        APP_InitWakeupSource();

        /* Set initial default resource constraints */
        PM_SetConstraints(PM_LP_STATE_NO_CONSTRAINT, APP_DEFAULT_RESCS);

        APP_GetDefaultPowerConfiguration (&pwrConfig);
    }


    /* This is function is used in a lab.  See lab guide */
    APP_LabCustomUseCase();

    while (1)
    {
        menu_main(&pwrConfig);

        APP_InitSystemSRAM(&pwrConfig);

        if(pwrConfig.powerMode == kAPP_Active)
        {
            APP_ActiveLoop(&pwrConfig);
        }

        APP_PrintPowerModeToEnter(DURATION_SECONDS(g_pmDuration));
        PM_EnterLowPower(DURATION_SECONDS(g_pmDuration));
        PRINTF("Woke from low-power mode\n\r"); 
    }
}


void menu_main(app_power_config_t *pwr)
{
    uint8_t         userInput;
        
    do 
    {
        PRINTF("\r\nMain Menu, press a key below: \n\r\n");
        PRINTF("\tp: Power Modes Menu \r\n");
        PRINTF("\ta: Analog Peripherals Menu\r\n");
        PRINTF("\tc: Clocks Menu      \r\n");
        //PRINTF("\tv: Voltage Menu     \r\n");
        PRINTF("\ts: System SRAM Menu \r\n");
        PRINTF("\te: Enter Selected Mode \r\n");
        
        do
        {
            userInput = GETCHAR();
        } while(userInput == 0xFFU); /* addresses issue after wake-up when 1st char is 0xFF */

        switch (userInput)
        {
            case 'p':
            case 'P':
                nextMenu = kMenu_Power;
                nextMenu = menu_power(pwr);
                break;
            
            case 'a':
            case 'A':
                nextMenu = kMenu_Analog;
                nextMenu = menu_analog(pwr);
                break;

            case 'c':
            case 'C':
                nextMenu = kMenu_Clocks;
                nextMenu = menu_clocks(pwr);
                break;
/*
            case 'v':
            case 'V':
                nextMenu = kMenu_Voltage;
                nextMenu = menu_voltage(pwr);
                break;
*/
            case 's':
            case 'S':
                nextMenu = kMenu_SRAM;
                nextMenu = menu_sram(pwr);
                break;

            case 'e':
            case 'E':
                nextMenu = kMenu_Done;
                break;

            default:
                PRINTF("Invalid input \"%c\", please re-try!\r\n", userInput);
                nextMenu = kMenu_Main;
                break;

        }
    }while(nextMenu != kMenu_Done);

}

menu_status_t menu_power(app_power_config_t *pwr)
{
    uint8_t userInput = '0';
    app_power_mode_t user_mode;

    do 
    {
    	PRINTF("\r\nPower Menu, press a key below: \n\r");
        PRINTF("\tCurrent Selection = %s \r\n\n", powerStrings[pwr->powerMode]);
        PRINTF("\t0. Choose a power mode       \r\n");
        PRINTF("\te. Enter selected power mode \r\n");
        PRINTF("\tw. Toggle CORE_WAKE domain constraint, currently ");
            print_enabled(PM_GetRescEnabled(kResc_CORE_WAKE));
        PRINTF("\r\n\t\tOnly applies in Power Down mode. If set, WAKE domain will\r\n");
        PRINTF("\t\tbe in Deep Sleep mode. Otherwise, WAKE domain will be in\r\n");
        PRINTF("\t\tPower Down mode\r\n");

        PRINTF("\r\n\n\tb. Back to main menu         \r\n");
        
        while (!(kLPUART_TransmissionCompleteFlag & LPUART_GetStatusFlags((LPUART_Type *)BOARD_DEBUG_UART_BASEADDR)));
        userInput = GETCHAR();
        
        switch (userInput)
        {
        case '0':
            user_mode = APP_GetPowerModefromUser();

            if(user_mode != pwr->powerMode)
            {
                /* Release any existing power mode constraint */
                switch (pwr->powerMode)
                {
                    case kAPP_Sleep:
                        PM_ReleaseConstraints(PM_LP_STATE_SLEEP, 0);
                        break;

                    case kAPP_DeepSleep:
                        PM_ReleaseConstraints(PM_LP_STATE_DEEP_SLEEP, 0);
                        break;

                    case kAPP_PowerDown:
                        PM_ReleaseConstraints(PM_LP_STATE_POWER_DOWN, 0);
                        break;

                    case kAPP_DeepPowerDown:
                        PM_ReleaseConstraints(PM_LP_STATE_DEEP_POWER_DOWN, 0);
                        break;

                    default:
                        break;
                }

                /* Set new power mode constraint */
                switch (user_mode)
                {
                    case kAPP_Sleep:
                        PM_SetConstraints(PM_LP_STATE_SLEEP, 0);
                        break;

                    case kAPP_DeepSleep:
                        PM_SetConstraints(PM_LP_STATE_DEEP_SLEEP, 0);
                        break;

                    case kAPP_PowerDown:
                        PM_SetConstraints(PM_LP_STATE_POWER_DOWN, 0);
                        break;

                    case kAPP_DeepPowerDown:
                        PM_SetConstraints(PM_LP_STATE_DEEP_POWER_DOWN, 0);
                        break;

                    default:
                        break;
                }

                pwr->powerMode = user_mode;
            }

            break;

        case 'e':
        case 'E':
            nextMenu = kMenu_Done;
            break;

        case 'b':
        case 'B':
            nextMenu = kMenu_Back;
            break;
        
        case 'w':
        case 'W':
            nextMenu = kMenu_Power;
            PM_ToggleConstraint(kResc_CORE_WAKE);
            PRINTF("\r\nCORE_WAKE constraint ");
                print_enabled(PM_GetRescEnabled(kResc_CORE_WAKE));
            PRINTF("\r\n");
            break;

        default:
            PRINTF("Invalid input, please re-try!\r\n");
            break;
        }

    }while(nextMenu == kMenu_Power);

    return nextMenu;
}


menu_status_t menu_clocks(app_power_config_t *pwr)
{
    uint8_t userInput;
    
    do 
    {
    	PRINTF("\r\nClocks Menu, press a key below: \n\r\n");
        PRINTF("\t0. Main clock \r\n");
        /*PRINTF("\t1. Enabled-Disabled clocks during Active Mode \r\n");
        PRINTF("\t2. Enabled-Disabled clocks during Deep Sleep, Power Down, Deep Power Down Modes\r\n");*/
        PRINTF("\tb. Back to main menu \r\n");
        userInput = GETCHAR();

        switch (userInput)
        {
        case '0':
            pwr->clockConfig.mainClock = APP_GetMainClockfromUser();
            break;
        
        /*case '1':
            pwr->clockConfig.aEnabledClocks = APP_GetEnabledClockfromUser(pwr->powerMode);
            break;
        
        case '2':
            pwr->clockConfig.lpEnabledClocks = APP_GetEnabledClockfromUser(pwr->powerMode);
            break;*/

        case 'b':
        case 'B':
            nextMenu = kMenu_Back;
            break;
        
        default:
            PRINTF("Invalid input, please re-try!\r\n");
            break;
        }
    }while(nextMenu == kMenu_Clocks);
    
    return nextMenu;
}

menu_status_t menu_voltage(app_power_config_t *pwr)
{
    uint8_t userInput;    
   
    do 
    {
    	PRINTF("\r\nVoltage Menu, press a key below: \n\r\n");
        PRINTF("\t0. VDD_CORE Source \r\n");
        PRINTF("\t1. VDD_CORE Level \r\n");
        PRINTF("\t2. VDD_CORE Drive Strength \r\n");
        PRINTF("\tb. Back to main menu \r\n");
        userInput = GETCHAR();

        switch (userInput)
        {
        case '0':
            pwr->voltageConfig.VDDCoreSrc = APP_GetVddCoreSrcfromUser();
            break;
        
        case '1':
            pwr->voltageConfig.VDDCoreLvl = APP_GetVddCoreLvlfromUser();
            break;
        
        case '2':
            pwr->voltageConfig.VDDCoreDS = APP_GetVddCoreDriveStregth(pwr->voltageConfig.VDDCoreSrc);
            break;

        case 'b':
        case 'B':
            nextMenu = kMenu_Back;
            break;
        
        default:
            PRINTF("Invalid input, please re-try!\r\n");
            break;
        }
    }while(nextMenu == kMenu_Voltage);

    return nextMenu;
}

menu_status_t menu_sram(app_power_config_t *pwr)
{  
    uint8_t userInput;
    do
    {
        PRINTF("\n\rCurrent register settings: \n\r");
        PRINTF("\tSRAMDIS = 0x%x\n\r", CMC0->SRAMDIS[0]);
        PRINTF("\tSRAMRET = 0x%x\n\r", CMC0->SRAMRET[0]);
    	PRINTF("\r\nSystem SRAM Menu, press a key below: \n\r\n");
        PRINTF("\t0. ENABLE System SRAM:\r\n");
        PRINTF("\t\tSRAM array is enabled; array bit in SRAMDIS cleared.\r\n");
        PRINTF("\t\tEnabled arrays are retained down to Deep Sleep mode\r\n");
        PRINTF("\t\tDisabled arrays are not usable or retained in any power mode.\r\n\n");
        PRINTF("\t1. RETAIN System SRAM:\r\n");
        PRINTF("\t\tRetained arrays are also retained in Power Down mode.\r\n");
        PRINTF("\t\tClears array bit in SRAMRET register.\r\n");
        PRINTF("\t\tRetaining the array will also enable the array.\r\n\n");
        //ToDo custom values for SRAMDIS, SRAMRET
        PRINTF("\ta. Apply System SRAM Configuration \r\n");
        PRINTF("\tb. Back to main menu \r\n");
        userInput = GETCHAR();
        switch (userInput)
        {
        case '0':
            PRINTF ("\r\nEnabling arrays for Active, Sleep, and Deep Sleep\r\n");
            pwr->sramConfig.enabled = APP_GetSRAMfromUser();
            if(pwr->sramConfig.enabled < pwr->sramConfig.retained)
            {
                /* When SRAM arrays are disabled, also stop retaining them */
                pwr->sramConfig.retained = pwr->sramConfig.enabled;
            }
            break;

        case '1':
            PRINTF ("\r\nRETAIN the SRAM in A, S, DS, and PD\r\n");
            pwr->sramConfig.retained = APP_GetSRAMfromUser();
            if(pwr->sramConfig.retained > pwr->sramConfig.enabled)
            {
                /* When SRAM arrays are retained, also need to be enabled */
                pwr->sramConfig.enabled = pwr->sramConfig.retained;
            }
            break;
        
        case 'a':
        case 'A':
            APP_InitSystemSRAM(pwr);
            PRINTF("\n\rCurrent register settings: \n\r");
            PRINTF("\tSRAMDIS = 0x%x\n\r", CMC0->SRAMDIS[0]);
            PRINTF("\tSRAMRET = 0x%x\n\r", CMC0->SRAMRET[0]);
            nextMenu = kMenu_Back;

        case 'b':
        case 'B':
            nextMenu = kMenu_Back;
            break;
        
        default:
            PRINTF("Invalid input, please re-try!\r\n");
            break;
        }    
    } while (nextMenu == kMenu_SRAM);

    return nextMenu;
    
}

menu_status_t menu_analog_LP(app_power_config_t *pwr)
{  
    uint8_t userInput;
    do
    {
        PRINTF("\n\rAnalog peripherals in low-power modes\n\r");
        PRINTF("Current register settings:\n\r");
    	PRINTF("\tACTIVE_CFG1 = 0x%x\r\n", SPC0->ACTIVE_CFG1);
    	PRINTF("\tLP_CFG1     = 0x%x\r\n", SPC0->LP_CFG1);
    	PRINTF("Listing current status.  Press a key to toggle that status:\r\n");
        PRINTF("\t0. Toggle VREF,\t\t currently ");
            print_enabled(PM_GetRescEnabled(kResc_VREF));
        PRINTF("\r\n\t1. Toggle DAC0,\t\t currently ");
            print_enabled(PM_GetRescEnabled(kResc_DAC0));
        PRINTF("\r\n\t2. Toggle DAC1,\t\t currently ");
            print_enabled(PM_GetRescEnabled(kResc_DAC1));
        PRINTF("\r\n\t3. Toggle DAC2,\t\t currently ");
            print_enabled(PM_GetRescEnabled(kResc_DAC2));
        PRINTF("\r\n\t4. Toggle OPAMP0,\t currently ");
            print_enabled(PM_GetRescEnabled(kResc_OPAMP0));
        PRINTF("\r\n\t5. Toggle OPAMP1,\t currently ");
            print_enabled(PM_GetRescEnabled(kResc_OPAMP1));
        PRINTF("\r\n\t6. Toggle OPAMP2,\t currently ");
            print_enabled(PM_GetRescEnabled(kResc_OPAMP2));
        PRINTF("\r\n\t7. Toggle CMP0,\t\t currently ");
            print_enabled(PM_GetRescEnabled(kResc_CMP0));
        PRINTF("\r\n\t8. Toggle CMP1,\t\t currently ");
            print_enabled(PM_GetRescEnabled(kResc_CMP1));
        PRINTF("\r\n\t9. Toggle CMP2,\t\t currently ");
            print_enabled(PM_GetRescEnabled(kResc_CMP2));

        PRINTF("\r\n\tb. Back to main menu \r\n");
        userInput = GETCHAR();
        switch (userInput)
        {
            case 'b':
            case 'B':
                nextMenu = kMenu_Main;
                break;

            case '0':
                PM_ToggleConstraint(kResc_VREF);
                if(PM_GetRescEnabled(kResc_VREF))
                    App_Enable_VREF();
                break;

            case '1':
                PM_ToggleConstraint(kResc_DAC0);
                if(PM_GetRescEnabled(kResc_DAC0))
                    App_Enable_DAC(DAC0);
                break;

            case '2':
                PM_ToggleConstraint(kResc_DAC1);
                if(PM_GetRescEnabled(kResc_DAC1))
                    App_Enable_DAC(DAC1);
                break;

            case '3':
                PM_ToggleConstraint(kResc_DAC2);
                if(PM_GetRescEnabled(kResc_DAC2))
                    App_Enable_DAC14();
                break;

            case '4':
                PM_ToggleConstraint(kResc_OPAMP0);
                if(PM_GetRescEnabled(kResc_OPAMP0))
                    App_Enable_OpAmp(OPAMP0);
                break;

            case '5':
                PM_ToggleConstraint(kResc_OPAMP1);
                if(PM_GetRescEnabled(kResc_OPAMP1))
                    App_Enable_OpAmp(OPAMP1);
                break;

            case '6':
                PM_ToggleConstraint(kResc_OPAMP2);
                if(PM_GetRescEnabled(kResc_OPAMP2))
                    App_Enable_OpAmp(OPAMP2);
                break;

            case '7':
                PM_ToggleConstraint(kResc_CMP0);
                if(PM_GetRescEnabled(kResc_CMP0))
                    App_Enable_CMP(CMP0);
                break;
                break;

            case '8':
                PM_ToggleConstraint(kResc_CMP1);
                if(PM_GetRescEnabled(kResc_CMP1))
                    App_Enable_CMP(CMP1);
                break;

            case '9':
                PM_ToggleConstraint(kResc_CMP2);
                if(PM_GetRescEnabled(kResc_CMP2))
                    App_Enable_CMP(CMP2);
                break;

            default:
                PRINTF("Invalid input, please re-try!\r\n");
                break;
        }

        FORCE_RESOURCE_UPDATE(g_pmHndle);
    } while (nextMenu == kMenu_Analog_LP);

    return nextMenu;
    
}

menu_status_t menu_analog_Act(app_power_config_t *pwr)
{  
    uint8_t userInput;
    do
    {
        PRINTF("\n\rAnalog peripherals in Active mode\n\r");
        PRINTF("Current register settings:\n\r");
    	PRINTF("\tACTIVE_CFG1 = 0x%x\r\n", SPC0->ACTIVE_CFG1);
    	PRINTF("\tLP_CFG1     = 0x%x\r\n", SPC0->LP_CFG1);
    	PRINTF("Listing active status.  Press a key to toggle that status:\r\n");
        PRINTF("\r\n\t0. Toggle VREF,\t\t currently ");
            print_enabled(App_GetAnalogActive(kSPC_controlVref));
        PRINTF("\r\n\t1. Toggle DAC0,\t\t currently ");
            print_enabled(App_GetAnalogActive(kSPC_controlDac0));
        PRINTF("\r\n\t2. Toggle DAC1,\t\t currently ");
            print_enabled(App_GetAnalogActive(kSPC_controlDac1));
        PRINTF("\r\n\t3. Toggle DAC2,\t\t currently ");
            print_enabled(App_GetAnalogActive(kSPC_controlDac2));
        PRINTF("\r\n\t4. Toggle OPAMP0,\t currently ");
            print_enabled(App_GetAnalogActive(kSPC_controlOpamp0));
        PRINTF("\r\n\t5. Toggle OPAMP1,\t currently ");
            print_enabled(App_GetAnalogActive(kSPC_controlOpamp1));
        PRINTF("\r\n\t6. Toggle OPAMP2,\t currently ");
            print_enabled(App_GetAnalogActive(kSPC_controlOpamp2));
        PRINTF("\r\n\t7. Toggle CMP0,\t\t currently ");
            print_enabled(App_GetAnalogActive(kSPC_controlCmp0));
        PRINTF("\r\n\t8. Toggle CMP1,\t\t currently ");
            print_enabled(App_GetAnalogActive(kSPC_controlCmp1));
        PRINTF("\r\n\t9. Toggle CMP2,\t\t currently ");
            print_enabled(App_GetAnalogActive(kSPC_controlCmp2));

        PRINTF("\r\n\tb. Back to main menu \r\n");
        userInput = GETCHAR();
        switch (userInput)
        {
            case 'b':
            case 'B':
                nextMenu = kMenu_Main;
                break;

            case '0':
                if(App_GetAnalogActive(kSPC_controlVref))
                {
                    App_Disable_VREF();
                    PM_ReleaseConstraints(PM_LP_STATE_NO_CONSTRAINT, 1U, PM_RESC_VREF_ON);
                } else
                {
                    App_Enable_VREF();
                }
                break;

            case '1':
                if(App_GetAnalogActive(kSPC_controlDac0))
                {
                    App_Disable_DAC(DAC0);
                    PM_ReleaseConstraints(PM_LP_STATE_NO_CONSTRAINT, 1U, PM_RESC_DAC0_ON);
                } else
                {
                    App_Enable_DAC(DAC0);
                }
                break;

            case '2':
                if(App_GetAnalogActive(kSPC_controlDac1))
                {
                    App_Disable_DAC(DAC1);
                    PM_ReleaseConstraints(PM_LP_STATE_NO_CONSTRAINT, 1U, PM_RESC_DAC1_ON);
                } else
                {
                    App_Enable_DAC(DAC1);
                }
                break;

            case '3':
                if(App_GetAnalogActive(kSPC_controlDac2))
                {
                    App_Disable_DAC14();
                    PM_ReleaseConstraints(PM_LP_STATE_NO_CONSTRAINT, 1U, PM_RESC_DAC2_ON);
                } else
                {
                    App_Enable_DAC14();
                }
                break;

            case '4':
                if(App_GetAnalogActive(kSPC_controlOpamp0))
                {
                    App_Disable_OpAmp(OPAMP0);
                    PM_ReleaseConstraints(PM_LP_STATE_NO_CONSTRAINT, 1U, PM_RESC_OPAMP0_ON);
                } else
                {
                    App_Enable_OpAmp(OPAMP0);
                }
                break;

            case '5':
                if(App_GetAnalogActive(kSPC_controlOpamp1))
                {
                    App_Disable_OpAmp(OPAMP1);
                    PM_ReleaseConstraints(PM_LP_STATE_NO_CONSTRAINT, 1U, PM_RESC_OPAMP1_ON);
                } else
                {
                    App_Enable_OpAmp(OPAMP1);
                }
                break;

            case '6':
                if(App_GetAnalogActive(kSPC_controlOpamp2))
                {
                    App_Disable_OpAmp(OPAMP2);
                    PM_ReleaseConstraints(PM_LP_STATE_NO_CONSTRAINT, 1U, PM_RESC_OPAMP2_ON);
                } else
                {
                    App_Enable_OpAmp(OPAMP2);
                }
                break;

            case '7':
                if(App_GetAnalogActive(kSPC_controlCmp0))
                {
                    App_Disable_CMP(CMP0);
                    PM_ReleaseConstraints(PM_LP_STATE_NO_CONSTRAINT, 1U, PM_RESC_CMP0_ON);
                } else
                {
                    App_Enable_CMP(CMP0);
                }
                break;

            case '8':
                if(App_GetAnalogActive(kSPC_controlCmp1))
                {
                    App_Disable_CMP(CMP1);
                    PM_ReleaseConstraints(PM_LP_STATE_NO_CONSTRAINT, 1U, PM_RESC_CMP1_ON);
                } else
                {
                    App_Enable_CMP(CMP1);
                }
                break;

            case '9':
                if(App_GetAnalogActive(kSPC_controlCmp2))
                {
                    App_Disable_CMP(CMP2);
                    PM_ReleaseConstraints(PM_LP_STATE_NO_CONSTRAINT, 1U, PM_RESC_CMP2_ON);
                } else
                {
                    App_Enable_CMP(CMP2);
                }
                break;

            default:
                PRINTF("Invalid input, please re-try!\r\n");
                break;
        }

        FORCE_RESOURCE_UPDATE(g_pmHndle);
    } while (nextMenu == kMenu_Analog_Act);

    return nextMenu;
}

menu_status_t menu_analog(app_power_config_t *pwr)
{  
    uint8_t userInput;
    do
    {
        PRINTF("\n\rAnalog peripherals\n\r");
        PRINTF("Current register settings:\n\r");
    	PRINTF("\tACTIVE_CFG1 = 0x%x\r\n", SPC0->ACTIVE_CFG1);
    	PRINTF("\tLP_CFG1     = 0x%x\r\n", SPC0->LP_CFG1);
    	PRINTF("\t0: Change peripherals enabled in ACTIVE    mode\r\n");
    	PRINTF("\t1: Change peripherals enabled in Low-Power modes\r\n");

        PRINTF("\r\n\tb. Back to main menu \r\n");
        userInput = GETCHAR();
        switch (userInput)
        {
            case 'b':
            case 'B':
                nextMenu = kMenu_Main;
                break;

            case '0':
                nextMenu = kMenu_Analog_Act;
                nextMenu = menu_analog_Act(pwr);
                break;

            case '1':
                nextMenu = kMenu_Analog_LP;
                nextMenu = menu_analog_LP(pwr);
                break;

            default:
                PRINTF("Invalid input, please re-try!\r\n");
                break;
        }
    } while (nextMenu == kMenu_Analog);

    return nextMenu;
}

void print_enabled(bool status)
{
    if(status)
        PRINTF("enabled");
    else
        PRINTF("disabled");
}

/*!
 * @brief hook called very early in startup code
 */
void SystemInitHook(void)
{
    uint32_t status;

    /* Set WOKEN pin high */
    /* enable clocks to WOKEN pin Port and GPIO peripherals */
    SYSCON0->AHBCLKCTRLSET[0] = (BOARD_WOKEN_PORT_CLOCK | BOARD_WOKEN_GPIO_CLOCK);
    /* Set pin mux for WOKEN pin */
    BOARD_WOKEN_PORT->PCR[BOARD_WOKEN_GPIO_PIN] = 0;
    /* Set WOKEN GPIO as output and logic on */
    BOARD_WOKEN_GPIO->PSOR  = (LOGIC_LED_ON << BOARD_WOKEN_GPIO_PIN);
    BOARD_WOKEN_GPIO->PDDR |= (1 << BOARD_WOKEN_GPIO_PIN);

    /* Check if waking from Deep Power Down mode */
    status = CMC_GetStickySystemResetStatus(CMC0);
    if((status & CMC_SSRS_WAKEUP_MASK) == CMC_SSRS_WAKEUP_MASK)
    {
        /* With pins configured, clear SPC isolation in case waking from Deep Power Down */
        SPC0->SC |= SPC_SC_ISO_CLR_MASK;

        /* RAM already initialized, skip the copydown and runtime library init */
        __asm volatile("cpsie i");  /* Enable interrupts */
        main();
    }
}

void APP_GetDefaultPowerConfiguration (app_power_config_t *pwr)
{
    pwr->powerMode = kAPP_Active;
    /*Clock*/
    pwr->clockConfig.mainClock = kAPP_FRO48M;
    pwr->clockConfig.aEnabledClocks = FRO_144M_MASK | FRO_12M_MASK;
    pwr->clockConfig.lpEnabledClocks = 0;
    /*Voltage*/
    pwr->voltageConfig.VDDSysLvl  = kAPP_1p8V;
    pwr->voltageConfig.VDDSysDS   = kAPP_LDOSys_40mA;
    pwr->voltageConfig.VDDCoreSrc = kAPP_DCDC;
    pwr->voltageConfig.VDDCoreLvl = kAPP_1p20V;
    pwr->voltageConfig.VDDCoreDS  = kAPP_DCDC_100mA;
    /*System SRAM*/
    pwr->sramConfig.retained = kAPP_SRAM_0kB;
    pwr->sramConfig.enabled = kAPP_SRAM_0kB;
}


void APP_ActiveLoop (app_power_config_t *pwr)
{
    app_voltage_t initialVoltage;
        
    /* 1. Go to a safe VDD_CORE high drive = 1.2V*/
    initialVoltage.VDDSysLvl = kAPP_1p8V;
    initialVoltage.VDDSysDS = kAPP_LDOSys_40mA;
    initialVoltage.VDDCoreLvl = kAPP_1p20V;
    initialVoltage.VDDCoreDS = kAPP_DCDC_100mA;
    APP_SetRegulatorsVoltage(initialVoltage, kAPP_Active);

    /* 2. Set main clock frequency */
    APP_InitMainClock(pwr->clockConfig.mainClock);
    if (pwr->clockConfig.mainClock >= kAPP_FRO144M)
    {
    	PRINTF("    VDD_CORE = 1.2V \r\n\r\n");
    }

    /* 3. Adjust the voltage to the selected user configuration */
    APP_FindLowestVDDCoreVoltage(pwr);
    APP_CheckValidVoltageConfig(pwr);
    APP_SetRegulatorsVoltage(pwr->voltageConfig, kAPP_Active);



    /* 4. Disable un-used SRAM and peripheral RAM */
    APP_InitSystemSRAM(&pwrConfig);
    
    /* 5. Disable un-used peripherals */
    APP_ActiveModeDisablePeripherals(pwr);

    while (1)
    {

    }
}

void APP_InitMainClock(app_clk_src_t mainClk)
{
    uint32_t freq;
    
    switch (mainClk)
    {
    case kAPP_PLL150M:
        BOARD_BootClockPLL150M();
        break;

    case kAPP_FRO144M:
        BOARD_BootClockFROHF144M();
        break;
    
    case kAPP_PLL100M:
        BOARD_BootClockPLL100M();
        break;
    
    case kAPP_FRO48M:
        BOARD_BootClockFROHF48M();
        break;

    case kAPP_FRO12M:
        BOARD_BootClockFRO12M();
        break;
    
    default:
        break;
    }

    PRINTF("\r\nMain Clock Source is %s \r\n", clockStrings[mainClk]);
    freq = CLOCK_GetFreq(kCLOCK_CoreSysClk);
    PRINTF("    Core Clock = %dHz \r\n", freq);

}


void APP_ActiveModeDisablePeripherals (app_power_config_t *pwr)
{
    /* a. PLL LDO */
    if (kAPP_PLL150M != pwr->clockConfig.mainClock || kAPP_PLL100M != pwr->clockConfig.mainClock)
    {
        SCG0->LDOCSR &= ~(uint32_t)(SCG_LDOCSR_LDOEN_MASK);
    }

    /* b. LDO_CORE regulator if using DCDC or PMIC */
    if (kAPP_LDO != pwr->voltageConfig.VDDCoreSrc)
    {
        SPC_EnableCoreLDORegulator(SPC0, false);
    }

    /* c. Disable UART */
    APP_DeinitDebugConsole();

    /* d. TurnOFF Wakeup LED */
    GPIO_PinWrite(BOARD_WOKEN_GPIO, BOARD_WOKEN_GPIO_PIN, LOGIC_LED_OFF);

}

void APP_InitSystemSRAM (app_power_config_t *pwr)
{
    /* The PM framework manages the Retained arrays, and configures SRAMRET.
     * The application must manage Enabled arrays and configures SRAMDIS. */
    uint32_t enabledRAMs = APP_INIT_SRAMS_ENABLED;

    switch (pwr->sramConfig.enabled)
    {
        case kAPP_SRAM_0kB:
            break;

        case kAPP_SRAM_32kB:
            enabledRAMs |= kCMC_RAMX0;
            break;
    
        case kAPP_SRAM_64kB:
            enabledRAMs |= (kCMC_RAMX0 | kCMC_RAMX1);
            break;

        case kAPP_SRAM_256kB:
            enabledRAMs |= ENABLED_HALF_SRAM;
            break;

        case kAPP_SRAM_480kB:
            enabledRAMs |= ENABLED_ALL_SRAM;
            break;
    
        default:
            PRINTF("!!!ERROR Invalid SRAM LOW_POWER configuration \r\n");
            break;
    }

    PM_RELEASE_ALL_SRAM_CONSTRAINTS;
    switch (pwr->sramConfig.retained)
    {
        case kAPP_SRAM_0kB:
            break;

        case kAPP_SRAM_32kB:
            enabledRAMs |= kCMC_RAMX0;
            PM_SetConstraints   (PM_LP_STATE_NO_CONSTRAINT, 1U, PM_RESC_RAMX0_32K_RETAINED);
            break;
    
        case kAPP_SRAM_64kB:
            enabledRAMs |= (kCMC_RAMX0 | kCMC_RAMX1);
            PM_SetConstraints   (PM_LP_STATE_NO_CONSTRAINT, 2U, 
                            PM_RESC_RAMX0_32K_RETAINED, PM_RESC_RAMX1_32K_RETAINED);
            break;

        case kAPP_SRAM_256kB:
            enabledRAMs |= ENABLED_HALF_SRAM;
            PM_SetConstraints   (PM_LP_STATE_NO_CONSTRAINT, PM_RESC_SRAM_HALF_NUM, 
                                PM_RESC_SRAM_HALF(PM_RESOURCE_FULL_ON));
            break;

        case kAPP_SRAM_480kB:
            enabledRAMs |= ENABLED_ALL_SRAM;
            PM_SetConstraints   (PM_LP_STATE_NO_CONSTRAINT, PM_RESC_SRAM_ALL_NUM, 
                                PM_RESC_SRAM_ALL(PM_RESOURCE_FULL_ON));
            break;
    
        default:
            PRINTF("!!!ERROR Invalid SRAM RETAINED configuration \r\n");
            break;
    }
    CMC_PowerOnSRAMAllMode        (CMC0,  enabledRAMs);
    CMC_PowerOffSRAMAllMode_to_add(CMC0, ~enabledRAMs);
    FORCE_RESOURCE_UPDATE(g_pmHndle);
}

void APP_InitWakeupSource(void)
{
    PM_InitWakeupSource(&g_lptmr0WakeupSource, PM_WSID_LPTMR0, APP_Lptmr0WakeupService, true);
    PM_RegisterTimerController(&g_pmHndle, APP_StartLptmr, APP_StopLptmr, NULL, NULL);
}

/*******************************************************************************
 * ToDo Menus to add:
 * 
 * Main Menu:
 *      Memory
 * Memory Menu:
 *      1: RAMA VBAT SRAM
 *      2: Flash
 *      3: Monitors
 *      4: Other Peripherals
 * 
 *      RAMA: VBAT SRAM Menu:
 *          prints current RAMA retention settings
 *          1: RAMA1 - toggle retention setting
 *          2: RAMA2 - toggle retention setting
 *          3: RAMA3 - toggle retention setting
 *          4: RAMA4 - toggle retention setting
 *          a: retain All RAMA
 *          n: No RAMA retention
 *          m: Main Menu
 *          b: Back
 *          prints again after selection
 * 
 *      Clocks menu:
 *      Flash Menu:
 *      Regulators Menu:
 *          Drive strengths for 3 regulators
 *          Voltage levels
 * 
 *      Timer Menu:
 *          Change duration in low-power mode
 *
 *      Monitors Menu:
 *          Enable/Disable these monitors:
 *          kResc_GDET,     
 *          kResc_HVD_CORE, 
 *          kResc_HVD_SYS,  
 *          kResc_HVD_IO,   
 *          kResc_LVD_CORE, 
 *          kResc_LVD_SYS,  
 *          kResc_LVD_IO,   
 *          kResc_IO_Det,   
 *          
 *      Peripherals Menu:
 *          Enable/Disable these peripherals:
 *          kResc_ADC,    
 *          kResc_SINC,   
 *          kResc_BG_CORE,
 *          kResc_BG_VBAT,
*******************************************************************************/

