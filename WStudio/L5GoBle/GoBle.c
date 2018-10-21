
#include "GoBleThread.h"
#include "wiced.h"


/*******************************************************************
 * Function Definitions
 ******************************************************************/
void application_start(void)
{
    wiced_init();
    GoBleThread_start();

}


