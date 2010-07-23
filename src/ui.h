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
// ui.h -

#ifndef __UI_H__
#define __UI_H__

#include "shared.h"

#define UI_BAR_PADDING 3


enum {
	ALIGN_CENTER,
	ALIGN_LEFT,
	ALIGN_RIGHT
};


typedef struct Text Text;
struct Text {
	char play[ STRING_LENGTH ];
	char random[ STRING_LENGTH ];

	char edit[ STRING_LENGTH ];

	char main_menu[ STRING_LENGTH ];
	char lang_menu[ STRING_LENGTH ];
	char option[ STRING_LENGTH ];

	char new_game[ STRING_LENGTH ];
	char quit[ STRING_LENGTH ];

	char score[ STRING_LENGTH ];
	char player[ STRING_LENGTH ];
	char win[ STRING_LENGTH ];
	char no_win[ STRING_LENGTH ];

};
Text text;


typedef struct Button Button;
struct Button {
	SDL_Surface *surface;
	int selected;
	int x1, y1, x2, y2, w, h;
	int textOffsetX, textOffsetY;
	void (*func)();
	struct Button *next;
};


typedef struct Font {
	TTF_Font* data;
	int size;
	int w, h;
	char name[ STRING_LENGTH ];
} Font;

typedef struct Message Message;
struct Message {
	int active;
	int time;
	char text[ LONG_STRING_LENGTH ];
};
Message ui_message;

// function
void ui_display_message();
void ui_new_message(char*);
void ui_button_function();
int ui_button_check_click(Button **);
Button* ui_new_button(int, int, int, int, int, int, int, char*, void func(), Button **);
Button* ui_button_check_pos(Button **);

// variable
char ui_language[ SMALL_STRING_LENGTH ];

Button *ui_pressed_button;
Button *ui_highlight_button;

Font button_font;

Button *button_editor;
Button *button_game;

Button *button_topbar;
Button *button_dropmenu_main;
Button *button_dropmenu_lang;
Button *active_dropmenu;
Button *active_dropmenu_parent;

#endif
