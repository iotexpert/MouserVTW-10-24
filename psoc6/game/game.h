#pragma once
#include <stdint.h>

typedef enum {
    MSG_POSITION,
    MSG_BUTTON0,
    MSG_BUTTON1,
} game_evt_t;

typedef struct {
    game_evt_t evt;
    uint32_t val;
} game_msg_t;

void gameThread(void *arg);

