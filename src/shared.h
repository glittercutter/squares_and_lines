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
// shared.h -

#ifndef __SHARED_H__
#define __SHARED_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include <unistd.h>

#include <SDL/SDL.h>
#include <SDL/SDL_gfxPrimitives.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_net.h>


#define TRUE 1
#define FALSE 0
#define RUNNING 1
#define STOPPED 0


#define WINDOW_TITLE "crucially square"

#define MAX_FPS 40 //milliseconds
#define LIMIT_FPS 1

#define CONFIG_FILENAME "game.conf"

#define LONG_STRING_LENGTH 88
#define STRING_LENGTH 50
#define SMALL_STRING_LENGTH 20

#ifdef NDEBUG
#define DEBUG(debug_cmd)
#else
#define DEBUG(debug_cmd) debug_cmd
#endif

#define TAB_LENGTH(X) (sizeof(X)/sizeof(*X))


enum {
	EDITOR,
	GAME,
	QUIT
};

enum {
	NONE,
	PLAYER_0,
	PLAYER_1,
	OUTLINE,
};

enum {
	UP,
	RIGHT,
	DOWN,
	LEFT,
};


typedef struct ColorRGB ColorRGB;
struct ColorRGB {
	int r, g, b;
};


typedef struct Color Color;
struct Color {
	ColorRGB square_owner[4];
	ColorRGB ed_outline;
	ColorRGB ed_grid;
	ColorRGB text;
	ColorRGB topbar;
	ColorRGB button_highlight;
};
Color color;


typedef struct Square Square;
struct Square {
	
	int active;
	int x1, y1;
	int x2, y2;

	int owner;

	int owner_up;
	int *neighbour_up;

	int owner_right;
	int *neighbour_right;

	int owner_down;
	int *neighbour_down;

	int owner_left;
	int *neighbour_left;

};

Square **squares;

// variables
SDL_Surface *screen;
int gamestate;
int display_width;
int display_height;
int display_fullscreen;
int min_square_size;

#endif
