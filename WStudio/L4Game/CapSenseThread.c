#include "wiced.h"
#include "GameThread.h"
#include "SystemGlobal.h"

void capSenseThread(wiced_thread_arg_t arg)
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
