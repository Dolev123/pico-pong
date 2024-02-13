#include "pong.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "LCD_1in14.h"
#include "Infrared.h"

static Player* p1;
static Player* p2;
static Ball* ball;
static UWORD* screen;
static UWORD timer = BOARD_W;
#define P1_GOAL_x (p1->_x_pos+PIXEL_SIZE)
#define P2_GOAL_x (p2->_x_pos-PIXEL_SIZE)
// setups
Errors create_player(Player** p, UBYTE x);
void position_ball(bool is_rand);
// movement
BYTE ai_calculate_speed();
BYTE player_speed(BYTE key_up, BYTE key_down);
Errors move_player(Player* p);
Errors move_ball();
// colisions
Directions ball_player_collision(Player* p);
inline bool point_in_rect(UBYTE x1, UBYTE y1, UBYTE x2, UBYTE y2, UBYTE p_x, UBYTE p_y);
bool check_for_score();
// prints
Errors print_board();
Errors print_score();
Errors print_end_screen();

Errors setup_hw_board() {
	DEV_Delay_ms(200);
	// DEV_Delay_ms(50);
	if (DEV_Module_Init()!=0) {
		return RPI_ERROR;
	}
	DEV_SET_PWM(50);
	LCD_1IN14_Init(HORIZONTAL);
	LCD_1IN14_Clear(BG_COLOR);

	SET_Infrared_PIN(PIN_JOY_UP);
	SET_Infrared_PIN(PIN_JOY_DOWN);
	SET_Infrared_PIN(PIN_JOY_LEFT);
	SET_Infrared_PIN(PIN_JOY_RIGHT);
	SET_Infrared_PIN(PIN_JOY_CTRL);
	SET_Infrared_PIN(PIN_KEY_A);
	SET_Infrared_PIN(PIN_KEY_B);
	
	return NOERROR;
}

Errors setup_game() {
	Errors err = NOERROR;
	srand(time(NULL));
	if ( (err = create_player(&p1, (UBYTE)(0.9*BOARD_W))) ) {
		goto clean_setup;
	}
	if ( err = (create_player(&p2, (UBYTE)(0.1*BOARD_W))) ) {
		goto clean_setup;
	}
	
	ball = (Ball*)malloc(sizeof(Ball));
	if (!ball) {
		err = MEM_ERROR;
		goto clean_setup;
	}
	position_ball(false);
	screen = (UWORD*)malloc(LCD_1IN14_HEIGHT*LCD_1IN14_WIDTH*sizeof(UWORD));
	if (!screen) {
		err = MEM_ERROR;
		goto clean_setup;
	}
	Paint_NewImage((UBYTE*)screen,LCD_1IN14.WIDTH,LCD_1IN14.HEIGHT, 0, BLACK);
	Paint_SetRotate(ROTATE_0);
	Paint_SetScale(65);
	goto setup_done;
clean_setup:
	free(p1);
	free(p2);
	free(ball);
	free(screen);
setup_done:
	return err;
}

void position_ball(bool is_rand) {
	if (is_rand) {
		ball->x = MID_WIDTH - 10 + (random() % 20);
		ball->y = MID_HIGHT - 30 + (random() % 60);
	} else {
		ball->x = MID_WIDTH;
		ball->y = MID_HIGHT;
	}
	ball->x_speed = (random() % 2 ? 1 : -1) * (random() % 3 + 2);
	ball->y_speed = (random() % 2 ? 1 : -1) * (random() % 2 + 1);
}

Errors create_player(Player** p, UBYTE x) {
	Player* tmp_p = (Player*)malloc(sizeof(Player));
	if (!tmp_p) {
		return MEM_ERROR;
	}
	tmp_p->color = FG_COLOR;
	tmp_p->score = 0;
	tmp_p->pos = MID_HIGHT;
	tmp_p->_x_pos = x;
	*p = tmp_p;
	return NOERROR;
}

void end_game() {
	print_end_screen();
	free(p1);
	free(p2);
	free(ball);
	free(screen);
	DEV_Module_Exit();
}

Errors print_board() {
	Paint_SelectImage((UBYTE*)screen);
	Paint_Clear(BLACK);
	// timer
	UWORD color;
	if (timer > MID_WIDTH) {
		color = GREEN;
	} else if (timer > MID_WIDTH/2) {
		color = YELLOW;
	} else {
		color = RED;
	}
	Paint_DrawLine(1, 1, timer, 1, color, 1, DOT_FILL_RIGHTUP);
	// board lines
	Paint_DrawLine(
		MID_WIDTH, 0,
		MID_WIDTH, BOARD_H,
		GRAY, 1, DOT_FILL_RIGHTUP
	);
	Paint_DrawLine(
		P1_GOAL_x, 0,
		P1_GOAL_x, BOARD_H,
		GRAY, 1, DOT_FILL_RIGHTUP
	);
	Paint_DrawLine(
		P2_GOAL_x, 0,
		P2_GOAL_x, BOARD_H,
		GRAY, 1, DOT_FILL_RIGHTUP
	);
	// print score
	Paint_DrawNum(
		MID_WIDTH + 5, 16,
		p1->score, 
		&Font16, 0, GRAY, BLACK
	);
	Paint_DrawNum(
		MID_WIDTH - 16, 16,
		p2->score, 
		&Font16, 0, GRAY, BLACK
	);
	// objects
	Paint_DrawPoint(ball->x, ball->y, WHITE, PIXEL_SIZE, DOT_FILL_RIGHTUP);
	Paint_DrawLine(
		p1->_x_pos, p1->pos - (PLAYER_SIZE/2), 
		p1->_x_pos, p1->pos + (PLAYER_SIZE/2), 
		WHITE, PIXEL_SIZE, DOT_FILL_RIGHTUP
	);
	Paint_DrawLine(
		p2->_x_pos, p2->pos - (PLAYER_SIZE/2), 
		p2->_x_pos, p2->pos + (PLAYER_SIZE/2), 
		WHITE, PIXEL_SIZE, DOT_FILL_RIGHTUP
	);
	// flush to screen
	LCD_1IN14_Display(screen);
}

Errors run_game() {
	print_board();
	UBYTE ticks = 0;
	while(p1->score < MAX_SCORE && p2->score < MAX_SCORE) {
		if (ticks++ % TIMER_TICK == 0 && timer-- == 0) {
			position_ball(true);
			timer = BOARD_W;
		}
		move_ball();
		if (check_for_score()) {
			position_ball(true);
			timer = BOARD_W;
			ticks = 0;
		}
#ifdef P1_COMP
		if (rand() % 5 == 0) {
			p1->speed =  ai_calculate_speed();
		}
#else
		p1->speed = player_speed(PIN_KEY_A, PIN_KEY_B);
#endif
#ifndef P2_PLAYER
		if (rand() % 5 == 0) {
			p2->speed =  ai_calculate_speed();
		}
#else
		p2->speed = player_speed(PIN_JOY_UP, PIN_JOY_DOWN);
#endif
		move_player(p1);
		move_player(p2);
		print_board();
	}
}

Errors move_ball() {
	ball->x += ball->x_speed;
	ball->y += ball->y_speed;
	// check walls collision
	if (0 == ball->y || BOARD_H <= ball->y + PIXEL_SIZE-1) {
		ball->y_speed = (-1) * ball->y_speed;
	}
	if (0 == ball->x || BOARD_W <= ball->x + PIXEL_SIZE-1) {
		ball->x_speed = (-1) * ball->x_speed;
	}
	// check players collision
	Directions ball_change = NODIRECTION;
	switch (ball_player_collision(p1)) {
		case LEFT:
		case RIGHT:
			ball_change = LEFT;
			break;
		case UP:
		case DOWN:
			ball_change = UP;
			break;
	}
	switch (ball_player_collision(p2)) {
		case LEFT:
		case RIGHT:
			ball_change = LEFT;
			break;
		case UP:
		case DOWN:
			ball_change = UP;
			break;
	}
	switch (ball_change) {
		case LEFT:
			ball->x_speed = (-1) * ball->x_speed;
			ball->x += 2 * ball->x_speed;
			break;
		case UP:
			ball->y_speed = (-1) * ball->y_speed;
			ball->x_speed = (-1) * ball->x_speed;
			ball->x += 2 * ball->x_speed;
			ball->y += 2 * ball->y_speed;
			break;
	}
}

inline bool point_in_rect(UBYTE x1, UBYTE y1, UBYTE x2, UBYTE y2, UBYTE p_x, UBYTE p_y) {
	return x1 <= p_x && p_x <= x2 && y1 <= p_y && p_y <= y2;
}

Directions ball_player_collision(Player* p) {

	UBYTE p_left =  p->_x_pos - PIXEL_SIZE+1;
	UBYTE p_right = p->_x_pos + PIXEL_SIZE-1;
	UBYTE p_up =    p->pos    - PIXEL_SIZE+1 - (PLAYER_SIZE/2);
	UBYTE p_down =  p->pos    + PIXEL_SIZE-1 + (PLAYER_SIZE/2);

	bool top_left = point_in_rect(
		p_left, p_up, p_right, p_down,
		ball->x-1 , ball->y-1
	);
	bool bottom_left = point_in_rect(
		p_left, p_up, p_right, p_down,
		ball->x-1, ball->y+1
	);
	bool top_right = point_in_rect(
		p_left, p_up, p_right, p_down,
		ball->x+1 , ball->y-1
	);
	bool bottom_right = point_in_rect(
		p_left, p_up, p_right, p_down,
		ball->x+1, ball->y+1
	);

	if (top_left && bottom_left) {
		return RIGHT;
	} else if (top_right && bottom_right) {
		return LEFT;
	} else if (top_right && top_left) {
		return DOWN;
	} else if (bottom_right && bottom_left) {
		return UP;
	} else if(bottom_right || top_right) {
		return LEFT;
	} else if (bottom_left || top_left) {
		return RIGHT;
	}
	return NODIRECTION;
}

bool check_for_score() {
	// check if all the ball is beyond the goal line
	if (ball->x-1 > P1_GOAL_x) {
		p2->score++;
		return true;
	}
	if (ball->x+1 < P2_GOAL_x) {
		p1->score++;
		return true;
	}
	return false;
}

Errors move_player(Player* p) {
	UBYTE new_y = p->pos + p->speed;
	if (
		// check by border location
		0       == new_y - (PLAYER_SIZE/2) - PIXEL_SIZE+1 || 
		BOARD_H <= new_y + (PLAYER_SIZE/2) + PIXEL_SIZE-1 ||
		// check for overflow
		(p->speed > 0 && new_y < p->pos) || (p->speed < 0 && new_y > p->pos)
	) {
		return PLAYER_OUTBOUND;
	}
	p->pos = new_y;
	return NOERROR;
}

BYTE ai_calculate_speed() {
	return rand() % BOARD_H > ball->y ? (BYTE)(-1) : 1;
}

BYTE player_speed(BYTE key_up, BYTE key_down) {
	BYTE speed = 0;
	speed -= DEV_Digital_Read(key_up) == 0 ? 1 : 0;
	speed += DEV_Digital_Read(key_down) == 0 ? 1 : 0;
	return speed;
}

Errors print_end_screen() {
	// assuming a game was actually won...
	Paint_SelectImage((UBYTE*)screen);
	Paint_Clear(BLACK);
	char msg[14] = {0};
	sprintf(msg, "Player %d Won!", p1->score >= MAX_SCORE ? 1 : 2);
	Paint_DrawString_EN(
		MID_WIDTH - ((sizeof(msg)/2*11)), MID_HIGHT-16,
		msg, &Font16, BLACK, p1->score >= MAX_SCORE ? GREEN : BLUE
	);
	LCD_1IN14_Display(screen);
}
