/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _ANALOG_H_
#define _ANALOG_H_

#include "fsl_common.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
bool App_GetAnalogActive(uint32_t mod);
void App_Enable_CMP   (LPCMP_Type *base);
void App_Disable_CMP  (LPCMP_Type *base);
void App_Enable_DAC   (LPDAC_Type *base);
void App_Disable_DAC  (LPDAC_Type *base);
void App_Enable_DAC14 (void);
void App_Disable_DAC14(void);
void App_Enable_OpAmp (OPAMP_Type *base);
void App_Disable_OpAmp(OPAMP_Type *base);
void App_Enable_VREF  (void);
void App_Disable_VREF (void);

#endif //_ANALOG_H_
