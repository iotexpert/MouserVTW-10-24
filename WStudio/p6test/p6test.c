/*
 * Copyright 2018, Cypress Semiconductor Corporation or a subsidiary of 
 * Cypress Semiconductor Corporation. All Rights Reserved.
 * 
 * This software, associated documentation and materials ("Software"),
 * is owned by Cypress Semiconductor Corporation
 * or one of its subsidiaries ("Cypress") and is protected by and subject to
 * worldwide patent protection (United States and foreign),
 * United States copyright laws and international treaty provisions.
 * Therefore, you may use this Software only as provided in the license
 * agreement accompanying the software package from which you
 * obtained this Software ("EULA").
 * If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
 * non-transferable license to copy, modify, and compile the Software
 * source code solely for use in connection with Cypress's
 * integrated circuit products. Any reproduction, modification, translation,
 * compilation, or representation of this Software except as specified
 * above is prohibited without the express written permission of Cypress.
 *
 * Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
 * reserves the right to make changes to the Software without notice. Cypress
 * does not assume any liability arising out of the application or use of the
 * Software or any product or circuit described in the Software. Cypress does
 * not authorize its products for use in any products where a malfunction or
 * failure of the Cypress product may reasonably be expected to result in
 * significant property damage, injury or death ("High Risk Product"). By
 * including Cypress's product in a High Risk Product, the manufacturer
 * of such system or application assumes all risk of such use and in doing
 * so agrees to indemnify Cypress against all liability.
 */

/** @file
 *
 * Scan Application
 *
 * Features demonstrated
 *  - WICED scan API
 *
 * This application snippet regularly scans for nearby Wi-Fi access points
 *
 * Application Instructions
 *   Connect a PC terminal to the serial port of the WICED Eval board,
 *   then build and download the application as described in the WICED
 *   Quick Start Guide
 *
 *   Each time the application scans, a list of Wi-Fi access points in
 *   range is printed to the UART
 *
 */

#include <stdlib.h>
#include "wiced.h"
#include "cy_tft_display.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/


/******************************************************
 *               Static Function Declarations
 ******************************************************/
UG_GUI   gui;

/******************************************************
 *               Variable Definitions
 ******************************************************/

wiced_thread_t blinkThreadHandle;
wiced_thread_t capsenseThreadHandle;

/******************************************************
 *               Function Definitions
 ******************************************************/

void wiced_hal_blink()
{
    uint32_t val=0;

     while(1)
     {
         if(val)
         {
             wiced_gpio_output_high(WICED_LED1);
         }
         else
         {
             wiced_gpio_output_low(WICED_LED1);
         }

         WPRINT_APP_INFO(("LED %d\r\n",val));

         val = !val;
         wiced_rtos_delay_milliseconds(200);
     }

}

void startTFT()
{

    /*Initialize the TFT LCD*/
    Cy_TFT_Init();
    /*Connect ugui display driver*/
    UG_Init( &gui, Cy_TFT_displayDriver, 320, 240 );

    /*Setup display for this demo*/
    /*Fill screen with black*/
    UG_FillScreen( C_BLACK );
    UG_SetBackcolor( C_BLACK );
    UG_SetForecolor( C_WHITE );

    UG_FontSelect( &FONT_22X36 );

    /*Print welcome message*/
    UG_PutString( 10, 10,  "PSoC 6 Test");
    //UG_FillCircle(100,100,20,C_BLUE);
}

void capsenseThread(void *arg)
{

    uint16_t x=20,y=100;

    char buff[128];

    uint8_t circleState=1;

    CapSense_Start();
    CapSense_ScanAllWidgets();
    while(1)
     {
         if(!CapSense_IsBusy())
         {

             CapSense_ProcessAllWidgets();
             if(CapSense_IsWidgetActive(CapSense_BUTTON0_WDGT_ID))
             {
                 WPRINT_APP_INFO(("B0 Active \r\n"));
                 if(circleState == 0)
                 {
                     circleState = 1;
                 }

             }

             if(CapSense_IsWidgetActive(CapSense_BUTTON1_WDGT_ID))
             {
                 WPRINT_APP_INFO(("B1 Active \r\n"));
                 if(circleState == 1)
                 {
                     circleState = 0;
                 }
             }

             uint32_t val = CapSense_GetCentroidPos(CapSense_LINEARSLIDER0_WDGT_ID);
             if(val < 0xFFFF)
             {
                 WPRINT_APP_INFO(("Slider = %d\r\n",val));
                 UG_FillCircle(x,y,20,C_BLACK);
                 x = 20 + (val * (320-40))/100;
                 UG_FillCircle(x,y,20,C_BLUE);


             }


             CapSense_ScanAllWidgets();
         }
         wiced_rtos_delay_milliseconds(25);
         //wiced_rtos_thread_yield();
     }


}

void pdlBlinkThread(void *arg)
{
    char buff[128];

    CapSense_Start();
    CapSense_ScanAllWidgets();
     while(1)
     {
         Cy_GPIO_Inv(GPIO_PRT0,3);
         sprintf(buff,"Val = %d\r\n",Cy_GPIO_ReadOut(GPIO_PRT0,3));
 //        Cy_SCB_UART_PutString(SCB5,buff);
         wiced_rtos_delay_milliseconds(200);
     }

}


void application_start( )
{
    wiced_init( );
    startTFT();
    //wiced_hal_blink();
    wiced_rtos_create_thread(&blinkThreadHandle,7,"Blink Thread",pdlBlinkThread,500,0);
    wiced_rtos_create_thread(&capsenseThreadHandle,7,"CapSense Thread",capsenseThread,500,0);

 }

