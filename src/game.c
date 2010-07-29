/*
===========================================================================
Copyright © 2010 Sebastien Raymond <glittercutter@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
===========================================================================
*/
// game.c - 


#include "editor.h"

#include "draw.h"
#include "fx.h"
#include "game.h"
#include "input.h"
#include "main.h"
#include "menu.h"
#include "ui.h"


/* 
====================
g_change_state

Start a new game
====================
*/
void g_change_state()
{
	gamestate = GAME;
	active_dropmenu = NULL;

	if (g_init()) ed_change_state();
}

/* 
====================
g_end

Create end game winner message
====================
*/
void g_end()
{
	char string[ STRING_LENGTH ];

	if (score[0] == score[1]) {
		snprintf(string, STRING_LENGTH - 1, "%s !", text.no_win);
		winning_player = 0;
	} else {
		if (score[0] > score[1]) {
			winning_player = 1;
		} else winning_player = 2;
		snprintf(string, STRING_LENGTH - 1, "%s %d %s !", text.player, 
				winning_player, text.win);
	}
	ui_new_message(string);
}


void g_check_complete_square(Square *square)
{	
	if (square->owner != NONE) return;

	if (square->owner_up == NONE) return;
	if (square->owner_right == NONE) return;
	if (square->owner_down == NONE) return;
	if (square->owner_left == NONE) return;
	
	square->owner = player_turn;

	++score[player_turn - 1];
	--squares_remaining;

	if (!squares_remaining) g_end();
}


void g_add_segment()
{
	int pos_x = g_min_x + (input.mouse_x - g_offset_x) / g_square_size;
	int pos_y = g_min_y + (input.mouse_y - g_offset_y) / g_square_size;

 	int center_x = squares[pos_y][pos_x].x1 + 
			(squares[pos_y][pos_x].x2 - squares[pos_y][pos_x].x1) / 2;
 	int center_y = squares[pos_y][pos_x].y1 + 
			(squares[pos_y][pos_x].y2 - squares[pos_y][pos_x].y1) / 2;

	int current_dist;
	int shortest_dist = g_square_size * 2; // higher then max distance
	int closest_segment;
	
	if (!squares[pos_y][pos_x].active) return;
	
	// top

	current_dist = sqrt(ipow(input.mouse_y - squares[pos_y][pos_x].y1, 2) + 
			ipow(input.mouse_x - center_x, 2));
	if (current_dist < shortest_dist) {
		shortest_dist = current_dist;
		losest_segment = UP;
	}
	
	// right
	current_dist = sqrt(ipow(input.mouse_y - center_y, 2) + 
			ipow(input.mouse_x - squares[pos_y][pos_x].x2, 2));
	if (current_dist < shortest_dist) {
		shortest_dist = current_dist;
		closest_segment = RIGHT;
	}

	// down
	current_dist = sqrt(ipow(input.mouse_x - center_x, 2) + 
			ipow(input.mouse_y - squares[pos_y][pos_x].y2, 2));
	if (current_dist < shortest_dist) {
		shortest_dist = current_dist;
		closest_segment = DOWN;
	}

	// left
	current_dist = sqrt(ipow(input.mouse_x - squares[pos_y][pos_x].x1, 2) + 
			ipow(input.mouse_y - center_y, 2));
	if (current_dist < shortest_dist) {
		shortest_dist = current_dist;
		closest_segment = LEFT;
	}

	switch (closest_segment) {
		case UP:
			if (squares[pos_y][pos_x].owner_up == NONE) {
				squares[pos_y][pos_x].owner_up = player_turn;
				*squares[pos_y][pos_x].neighbour_up = player_turn;
			} else return;
		break;

		case RIGHT:
			if (squares[pos_y][pos_x].owner_right == NONE) {
				squares[pos_y][pos_x].owner_right = player_turn;
				*squares[pos_y][pos_x].neighbour_right = player_turn;
			} else return;
		break;

		case DOWN:
			if (squares[pos_y][pos_x].owner_down == NONE) {
				squares[pos_y][pos_x].owner_down = player_turn;
				*squares[pos_y][pos_x].neighbour_down = player_turn;
			} else return;
		break;
		
		case LEFT:
			if (squares[pos_y][pos_x].owner_left == NONE) {
				squares[pos_y][pos_x].owner_left = player_turn;
				*squares[pos_y][pos_x].neighbour_left = player_turn;
			} else return;
		break;
	}

	g_check_complete_square(&squares[pos_y][pos_x]);
	// check neighbour sqares
	if (pos_y != 0) g_check_complete_square(&squares[pos_y - 1][pos_x]);
	if (pos_x != ed_grid_w) g_check_complete_square(&squares[pos_y][pos_x + 1]);
	if (pos_x != ed_grid_h) g_check_complete_square(&squares[pos_y + 1][pos_x]);
	if (pos_x != 0) g_check_complete_square(&squares[pos_y][pos_x - 1]);

	input.mouse_button_left = FALSE;
	
	// game ended
	if (!squares_remaining) {
		if (player_turn == 0) return; // no winner
		// move current player moving square to winning player
		if (player_turn != winning_player) {
			player_turn = winning_player;
			fx_new_transition(NULL, 5, FX_PLAYER_CHANGE);
		}
		return;
	}

	if (player_turn == PLAYER_0) {
		player_turn = PLAYER_1;
	} else {
		player_turn = PLAYER_0;
	}
	fx_new_transition(NULL, 5, FX_PLAYER_CHANGE);
}



void g_do_game()
{
	if (input.mouse_button_left || input.mouse_button_right) {
		if (ui_button_check_click(&button_game)) return;
		g_add_segment();
	}
	
	fx_game();
}


void g_init_ui()
{
// 	int min_w = 70;
// 	int max_w = 100;
// 	int w;
// 	int h = button_font.size + UI_BAR_PADDING;
// 	int x, y;
// 	
// 	x = 0; y = 0;
// 	w = strlen(text.edit) * button_font.w + UI_BAR_PADDING;
// 	ui_new_button(x, y, w, h, min_w, max_w, ALIGN_CENTER, text.edit,
// 			*ed_change_state, &button_game);
}


/* 
====================
g_init_square

Set square size and position for the new game
====================
*/
int g_init_square()
{	
	int tmp_size;
	int w = display_width - 1;
	int h = display_height - button_topbar->h - 1;

	g_min_x = ed_grid_w;
	g_max_x = 0;;
	g_min_y = ed_grid_h;
	g_max_y = 0;

	// find dimension
	for (int i = 0; i < ed_grid_h; i++) {
		for (int j = 0; j < ed_grid_w; j++) {
			if (squares[i][j].active) {
				if (i < g_min_y) g_min_y = i;
				if (i > g_max_y) g_max_y = i;
				if (j < g_min_x) g_min_x = j;
				if (j > g_max_x) g_max_x = j;
			}
		}
	}
	
	if ((g_max_x - g_min_x <= 1) || (g_max_y - g_min_y <= 1)) {
		ui_new_message("not enough square!");
		return 1;
	}

	g_max_x += 1;
	g_max_y += 1;


	// find new square size
	tmp_size = w / (g_max_x - g_min_x);
	g_square_size = h / (g_max_y - g_min_y);
	if (tmp_size < g_square_size) g_square_size = tmp_size;

	g_offset_x = (display_width - ((g_max_x - g_min_x) * g_square_size)) / 2 ;
	g_offset_y = button_topbar->h + (h - ((g_max_y - g_min_y) * g_square_size)) / 2;

	int x = g_offset_x;
	int y = g_offset_y;

	for (int i = g_min_y; i < g_max_y; i++) {
		for (int j = g_min_x; j < g_max_x; j++) {
			squares[i][j].x1 = x;
			squares[i][j].x2 = x + g_square_size;
			squares[i][j].y1 = y;
			squares[i][j].y2 = y + g_square_size;
			x += g_square_size;
		}
		x = g_offset_x;
		y += g_square_size;
	}
	return 0;
}


/* 
====================
g_init

Called before every game
====================
*/
int g_init()
{
	player_turn = PLAYER_0;	
	
	score[0] = 0; score[1] = 0;

	squares_remaining = 0;
	
	if (g_init_square()) return 1;

	for (int i = 0; i < ed_grid_h; i++) {
		for (int j = 0; j < ed_grid_w; j++) {

			if (!squares[i][j].active) continue;
			
			squares[i][j].owner = NONE;
			++squares_remaining;

			// up
			if ((i == 0) || (!squares[i - 1][j].active)) {
				squares[i][j].owner_up = OUTLINE;
			} else {
				squares[i][j].owner_up = NONE;
			}
			// right
			if ((j == ed_grid_w - 1) || (!squares[i][j + 1].active)) {	
				squares[i][j].owner_right = OUTLINE;
			} else {
				squares[i][j].owner_right = NONE;
			}
			// bottom
			if ((i == ed_grid_h - 1) || (!squares[i + 1][j].active)) {
				squares[i][j].owner_down = OUTLINE;
			} else {
				squares[i][j].owner_down = NONE;
			}
			// left
			if ((j == 0) || (!squares[i][j - 1].active)) {	
				squares[i][j].owner_left = OUTLINE;
			} else {
				squares[i][j].owner_left = NONE;
			}			
		}
	}
	return 0;
}


