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

#include "common.h"
#include "draw.h"
#include "fx.h"
#include "game.h"
#include "input.h"
#include "main.h"
#include "net.h"
#include "ui.h"


#define DEFAULT_SQUARE_SIZE 20

void ed_clear_squares(void);
void ed_gen_random(void);
void ed_clear_squares(void);
void ed_set_square_pos(void);


static void ed_add_square()
{
	int x = (input.mouse_x - ed_start_x) / ed_square_size;
	int y = (input.mouse_y - ed_start_y) / ed_square_size;

	if ((x >= ed_grid_w) || (x < 0)) return;
	if ((y >= ed_grid_h) || (y < 0)) return;
	if (squares[y][x].active == true) return;
	squares[y][x].active = true;
	if (net_game)
		net_write_int(ED_ADD_SQUARE_BYTE, 2, x, y);
}

static void ed_rmv_square()
{
	int x = (input.mouse_x - ed_start_x) / ed_square_size;
	int y = (input.mouse_y - ed_start_y) / ed_square_size;

	if ((x >= ed_grid_w) || (x < 0)) return;
	if ((y >= ed_grid_h) || (y < 0)) return;
	if (squares[y][x].active == false) return;
	squares[y][x].active = false;
	if (net_game)
		net_write_int(ED_RM_SQUARE_BYTE, 2, x, y);
}

void ed_net_add_square(int x, int y)
{
	if ((x < ed_grid_w) && (x >= 0)) {
		if ((y < ed_grid_h) && (y >= 0)) {
			squares[y][x].active = true;
		}
	}
}

void ed_net_rm_square(int x, int y)
{
	if ((x < ed_grid_w) && (x >= 0)) {
		if ((y < ed_grid_h) && (y >= 0)) {
			squares[y][x].active = false;
		}
	}
}


void ed_button_play()
{
	if (net_is_client) return;
	fx_new_transition(*g_change_state, 3, FX_FADE);
}


void ed_change_state()
{
	gamestate = EDITOR;
	ed_clear_squares();
	ed_set_square_pos();

	if (net_is_server) {
		net_write_int(STATE_CHANGE_BYTE, 1, EDITOR);
		net_write_sync_square();
	}
}


void editor_main()
{
	if (input.mouse_button_left) {
		// check button if mouse is on the topbar
		if (input.mouse_y <= button_topbar->y2) {
			// don't check other element if a button was pressed
			if (ui_button_check_click(&button_editor))
				return;
		}
		ed_add_square();
	}
	if (input.mouse_button_right)
		ed_rmv_square();
}


void ed_init_ui()
{	
	int min_w = 47;
	int max_w = 140;
	int w;
	int h = button_font.size + UI_BAR_PADDING;
	int x, y;
	int mirror_x;

	// align properly with last button created
	button_s *last_button = button_topbar;
	if (!last_button)
		return;
	while (last_button->next) {
		last_button = last_button->next;
	}	

	x = last_button->x2; y = 0;
	w = strlen(text.play) * button_font.w;
	last_button = ui_new_button(x, y, w, h, min_w, max_w, ALIGN_CENTER, text.play,
			*ed_button_play, 0, 1, &button_editor);
	last_button->x1 = display_width - last_button->w;
	last_button->x2 = last_button->x1 + last_button->w;
	mirror_x = last_button->x1;

	x = last_button->x2; y = 0;
	w = strlen(text.random) * button_font.w;
	last_button = ui_new_button(x, y, w, h, min_w, max_w, ALIGN_CENTER, text.random,
			*ed_gen_random, 0, 1, &button_editor);
	last_button->x1 = mirror_x - last_button->w;
	last_button->x2 = mirror_x;

}


void ed_clear_squares()
{
	for (int i = 0; i < ed_grid_h; i++) {
		for (int j = 0; j < ed_grid_w; j++) {
			squares[i][j].active = false;
			squares[i][j].owner = false;
			squares[i][j].owner_up = false;
			squares[i][j].owner_right = false;
			squares[i][j].owner_down = false;
			squares[i][j].owner_left = false;
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
			squares[i][j].x2 = x + ed_square_size;
			squares[i][j].y1 = y;
			squares[i][j].y2 = y + ed_square_size;
			x += ed_square_size;
		}
		x = ed_start_x;
		y += ed_square_size;
	}
}


int ed_init()
{
	// set playable area
	int w = display_width - 1;
	int h = display_height - button_topbar->h;
	
	// are the square size valid ?
	if (!ed_square_size) ed_square_size = DEFAULT_SQUARE_SIZE;
	ed_grid_w = w / ed_square_size;
	ed_grid_h = h / ed_square_size;

	// center the editor grid
	ed_start_x = (w - (ed_grid_w * ed_square_size)) / 2;
	ed_start_y = button_topbar->h + (h - (ed_grid_h * ed_square_size)) / 2;

	// create 2d array for the squares
	// allocate memory for rows
	squares = malloc( ed_grid_h * sizeof(*squares) );
	if(squares != NULL) {
		for(int i = 0; i < ed_grid_h; i++) {
			// allocate memory for collums
			squares[i] = malloc(ed_grid_w * sizeof(**squares)); 
			if(squares[i] == NULL)
				eprint("Memory allocation failed. Exiting....");
		}
	} else {
		eprint("Memory allocation failed. Exiting....");
	}
	
	// create connection between neigbour square
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

	// activate center squares as starting point
	squares[ed_grid_h / 2][ed_grid_w / 2].active = true;
	squares[ed_grid_h / 2 + 1][ed_grid_w / 2].active = true;
	squares[ed_grid_h / 2 - 1][ed_grid_w / 2].active = true;
	squares[ed_grid_h / 2][ed_grid_w / 2 + 1].active = true;
	squares[ed_grid_h / 2][ed_grid_w / 2 - 1].active = true;
	squares[ed_grid_h / 2 + 1][ed_grid_w / 2 - 1].active = true;
	squares[ed_grid_h / 2 - 1][ed_grid_w / 2 + 1].active = true;
	squares[ed_grid_h / 2 + 1][ed_grid_w / 2 + 1].active = true;
	squares[ed_grid_h / 2 - 1][ed_grid_w / 2 - 1].active = true;

	for (int i = 0; i < RAND; i++) {
		rand_w = get_random_number(max_w);
		rand_h = get_random_number(max_h);
		
		if (rand_w == 0 || rand_h == 0) continue;

		// activate only if a neigbour square is active
		if (squares[rand_h + 1][rand_w].active || 
				squares[rand_h - 1][rand_w].active ||
				squares[rand_h][rand_w + 1].active || 
				squares[rand_h][rand_w - 1].active) {
			squares[rand_h][rand_w].active = true;
		}
	}
	
}


