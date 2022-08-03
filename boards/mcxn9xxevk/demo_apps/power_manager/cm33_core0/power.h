/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _POWER_H_
#define _POWER_H_

#include "fsl_common.h"
#include "fsl_pm_board.h"
#include "fsl_pm_core.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define APP_INIT_SRAMS_ENABLED  kCMC_LPCAC	/* All System SRAMs disabled except LPCAC, ToDo disable LPCAC */
#define APP_INIT_SRAM_CLOCKS    0U              /* All System SRAM clocks disabled */

typedef enum
{
    kAPP_Active = 0,        /* Active */
    kAPP_Sleep,             /* Sleep  */
    kAPP_DeepSleep,         /* Deep Sleep  */
    kAPP_PowerDown,         /* Power Down */
    kAPP_DeepPowerDown,     /* Deep Power Down */
    kAPP_VBAT               /* VBAT */
}app_power_mode_t;

typedef enum
{
    kAPP_FRO12M=0,      /*FRO @  12 MHz clock */
    kAPP_FRO48M,        /*FRO @  48 MHz clock */
    kAPP_PLL100M,       /*PLL @ 100 MHz clock */
    kAPP_FRO144M,       /*FRO @ 144 MHz clock */
    kAPP_PLL150M        /*PLL @ 150 MHz clock */
}app_clk_src_t;

typedef enum
{
    kAPP_DCDC = 0, 
    kAPP_LDO, 
    kAPP_PMIC
}app_vdd_core_src_t;

typedef enum 
{
    kAPP_reserved = 0,
    kAPP_1p00V,
    kAPP_1p10V,
    kAPP_1p20V
}app_vdd_core_lvl_t;

typedef enum 
{
    kAPP_DCDC_PulseRefresh = 0,
    kAPP_DCDC_15mA = 1,
    kAPP_DCDC_100mA = 2,
    kAPP_LDO_LowDS = 0,
    kAPP_LDO_NormalDS = 1
}app_vdd_core_ds_t;

typedef enum
{
    kAPP_1p8V,
    kAPP_2p5V
}app_vdd_sys_lvl_t;

typedef enum 
{
    kAPP_LDOSys_2mA = 0,
    kAPP_LDOSys_40mA =1
}app_vdd_sys_ds_t;

typedef enum 
{
    kAPP_SRAM_0kB = 0,
    kAPP_SRAM_32kB,
    kAPP_SRAM_64kB, 
    kAPP_SRAM_256kB,
    kAPP_SRAM_480kB
}app_sram_size_t;

typedef enum
{
    kAPP_Wakeup_Reset = 0,
    kAPP_POR_Reset,
    kAPP_Warm_Reset
}app_reset_src_t;

typedef struct app_voltage
{
    app_vdd_core_src_t VDDCoreSrc;
    app_vdd_core_lvl_t VDDCoreLvl;
    app_vdd_core_ds_t VDDCoreDS;
    app_vdd_sys_lvl_t VDDSysLvl;
    app_vdd_sys_ds_t  VDDSysDS;
}app_voltage_t;

typedef struct app_clock 
{
    app_clk_src_t mainClock;
    app_clk_src_t aEnabledClocks;
    app_clk_src_t lpEnabledClocks;
}app_clock_t;

typedef struct app_sram
{
    app_sram_size_t retained;
    app_sram_size_t enabled;
}app_sram_t;

typedef struct power_config
{
    app_power_mode_t    powerMode;
    app_clock_t         clockConfig;
    app_voltage_t       voltageConfig;
    app_sram_t          sramConfig;
}app_power_config_t;

#define FRO_12M_MASK      (1<<kAPP_FRO12M)
#define FRO_48M_MASK      (1<<kAPP_FRO48M)
#define PLL_100M_MASK     (1<<kAPP_PLL100M)
#define FRO_144M_MASK     (1<<kAPP_FRO144M)
#define PLL_150M_MASK     (1<<kAPP_PLL100M)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

extern const char powerStrings [6][16];
extern const char clockStrings [5][19];

app_power_mode_t APP_GetPowerModefromUser (void);
app_clk_src_t APP_GetMainClockfromUser (void);
app_clk_src_t APP_GetEnabledClockfromUser (app_power_mode_t powerMode);
app_vdd_core_src_t APP_GetVddCoreSrcfromUser (void);
app_vdd_core_lvl_t APP_GetVddCoreLvlfromUser (void);
app_vdd_sys_ds_t APP_GetVddCoreDriveStregth (app_vdd_core_src_t vddSrc);
app_sram_size_t APP_GetSRAMfromUser (void);
void APP_ConfigureSRAMclks(uint32_t enabledRAMs);

void APP_FindLowestVDDCoreVoltage(app_power_config_t *pwr);
void APP_CheckValidVoltageConfig(app_power_config_t *pwr);
status_t APP_SetRegulatorsVoltage (app_voltage_t volt, app_power_mode_t powerMode);

app_reset_src_t APP_GetResetSource(void);
void APP_InitDebugConsole(void);
void APP_DeinitDebugConsole(void);
status_t APP_PowerSwitchNotification(pm_event_type_t eventType, uint8_t powerState, void *data);
void APP_PrintPowerModeToEnter(uint64_t duration);
void APP_OptimizePower(void);
bool PM_GetRescEnabled(resc_name_t resc);
void PM_ToggleConstraint(resc_name_t resc);



#endif //_POWER_H_
