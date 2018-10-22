
#include "../WStudio/L8GameBleAws/GameThread.h"

#include "../WStudio/L8GameBleAws/cy_tft_display.h"
#include "../WStudio/L8GameBleAws/SystemGlobal.h"
#include "ugui.h"

/******************************************************
 *                      Macros
 ******************************************************/
#define UPDATE_SCREEN_TIME (20) // Update the screen every 20ms
#define SPEED (2)
#define SCREEN_X (320)
#define SCREEN_Y (240)
#define TOP_FIELD (21)
#define PD_WIDTH (10)
#define PD_LEN (70)
#define DOTS (3)
#define PADDLE0_COLOR (C_BLUE)
#define BALL_COLOR (C_GREEN)
#define BALL_SIZE (10)

/******************************************************
 *                    Constants
 ******************************************************/

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/
// States of the game
typedef enum {
    GS_SPLASH,
    GS_START,
    GS_RUNNING,
    GS_OVER
} game_state_t;

// Methods to move the paddle
typedef enum {
    PADDLE_INCREMENT,
    PADDLE_ABSOLUTE
} paddle_update_t;

/******************************************************
 *                    Structures
 ******************************************************/


/******************************************************
 *               Static Function Declarations
 ******************************************************/
static void UG_PutStringCenter(uint32_t x, uint32_t y, uint32_t fontx, uint32_t fonty,char *string);
static void displayStartButton();
static void displayScore();
static void displaySpeed();
static void endGame();
static inline uint32_t calcPaddleTop();
static inline uint32_t calcPaddleBottom();
static void updatePaddle(paddle_update_t type);
static void updateBall();
static void updateScreen(void *arg);
static void displaySplashScreen();
static void  displayStartScreen();

/******************************************************
 *               Variable Definitions
 ******************************************************/
static UG_GUI   gui;
static wiced_timer_t updateScreenTimer;

static uint32_t gameScore;
static game_state_t gameState;

// position of the paddle
static uint32_t paddle0_desire_pos=0;
static uint32_t paddle0_cur_pos=0;

// Position, direction and speed of the ball
static uint32_t ballx,bally;
static int32_t ballXdir, ballYdir;
static uint32_t ballSpeed;


/******************************************************
 *               Functions
 ******************************************************/

// ARH Function to put text in the center of a point (UG_PutString does upper left)
static void UG_PutStringCenter(uint32_t x, uint32_t y, uint32_t fontx, uint32_t fonty,char *string)
{
    y = y - fonty/2;
    x = x - (strlen(string)/2)*fontx;
    if(strlen(string)%2)
        x = x - fontx/2;
    UG_PutString(x,y,string);
}

// This function displays the start button message
static void displayStartButton()
{
    UG_FontSelect(&FONT_12X20);
    UG_PutStringCenter(SCREEN_X/2 , SCREEN_Y - 30 ,12,22,  "Press B0 To Start");
}

// This function displays the score
static void displayScore()
{
    char buff[10];
    sprintf(buff,"%2X",(unsigned int)gameScore);
    UG_FontSelect(&FONT_12X20);
    UG_PutString( 75, 0, buff);
}

// This function displays the speed
static void displaySpeed()
{
    char buff[10];
    sprintf(buff,"%2X",(unsigned int)ballSpeed-1);
    UG_FontSelect(&FONT_12X20);
    UG_PutString( 275, 0, buff);
}


// This function initializes everything and starts a new game
static void startGame()
{
    gameScore = 0;

    paddle0_desire_pos = 50; // start the game with the paddle moving
    paddle0_cur_pos = 0;

    ballx = PD_WIDTH ;                   // start the ball on the paddle on the right of the screen
    bally  = calcPaddleTop() + PD_LEN/2; // start the ball in the middle of the paddle

    ballSpeed = SPEED;

    ballXdir = ballSpeed;
    ballYdir = ballSpeed;

    UG_FillScreen( C_BLACK );  // clear screen
    UG_FontSelect(&FONT_12X20);
    UG_PutString( 0, 0,  "Score:");
    displayScore();
    UG_PutString(200,0,"Speed:");
    displaySpeed();
    UG_DrawLine(0,20,SCREEN_X,20,C_RED); // red line under text to represent top of play screen

    gameState = GS_RUNNING;
    wiced_rtos_start_timer(&updateScreenTimer); // Timer to update screen
}

// Stop the game
static void endGame()
{
    gameState = GS_OVER;
    wiced_rtos_stop_timer(&updateScreenTimer);
    UG_FontSelect( &FONT_22X36 );
    UG_PutStringCenter(SCREEN_X/2,SCREEN_Y/2,22,36,"Game Over");
    displayStartButton();
}


// Figure out the y position of the top of the paddle
static inline uint32_t calcPaddleTop()
{
    return (paddle0_cur_pos)*DOTS+TOP_FIELD;
}

// Figure out the y position of the bottom of the paddle
static inline uint32_t calcPaddleBottom()
{
    return (paddle0_cur_pos)*DOTS+PD_LEN+TOP_FIELD;
}

// Move the paddle either to : PADDLE_INCREMENT the next location or PADDLE_ABSOLUTE - final location
static void updatePaddle(paddle_update_t type)
{
    // If the paddle is where it is supposed to be then just return
    if(paddle0_cur_pos == paddle0_desire_pos)
        return;

    // erase the current paddle
    UG_FillFrame(0,calcPaddleTop(),PD_WIDTH,calcPaddleBottom(),C_BLACK);

    switch (type)
    {

    case PADDLE_INCREMENT:

        if(paddle0_cur_pos < paddle0_desire_pos)
            paddle0_cur_pos += SPEED;
        else
            paddle0_cur_pos -= SPEED;

        // If the paddle is within one move of the final spot, put it there
        if(abs((int)paddle0_cur_pos-(int)paddle0_desire_pos) < SPEED)
            paddle0_cur_pos = paddle0_desire_pos;
        break;

    case PADDLE_ABSOLUTE:
        paddle0_cur_pos = paddle0_desire_pos;
        break;
    }
    // draw the paddle
    UG_FillFrame(0,calcPaddleTop(),PD_WIDTH,calcPaddleBottom(),PADDLE0_COLOR);
}

// Move the ball to the next location
static void updateBall()
{
    static const uint32_t BallFudgeFactor=3;

    UG_DrawCircle(ballx,bally,BALL_SIZE,C_BLACK);

    ballx += ballXdir;
    bally += ballYdir;

    // Check to see if the ball hit the far right side
    if(ballx > SCREEN_X - BALL_SIZE)
    {

        ballx = SCREEN_X - BALL_SIZE;
        ballXdir = -ballSpeed;
    }

    // check to see if the ball hit the far left side... or the paddle
    if(ballx < (BALL_SIZE + PD_WIDTH + BallFudgeFactor))
    {
        // See if the ball missed the paddle
        if(bally + BALL_SIZE < calcPaddleTop() || bally - BALL_SIZE > calcPaddleBottom())
        {
            endGame();
            //WPRINT_APP_INFO(("Missed Paddle\r\n"));
        }

        gameScore = gameScore + 1;
        displayScore();
        if(gameScore % 3 == 0) // Speed up every three hits
        {
            ballSpeed +=1;
            displaySpeed();
        }

        ballx = BALL_SIZE + PD_WIDTH + BallFudgeFactor;
        ballXdir = +ballSpeed;
    }
    // Check to see if the ball hit the top or bottom
    if(bally > SCREEN_Y - BALL_SIZE) // bottom
    {
        bally = SCREEN_Y - BALL_SIZE;
        ballYdir = -ballSpeed;
    }

    if(bally < TOP_FIELD+BALL_SIZE) // top
    {
        bally = BALL_SIZE+TOP_FIELD;
        ballYdir = +ballSpeed;
    }
    UG_DrawCircle(ballx,bally,BALL_SIZE,BALL_COLOR);
}

// This function is called every UPADTE_SCREEN_TIME milliseconds by the updateScreenTimer
static void updateScreen(void *arg)
{
    updatePaddle(PADDLE_INCREMENT);
    updateBall();
}

// Display the splash screen
static void displaySplashScreen()
{
    gameState = GS_SPLASH;
    UG_FontSelect( &FONT_22X36 );
    UG_PutStringCenter(SCREEN_X/2,SCREEN_Y/5,22,36,"Cypress");
    UG_PutStringCenter(SCREEN_X/2,SCREEN_Y/5*2,22,36,"Mouser");
    UG_PutStringCenter(SCREEN_X/2,SCREEN_Y/5*3,22,36,"PSoC 6");
    UG_PutStringCenter(SCREEN_X/2,SCREEN_Y/5*4,22,36,"WICED 4343");

    wiced_rtos_delay_milliseconds(2000);
}


// Display the Start Screen
static void  displayStartScreen()
{
    gameState = GS_START;
    UG_FillScreen( C_BLACK );
    UG_FontSelect( &FONT_22X36 );
    UG_PutStringCenter(SCREEN_X/2,SCREEN_Y/2 -2 - 18 ,22,36,"Ready");
    UG_PutStringCenter(SCREEN_X/2,SCREEN_Y/2 + 2 + 18 ,22,36,"Player 1");
    displayStartButton();
}

// Main game thread
void gameThread(wiced_thread_arg_t arg)
{
    game_msg_t msg;

    Cy_TFT_Init();                                             // Init the TFT
    UG_Init( &gui, Cy_TFT_displayDriver, SCREEN_X, SCREEN_Y ); // Connect the driver

    UG_FillScreen( C_BLACK );   // Clear the screen
    UG_SetBackcolor( C_BLACK );
    UG_SetForecolor( C_WHITE );

    wiced_rtos_init_timer(&updateScreenTimer,UPDATE_SCREEN_TIME,updateScreen,0);
    displaySplashScreen();
    displayStartScreen();

    while(1)
    {
        wiced_rtos_pop_from_queue(&paddleQueue,&msg,WICED_WAIT_FOREVER);
        switch(msg.evt)
        {
        case MSG_POSITION:
            if(gameState == GS_RUNNING)
                paddle0_desire_pos = msg.val/2;
            if(gameState == GS_OVER)
            {
                paddle0_desire_pos = msg.val/2;
                updatePaddle(PADDLE_ABSOLUTE);
            }
            break;

        case MSG_BUTTON0:
            if(gameState == GS_OVER || gameState == GS_START)
                startGame();
            break;
        case MSG_BUTTON1:
            break;
        }
    }
}
