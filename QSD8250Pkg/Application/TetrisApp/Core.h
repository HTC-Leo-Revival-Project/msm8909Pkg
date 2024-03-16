#pragma once

#include "Board.h"

// 1ms = 10000
#define TIMER_PERIOD 100000
#define GAME_REFRESH_PERIOD 35
// the time between subsequent descents of active tetrimino (on lvl 1) equals TIMER_PERIOD * GAME_REFRESH_PERIOD

#define MAX_LEVEL 10

#if MAX_LEVEL < 1 || MAX_LEVEL > 10 
#error Max level can not be lower than 1 or higher than 10!
#endif // level constraints

typedef enum {
	GAME_STATE_WAITING_FOR_START,
	GAME_STATE_RUNNING,
	GAME_STATE_PAUSED,
	GAME_STATE_OVER,
	GAME_STATE_EXIT
} GAME_STATE;

typedef struct _Core Core;

typedef void( *HANDLE_INPUT )( IN Core* this );
typedef void( *DRAW_WINDOW )( IN Core* this );

struct _Core {
	EFI_EVENT timerEvent;
	GAME_STATE gameState;
	Board* board;
	int tickCounter;

	HANDLE_INPUT handleInput;
	DRAW_WINDOW drawWindow;
};

void ConstructCore( OUT Core** this );
void DestructCore( IN Core* this );