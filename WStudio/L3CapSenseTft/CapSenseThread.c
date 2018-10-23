#include "wiced.h"

void capSenseThread(wiced_thread_arg_t arg)
{

    CapSense_Start();
    CapSense_ScanAllWidgets();
    while(1)
    {
        if(!CapSense_IsBusy())
        {

            CapSense_ProcessAllWidgets();
            if(CapSense_IsWidgetActive(CapSense_BUTTON0_WDGT_ID))
            {
                WPRINT_APP_INFO(("Button 0 Active\n"));
            }

            if(CapSense_IsWidgetActive(CapSense_BUTTON1_WDGT_ID))
            {
                WPRINT_APP_INFO(("Button 1 Active\n"));
            }

            uint32_t val = CapSense_GetCentroidPos(CapSense_LINEARSLIDER0_WDGT_ID);
            if(val < 0xFFFF)
            {
                WPRINT_APP_INFO(("Slider = %d\n",(int)val));

            }
            CapSense_ScanAllWidgets();
        }
        wiced_rtos_delay_milliseconds(25); // Poll every 25ms (actual scan time ~8ms)
    }
}
