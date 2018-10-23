#include "wiced.h"
#include "CapSenseThread.h"
#include "GameThread.h"

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
wiced_thread_t capSenseThreadHandle;
wiced_thread_t gameThreadHandle;



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
    wiced_init();
    WPRINT_APP_INFO(("Started Application\n"));
    wiced_rtos_create_thread(&blinkThreadHandle,7,"Blink Thread",pdlBlinkThread,500,0);
    wiced_rtos_create_thread(&capSenseThreadHandle,7,"CapSense Thread",capSenseThread,1024,0);
    wiced_rtos_create_thread(&gameThreadHandle,7,"game Thread",gameThread,4096,0);


}
