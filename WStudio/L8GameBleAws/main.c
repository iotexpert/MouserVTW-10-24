#include "../WStudio/L8GameBleAws/GameThread.h"
#include "../WStudio/L8GameBleAws/GoBleThread.h"
#include "../WStudio/L8GameBleAws/subscriber.h"
#include "wiced.h"

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


/******************************************************
 *               Variable Definitions
 ******************************************************/

wiced_thread_t blinkThreadHandle;
wiced_thread_t capsenseThreadHandle;
wiced_thread_t gameThreadHandle;
wiced_thread_t awsThreadHandle;

wiced_queue_t paddleQueue;

/******************************************************
 *               Function Definitions
 ******************************************************/

void capsenseThread(wiced_thread_arg_t arg)
{

    game_msg_t msg;

    CapSense_Start();
    CapSense_ScanAllWidgets();
    while(1)
    {
        if(!CapSense_IsBusy())
        {

            CapSense_ProcessAllWidgets();
            if(CapSense_IsWidgetActive(CapSense_BUTTON0_WDGT_ID))
            {
                msg.evt = MSG_BUTTON0;
                wiced_rtos_push_to_queue(&paddleQueue,&msg,0);
            }

            if(CapSense_IsWidgetActive(CapSense_BUTTON1_WDGT_ID))
            {
                msg.evt = MSG_BUTTON1;
                wiced_rtos_push_to_queue(&paddleQueue,&msg,0);
            }

            uint32_t val = CapSense_GetCentroidPos(CapSense_LINEARSLIDER0_WDGT_ID);
            if(val < 0xFFFF)
            {
                msg.evt = MSG_POSITION;
                msg.val = val;
                wiced_rtos_push_to_queue(&paddleQueue,&msg,0);
            }
            CapSense_ScanAllWidgets();
        }
        wiced_rtos_delay_milliseconds(25); // Poll every 25ms (actual scan time ~8ms)
    }
}


void pdlBlinkThread(wiced_thread_arg_t arg)
{
     while(1)
     {
         Cy_GPIO_Inv(GPIO_PRT0,3);
         wiced_rtos_delay_milliseconds(500);
     }
}


void application_start( )
{
    wiced_init( );
    wiced_rtos_init_queue(&paddleQueue,"paddleQueue",sizeof(game_msg_t),10);
    wiced_rtos_create_thread(&blinkThreadHandle,7,"Blink Thread",pdlBlinkThread,500,0);
    wiced_rtos_create_thread(&capsenseThreadHandle,7,"CapSense Thread",capsenseThread,1024,0);
    wiced_rtos_create_thread(&gameThreadHandle,7,"game Thread",gameThread,4096,0);
    GoBleThread_start();
    wiced_rtos_create_thread(&awsThreadHandle,7,"AWS Thread",awsThread,4096,0);
 }

