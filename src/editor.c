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
// editor.c -

#include "editor.h"

#include "draw.h"
#include "fx.h"
#include "game.h"
#include "input.h"
#include "main.h"
#include "ui.h"


void ed_clear_squares();
void ed_gen_random();


void ed_add_square()
{
	int pos_x = (input.mouse_x - ed_start_x) / min_square_size;
	int pos_y = (input.mouse_y - ed_start_y) / min_square_size;

	if ((pos_x < ed_grid_w) && (pos_x >= 0)) {
		if ((pos_y < ed_grid_h) && (pos_y >= 0)) {
			squares[pos_y][pos_x].active = TRUE;
		}
	}
}


void ed_rmv_square()
{
	int pos_x = (input.mouse_x - ed_start_x) / min_square_size;
	int pos_y = (input.mouse_y - ed_start_y) / min_square_size;

	if ((pos_x < ed_grid_w) && (pos_x >= 0)) {
		if ((pos_y < ed_grid_h) && (pos_y >= 0)) {
			squares[pos_y][pos_x].active = FALSE;
		}
	}
}


void ed_button_play()
{
	fx_new_transition(*g_change_state, 3, FX_FADE);
}


void ed_change_state()
{
	gamestate = EDITOR;
	active_dropmenu = NULL;
	ed_init();
}


void ed_do_editor()
{
	if (input.mouse_button_left) {
		if (ui_button_check_click(&button_editor)) return;
		// TODO hashing
		ed_add_square();
	}
	if (input.mouse_button_right) {
		ed_rmv_square();
	}
}

void ed_init_ui()
{	
	int min_w = 70;
	int max_w = 140;
	int w;
	int h = button_font.size + UI_BAR_PADDING;
	int x, y;

	// align properly with last button
	Button *last_button = button_topbar;
	if (!last_button) return;
	while (last_button->next) {
		last_button = last_button->next;
	}	

	x = last_button->x2; y = 0;
	w = strlen(text.play) * button_font.w + UI_BAR_PADDING;
	last_button = ui_new_button(x, y, w, h, min_w, max_w, ALIGN_CENTER, text.play,
			*ed_button_play, &button_editor);
	x = last_button->x2; y = 0;
	w = strlen(text.random) * button_font.w + UI_BAR_PADDING;
	last_button = ui_new_button(x, y, w, h, min_w, max_w, ALIGN_CENTER, text.random,
			*ed_gen_random, &button_editor);
}


void ed_clear_squares()
{
	for (int i = 0; i < ed_grid_h; i++) {
		for (int j = 0; j < ed_grid_w; j++) {
			squares[i][j].active = FALSE;
			squares[i][j].owner_up = FALSE;
			squares[i][j].owner_right = FALSE;
			squares[i][j].owner_down = FALSE;
			squares[i][j].owner_left = FALSE;
		}
	}
}


void ed_set_square_pos()
{
	int x = ed_start_x;
	int y = ed_start_y;

	for (int i = 0; i < ed_grid_h; i++) {
		for (int j = 0; j < ed_grid_w; j++) {
			squares[i][j].x1 = x;
			squares[i][j].x2 = x + min_square_size;
			squares[i][j].y1 = y;
			squares[i][j].y2 = y + min_square_size;
			x += min_square_size;
		}
		x = ed_start_x;
		y += min_square_size;
	}
}


int ed_init()
{
	// playable area
	int w = display_width - 1;
	int h = display_height - button_topbar->h;

	if (!min_square_size) min_square_size = DEFAULT_SQUARE_SIZE;
	ed_grid_w = w / min_square_size;
	ed_grid_h = h / min_square_size;

	// center grid
	ed_start_x = (w - (ed_grid_w * min_square_size)) / 2;
	ed_start_y = button_topbar->h + (h - (ed_grid_h * min_square_size)) / 2;

	// create squares array
	squares = malloc( ed_grid_h * sizeof(*squares) ); // allocate memory for rows
	if(squares != NULL) {
		for(int i = 0; i < ed_grid_h; i++) {
			squares[i] = malloc(ed_grid_w * sizeof(**squares)); // allocate memory for collums
			if(squares[i] == NULL) {
				printf("Memory allocation failed. Exiting....");
				return 1;
			}
		}
	} else {
		printf("Memory allocation failed. Exiting....");
		return 1;
	}
	
	// create connection between neigbours
	for (int i = 0; i < ed_grid_h; i++) {
		for (int j = 0; j < ed_grid_w; j++) {

			if (i != 0) {
				squares[i][j].neighbour_up = &squares[i - 1][j].owner_down;
			} else {
				squares[i][j].neighbour_up = NULL;
			}

			if (i != ed_grid_h - 1) {
				squares[i][j].neighbour_down = &squares[i + 1][j].owner_up;
			} else {
				squares[i][j].neighbour_down = NULL;
			}

			if (j != 0) {
				squares[i][j].neighbour_left = &squares[i][j - 1].owner_right;
			} else {
				squares[i][j].neighbour_left = NULL;
			}

			if (j != ed_grid_w - 1) {
				squares[i][j].neighbour_right = &squares[i][j + 1].owner_left;
			} else {
				squares[i][j].neighbour_right = NULL;
			}

		}
	}

	ed_clear_squares();

	ed_set_square_pos();

	return 0;
}


#define RAND 200

void ed_gen_random()
{
	int rand_w;
	int rand_h;
	int max_w = ed_grid_w - 1;
	int max_h = ed_grid_h - 1;

	// activate center squares
	squares[ed_grid_h / 2][ed_grid_w / 2].active = TRUE;
	squares[ed_grid_h / 2 + 1][ed_grid_w / 2].active = TRUE;
	squares[ed_grid_h / 2 - 1][ed_grid_w / 2].active = TRUE;
	squares[ed_grid_h / 2][ed_grid_w / 2 + 1].active = TRUE;
	squares[ed_grid_h / 2][ed_grid_w / 2 - 1].active = TRUE;
	squares[ed_grid_h / 2 + 1][ed_grid_w / 2 - 1].active = TRUE;
	squares[ed_grid_h / 2 - 1][ed_grid_w / 2 + 1].active = TRUE;
	squares[ed_grid_h / 2 + 1][ed_grid_w / 2 + 1].active = TRUE;
	squares[ed_grid_h / 2 - 1][ed_grid_w / 2 - 1].active = TRUE;

	for (int i = 0; i < RAND; i++) {
		rand_w = get_random_number(max_w);
		rand_h = get_random_number(max_h);
		
		if (rand_w == 0 || rand_h == 0) continue;

		// check if neigbour square are active
		if (squares[rand_h + 1][rand_w].active || squares[rand_h - 1][rand_w].active ||
			squares[rand_h][rand_w + 1].active || squares[rand_h][rand_w - 1].active) {

			squares[rand_h][rand_w].active = TRUE;
		}
	}
	
}


