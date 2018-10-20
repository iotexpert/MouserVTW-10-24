/******************************************************************************
* File Name: cy8ckit_062_demo.c
*
* Version 1.0
*
* This application demonstrates a PSoC 6 device hosting an http webserver
* The PSoC 6 measures the voltage of the ambient light sensor on the CY8CKIT-028
* It then displays that information on the webserver.
* The PSoC 6 also controls the brightness of the RED led on the board. The brightness
* can be controlled by the two capsense buttons, capsense slider, or webpage.
*
* Related Document: CE_Title.pdf
*
* Hardware Dependency: CY8CKIT_062_WiFi with CY8CKIT_028
*
*******************************************************************************
* Copyright (2017), Cypress Semiconductor Corporation. All rights reserved.
*******************************************************************************
* This software, including source code, documentation and related materials
* (“Software”), is owned by Cypress Semiconductor Corporation or one of its
* subsidiaries (“Cypress”) and is protected by and subject to worldwide patent
* protection (United States and foreign), United States copyright laws and
* international treaty provisions. Therefore, you may use this Software only
* as provided in the license agreement accompanying the software package from
* which you obtained this Software (“EULA”).
*
* If no EULA applies, Cypress hereby grants you a personal, nonexclusive,
* non-transferable license to copy, modify, and compile the Software source
* code solely for use in connection with Cypress’s integrated circuit products.
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
* significant property damage, injury or death (“High Risk Product”). By
* including Cypress’s product in a High Risk Product, the manufacturer of such
* system or application assumes all risk of such use and in doing so agrees to
* indemnify Cypress against all liability.
* *******************************************************************************/


#include "CE222494_PSoC6_WICED_WiFi.h"

/******************************************************
 *               Global Variable Definitions
 ******************************************************/
/* Holds lightsensor voltage and time stamp*/
static light_sensor_t      lightSensor;
/* Holds pwm duty cycle*/
static pwm_duty_t          pwmDuty;

/* Variables for server*/
static wiced_http_server_t httpServer;
static wiced_timed_event_t lightSensorTimedEvent;

/*Thread pointer for capsense thread*/
static wiced_thread_t capsenseThread;

/* ugui instance*/
UG_GUI   gui;

/*Global to hold display row position*/
uint16_t row = TOP_DISPLAY;

/*Flag to indicate is screen is ready for printing duty and voltage*/
uint8_t screenReady = 0;

/*Pointers to webpages*/
static START_OF_HTTP_PAGE_DATABASE(web_pages)
    ROOT_HTTP_PAGE_REDIRECT("/apps/cy8ckit_062_demo/main.html"),
    { "/apps/cy8ckit_062_demo/main.html","text/html",                WICED_RESOURCE_URL_CONTENT,  .url_content.resource_data  = &resources_apps_DIR_CE222494_PSoC6_WICED_WiFi_DIR_main_html, },
    { "/temp_report.html",               "text/html",                WICED_DYNAMIC_URL_CONTENT,   .url_content.dynamic_data   = {processLightsensorUpdate, 0 }, },
    { "/temp_up",                        "text/html",                WICED_DYNAMIC_URL_CONTENT,   .url_content.dynamic_data   = {processDutyUp, 0 }, },
    { "/temp_down",                      "text/html",                WICED_DYNAMIC_URL_CONTENT,   .url_content.dynamic_data   = {processDutyDown, 0 }, },
    { "/images/favicon.ico",             "image/vnd.microsoft.icon", WICED_RESOURCE_URL_CONTENT,  .url_content.resource_data  = &resources_images_DIR_favicon_ico, },
    { "/scripts/general_ajax_script.js", "application/javascript",   WICED_RESOURCE_URL_CONTENT,  .url_content.resource_data  = &resources_scripts_DIR_general_ajax_script_js, },
    { "/images/cypresslogo.png",         "image/png",                WICED_RESOURCE_URL_CONTENT,  .url_content.resource_data  = &resources_images_DIR_cypresslogo_png, },
    { "/images/cypresslogo_line.png",    "image/png",                WICED_RESOURCE_URL_CONTENT,  .url_content.resource_data  = &resources_images_DIR_cypresslogo_line_png, },
    { "/styles/buttons.css",             "text/css",                 WICED_RESOURCE_URL_CONTENT,  .url_content.resource_data  = &resources_styles_DIR_buttons_css, },
END_OF_HTTP_PAGE_DATABASE();


void application_start( void )
{
    /*Maximum socket connections*/
    uint16_t                maxSockets = MAX_SOCKETS;

    /*Semaphore used for sleeping application thread*/
    wiced_semaphore_t       semaphore;

    /*Hold results from WICED API*/
    wiced_result_t          result;

    /*Indicates if there is data in DCT or not*/
    wiced_bool_t*           device_configured;
    /*Location for device_configured to point to when clearing DCT*/
    wiced_bool_t            clearDCT = 0;

    /*IP address for device when connected to another network*/
    wiced_ip_address_t      ipv4Address;
    uint32_t                ipAddress;
    char                    buffer[50];

    /* information on configuration access point*/
    wiced_config_soft_ap_t*  configAp;

    /* Initialize the device and WICED framework */
    wiced_init( );

    /*Initialize the semaphore used to sleep the application thread*/
    wiced_rtos_init_semaphore( &semaphore );

    /* Initialize light sensor data. Light sensor data is shared among multiple threads; therefore a mutex is required */
    memset( &lightSensor, 0, sizeof( lightSensor ) );
    wiced_rtos_init_mutex( &lightSensor.mutex );


    /* Initialize PWM duty cycle. Duty cycle is shared among multiple threads; therefore a mutex is required */
    memset( &pwmDuty, 0, sizeof( pwmDuty ) );
    wiced_rtos_init_mutex( &pwmDuty.mutex );
    pwmDuty.duty = DEFAULT_DUTYCYCLE;


    /*Initialize the TFT LCD*/
    Cy_TFT_Init();
    /*Connect ugui display driver*/
    UG_Init( &gui, Cy_TFT_displayDriver, 320, 240 );

    /*Setup display for this demo*/
    /*Fill screen with black*/
    UG_FillScreen( C_BLACK );
    UG_SetBackcolor( C_BLACK );
    UG_SetForecolor( C_WHITE );
    UG_FontSelect( &HEADING_FONT );

    /*Print welcome message*/
    UG_PutString( 10, row,  "CE222494 PSoC6 WICED WiFi Demo");
    /*Increment row for next line*/
    row = (HEADING_FONT.char_height + EXTRA_FONT_SPACE);

    /*Check if there is a network stored in DCT, if not display instructions*/
    wiced_dct_read_lock( (void**) &device_configured, WICED_FALSE, DCT_WIFI_CONFIG_SECTION, OFFSETOF(platform_dct_wifi_config_t, device_configured), sizeof(wiced_bool_t) );

    if ( *device_configured != WICED_TRUE)
    {

        /*Read out details of the configuration AP, for printing out*/
        wiced_dct_read_lock( (void**) &configAp, WICED_FALSE, DCT_WIFI_CONFIG_SECTION, OFFSETOF(platform_dct_wifi_config_t, config_ap_settings), sizeof(wiced_config_soft_ap_t) );

        /*Insert instruction on LCD for logging into WiCED network*/
        UG_FontSelect( &TEXT_FONT );

        UG_PutString( 0, row, "Using another device connect to");
        row += (TEXT_FONT.char_height + EXTRA_FONT_SPACE);
        UG_PutString( 0, row, "the following WiFi network:" );
        row += (TEXT_FONT.char_height + EXTRA_FONT_SPACE);
        sprintf(buffer, "SSID    :  %s", configAp->SSID.value);
        UG_PutString( 0, row, buffer );
        row += (TEXT_FONT.char_height + EXTRA_FONT_SPACE);
        sprintf(buffer, "Password:  %s\n", configAp->security_key);
        UG_PutString( 0, row, buffer );
        row += (TEXT_FONT.char_height + EXTRA_FONT_SPACE);
        UG_PutString( 0, row, "Open a web browser and go to " );
        row += (TEXT_FONT.char_height + EXTRA_FONT_SPACE);
        UG_PutString( 0, row, "http://192.168.0.1" );
        row += (TEXT_FONT.char_height + EXTRA_FONT_SPACE);
        UG_PutString( 0, row, "On the page click the Wi-Fi Setup");
        row += (TEXT_FONT.char_height + EXTRA_FONT_SPACE);
        UG_PutString( 0, row, "button. Select your WiFi network");
        row += (TEXT_FONT.char_height + EXTRA_FONT_SPACE);
        UG_PutString( 0, row, "type in password, press Connect");
        row += (TEXT_FONT.char_height + EXTRA_FONT_SPACE);

        /*Print message out UART as well*/
        WPRINT_APP_INFO(("*******************************************************\n"));
        WPRINT_APP_INFO(("Using another device connect to the following WiFi network \n\r"));
        WPRINT_APP_INFO(("SSID    :  %s\n", configAp->SSID.value));
        WPRINT_APP_INFO(("Password:  %s\n", configAp->security_key));
        WPRINT_APP_INFO(("Open a web browser and go to  http://192.168.0.1 \n\r"));
        WPRINT_APP_INFO(("On the page click the Wi-Fi Setup button. Select your WiFi network \n\r"));
        WPRINT_APP_INFO(("type in the password, press connect \n\r"));
        WPRINT_APP_INFO(("*******************************************************\n"));

        /* Configure the device */
        wiced_configure_device( NULL );

        /*Unlock read of DCT*/
        wiced_dct_read_unlock( configAp, WICED_FALSE );
    }

    /*Unlock read of DCT*/
    wiced_dct_read_unlock( device_configured, WICED_FALSE );

    /*Display message that we are connecting to the network entered on configuration webpage*/
    UG_FontSelect( &TEXT_FONT );
    UG_PutString( 0, row,  "Connecting please wait");
    row += (TEXT_FONT.char_height + EXTRA_FONT_SPACE);

    /* Bring up the network interface */
    result = wiced_network_up( WICED_STA_INTERFACE, WICED_USE_EXTERNAL_DHCP_SERVER, NULL );
    if(result == WICED_SUCCESS)
    {
        WPRINT_APP_INFO(("\n\r"));
        WPRINT_APP_INFO( ( "Connected to network \n\r" ) );
        UG_PutString( 0, row, "Connected to network");
        row += (TEXT_FONT.char_height + EXTRA_FONT_SPACE);
    }
    else
    {

        /*Failed to connect to network try again*/
        WPRINT_APP_INFO(("Could not connect to network \n\r"));
        WPRINT_APP_INFO(("Hold SW2 for 1 sec to clear saved network \n\r" ) );
        WPRINT_APP_INFO(("Then reset device and try again\n\r" ) );

        /*Clear the screen*/
        UG_FillScreen( C_BLACK );
        row = TOP_DISPLAY;

        UG_FontSelect( &HEADING_FONT );

        /*Print welcome message*/
        UG_PutString( 10, row,  "CE222494 PSoC6 WICED WiFi Demo");
        /*Increment row for next line*/
        row = (HEADING_FONT.char_height + EXTRA_FONT_SPACE);

        UG_FontSelect( &TEXT_FONT );

        /*print failure messages*/
        UG_PutString( 0, row, "Could not connect to network");
        row += (TEXT_FONT.char_height + EXTRA_FONT_SPACE);
        UG_PutString( 0, row, "Hold SW2 for 1 sec to clear saved");
        row += (TEXT_FONT.char_height + EXTRA_FONT_SPACE);
        UG_PutString( 0, row, "network, reset device and try again");
        row += (TEXT_FONT.char_height + EXTRA_FONT_SPACE);

        /*Wait for user to either press SW2 or reset the device*/
        do
        {
            /*is the button pressed*/
            if ( !wiced_gpio_input_get( WICED_BUTTON1 ) )
            {
                /*Delay for a second and check again*/
                wiced_rtos_delay_milliseconds( 1000 );

                if ( !wiced_gpio_input_get( WICED_BUTTON1 ) )
                {
                    /*Set flag to indicate DCT needs to be cleared*/
                    clearDCT = 0;
                    device_configured = &clearDCT;

                    WPRINT_APP_INFO(( "Clearing saved network\n" ));
                    UG_PutString( 0, row, "Clearing saved network");
                    row += (TEXT_FONT.char_height + EXTRA_FONT_SPACE);

                    /*Clear out the DCT*/
                    wiced_dct_write( (void**) &device_configured, DCT_WIFI_CONFIG_SECTION, OFFSETOF(platform_dct_wifi_config_t, device_configured), sizeof(wiced_bool_t) );
                    wiced_rtos_delay_milliseconds( 1000 );

                    WPRINT_APP_INFO(( "Clearing done, reset the device\n" ));
                    UG_PutString( 0, row, "Clearing done, reset the device");

                    /*Wait for user to reset device*/
                    while(1);
                }
            }

        } while(1);
    }

    /*Print info that we are connected and starting the server*/
    WPRINT_APP_INFO(("\n\r"));
    WPRINT_APP_INFO(("Connecting to time server\n\r"));
    WPRINT_APP_INFO(("This may take a minute\n\r"));
    WPRINT_APP_INFO(("\n\r"));

    UG_PutString( 0, row,  "Connecting to time server");
    row += (TEXT_FONT.char_height + EXTRA_FONT_SPACE);
    UG_PutString( 0, row,  "This may take a minute");
    row += (TEXT_FONT.char_height + EXTRA_FONT_SPACE);

    /* Disable roaming to other access points */
    wiced_wifi_set_roam_trigger( -99 ); /* -99dBm ie. extremely low signal level */

    /* Start automatic time synchronization and synchronize once every day. */
    /* this is what takes a while */
    sntp_start_auto_time_sync( 1 * DAYS );

    /* Start web server to display current light sensor voltage  & PWM duty cycle */
    wiced_http_server_start( &httpServer, 80, maxSockets, web_pages, WICED_STA_INTERFACE, DEFAULT_URL_PROCESSOR_STACK_SIZE );

    /*Create Capense loop for processing button presses and slider*/
    wiced_rtos_create_thread(&capsenseThread, CAPSENSE_THREAD_PRIORITY, "CapSense", capsenseLoop, CAPSENSE_THREAD_STACK_SIZE, NULL);

    /*Initialize ADC for reading light sensor*/
    wiced_adc_init(WICED_ADC_1 , 0);

    /* Setup a timed event that will take a measurement */
    wiced_rtos_register_timed_event( &lightSensorTimedEvent, WICED_HARDWARE_IO_WORKER_THREAD, takeLightSensorReading, LIGHTSENSOR_READING_INTERVAL, NULL );

    /*Tell user we are ready to go*/
    WPRINT_APP_INFO( ( "Capsense Ready, Website Ready \n\r" ) );

    /*Display instructions on LCD*/
    UG_FillScreen( C_BLACK );
    UG_FontSelect( &HEADING_FONT );
    row = TOP_DISPLAY;
    UG_PutString( 10, row,  "CE222494 PSoC6 WICED WiFi Demo");

    UG_FontSelect( &TEXT_FONT );
    row = (HEADING_FONT.char_height + EXTRA_FONT_SPACE);

    /*Grab IP address to tell user which IP address to connect to*/
    wiced_ip_get_ipv4_address(WICED_STA_INTERFACE, &ipv4Address);

    /*Print message to connect to that ip address*/
    ipAddress = (uint32_t)ipv4Address.ip.v4;
    sprintf(buffer, "%u.%u.%u.%u",  (unsigned char) ( ( ipAddress >> 24 ) & 0xff ),
                                    (unsigned char) ( ( ipAddress >> 16 ) & 0xff ),
                                    (unsigned char) ( ( ipAddress >> 8 ) & 0xff ),
                                    (unsigned char) ( ( ipAddress >> 0 ) & 0xff ));

    UG_PutString( 0, row, "On a device connected to the");
    row += (TEXT_FONT.char_height + EXTRA_FONT_SPACE);
    UG_PutString( 0, row, "same network open a web browser");
    row += (TEXT_FONT.char_height + EXTRA_FONT_SPACE);
    UG_PutString( 0, row, "and go to: ");
    UG_PutString( 120, row, buffer);
    row += (TEXT_FONT.char_height + EXTRA_FONT_SPACE);

    WPRINT_APP_INFO(("\n\r"));
    WPRINT_APP_INFO(("*******************************************************\n"));
    WPRINT_APP_INFO(("On a device connected the same network \n\r"));
    WPRINT_APP_INFO(("Open a web browser and go to: \n\r"));
    WPRINT_APP_INFO(("%u.%u.%u.%u\n", (unsigned char) ( ( ipAddress >> 24 ) & 0xff ),
                                      (unsigned char) ( ( ipAddress >> 16 ) & 0xff ),
                                      (unsigned char) ( ( ipAddress >> 8 )  & 0xff ),
                                      (unsigned char) ( ( ipAddress >> 0 )  & 0xff ) ) );
    WPRINT_APP_INFO(("\n\r"));
    WPRINT_APP_INFO(("Use the webpage to observe light sensor voltage and change LED dutycycle \n\r"));
    WPRINT_APP_INFO(("*******************************************************\n"));

    UG_FontSelect( &HEADING_FONT );

    UG_PutString( 0, 90, "PWM Duty Cycle: ");

    UG_PutString( 0, 115, "Light Sensor Voltage: ");

    /*Indicate to threads it is okay to print duty and lightsensor voltage*/
    screenReady = 1;

    /*Set the duty cycle to the default value*/
    adjustLedBrightness( );

    /* Sleep this thread */
    wiced_rtos_get_semaphore( &semaphore, WICED_WAIT_FOREVER );
}

/*
 * Takes a temperature reading
 */
wiced_result_t takeLightSensorReading( void* arg )
{
    UNUSED_PARAMETER(arg);

    uint16_t    currLightSensorVoltage;
    char        tempCharBuffer[10];

    /*lock light sensor mutex*/
    wiced_rtos_lock_mutex( &lightSensor.mutex );

    /* Get the current ISO8601 time */
    wiced_time_get_iso8601_time( &lightSensor.timestamp );

    /*Read voltage on light sensor*/
    wiced_adc_take_sample(WICED_ADC_1 , &lightSensor.lightSensorReading);

    /*Convert to voltage*/
    currLightSensorVoltage = (lightSensor.lightSensorReading * LIGHTSENSOR_ADC_MAX_VOLTAGE) / LIGHTSENSOR_ADC_MAX_COUNT;

    /*Convert to string for printing*/
    sprintf(tempCharBuffer, "%04d mV", currLightSensorVoltage);

    /*make sure it is okay to print data*/
    if(screenReady)
    {
        UG_FontSelect( &HEADING_FONT );

        /*Print on LCD the voltage*/
        UG_PutString( 230, 115, tempCharBuffer);
    }

    wiced_rtos_unlock_mutex( &lightSensor.mutex );

    return WICED_SUCCESS;
}

/*
 * Gets current PWM duty cycle
 */
uint8_t getDutycycle( void )
{
    uint8_t currentDutycycle;

    wiced_rtos_lock_mutex( &pwmDuty.mutex );
    currentDutycycle = pwmDuty.duty;
    wiced_rtos_unlock_mutex( &pwmDuty.mutex );

    return currentDutycycle;
}

/*
 * Adjusts brightness of the LED
 */
void adjustLedBrightness( void )
{
    char tempCharBuffer[10];

    wiced_rtos_lock_mutex( &pwmDuty.mutex );

    /*Change dutycycle of LED*/
    wiced_pwm_init ( WICED_PWM_1, LED_PWM_FREQ_HZ, (float)pwmDuty.duty );
    wiced_pwm_start( WICED_PWM_1 );

    /*print out to screen*/
    sprintf(tempCharBuffer, "%03d%%", pwmDuty.duty);
    if(screenReady)
    {
        UG_FontSelect( &HEADING_FONT );
        /*Print on LCD the duty cycle*/
        UG_PutString( 170, 90, tempCharBuffer);
    }

    wiced_rtos_unlock_mutex( &pwmDuty.mutex );
}

/*
 * Increases PWM duty cycle
 */
void increaseDutycycle( void )
{
    wiced_rtos_lock_mutex( &pwmDuty.mutex );
    pwmDuty.duty += ( pwmDuty.duty < (MAX_DUTYCYCLE - (DUTYCYCLE_INCREMENT - 1)) ) ? DUTYCYCLE_INCREMENT : 0;
    adjustLedBrightness( );
    wiced_rtos_unlock_mutex( &pwmDuty.mutex );
}

/*
 * Decreases PWM duty cycle
 */
void decreaseDutycycle( void )
{
    wiced_rtos_lock_mutex( &pwmDuty.mutex );
    pwmDuty.duty -= ( pwmDuty.duty > (MIN_DUTYCYCLE + (DUTYCYCLE_INCREMENT - 1)) ) ? DUTYCYCLE_INCREMENT : 0;
    adjustLedBrightness( );
    wiced_rtos_unlock_mutex( &pwmDuty.mutex );
}

/*
 * Set PWM duty cycle
 */
void setDutycycle( uint8_t dutyCycle )
{
    wiced_rtos_lock_mutex( &pwmDuty.mutex );
    pwmDuty.duty = dutyCycle;
    adjustLedBrightness( );
    wiced_rtos_unlock_mutex( &pwmDuty.mutex );
}

/*
 * Updates light sensor information on the web page
 */
int32_t processLightsensorUpdate( const char* url_path, const char* url_parameters, wiced_http_response_stream_t* stream, void* arg, wiced_http_message_body_t* http_data )
{
    wiced_iso8601_time_t*   curr_time;
    uint8_t ledDutyCycle = getDutycycle();
    char                    tempCharBuffer[6];
    int                     tempCharBufferLength;
    uint16_t                currLightSensorReading;
    uint16_t                currLightSensorVoltage;

    UNUSED_PARAMETER( url_path );
    UNUSED_PARAMETER( http_data );

    /*Get the current time and lightsensor voltage while mutex is locked*/
    wiced_rtos_lock_mutex( &lightSensor.mutex );

    curr_time = &lightSensor.timestamp;

    /*convert current light sensor adc counts to volts*/
    currLightSensorReading = lightSensor.lightSensorReading;
    currLightSensorVoltage = (currLightSensorReading * LIGHTSENSOR_ADC_MAX_VOLTAGE) / LIGHTSENSOR_ADC_MAX_COUNT;

    wiced_rtos_unlock_mutex( &lightSensor.mutex );

    /* Write the time to the web page */
    wiced_http_response_stream_write_resource( stream, &resources_apps_DIR_CE222494_PSoC6_WICED_WiFi_DIR_data_html_time_start );
    wiced_http_response_stream_write( stream, curr_time->hour, sizeof(curr_time->hour)   +
                                                     sizeof(curr_time->colon1) +
                                                     sizeof(curr_time->minute) +
                                                     sizeof(curr_time->colon2) +
                                                     sizeof(curr_time->second) );
    wiced_http_response_stream_write_resource( stream, &resources_apps_DIR_CE222494_PSoC6_WICED_WiFi_DIR_data_html_time_end );

    /* Write the date to the webpage */
    wiced_http_response_stream_write_resource( stream, &resources_apps_DIR_CE222494_PSoC6_WICED_WiFi_DIR_data_html_date_start );
    wiced_http_response_stream_write(stream, curr_time->year, sizeof(curr_time->year)  +
                                                    sizeof(curr_time->dash1) +
                                                    sizeof(curr_time->month) +
                                                    sizeof(curr_time->dash2) +
                                                    sizeof(curr_time->day) );
    wiced_http_response_stream_write_resource( stream, &resources_apps_DIR_CE222494_PSoC6_WICED_WiFi_DIR_data_html_date_end );

    /* Write the lightsensor voltage to the webpage */
    wiced_http_response_stream_write_resource( stream, &resources_apps_DIR_CE222494_PSoC6_WICED_WiFi_DIR_data_html_temp_start );
    tempCharBufferLength = sprintf(tempCharBuffer, "%04d", currLightSensorVoltage);
    wiced_http_response_stream_write(stream, tempCharBuffer, tempCharBufferLength );
    wiced_http_response_stream_write_resource(  stream, &resources_apps_DIR_CE222494_PSoC6_WICED_WiFi_DIR_data_html_temp_end);

    /* Write the led dutycycle to the webpage */
    wiced_http_response_stream_write_resource( stream, &resources_apps_DIR_CE222494_PSoC6_WICED_WiFi_DIR_data_html_set_start );
    tempCharBufferLength = sprintf(tempCharBuffer, "%03d", ledDutyCycle);
    wiced_http_response_stream_write(stream, tempCharBuffer, tempCharBufferLength );
    wiced_http_response_stream_write_resource( stream, &resources_apps_DIR_CE222494_PSoC6_WICED_WiFi_DIR_data_html_set_end );

    wiced_http_response_stream_write_resource( stream, &resources_apps_DIR_CE222494_PSoC6_WICED_WiFi_DIR_data_html_page_end );

    return 0;
}

/*
 * Handles increase in duty cycle value from button on page
 */
int32_t processDutyUp( const char* url_path, const char* url_parameters, wiced_http_response_stream_t* stream, void* arg, wiced_http_message_body_t* http_data )
{
    UNUSED_PARAMETER( url_path );
    UNUSED_PARAMETER( url_parameters );
    UNUSED_PARAMETER( arg );
    UNUSED_PARAMETER( http_data );
    increaseDutycycle( );
    return 0;
}

/*
 * Handles decrease in duty cycle value from button on page
 */
int32_t processDutyDown( const char* url_path, const char* url_parameters, wiced_http_response_stream_t* stream, void* arg, wiced_http_message_body_t* http_data )
{
    UNUSED_PARAMETER( url_path );
    UNUSED_PARAMETER( url_parameters );
    UNUSED_PARAMETER( arg );
    UNUSED_PARAMETER( http_data );
    decreaseDutycycle( );
    return 0;
}


void capsenseLoop(wiced_thread_arg_t arg)
{
    uint8_t             currSliderPos = 0;
    static uint8_t      prevAction = 0;

    UNUSED_PARAMETER(arg);

    /*Start CapSense*/
    CapSense_Start();
    CapSense_ScanAllWidgets();

    /*enter forever loop, remember this is a thread*/
    for(;;)
    {
        /* Do this only when the CapSense isn't busy with a previous operation*/
        if (CapSense_NOT_BUSY == CapSense_IsBusy())
        {
            /* Process data from all the sensors and find out the touch
               information */
            CapSense_ProcessAllWidgets();

            /* Check if button 0 is already touched */
            if (CapSense_IsWidgetActive(CapSense_BUTTON0_WDGT_ID))
            {
                /*Only process if coming from no presses*/
                if(!(prevAction))
                {
                    /*Set flag indicating button 0 was previously pressed*/
                    prevAction |= BUTTON0_PRESSED;
                    /* Print Button 0 touch detection */
                    WPRINT_APP_INFO( ( "Button 0 Pressed \n\r" ) );
                    decreaseDutycycle();
               }

            }
            /* Check if button 1 is touched */
            else if (CapSense_IsWidgetActive(CapSense_BUTTON1_WDGT_ID))
            {
                /*Only process if coming from no presses*/
                if(!(prevAction))
                {
                    /*Set flag indicating button  was previously pressed*/
                    prevAction |= BUTTON1_PRESSED;
                    /* Print button 1 touch detection */
                    WPRINT_APP_INFO( ( "Button 1 Pressed \n\r" ) );
                    increaseDutycycle();
                }
            }
            /* Check if the slider is touched */
            else if (CapSense_IsWidgetActive(CapSense_LINEARSLIDER0_WDGT_ID))
            {

                prevAction |= SLIDER_PRESSED;
                /* Read Current slider position*/
                currSliderPos = CapSense_GetCentroidPos
                                (CapSense_LINEARSLIDER0_WDGT_ID);

                setDutycycle( currSliderPos );
                WPRINT_APP_INFO( ( "Slider Position: %d \n\r", currSliderPos ) );

            }
            else
            {
                /*Clear previous action*/
                prevAction = 0;
            }

            /* Start CapSense scan */
            CapSense_ScanAllWidgets();

        }
        /*Delay so as not to update too quickly*/
        host_rtos_delay_milliseconds(CAPSENSE_UPDATE_TIME);
    }
}
