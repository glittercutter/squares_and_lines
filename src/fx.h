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
// fx.h - 

#ifndef __FX_H__
#define __FX_H__

#include "shared.h"

// max no. of glow managed
#define MAX_GLOWING_SEGMENT 10


enum {
	FX_FADE,
	FX_PLAYER_CHANGE,
};

#define NUM_OF_TRANSITION 2
enum {
	TRANSITION_FADE,
	TRANSITION_SWITCH_PLAYER,
	TRANSITION_NEW_POINT, // TODO
};
typedef struct Fx_transition Fx_transition;
struct Fx_transition {
	int active;
	int current_step;
	int max_step;
	int halfway;
	int fx_type;
	void (*func)();
};
Fx_transition fx_transition[NUM_OF_TRANSITION];

typedef struct Seg_glow Seg_glow;
struct Seg_glow {
	Square *square;
	int pos;
	int glow_level;
	int player;
	int x1, y1, x2, y2;
};

Seg_glow seg_glow[MAX_GLOWING_SEGMENT];

Seg_glow seg_glow_current;


void fx_new_transition(void (*func)(), int, int);
void fx_game();
void fx_main();


#endif
