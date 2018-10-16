/*****************************************************************************
* File Name: cy_tft_display.h
*
* Description: This file contains the function prototypes and constants used in
*  cy_tft_display.c
*
******************************************************************************
* Copyright (2018), Cypress Semiconductor Corporation.
******************************************************************************
* This software is owned by Cypress Semiconductor Corporation (Cypress) and is
* protected by and subject to worldwide patent protection (United States and
* foreign), United States copyright laws and international treaty provisions.
* Cypress hereby grants to licensee a personal, non-exclusive, non-transferable
* license to copy, use, modify, create derivative works of, and compile the
* Cypress Source Code and derivative works for the sole purpose of creating
* custom software in support of licensee product to be used only in conjunction
* with a Cypress integrated circuit as specified in the applicable agreement.
* Any reproduction, modification, translation, compilation, or representation of
* this software except as specified above is prohibited without the express
* written permission of Cypress.
*
* Disclaimer: CYPRESS MAKES NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, WITH
* REGARD TO THIS MATERIAL, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
* Cypress reserves the right to make changes without further notice to the
* materials described herein. Cypress does not assume any liability arising out
* of the application or use of any product or circuit described herein. Cypress
* does not authorize its products for use as critical components in life-support
* systems where a malfunction or failure may reasonably be expected to result in
* significant injury to the user. The inclusion of Cypress' product in a life-
* support systems application implies that the manufacturer assumes all risk of
* such use and in doing so indemnifies Cypress against all charges. Use may be
* limited by and subject to the applicable Cypress software license agreement.
*****************************************************************************/
#include "wiced.h"
#include "ugui.h"

#pragma once

#ifdef __cplusplus
extern "C" {
#endif
/*defines for TFT pins*/
#define TFT_D_C_PIN 1u
#define TFT_D_C_PORT GPIO_PRT12

#define TFT_RD_PIN 3u
#define TFT_RD_PORT GPIO_PRT12

#define TFT_RST_PIN 2u
#define TFT_RST_PORT GPIO_PRT12

#define TFT_WR_PIN 0u
#define TFT_WR_PORT GPIO_PRT12

#define TFT_DATA0_PIN 0u
#define TFT_DATA0_PORT GPIO_PRT9

#define TFT_DATA1_PIN 1u
#define TFT_DATA1_PORT GPIO_PRT9

#define TFT_DATA2_PIN 2u
#define TFT_DATA2_PORT GPIO_PRT9

#define TFT_DATA3_PIN 4u
#define TFT_DATA3_PORT GPIO_PRT9

#define TFT_DATA4_PIN 5u
#define TFT_DATA4_PORT GPIO_PRT9

#define TFT_DATA5_PIN 2u
#define TFT_DATA5_PORT GPIO_PRT0

#define TFT_DATA6_PIN 0u
#define TFT_DATA6_PORT GPIO_PRT13

#define TFT_DATA7_PIN 1u
#define TFT_DATA7_PORT GPIO_PRT13


void Cy_TFT_writeCommand(uint8_t command);
void Cy_TFT_writeData(uint8_t data);
void Cy_TFT_CTRL_Write(uint8_t data);
void Cy_TFT_displayDriver(UG_S16 x, UG_S16 y, UG_COLOR color);
void Cy_TFT_Init(void);

#ifdef __cplusplus
} /* extern "C" */
#endif
