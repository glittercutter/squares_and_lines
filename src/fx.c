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
// fx.c - 

#include "fx.h"

#include "common.h"
#include "editor.h"
#include "game.h"
#include "input.h"
#include "main.h"


void fx_new_transition(void (*func)(), int step, int type)
{
	Fx_transition *fx = NULL;
	switch (type) {
		case FX_PLAYER_CHANGE:
			fx = &fx_transition[FX_PLAYER_CHANGE];
		break;

		case FX_FADE:
			fx = &fx_transition[FX_FADE];
		break;
	}
	fx->active = TRUE;
	fx->max_step = step;
	fx->current_step = step;
	fx->halfway = FALSE;
	fx->fx_type = type;
	fx->func = func;
}

/* 
====================
fx_do_transition

Step start at max value, decrease to 0; excute the function
then increase to max value; end
====================
*/
static void fx_do_transition()
{
	Fx_transition *fx;

	for (int i = 0; i < NUM_OF_TRANSITION; i++) {

		if (fx_transition[i].active) {
			fx = &fx_transition[i];
		} else continue;
		
		if ((fx->halfway) && (fx->current_step ==
				fx->max_step)) {
			fx->active = FALSE;
			return;
		}

		if (fx->current_step == 0) {
			if (*fx->func != NULL) fx->func();
			fx->halfway = TRUE;
		}

		if (fx->halfway) {
			++fx->current_step;
		} else --fx->current_step;
	}
}

/* 
====================
fx_new_glow_segment

Find empty slot to insert new glowing segment
====================
*/
static void fx_new_glow_segment()
{
	for (int i = 0; i < MAX_GLOWING_SEGMENT; i++) {
		if (!seg_glow[i].square) {
			seg_glow[i].square = seg_glow_current.square;
			seg_glow[i].pos = seg_glow_current.pos;
			seg_glow[i].glow_level = 0;
			seg_glow[i].player = seg_glow_current.player;
			switch (seg_glow_current.pos) {
				case UP:
					seg_glow[i].x1 = seg_glow_current.square->x1;
					seg_glow[i].y1 = seg_glow_current.square->y1;
					seg_glow[i].x2 = seg_glow_current.square->x2;
					seg_glow[i].y2 = seg_glow_current.square->y1;
				break;

				case RIGHT:
					seg_glow[i].x1 = seg_glow_current.square->x2;
					seg_glow[i].y1 = seg_glow_current.square->y1;
					seg_glow[i].x2 = seg_glow_current.square->x2;
					seg_glow[i].y2 = seg_glow_current.square->y2;
				break;

				case DOWN:
					seg_glow[i].x1 = seg_glow_current.square->x1;
					seg_glow[i].y1 = seg_glow_current.square->y2;
					seg_glow[i].x2 = seg_glow_current.square->x2;
					seg_glow[i].y2 = seg_glow_current.square->y2;
				break;

				case LEFT:
					seg_glow[i].x1 = seg_glow_current.square->x1;
					seg_glow[i].y1 = seg_glow_current.square->y1;
					seg_glow[i].x2 = seg_glow_current.square->x1;
					seg_glow[i].y2 = seg_glow_current.square->y2;
				break;
			}
			return; // segment added
		}
	}
}

static void fx_glow_closest_segment()
{	
	int current_dist, shortest_dist, closest_seg;

	// Find closest square
	int pos_x = g_min_x + (input.mouse_x - g_offset_x) / g_square_size;
	int pos_y = g_min_y + (input.mouse_y - g_offset_y) / g_square_size;
	// Closest square's center position
 	int center_x = squares[pos_y][pos_x].x1 + 
			(squares[pos_y][pos_x].x2 - squares[pos_y][pos_x].x1) / 2;
 	int center_y = squares[pos_y][pos_x].y1 + 
			(squares[pos_y][pos_x].y2 - squares[pos_y][pos_x].y1) / 2;

	/* Find closest segment */
	// up
	shortest_dist = sqrt(ipow(input.mouse_y - squares[pos_y][pos_x].y1, 2) + 
			ipow(input.mouse_x - center_x, 2));
	closest_seg = UP;	
	// right
	current_dist = sqrt(ipow(input.mouse_y - center_y, 2) + 
			ipow(input.mouse_x - squares[pos_y][pos_x].x2, 2));
	if (current_dist < shortest_dist) {
		shortest_dist = current_dist;
		closest_seg = RIGHT;
	}
	// down
	current_dist = sqrt(ipow(input.mouse_x - center_x, 2) + 
			ipow(input.mouse_y - squares[pos_y][pos_x].y2, 2));
	if (current_dist < shortest_dist) {
		shortest_dist = current_dist;
		closest_seg = DOWN;
	}
	// left
	current_dist = sqrt(ipow(input.mouse_x - squares[pos_y][pos_x].x1, 2) + 
			ipow(input.mouse_y - center_y, 2));
	if (current_dist < shortest_dist) {
		shortest_dist = current_dist;
		closest_seg = LEFT;
	}

	// segment currently glowing
	switch (closest_seg) {
		case UP:
			if (squares[pos_y][pos_x].owner_up == NONE) {
				seg_glow_current.square = &squares[pos_y][pos_x];
				seg_glow_current.pos = UP;
				seg_glow_current.player = player_turn;
			} else seg_glow_current.square = NULL;
		break;

		case RIGHT:
			if (squares[pos_y][pos_x].owner_right == NONE) {
				seg_glow_current.square = &squares[pos_y][pos_x];
				seg_glow_current.pos = RIGHT;
				seg_glow_current.player = player_turn;
			} else seg_glow_current.square = NULL;
		break;

		case DOWN:
			if (squares[pos_y][pos_x].owner_down == NONE) {
				seg_glow_current.square = &squares[pos_y][pos_x];
				seg_glow_current.pos = DOWN;
				seg_glow_current.player = player_turn;
			} else seg_glow_current.square = NULL;
		break;
		
		case LEFT:
			if (squares[pos_y][pos_x].owner_left == NONE) {
				seg_glow_current.square = &squares[pos_y][pos_x];
				seg_glow_current.pos = LEFT;
				seg_glow_current.player = player_turn;
			} else seg_glow_current.square = NULL;
		break;
	}
}


static const int inc_glow_rate = 85;
static const int dec_glow_rate = 20;

static void fx_glow_segment()
{
	bool new_segment = true;

	for (int i = 0; i < MAX_GLOWING_SEGMENT; i++) {
		if (seg_glow[i].square) {
			// increase glow level of closest segment
			if (seg_glow_current.square == seg_glow[i].square && 
				seg_glow_current.pos == seg_glow[i].pos) {
				if (seg_glow[i].glow_level < 255) {
					seg_glow[i].glow_level += inc_glow_rate;
					if (seg_glow[i].glow_level > 255) seg_glow[i].glow_level = 255;
				} else seg_glow[i].glow_level = 255;
				new_segment = false; // was glowing

			} else {
				/* 	Decrease glow level of other segment and remove from array if 
				glow level is less than 0 */
				seg_glow[i].glow_level -= dec_glow_rate;
				if (seg_glow[i].glow_level <= 0) {
					seg_glow[i].square = NULL;
				}
			}
		}
	}
	/* Closest segment was not glowing already */
	if (new_segment && seg_glow_current.square) {
		if (seg_glow_current.square->active) fx_new_glow_segment();
	}
}

void fx_main()
{
	fx_do_transition();
}

void fx_game()
{	
	fx_glow_closest_segment();
	fx_glow_segment();
}

