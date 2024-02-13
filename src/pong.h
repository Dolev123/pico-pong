#ifndef __PONG
#define __PONG

#include "GUI_Paint.h"

#define BYTE int8_t

#define PIN_JOY_UP 2
#define PIN_JOY_DOWN 18
#define PIN_JOY_LEFT 16
#define PIN_JOY_RIGHT 20
#define PIN_JOY_CTRL 3
#define PIN_KEY_A 15
#define PIN_KEY_B 17

#define BG_COLOR BLACK
#define FG_COLOR WHITE

// currently p1 is the player and p2 is the computer...
// #define P1_COMP // make p1 a computer played charecter
// #define P2_PLAYER // make p2 a playable charecter (make sure the joystick is working well)

#define PLAYER_SIZE 20
#define TIMER_TICK 10
#define MAX_SCORE 9

// Because we set it as HORIZONTAL:
// HEIGHT = LCD_1IN14_WIDTH  = 135 
// WIDTH  = LCD_1IN14_HEIGHT = 240
#define PIXEL_SIZE 3
#define BOARD_H ((UWORD)(LCD_1IN14_WIDTH))
#define BOARD_W ((UWORD)(LCD_1IN14_HEIGHT))
#define MID_HIGHT ((UWORD)(BOARD_H / 2))
#define MID_WIDTH ((UWORD)(BOARD_W / 2))

typedef struct _Player {
	UWORD color;
	UBYTE score;
	UBYTE pos;
	BYTE speed;
	// should not change after initialization.
	UBYTE _x_pos;
} Player;

typedef struct _Ball {
	UBYTE x;
	UBYTE y;
	BYTE x_speed;
	BYTE y_speed;
} Ball;

typedef enum _Errors {
	NOERROR = 0,
	ERROR,
	RPI_ERROR,
	LCD_ERROR,
	MEM_ERROR,
	PLAYER_OUTBOUND,
} Errors;

typedef enum _Directions {
	NODIRECTION = 0,
	UP,
	DOWN,
	LEFT,
	RIGHT,
} Directions;

Errors setup_game();
Errors setup_hw_board();
Errors run_game();
void end_game();

#endif
