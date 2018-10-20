/*****************************************************************************
* File Name: CE222494_PSoC6_WICED_WiFi.h
*
* Description: This file contains the function prototypes and constants used in
*  CE222494_PSoC6_WICED_WiFi.c. This driver is intended for AK4954A.
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

#include <math.h>
#include "cy_tft_display.h"
#include "wiced.h"
#include "http_server.h"
#include "sntp.h"
#include "gedday.h"
#include "resources.h"

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/** @cond */
/******************************************************
 *                     Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

#define LED_PWM_FREQ_HZ                 (2000)                      /* Default LED frequency */
#define DEFAULT_DUTYCYCLE               (50u)                       /* Default LED dutycycle */
#define MAX_DUTYCYCLE                   (100u)                      /* Maximum dutycycle */
#define MIN_DUTYCYCLE                   (0u)                        /* Minimum dutycycle */
#define DUTYCYCLE_INCREMENT             (5u)                        /* Dutycycle increment */
#define MAX_SOCKETS                     (10u)                       /* Maximum number of sockets */
#define LIGHTSENSOR_READING_INTERVAL    (250u)                      /* How often the light sensor is read */
#define LIGHTSENSOR_ADC_MAX_VOLTAGE     (3300u)                     /* Max voltage of ADC reading  */
#define LIGHTSENSOR_ADC_MAX_COUNT       (4096u)                     /* Max count of ADC reading  */
#define CAPSENSE_THREAD_PRIORITY        (8u)                        /* Priority of Capsense thread*/
#define CAPSENSE_THREAD_STACK_SIZE      (4096u)                     /* Capsense thread stack size*/
#define BUTTON0_PRESSED                 (1u)                        /* Flag indicating Button 0 was pressed */
#define BUTTON1_PRESSED                 (2u)                        /* Flag indicating Button 1 was pressed */
#define SLIDER_PRESSED                  (4u)                        /* Flag indicating slider was pressed */
#define CAPSENSE_UPDATE_TIME            (10u)                       /* Capsense thread update time */
#define TOP_DISPLAY                     (0u)                        /* Top of the display*/
#define EXTRA_FONT_SPACE                (3u)                        /* Spacing between printed lines*/
#define TEXT_FONT                       (FONT_8X12)                 /* Font for display text*/
#define HEADING_FONT                    (FONT_8X14)                 /* Font for heading text */
/******************************************************
 *                   Enumerations
 ******************************************************/


/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/
typedef struct
{
    uint8_t         duty;
    wiced_mutex_t   mutex;
} pwm_duty_t;

typedef struct
{
    uint16_t                lightSensorReading;
    wiced_iso8601_time_t    timestamp;
    wiced_mutex_t           mutex;
} light_sensor_t;

/******************************************************
 *                 Global Variables
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/
wiced_result_t  takeLightSensorReading         ( void* arg );
uint8_t         getDutycycle                   ( void );
void            increaseDutycycle              ( void );
void            decreaseDutycycle              ( void );
void            setDutycycle                   ( uint8_t dutyCycle );
void            adjustLedBrightness            ( void );

int32_t         processLightsensorUpdate       ( const char* url_path, const char* url_parameters, wiced_http_response_stream_t* stream, void* arg, wiced_http_message_body_t* http_data );
int32_t         processDutyUp                  ( const char* url_path, const char* url_parameters, wiced_http_response_stream_t* stream, void* arg, wiced_http_message_body_t* http_data );
int32_t         processDutyDown                ( const char* url_path, const char* url_parameters, wiced_http_response_stream_t* stream, void* arg, wiced_http_message_body_t* http_data );

void            capsenseLoop                   (wiced_thread_arg_t arg);


/** @endcond */
#ifdef __cplusplus
} /*extern "C" */
#endif

