/******************************************************************************
* File Name: cy_tft_display.c
*
* Version 1.0
*
* This file contains low level code for communicating with the TFT display on
* the CY8CKIT-028. This communication is done via GPIO writes and a write to
* a UDB control register
*
* Related Document: CE222494.pdf
*
* Hardware Dependency:CY8CKIT_062-WiFi-BT with CY8CKIT_028
*
*******************************************************************************
* Copyright (2017), Cypress Semiconductor Corporation. All rights reserved.
*******************************************************************************
* This software, including source code, documentation and related materials
* (�Software�), is owned by Cypress Semiconductor Corporation or one of its
* subsidiaries (�Cypress�) and is protected by and subject to worldwide patent
* protection (United States and foreign), United States copyright laws and
* international treaty provisions. Therefore, you may use this Software only
* as provided in the license agreement accompanying the software package from
* which you obtained this Software (�EULA�).
*
* If no EULA applies, Cypress hereby grants you a personal, nonexclusive,
* non-transferable license to copy, modify, and compile the Software source
* code solely for use in connection with Cypress�s integrated circuit products.
* Any reproduction, modification, translation, compilation, or representation
* of this Software except as specified above is prohibited without the express
* written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
* reserves the right to make changes to the Software without notice. Cypress
* does not assume any liability arising out of the application or use of the
* Software or any product or circuit described in the Software. Cypress does
* not authorize its products for use in any products where a malfunction or
* failure of the Cypress product may reasonably be expected to result in
* significant property damage, injury or death (�High Risk Product�). By
* including Cypress�s product in a High Risk Product, the manufacturer of such
* system or application assumes all risk of such use and in doing so agrees to
* indemnify Cypress against all liability.
* *******************************************************************************/

#include "../WStudio/L8GameBleAws/cy_tft_display.h"

void Cy_TFT_CTRL_Write(uint8_t data)
{
    /*A UDB control register is used to drive 8 pins on different ports for writing to the LCD*/
    (* (reg8 *) TFT_CTRL_Sync_ctrl_reg__CONTROL_REG) = data;
}

void Cy_TFT_writeCommand(uint8_t command)
{
    /*Sets data command line low indicating command write*/
    Cy_GPIO_Write(TFT_D_C_PORT,TFT_D_C_PIN, 0);
    /*Writes data to LCD*/
    Cy_TFT_CTRL_Write(command);
    /*Strobe write line*/
    Cy_GPIO_Write(TFT_WR_PORT, TFT_WR_PIN, 0);
    Cy_GPIO_Write(TFT_WR_PORT, TFT_WR_PIN, 1);

}

void Cy_TFT_writeData(uint8_t data)
{

    /*Sets data command line high indicating data write*/
    Cy_GPIO_Write(TFT_D_C_PORT,TFT_D_C_PIN, 1);
    /*Writes data to LCD*/
    Cy_TFT_CTRL_Write(data);
    /*Strobe write line*/
    Cy_GPIO_Write(TFT_WR_PORT, TFT_WR_PIN, 0);
    Cy_GPIO_Write(TFT_WR_PORT, TFT_WR_PIN, 1);
}

void Cy_TFT_displayDriver(UG_S16 x, UG_S16 y, UG_COLOR color)
{
    /*This function is called by the uGui library for writing a pixel at a time*/

    /*First set the Column Start and End, set to same since this writes one pixel*/
    Cy_TFT_writeCommand(0x2A);
    Cy_TFT_writeData((uint8_t)(x>>8));
    Cy_TFT_writeData((uint8_t)(x));
    Cy_TFT_writeData((uint8_t)(x>>8));
    Cy_TFT_writeData((uint8_t)(x));

    /*Then set the Row Start and End, set to same since this writes one pixel*/
    Cy_TFT_writeCommand(0x2B);
    Cy_TFT_writeData((uint8_t)(y>>8));
    Cy_TFT_writeData((uint8_t)(y));
    Cy_TFT_writeData((uint8_t)(y>>8));
    Cy_TFT_writeData((uint8_t)(y));

    /*Write the RGB data for specified pixel*/
    Cy_TFT_writeCommand(0x2C);
    Cy_TFT_writeData((uint8_t) (color>>16) );  /*RED*/
    Cy_TFT_writeData((uint8_t) (color>>8) );   /*GREEN*/
    Cy_TFT_writeData((uint8_t) (color));       /*BLUE*/
}

void Cy_TFT_Init(void)
{
    /* This function initializes the TFT display on the TFT shield*/

    /*Configure all of the TFT pins*/
    /*The following 8 pins are the pins connected to the control register, these are the
     * data pins
     */
    Cy_GPIO_Pin_FastInit(TFT_DATA0_PORT, TFT_DATA0_PIN, CY_GPIO_DM_STRONG_IN_OFF,  1, HSIOM_SEL_DSI_GPIO);
    Cy_GPIO_Pin_FastInit(TFT_DATA1_PORT, TFT_DATA1_PIN, CY_GPIO_DM_STRONG_IN_OFF,  1, HSIOM_SEL_DSI_GPIO);
    Cy_GPIO_Pin_FastInit(TFT_DATA2_PORT, TFT_DATA2_PIN, CY_GPIO_DM_STRONG_IN_OFF,  1, HSIOM_SEL_DSI_GPIO);
    Cy_GPIO_Pin_FastInit(TFT_DATA3_PORT, TFT_DATA3_PIN, CY_GPIO_DM_STRONG_IN_OFF,  1, HSIOM_SEL_DSI_GPIO);
    Cy_GPIO_Pin_FastInit(TFT_DATA4_PORT, TFT_DATA4_PIN, CY_GPIO_DM_STRONG_IN_OFF,  1, HSIOM_SEL_DSI_GPIO);
    Cy_GPIO_Pin_FastInit(TFT_DATA5_PORT, TFT_DATA5_PIN, CY_GPIO_DM_STRONG_IN_OFF,  1, HSIOM_SEL_DSI_GPIO);
    Cy_GPIO_Pin_FastInit(TFT_DATA6_PORT, TFT_DATA6_PIN, CY_GPIO_DM_STRONG_IN_OFF,  1, HSIOM_SEL_DSI_GPIO);
    Cy_GPIO_Pin_FastInit(TFT_DATA7_PORT, TFT_DATA7_PIN, CY_GPIO_DM_STRONG_IN_OFF,  1, HSIOM_SEL_DSI_GPIO);

    /*These are the control pins*/
    Cy_GPIO_Pin_FastInit(TFT_WR_PORT,   TFT_WR_PIN,  CY_GPIO_DM_STRONG_IN_OFF, 0, HSIOM_SEL_GPIO);
    Cy_GPIO_Pin_FastInit(TFT_D_C_PORT,  TFT_D_C_PIN, CY_GPIO_DM_STRONG_IN_OFF, 0, HSIOM_SEL_GPIO);
    Cy_GPIO_Pin_FastInit(TFT_RST_PORT,  TFT_RST_PIN, CY_GPIO_DM_STRONG_IN_OFF, 0, HSIOM_SEL_GPIO);
    Cy_GPIO_Pin_FastInit(TFT_RD_PORT,   TFT_RD_PIN,  CY_GPIO_DM_STRONG_IN_OFF, 0, HSIOM_SEL_GPIO);

    /* code is for a ST7789S, code comes from example code for this display*/
    Cy_GPIO_Write(TFT_RD_PORT, TFT_RD_PIN, 1);
    Cy_GPIO_Write(TFT_WR_PORT, TFT_WR_PIN, 0);
    Cy_GPIO_Write(TFT_RST_PORT, TFT_RST_PIN, 0);
    Cy_SysLib_Delay(100);
    Cy_GPIO_Write(TFT_RST_PORT, TFT_RST_PIN, 1);
    Cy_SysLib_Delay(100);
    Cy_TFT_writeCommand(0x11);  /*exit SLEEP mode*/
    Cy_SysLib_Delay(100);

    Cy_TFT_writeCommand(0x36);
    Cy_TFT_writeData(0xA0); /*MADCTL: memory data access control*/
    Cy_TFT_writeCommand(0x3A);
    Cy_TFT_writeData(0x66); /*COLMOD: Interface Pixel format*/
    Cy_TFT_writeCommand(0xB2);
    Cy_TFT_writeData(0x0C);
    Cy_TFT_writeData(0x0C);
    Cy_TFT_writeData(0x00);
    Cy_TFT_writeData(0x33);
    Cy_TFT_writeData(0x33); /*PORCTRK: Porch setting*/
    Cy_TFT_writeCommand(0xB7);
    Cy_TFT_writeData(0x35); /*GCTRL: Gate Control*/
    Cy_TFT_writeCommand(0xBB);
    Cy_TFT_writeData(0x2B); /*VCOMS: VCOM setting*/
    Cy_TFT_writeCommand(0xC0);
    Cy_TFT_writeData(0x2C); /*LCMCTRL: LCM Control*/
    Cy_TFT_writeCommand(0xC2);
    Cy_TFT_writeData(0x01);
    Cy_TFT_writeData(0xFF); /*VDVVRHEN: VDV and VRH Command Enable*/
    Cy_TFT_writeCommand(0xC3);
    Cy_TFT_writeData(0x11); /*VRHS: VRH Set*/
    Cy_TFT_writeCommand(0xC4);
    Cy_TFT_writeData(0x20); /*VDVS: VDV Set*/

    Cy_TFT_writeCommand(0xC6);
    Cy_TFT_writeData(0x0F); /*FRCTRL2: Frame Rate control in normal mode*/
    Cy_TFT_writeCommand(0xD0);
    Cy_TFT_writeData(0xA4);
    Cy_TFT_writeData(0xA1); /*PWCTRL1: Power Control 1*/
    Cy_TFT_writeCommand(0xE0);
    Cy_TFT_writeData(0xD0);
    Cy_TFT_writeData(0x00);
    Cy_TFT_writeData(0x05);
    Cy_TFT_writeData(0x0E);
    Cy_TFT_writeData(0x15);
    Cy_TFT_writeData(0x0D);
    Cy_TFT_writeData(0x37);
    Cy_TFT_writeData(0x43);
    Cy_TFT_writeData(0x47);
    Cy_TFT_writeData(0x09);
    Cy_TFT_writeData(0x15);
    Cy_TFT_writeData(0x12);
    Cy_TFT_writeData(0x16);
    Cy_TFT_writeData(0x19); /*PVGAMCTRL: Positive Voltage Gamma control*/
    Cy_TFT_writeCommand(0xE1);
    Cy_TFT_writeData(0xD0);
    Cy_TFT_writeData(0x00);
    Cy_TFT_writeData(0x05);
    Cy_TFT_writeData(0x0D);
    Cy_TFT_writeData(0x0C);
    Cy_TFT_writeData(0x06);
    Cy_TFT_writeData(0x2D);
    Cy_TFT_writeData(0x44);
    Cy_TFT_writeData(0x40);
    Cy_TFT_writeData(0x0E);
    Cy_TFT_writeData(0x1C);
    Cy_TFT_writeData(0x18);
    Cy_TFT_writeData(0x16);
    Cy_TFT_writeData(0x19); /*NVGAMCTRL: Negative Voltage Gamma control*/
    Cy_TFT_writeCommand(0x2A);
    Cy_TFT_writeData(0x00);
    Cy_TFT_writeData(0x00);
    Cy_TFT_writeData(0x01);
    Cy_TFT_writeData(0x3F); /*X address set*/
    Cy_TFT_writeCommand(0x2B);
    Cy_TFT_writeData(0x00);
    Cy_TFT_writeData(0x00);
    Cy_TFT_writeData(0x00);
    Cy_TFT_writeData(0xEF); /*Y address set*/

    Cy_SysLib_Delay(10);
    Cy_TFT_writeCommand(0x29); /*Enable Display*/

}
