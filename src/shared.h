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

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include <unistd.h>

#include "SDL/SDL.h"
#include "SDL/SDL_gfxPrimitives.h"
#include "SDL/SDL_ttf.h"
#include "SDL/SDL_net.h"
#include "zlib.h"


#define TRUE 1
#define FALSE 0
#define RUNNING 1
#define STOPPED 0

typedef unsigned char byte;

#define WINDOW_TITLE "crucially square"
#define CONFIG_FILENAME "game.conf"

#define MAX_FPS 40 //milliseconds
#define LIMIT_FPS 1

#define LONG_STRING_LENGTH 88
#define STRING_LENGTH 50
#define SMALL_STRING_LENGTH 20

#ifdef NDEBUG
#define DEBUG(debug_cmd)
#else
#define DEBUG(debug_cmd) debug_cmd
#endif

#define TAB_LENGTH(X) (sizeof(X)/sizeof(*X))


// gamestate
enum {
	EDITOR,
	GAME,
	QUIT
};

// square owner
enum {
	NONE,
	PLAYER_0,
	PLAYER_1,
	OUTLINE,
};

// square boundary
enum {
	UP,
	RIGHT,
	DOWN,
	LEFT,
};

#define LS_MAX_STRING 5
typedef struct string_list_s {
	char *string[LS_MAX_STRING];
	struct string_list_s *next;
} string_list_s;

typedef struct string_list_t {
	string_list_s *list;
	int element, max_element;
	int col_position[LS_MAX_STRING];
	struct button_s *col_name;
	struct widget_list_box_t *list_box;
} string_list_t;


typedef struct colorRGB_t {
	int r, g, b;
} colorRGB_t;

typedef struct color_s {
	colorRGB_t square_owner[4];
	colorRGB_t ed_outline;
	colorRGB_t ed_grid;
	colorRGB_t text;
	colorRGB_t topbar;
	colorRGB_t button_highlight;
} color_s;
color_s color;


typedef struct square_s {
	int active;
	int owner;
	int x1, y1;
	int x2, y2;

	int owner_up;
	int *neighbour_up;
	int owner_right;
	int *neighbour_right;
	int owner_down;
	int *neighbour_down;
	int owner_left;
	int *neighbour_left;
} square_s;
square_s **squares;


// variables
SDL_Surface *screen;
int gamestate;
int display_width;
int display_height;
int display_fullscreen;


#endif
