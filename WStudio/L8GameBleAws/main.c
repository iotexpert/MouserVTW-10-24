#include "GameThread.h"
#include "GoBleThread.h"
#include "CapSenseThread.h"
#include "subscriber.h"
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
    wiced_rtos_create_thread(&capsenseThreadHandle,7,"CapSense Thread",capSenseThread,1024,0);
    wiced_rtos_create_thread(&gameThreadHandle,7,"game Thread",gameThread,4096,0);
    GoBleThread_start();
    wiced_rtos_create_thread(&awsThreadHandle,7,"AWS Thread",awsThread,4096,0);
 }

