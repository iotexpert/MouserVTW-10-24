
#include "GameThread.h"

#include "cy_tft_display.h"

#define SCREEN_X (320)
#define SCREEN_Y (240)

static UG_GUI   gui;


// ARH Function to put text in the center of a point (UG_PutString does upper left)
static void UG_PutStringCenter(uint32_t x, uint32_t y, uint32_t fontx, uint32_t fonty,char *string)
{
    y = y - fonty/2;
    x = x - (strlen(string)/2)*fontx;
    if(strlen(string)%2)
        x = x - fontx/2;
    UG_PutString(x,y,string);
}


// Display the splash screen
static void displaySplashScreen()
{
    UG_FontSelect( &FONT_22X36 );
    UG_PutStringCenter(SCREEN_X/2,SCREEN_Y/5,22,36,"Cypress");
    UG_PutStringCenter(SCREEN_X/2,SCREEN_Y/5*2,22,36,"Mouser");
    UG_PutStringCenter(SCREEN_X/2,SCREEN_Y/5*3,22,36,"PSoC 6");
    UG_PutStringCenter(SCREEN_X/2,SCREEN_Y/5*4,22,36,"WICED 4343");

    wiced_rtos_delay_milliseconds(2000);
}

// This function displays the start button message
static void displayStartButton()
{
    UG_FontSelect(&FONT_12X20);
    UG_PutStringCenter(SCREEN_X/2 , SCREEN_Y - 30 ,12,22,  "Press B0 To Start");
}


// Display the Start Screen
static void  displayStartScreen()
{
    UG_FillScreen( C_BLACK );
    UG_FontSelect( &FONT_22X36 );
    UG_PutStringCenter(SCREEN_X/2,SCREEN_Y/2 -2 - 18 ,22,36,"Ready");
    UG_PutStringCenter(SCREEN_X/2,SCREEN_Y/2 + 2 + 18 ,22,36,"Player 1");
    displayStartButton();
}

// Main game thread
void gameThread(wiced_thread_arg_t arg)
{

    Cy_TFT_Init();                                             // Init the TFT
    UG_Init( &gui, Cy_TFT_displayDriver, SCREEN_X, SCREEN_Y ); // Connect the driver

    UG_FillScreen( C_BLACK );   // Clear the screen
    UG_SetBackcolor( C_BLACK );
    UG_SetForecolor( C_WHITE );

    displaySplashScreen();
    displayStartScreen();

    while(1)
    {
        wiced_rtos_delay_milliseconds(1000);
    }
}
