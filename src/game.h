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
// game.h - 

#ifndef __GAME_H__
#define __GAME_H__

#include "shared.h"

#define TEXT_MARGIN 10


typedef struct player_s player_s;
struct player_s {
	int score_text_pos_x;
	int score;
	char name[32];
};
player_s player[2]; // TODO rewrite


int winning_player;
int squares_remaining;
int g_min_x, g_max_x;
int g_min_y, g_max_y;
int g_square_size;
int g_offset_x, g_offset_y;
int g_line_thickness;


void set_gamestate_GAME(void);
void game_main(void);
void g_init_ui(void);
int g_init(void);
void g_add_segment(int x, int y, int side);


#endif
