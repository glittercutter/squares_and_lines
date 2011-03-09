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

#include "client.h"
#include "common.h"
#include "draw.h"
#include "fx.h"
#include "game.h"
#include "input.h"
#include "main.h"
#include "menu.h"
#include "net.h"
#include "server.h"
#include "ui.h"


/* 
====================
set_gamestate_GAME

Start a new game
====================
*/
void set_gamestate_GAME()
{
	if (g_init()) 
		return; // return to the editor if game init failed

	gamestate = GAME;

	if (net_is_server) {
		net_write_sync_square();
		srv_sync_player_name();
		net_write_int(STATE_CHANGE_BYTE, 1, GAME);
	}
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

	if (player[0].score == player[1].score) {
		snprintf(string, STRING_LENGTH - 1, "%s !", text.no_win);
		winning_player = NONE;
	} else {
		if (player[0].score > player[1].score) {
			winning_player = PLAYER_0;
		} else {
			winning_player = PLAYER_1;
		}
		if (net_game) {
			// show player name
			snprintf(string, STRING_LENGTH - 1, "%s %s !",
					player[winning_player].name, text.win);
		} else {
			snprintf(string, STRING_LENGTH - 1, "%s %d %s !", text.player, 
					winning_player, text.win);
		}
	}
	ui_new_message(string);
	fx_new_transition(set_gamestate_EDITOR, 8, FX_FADE);
}


static void g_check_complete_square(square_s *square)
{
	if (square->owner != NONE) return;

	if (square->owner_up == NONE) return;
	if (square->owner_right == NONE) return;
	if (square->owner_down == NONE) return;
	if (square->owner_left == NONE) return;
	
	// current player own the square
	square->owner = local_player.turn;

	++player[local_player.turn].score;
	--squares_remaining;

	if (!squares_remaining) g_end();
}


/* 
====================
g_add_segment

pos_x and pos_y are "-1" for local input,
otherwise they are coordinate from net
====================
*/
void g_add_segment(int pos_x, int pos_y, int side)
{
	if (pos_x == -1) {
		if(!net_game || (local_player.turn == local_player.player_n)) {
			pos_x = g_min_x + (input.mouse_x - g_offset_x) / g_square_size;
			pos_y = g_min_y + (input.mouse_y - g_offset_y) / g_square_size;
		}
	}
	if (pos_x < g_min_x || pos_x >= g_max_x) return;
	if (pos_y < g_min_y || pos_y >= g_max_y) return;
	if (!squares[pos_y][pos_x].active) return;
 	
	int center_x = squares[pos_y][pos_x].x1 + 
			(squares[pos_y][pos_x].x2 - squares[pos_y][pos_x].x1) / 2;
 	int center_y = squares[pos_y][pos_x].y1 + 
			(squares[pos_y][pos_x].y2 - squares[pos_y][pos_x].y1) / 2;

	int current_dist;
	int shortest_dist = g_square_size * 2; // start with maximum distance
	
	if (side == -1) {
		// top
		current_dist = sqrt(ipow(input.mouse_y - squares[pos_y][pos_x].y1, 2) + 
				ipow(input.mouse_x - center_x, 2));
		if (current_dist < shortest_dist) {
			shortest_dist = current_dist;
			side = UP;
		}
		
		// right
		current_dist = sqrt(ipow(input.mouse_y - center_y, 2) + 
				ipow(input.mouse_x - squares[pos_y][pos_x].x2, 2));
		if (current_dist < shortest_dist) {
			shortest_dist = current_dist;
			side = RIGHT;
		}

		// down
		current_dist = sqrt(ipow(input.mouse_x - center_x, 2) + 
				ipow(input.mouse_y - squares[pos_y][pos_x].y2, 2));
		if (current_dist < shortest_dist) {
			shortest_dist = current_dist;
			side = DOWN;
		}

		// left
		current_dist = sqrt(ipow(input.mouse_x - squares[pos_y][pos_x].x1, 2) + 
				ipow(input.mouse_y - center_y, 2));
		if (current_dist < shortest_dist) {
			shortest_dist = current_dist;
			side = LEFT;
		}
	}

	if (net_is_server || 
			(net_is_client && (local_player.turn == local_player.player_n)))
		net_write_int(G_ADD_SEG_BYTE, 3, pos_x, pos_y, side);

	switch (side) {
	case UP:
		if (squares[pos_y][pos_x].owner_up == NONE) {
			squares[pos_y][pos_x].owner_up = local_player.turn;
			*squares[pos_y][pos_x].neighbour_up = local_player.turn;
		} else return;
		break;

	case RIGHT:
		if (squares[pos_y][pos_x].owner_right == NONE) {
			squares[pos_y][pos_x].owner_right = local_player.turn;
			*squares[pos_y][pos_x].neighbour_right = local_player.turn;
		} else return;
		break;

	case DOWN:
		if (squares[pos_y][pos_x].owner_down == NONE) {
			squares[pos_y][pos_x].owner_down = local_player.turn;
			*squares[pos_y][pos_x].neighbour_down = local_player.turn;
		} else return;
		break;
	
	case LEFT:
		if (squares[pos_y][pos_x].owner_left == NONE) {
			squares[pos_y][pos_x].owner_left = local_player.turn;
			*squares[pos_y][pos_x].neighbour_left = local_player.turn;
		} else return;
		break;
	}

	g_check_complete_square(&squares[pos_y][pos_x]);
	// check neighbour squares
	if (pos_y != 0) 
		g_check_complete_square(&squares[pos_y - 1][pos_x]);
	if (pos_x != ed_grid_w)
		g_check_complete_square(&squares[pos_y][pos_x + 1]);
	if (pos_y < ed_grid_h - 1)
		g_check_complete_square(&squares[pos_y + 1][pos_x]);
	if (pos_x != 0)
		g_check_complete_square(&squares[pos_y][pos_x - 1]);

	input.mouse_button_left = false;
	input.mouse_button_right = false;
	
	if (!net_game || net_is_server) {
		// game ended
		if (!squares_remaining) {
			if (local_player.turn == NONE) return; // no winner
			// move current player moving square to winning player
			if (local_player.turn != winning_player) {
				local_player.turn = winning_player;

				fx_new_transition(NULL, 5, FX_PLAYER_CHANGE);
			}
			return;
		}

		if (local_player.turn == PLAYER_0) {
			local_player.turn = PLAYER_1;
		} else {
			local_player.turn = PLAYER_0;
		}

		if (net_is_server)
			net_write_int(G_PLAYER_TURN_BYTE, 1, local_player.turn);
		
		fx_new_transition(NULL, 5, FX_PLAYER_CHANGE);
	}
}



void game_main()
{
	if (input.mouse_button_left || input.mouse_button_right) {
		if (ui_button_check_click(&button_game)) return;
		g_add_segment(-1, -1, -1);
	}
	
	fx_game();

	if (net_is_server && !client && gamestate == GAME) {
		ui_new_message(text.warning_no_client);
		set_gamestate_EDITOR();
	}
}


void g_init_ui()
{
}


/* 
====================
g_init_square

Set square size and position for the new game
====================
*/
static int g_init_square()
{	
	int tmp_size;
	int w = display_width - 1;
	int h = display_height - button_topbar->h - 1;
	int x;
	int y;

	g_min_x = ed_grid_w;
	g_max_x = 0;
	g_min_y = ed_grid_h;
	g_max_y = 0;

	// remove uncompletable square
	for (int i = 0; i < ed_grid_h; i++) {
		for (int j = 0; j < ed_grid_w; j++) {
			if (squares[i][j].active) {	
				if (i == 0 || !squares[i - 1][j].active) {
					if (j == (ed_grid_w - 1) || !squares[i][j + 1].active) {
						if (i == (ed_grid_h - 1) || !squares[i + 1][j].active) {
							if (j == 0 || !squares[i][j - 1].active) {
								squares[i][j].active = false;
								/* 	skip min/max position check, we removed 
									the square */
								continue;
							}
						}
					}
				}
				// find dimension 
				if (i < g_min_y) g_min_y = i;
				if (i > g_max_y) g_max_y = i;
				if (j < g_min_x) g_min_x = j;
				if (j > g_max_x) g_max_x = j;
			}
		}
	}
	
	// not enough active square
	if ((g_max_x - g_min_x <= 1) || (g_max_y - g_min_y <= 1)) {
		ui_new_message(text.warning_select_square);
		return 1;
	}

	g_max_x += 1;
	g_max_y += 1;

	// find square size for the game
	tmp_size = w / (g_max_x - g_min_x);
	g_square_size = h / (g_max_y - g_min_y);
	if (tmp_size < g_square_size) g_square_size = tmp_size;

	g_offset_x = (display_width - ((g_max_x - g_min_x) * g_square_size)) / 2;
	g_offset_y = button_topbar->h + (h - ((g_max_y - g_min_y) *
			g_square_size)) / 2;

	// clear position from editor for all square
	for (int i = 0; i < ed_grid_h; i++) {
		for (int j = 0; j < ed_grid_w; j++) {
			squares[i][j].x1 = 0;
			squares[i][j].x2 = 0;
			squares[i][j].y1 = 0;
			squares[i][j].y2 = 0;
		}
	}
	
	x = g_offset_x;
	y = g_offset_y;
	// set square position for game
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

Called at the start of all games
====================
*/
int g_init()
{
	local_player.turn = PLAYER_0;	
	player[0].score = 0; player[1].score = 0;
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


