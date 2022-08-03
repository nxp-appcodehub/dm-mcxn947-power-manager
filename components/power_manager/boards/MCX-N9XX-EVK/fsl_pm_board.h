/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _FSL_PM_BOARD_H_
#define _FSL_PM_BOARD_H_

#include "fsl_common.h"

#include "fsl_pm_config.h"
#include "fsl_pm_board_config.h"

/*!
 * @addtogroup PM Framework: Power Manager Framework
 * @brief This section includes Power Mode macros, System Constraints macros, and Wakeup source macros.
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*!
 * @name Power Mode Definition
 * @{
 */

/* Power Mode Index */
#define PM_LP_STATE_SLEEP              (0U)
#define PM_LP_STATE_DEEP_SLEEP         (1U)
#define PM_LP_STATE_POWER_DOWN_WAKE_DS (2U)
#define PM_LP_STATE_POWER_DOWN_WAKE_PD (3U)
#define PM_LP_STATE_DEEP_POWER_DOWN    (4U)
#define PM_LP_STATE_VBAT               (5U)
#define PM_LP_STATE_NO_CONSTRAINT      (0xFFU)

/* Give alias for simplified Power Down Mode */
#define PM_LP_STATE_POWER_DOWN         PM_LP_STATE_POWER_DOWN_WAKE_PD

/* @} */

/*! @brief Helper macros
 * @{
 */
#define PM_RESC_MASK(resc_masks,   rescIndex) ((resc_masks->rescMask[rescIndex / 32UL] >> (rescIndex % 32UL)) & 1UL)
#define PM_RESC_GROUP(resc_groups, rescIndex) (resc_groups->groupSlice[rescIndex / 8UL] >> (4UL * (rescIndex % 8UL))) & 0xFUL
/* @} */

// ToDo need to add dedicated peripheral SRAMs, like FlexSPI, USB, etc.
/*! @brief Available constraints for resources 
 * @{
 */

typedef enum _resc_name
{
    kResc_BUS_SYS_Clk,          /*!<    Bus/System Clocks */
    kResc_CORE_WAKE,            /*!<    CORE_WAKE Domain */

    /* These SRAM rescs must remain in this order */
    kResc_SRAM_RAMA0_8K,        /*!<    RAMA0 SRAM */
    kResc_SRAM_RAMA1_8K,        /*!<    RAMA1 SRAM */
    kResc_SRAM_RAMA2_8K,        /*!<    RAMA2 SRAM */
    kResc_SRAM_RAMA3_8K,        /*!<    RAMA3 SRAM */
    kResc_SRAM_RAMX0_32K,       /*!<    RAMX0 SRAM */
    kResc_SRAM_RAMX1_32K,       /*!<    RAMX1 SRAM */
    kResc_SRAM_RAMX2_32K,       /*!<    RAMX2 SRAM */
    kResc_SRAM_RAMB0_32K,       /*!<    RAMB0 SRAM */
    kResc_SRAM_RAMC0_32K,       /*!<    RAMC0 SRAM */
    kResc_SRAM_RAMC1_32K,       /*!<    RAMC1 SRAM */
    kResc_SRAM_RAMD0_32K,       /*!<    RAMD0 SRAM */
    kResc_SRAM_RAMD1_32K,       /*!<    RAMD1 SRAM */
    kResc_SRAM_RAME0_32K,       /*!<    RAME0 SRAM */
    kResc_SRAM_RAME1_32K,       /*!<    RAME1 SRAM */
    kResc_SRAM_RAMF0_32K,       /*!<    RAMF0 SRAM */
    kResc_SRAM_RAMF1_32K,       /*!<    RAMF1 SRAM */
    kResc_SRAM_RAMG01_32K,      /*!<    RAMG0/1 SRAM */
    kResc_SRAM_RAMG23_32K,      /*!<    RAMG2/3 SRAM */
    kResc_SRAM_RAMH01_32K,      /*!<    RAMH0/1 SRAM */

    kResc_Flash,                /*!<    Flash Memory */
    kResc_DCDC_CORE,            /*!<    DCDC_CORE Regulator */
    kResc_LDO_CORE,             /*!<    LDO_CORE Regulator */
    kResc_LDO_SYS,              /*!<    LDO_SYS Regulator */
    kResc_FRO_144M,             /*!<    Internal 144 MHz Oscillator */
    kResc_FRO_12M,              /*!<    Internal  12 MHz Oscillator */
    kResc_FRO_16K,              /*!<    Internal  16 kHz Oscillator */
    kResc_OSC_RTC,              /*!<    32 kHz Crystal Oscillator */
    kResc_OSC_SYS,              /*!<    High-Frequency Crystal Oscillator */
    kResc_PLL0,                 /*!<    Auxillary APLL */
    kResc_PLL1,                 /*!< 31 System SPLL */
    kResc_VREF,                 /*!< 32 VREF peripheral */
    kResc_USB3V_DET,            /*!<    USB 3V Detect */
    kResc_DAC0,                 /*!<    DAC0 peripheral */
    kResc_DAC1,                 /*!<    DAC1 peripheral */
    kResc_DAC2,                 /*!<    DAC2 peripheral */
    kResc_OPAMP0,               /*!<    OPAMP0 peripheral */
    kResc_OPAMP1,               /*!<    OPAMP1 peripheral */
    kResc_OPAMP2,               /*!<    OPAMP2 peripheral */
    kResc_CMP0,                 /*!<    CMP0 peripheral */
    kResc_CMP1,                 /*!<    CMP1 peripheral */
    kResc_CMP2,                 /*!<    CMP2 peripheral */
    kResc_ADC,                  /*!<    ADC peripheral */
    kResc_SINC,                 /*!<    SINC Filter peripheral */
    kResc_BG_CORE,              /*!<    VDD_CORE Bandgap */
    kResc_BG_VBAT,              /*!<    VBAT Bandgap */
    kResc_GDET,                 /*!<    VDD_CORE Glitch Detect */
    kResc_HVD_CORE,             /*!<    VDD_CORE HVD */
    kResc_HVD_SYS,              /*!<    VDD_SYS HVD */
    kResc_HVD_IO,               /*!<    VDD IO HVD */
    kResc_LVD_CORE,             /*!<    VDD_CORE LVD */
    kResc_LVD_SYS,              /*!<    VDD_SYS LVD */
    kResc_LVD_IO,               /*!<    VDD IO LVD */
    kResc_IO_Det,               /*!<    VDD IO voltage detect */
    kResc_Max_Num               /*!< 55 Maximum Number of Resource Constraints */
} resc_name_t;

/* Helper macros for Resource Contraint Masks */
#define PM_MASK_RESC_RAMA      ((1 << kResc_SRAM_RAMA0_8K)   | (1 << kResc_SRAM_RAMA1_8K)  \
                              | (1 << kResc_SRAM_RAMA2_8K)   | (1 << kResc_SRAM_RAMA3_8K))

#define PM_MASK_RESC_RAMS       (1 << kResc_SRAM_RAMX0_32K)  | (1 << kResc_SRAM_RAMX1_32K) \
                              | (1 << kResc_SRAM_RAMX2_32K)  | (1 << kResc_SRAM_RAMB0_32K) \
                              | (1 << kResc_SRAM_RAMC0_32K)  | (1 << kResc_SRAM_RAMC1_32K) \
                              | (1 << kResc_SRAM_RAMD0_32K)  | (1 << kResc_SRAM_RAMD1_32K) \
                              | (1 << kResc_SRAM_RAME0_32K)  | (1 << kResc_SRAM_RAME1_32K) \
                              | (1 << kResc_SRAM_RAMF0_32K)  | (1 << kResc_SRAM_RAMF1_32K) \
                              | (1 << kResc_SRAM_RAMG01_32K) | (1 << kResc_SRAM_RAMG23_32K) \
                              | (1 << kResc_SRAM_RAMH01_32K) | PM_MASK_RESC_RAMA

#define PM_MASK_RESC_LOWEST_VBAT0       ((1 << kResc_FRO_16K) | (1 << kResc_OSC_RTC) | PM_MASK_RESC_RAMA)
#define PM_MASK_RESC_LOWEST_VBAT1        (1 << (kResc_BG_VBAT-32))

#define PM_MASK_RESC_LOWEST_DPD0       ((1 << kResc_LDO_SYS) | PM_MASK_RESC_LOWEST_VBAT0)
#define PM_MASK_RESC_LOWEST_DPD1       ((1 << (kResc_CMP0-32))    | (1 << (kResc_CMP1-32)) | (1 << (kResc_HVD_SYS-32)) \
                                      | (1 << (kResc_LVD_SYS-32)) | PM_MASK_RESC_LOWEST_VBAT1)

#define PM_MASK_RESC_LOWEST_PDPD0       ((1 << kResc_DCDC_CORE) | (1 << kResc_LDO_CORE) \
                                        | PM_MASK_RESC_RAMS | PM_MASK_RESC_LOWEST_DPD0)
#define PM_MASK_RESC_LOWEST_PDPD1      ((1 << (kResc_USB3V_DET-32)) | (1 << (kResc_BG_CORE-32) ) | (1 << (kResc_GDET-32))   \
                                      | (1 << (kResc_HVD_CORE-32))  | (1 << (kResc_LVD_CORE-32)) | (1 << (kResc_HVD_IO-32)) \
                                      | (1 << (kResc_LVD_IO-32))    | (1 << (kResc_IO_Det-32))   | PM_MASK_RESC_LOWEST_DPD1)

#define PM_MASK_RESC_LOWEST_PDDS0       ((1 << kResc_FRO_12M)   | (1 << kResc_CORE_WAKE) | PM_MASK_RESC_LOWEST_PDPD0)
#define PM_MASK_RESC_LOWEST_PDDS1       ((1 << (kResc_CMP2-32)) | PM_MASK_RESC_LOWEST_PDPD1)

#define PM_MASK_RESC_LOWEST_DEEP_SLEEP0  ((1 <<  kResc_Flash)      | PM_MASK_RESC_LOWEST_PDDS0)
#define PM_MASK_RESC_LOWEST_DEEP_SLEEP1  ((1 << (kResc_DAC0-32))   | (1 << (kResc_DAC1-32))   | (1 << (kResc_DAC2-32))   \
                                        | (1 << (kResc_OPAMP0-32)) | (1 << (kResc_OPAMP1-32)) | (1 << (kResc_OPAMP2-32)) \
                                        | (1 << (kResc_VREF-32))   | (1 << (kResc_SINC-32))   | PM_MASK_RESC_LOWEST_PDDS1)

#define PM_MASK_RESC_LOWEST_SLEEP0      ((1 << kResc_FRO_144M)    | (1 << kResc_BUS_SYS_Clk) | (1 << kResc_OSC_SYS) \
                                      | (1 << kResc_PLL0) | (1 << kResc_PLL1) | PM_MASK_RESC_LOWEST_DEEP_SLEEP0)
#define PM_MASK_RESC_LOWEST_SLEEP1     ((1 << (kResc_ADC-32))  | PM_MASK_RESC_LOWEST_DEEP_SLEEP1)

/* Resource constraints that are not variable, set by the power mode*/
#define PM_MASK_RESC_NOT_VAR0           ~((1 << kResc_BUS_SYS_Clk) & (1 << kResc_CORE_WAKE))
#define PM_MASK_RESC_NOT_VAR1           (0xFFFFFFFFU)

/* Constraints used by application. */
/*!
 * @brief Set the constraint that BUS and SYSTEM clock should be ON.
 */
#define PM_RESC_BUS_SYS_CLK_ON PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_BUS_SYS_Clk)
/*!
 * @brief Constraint only applies in Power Down mode.  Configures if CORE_WAKE
          domain should be in Deep Sleep, or can go into Power Down mode.
 */
#define PM_RESC_CORE_WAKE_DEEP_SLEEP PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_CORE_WAKE)

/*!
 * @brief Set the constraint that the RAMAs can be used in these modes:
 *  ToDo update these RAMA comments and mode names
 *  - RETAINED    - SRAM is retained in Power Down and Deep Sleep modes, Active in Sleep and Active modes
 *  - LOWER_POWER - SRAM is not retained in Power Down mode, is retained in Deep Sleep mode, Active in Sleep and Active modes
 *  - UNUSED      - SRAM is not used by application, powered off in all modes (PM_RESOURCE_OFF)
 */
#define PM_RESC_RAMA0_8K_ACTIVE     PM_ENCODE_RESC(PM_RESOURCE_FULL_ON,      kResc_SRAM_RAMA0_8K)
#define PM_RESC_RAMA1_8K_ACTIVE     PM_ENCODE_RESC(PM_RESOURCE_FULL_ON,      kResc_SRAM_RAMA1_8K)
#define PM_RESC_RAMA2_8K_ACTIVE     PM_ENCODE_RESC(PM_RESOURCE_FULL_ON,      kResc_SRAM_RAMA2_8K)
#define PM_RESC_RAMA3_8K_ACTIVE     PM_ENCODE_RESC(PM_RESOURCE_FULL_ON,      kResc_SRAM_RAMA3_8K)

#define PM_RESC_RAMA0_8K_RETENTION     PM_ENCODE_RESC(PM_RESOURCE_PARTABLE_ON1, kResc_SRAM_RAMA0_8K)
#define PM_RESC_RAMA1_8K_RETENTION     PM_ENCODE_RESC(PM_RESOURCE_PARTABLE_ON1, kResc_SRAM_RAMA1_8K)
#define PM_RESC_RAMA2_8K_RETENTION     PM_ENCODE_RESC(PM_RESOURCE_PARTABLE_ON1, kResc_SRAM_RAMA2_8K)
#define PM_RESC_RAMA3_8K_RETENTION     PM_ENCODE_RESC(PM_RESOURCE_PARTABLE_ON1, kResc_SRAM_RAMA3_8K)

/*!
 * @brief Set the constraint that the SRAM will be retained, configures SRAMRET bits.
 *          This limits deepest power mode to Power Down.
 */
#define PM_RESC_RAMX0_32K_RETAINED    PM_ENCODE_RESC(PM_RESOURCE_FULL_ON,      kResc_SRAM_RAMX0_32K)
#define PM_RESC_RAMX1_32K_RETAINED    PM_ENCODE_RESC(PM_RESOURCE_FULL_ON,      kResc_SRAM_RAMX1_32K)
#define PM_RESC_RAMX2_32K_RETAINED    PM_ENCODE_RESC(PM_RESOURCE_FULL_ON,      kResc_SRAM_RAMX2_32K)
#define PM_RESC_RAMB0_32K_RETAINED    PM_ENCODE_RESC(PM_RESOURCE_FULL_ON,      kResc_SRAM_RAMB0_32K)
#define PM_RESC_RAMC0_32K_RETAINED    PM_ENCODE_RESC(PM_RESOURCE_FULL_ON,      kResc_SRAM_RAMC0_32K)
#define PM_RESC_RAMC1_32K_RETAINED    PM_ENCODE_RESC(PM_RESOURCE_FULL_ON,      kResc_SRAM_RAMC1_32K)
#define PM_RESC_RAMD0_32K_RETAINED    PM_ENCODE_RESC(PM_RESOURCE_FULL_ON,      kResc_SRAM_RAMD0_32K)
#define PM_RESC_RAMD1_32K_RETAINED    PM_ENCODE_RESC(PM_RESOURCE_FULL_ON,      kResc_SRAM_RAMD1_32K)
#define PM_RESC_RAME0_32K_RETAINED    PM_ENCODE_RESC(PM_RESOURCE_FULL_ON,      kResc_SRAM_RAME0_32K)
#define PM_RESC_RAME1_32K_RETAINED    PM_ENCODE_RESC(PM_RESOURCE_FULL_ON,      kResc_SRAM_RAME1_32K)
#define PM_RESC_RAMF0_32K_RETAINED    PM_ENCODE_RESC(PM_RESOURCE_FULL_ON,      kResc_SRAM_RAMF0_32K)
#define PM_RESC_RAMF1_32K_RETAINED    PM_ENCODE_RESC(PM_RESOURCE_FULL_ON,      kResc_SRAM_RAMF1_32K)
#define PM_RESC_RAMG01_32K_RETAINED   PM_ENCODE_RESC(PM_RESOURCE_FULL_ON,      kResc_SRAM_RAMG01_32K)
#define PM_RESC_RAMG23_32K_RETAINED   PM_ENCODE_RESC(PM_RESOURCE_FULL_ON,      kResc_SRAM_RAMG23_32K)
#define PM_RESC_RAMH01_32K_RETAINED   PM_ENCODE_RESC(PM_RESOURCE_FULL_ON,      kResc_SRAM_RAMH01_32K)
/*!
 * @brief Set the constraints that the Flash can be Active or Low-Power modes.
 */
#define PM_RESC_FLASH_ACTIVE PM_ENCODE_RESC(PM_RESOURCE_FULL_ON,      kResc_Flash)
#define PM_RESC_FLASH_LP     PM_ENCODE_RESC(PM_RESOURCE_PARTABLE_ON1, kResc_Flash)
/*!
 * @brief Set the constraints for the regulators' drive strength modes.
 */
#define PM_RESC_DCDC_CORE_DS_NORMAL PM_ENCODE_RESC(PM_RESOURCE_FULL_ON,      kResc_DCDC_CORE)
#define PM_RESC_DCDC_CORE_DS_LOW    PM_ENCODE_RESC(PM_RESOURCE_PARTABLE_ON2, kResc_DCDC_CORE)
#define PM_RESC_DCDC_CORE_DS_PULSE  PM_ENCODE_RESC(PM_RESOURCE_PARTABLE_ON1, kResc_DCDC_CORE)

#define PM_RESC_LDO_CORE_DS_NORMAL  PM_ENCODE_RESC(PM_RESOURCE_FULL_ON,      kResc_LDO_CORE)
#define PM_RESC_LDO_CORE_DS_LOW     PM_ENCODE_RESC(PM_RESOURCE_PARTABLE_ON1, kResc_LDO_CORE)

#define PM_RESC_LDO_SYS_DS_NORMAL   PM_ENCODE_RESC(PM_RESOURCE_FULL_ON,      kResc_LDO_SYS)
#define PM_RESC_LDO_SYS_DS_LOW      PM_ENCODE_RESC(PM_RESOURCE_PARTABLE_ON1, kResc_LDO_SYS)
/*!
 * @brief Set the constraints for the VDD_CORE BandGap modes.
 */
#define PM_RESC_CORE_BG_ON_BUF_ON   PM_ENCODE_RESC(PM_RESOURCE_FULL_ON,      kResc_BG_CORE)
#define PM_RESC_CORE_BG_ON_BUF_OFF  PM_ENCODE_RESC(PM_RESOURCE_PARTABLE_ON1, kResc_BG_CORE)
/*!
 * @brief Set the constraints for the remaining peripherals/features that are On or Off.
 */
#define PM_RESC_FRO_144M_ON  PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_FRO_144M)
#define PM_RESC_FRO_12M_ON   PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_FRO_12M)
#define PM_RESC_FRO_16K_ON   PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_FRO_16K)
#define PM_RESC_OSC_RTC_ON   PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_OSC_RTC)
#define PM_RESC_OSC_SYS_ON   PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_OSC_SYS)
#define PM_RESC_PLL0_ON      PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_PLL0)
#define PM_RESC_PLL1_ON      PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_PLL1)
#define PM_RESC_ADC_ON       PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_ADC)
#define PM_RESC_CMP0_ON      PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_CMP0)
#define PM_RESC_CMP1_ON      PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_CMP1)
#define PM_RESC_CMP2_ON      PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_CMP2)
#define PM_RESC_DAC0_ON      PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_DAC0)
#define PM_RESC_DAC1_ON      PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_DAC1)
#define PM_RESC_DAC2_ON      PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_DAC2)
#define PM_RESC_OPAMP0_ON    PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_OPAMP0)
#define PM_RESC_OPAMP1_ON    PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_OPAMP1)
#define PM_RESC_OPAMP2_ON    PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_OPAMP2)
#define PM_RESC_VREF_ON      PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_VREF)
#define PM_RESC_SINC_ON      PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_SINC)
#define PM_RESC_USB3V_DET_ON PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_USB3V_DET)
#define PM_RESC_BG_VBAT_ON   PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_BG_VBAT)
#define PM_RESC_GDET_ON      PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_GDET)
#define PM_RESC_HVD_CORE_ON  PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_HVD_CORE)
#define PM_RESC_HVD_SYS_ON   PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_HVD_SYS)
#define PM_RESC_HVD_IO_ON    PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_HVD_IO)
#define PM_RESC_LVD_CORE_ON  PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_LVD_CORE)
#define PM_RESC_LVD_SYS_ON   PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_LVD_SYS)
#define PM_RESC_LVD_IO_ON    PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_LVD_IO)
#define PM_RESC_IO_DET_ON    PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_IO_Det)
/* @} */

#if FSL_PM_SUPPORT_WAKEUP_SOURCE_MANAGER

/*!
 * @name Wakeup enumerations
 * @{
 */

typedef enum _wakeup_inputType
{
    kWup_inputType_extPin = 0U,     /*!< External Pin type */
    kWup_inputType_intMod,          /*!< Internal Module type */
} wakeup_inputType_t;

typedef enum _wakeup_pin_det
{
    kWup_pinDet_RisingEdge = 1U,    /*!< Rising  Edge detection */
    kWup_pinDet_FallingEdge,        /*!< Falling Edge detection */
    kWup_pinDet_AnyChange,          /*!< Any Change   detection */
} wakeup_pin_det_t;

typedef enum _wakeup_pinID
{
    kWup_pinID_P1_3 = 7U,           /*!< Port1 Pin3 */
} wakeup_pinID_t;

typedef enum _wakeup_modID
{
    kWup_modID_LPTMR0 = 6U,         /*!< LPTMR0 */
} wakeup_modID_t;

/*! @} */

/*!
 * @name System Wakeup source definitions.
 * @{
 */

/*!
 * @brief Enable P1_3 as a wakeup pin, detect on rising edge.
 */
#define PM_WSID_P1_3_RISING_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(kWup_inputType_extPin, \
            kWup_pinID_P1_3, PORT_EFT_IRQn, kWup_pinDet_RisingEdge)

/*!
 * @brief Enable LPTMR0 as a wakeup source.
 */
#define PM_WSID_LPTMR0 PM_ENCODE_WAKEUP_SOURCE_ID(kWup_inputType_intMod, \
            kWup_modID_LPTMR0, LPTMR0_IRQn, 0UL)

/* ToDo add remaining wake-up sources and add to WUU_PIN_FLAG_MASK*/
/*! @} */

#endif /* FSL_PM_SUPPORT_WAKEUP_SOURCE_MANAGER */

/*******************************************************************************
 * APIs
 ******************************************************************************/

/* Reported in MCUX-65866 to keep reserved bits cleared */
void CMC_PowerOffSRAMAllMode_to_add(CMC_Type *base, uint32_t mask);
void CMC_PowerOffSRAMLowPowerOnly_to_add(CMC_Type *base, uint32_t mask);

#endif /* _FSL_PM_BOARD_H_ */
