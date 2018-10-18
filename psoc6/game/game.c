
#include "../game/game.h"

#include "../game/cy_tft_display.h"
#include "ugui.h"

extern wiced_queue_t paddleQueue;

UG_GUI   gui;
static wiced_timer_t updateScreenTimer;
static wiced_timer_t splashScreenTimer;

typedef enum {
    GS_SPLASH,
    GS_START,
    GS_RUNNING,
    GS_OVER
} game_state_t;

game_state_t gameState;



#define SPEED (2)
#define SCREEN_X 320
#define SCREEN_Y 240
#define TOP_FIELD 21
#define PD_WIDTH (10)
#define PD_LEN (70)
#define DOTS (3)
#define PADDLE0_COLOR (C_BLUE)
#define BALL_COLOR (C_GREEN)
#define BALL_SIZE (10)


static uint32_t paddle0_desire_pos=0;
static uint32_t paddle0_cur_pos=0;

static uint32_t ballx,bally;
static int32_t ballXdir, ballYdir;

static uint32_t ballSpeed;
static uint32_t gameScore;


static inline uint32_t calcPaddleTop();
static inline uint32_t calcPaddleBottom();

static void UG_PutStringCenter(uint32_t x, uint32_t y, uint32_t fontx, uint32_t fonty,char *string)
{
    uint32_t len = strlen(string);
    y = y - fonty/2;
    x = x - (len/2)*fontx;
    UG_PutString(x,y,string);

}
static void displayStartButton()
{
    UG_FontSelect(&FONT_12X20);
    UG_PutStringCenter(SCREEN_X/2 , SCREEN_Y - 30 ,12,22,  "Press B0 To Start");
}

static void displayScore()
{
    char buff[10];
    sprintf(buff,"%2X",gameScore);
    UG_FontSelect(&FONT_12X20);
    UG_PutString( 75, 0, buff);
}

static void displaySpeed()
{
    char buff[10];
    sprintf(buff,"%2X",ballSpeed-1);
    UG_FontSelect(&FONT_12X20);
    UG_PutString( 275, 0, buff);
}

static void startGame(void *arg)
{

    paddle0_desire_pos = 50;
    paddle0_cur_pos = 0;

    ballx = PD_WIDTH ;
    bally  = calcPaddleTop() + PD_LEN/2;

    ballSpeed = 2;
    gameScore = 0;

    ballXdir = ballSpeed;
    ballYdir = ballSpeed;

    // clear screen
    UG_FillScreen( C_BLACK );

    UG_FontSelect(&FONT_12X20);
    UG_PutString( 0, 0,  "Score:");
    UG_DrawLine(0,20,SCREEN_X,20,C_RED);
    displayScore();
    UG_PutString(200,0,"Speed:");
    displaySpeed();

    gameState = GS_RUNNING;

    wiced_rtos_start_timer(&updateScreenTimer);

}


static void endGame()
{
    gameState = GS_OVER;
    wiced_rtos_stop_timer(&updateScreenTimer);
    UG_FontSelect( &FONT_22X36 );
    UG_PutStringCenter(SCREEN_X/2,SCREEN_Y/2,22,36,"Game Over");
    displayStartButton();
}



static inline uint32_t calcPaddleTop()
{
    return (paddle0_cur_pos)*DOTS+TOP_FIELD;
}

static inline uint32_t calcPaddleBottom()
{
    return (paddle0_cur_pos)*DOTS+PD_LEN+TOP_FIELD;

}

static void updatePaddles()
{

    if(paddle0_cur_pos != paddle0_desire_pos)
    {
        UG_FillFrame(0,calcPaddleTop(),PD_WIDTH,calcPaddleBottom(),C_BLACK);

        if(paddle0_cur_pos < paddle0_desire_pos)
            paddle0_cur_pos += SPEED;
        else
            paddle0_cur_pos -= SPEED;

        if(abs((int)paddle0_cur_pos-(int)paddle0_desire_pos) < SPEED)
            paddle0_cur_pos = paddle0_desire_pos;

        UG_FillFrame(0,calcPaddleTop(),PD_WIDTH,calcPaddleBottom(),PADDLE0_COLOR);

    }

}

static void updateBall()
{

    UG_DrawCircle(ballx,bally,BALL_SIZE,C_BLACK);

    ballx += ballXdir;
    bally += ballYdir;

    // Check to see if the ball hit the far right side
    if(ballx > SCREEN_X - BALL_SIZE)
    {
        gameScore = gameScore + 1;
        displayScore();
        if(gameScore % 3 == 0)
        {
            ballSpeed +=1;
            displaySpeed();
        }

        ballx = SCREEN_X - BALL_SIZE;
        ballXdir = -ballSpeed;
    }

    // check to see if the ball hit the far left side... or the paddle
    if(ballx < (BALL_SIZE + PD_WIDTH + 3))
    {
        // See if the ball missed the paddle
        if(bally + BALL_SIZE < calcPaddleTop() || bally - BALL_SIZE > calcPaddleBottom())
        {
            endGame();
            //WPRINT_APP_INFO(("Missed Paddle\r\n"));
        }

        ballx = BALL_SIZE + PD_WIDTH + 3;
        ballXdir = +ballSpeed;
    }


    // Check to see if the ball hit the sides
    if(bally > SCREEN_Y - BALL_SIZE)
    {
        bally = SCREEN_Y - BALL_SIZE;
        ballYdir = -ballSpeed;
    }

    if(bally < TOP_FIELD+BALL_SIZE)
    {
        bally = BALL_SIZE+TOP_FIELD;
        ballYdir = +ballSpeed;
    }

    UG_DrawCircle(ballx,bally,BALL_SIZE,BALL_COLOR);

}

static void updateScreen(void *arg)
{
    updatePaddles();
    updateBall();
}


static void startTFT()
{

    /*Initialize the TFT LCD*/
    Cy_TFT_Init();
    /*Connect ugui display driver*/
    UG_Init( &gui, Cy_TFT_displayDriver, 320, 240 );

    /*Setup display for this demo*/
    /*Fill screen with black*/
    UG_FillScreen( C_BLACK );
    UG_SetBackcolor( C_BLACK );
    UG_SetForecolor( C_WHITE );

}

static void splashScreen()
{
    gameState = GS_SPLASH;
    UG_FontSelect( &FONT_22X36 );
    UG_PutStringCenter(SCREEN_X/2,SCREEN_Y/2,22,36,"PSoC 6");

    wiced_rtos_delay_milliseconds(2000);
}

static void  startScreen()
{
    gameState = GS_START;
    UG_FillScreen( C_BLACK );
    UG_FontSelect( &FONT_22X36 );
    UG_PutStringCenter(SCREEN_X/2,SCREEN_Y/2 -2 - 18 ,22,36,"Ready");
    UG_PutStringCenter(SCREEN_X/2,SCREEN_Y/2 + 2 + 18 ,22,36,"Player 1");
    displayStartButton();

}


void gameThread(void *arg)
{
    startTFT();
    game_msg_t msg;

    wiced_rtos_init_timer(&updateScreenTimer,20,updateScreen,0);
    splashScreen();
    startScreen();

    while(1)
    {
        wiced_rtos_pop_from_queue(&paddleQueue,&msg,0xFFFFFFFF);
        switch(msg.evt)
        {
        case MSG_POSITION:
            paddle0_desire_pos = msg.val/2;
            break;
        case MSG_BUTTON0:
            if(gameState == GS_OVER || gameState == GS_START)
                startGame(0);
            break;
        case MSG_BUTTON1:
            break;
        }
    }
}
