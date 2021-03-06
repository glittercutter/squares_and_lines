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
#define SCROLLBAR_SIZE 9


enum {
	ALIGN_CENTER,
	ALIGN_LEFT,
	ALIGN_RIGHT
};


typedef struct text_s {
	char play[ STRING_LENGTH ];
	char random[ STRING_LENGTH ];

	char main_menu[ STRING_LENGTH ];
	char option[ STRING_LENGTH ];
	char new_game[ STRING_LENGTH ];
	char quit[ STRING_LENGTH ];
	
	char multiplayer[ STRING_LENGTH ];
	char host_game[ STRING_LENGTH ];
	char join_game[ STRING_LENGTH ];
	char disconnect[ STRING_LENGTH ];

	char score[ STRING_LENGTH ];
	char player[ STRING_LENGTH ];
	char win[ STRING_LENGTH ];
	char no_win[ STRING_LENGTH ];

	char lbox_server[ STRING_LENGTH ];
	char lbox_ping[ STRING_LENGTH ];
	char lbox_player[ STRING_LENGTH ];
	
	char configure[ STRING_LENGTH ];
	char join[ STRING_LENGTH ];
	char update[ STRING_LENGTH ];

	char txt_srv_name_is[ STRING_LENGTH ];
	char txt_player_name_is[ STRING_LENGTH ];

	char cl_connected[ STRING_LENGTH ];
	char srv_full[ STRING_LENGTH ];
	char warning_select_square[ STRING_LENGTH ];
	char warning_option[ STRING_LENGTH ];
	char warning_no_client[ STRING_LENGTH ];
	char warning_no_server_selected[ STRING_LENGTH ];
	char warning_only_server[ STRING_LENGTH ];

} text_s;
text_s text;


typedef struct button_s {
	SDL_Surface *surface;
	bool selected;
	int x1, y1, x2, y2, w, h;
	int textOffsetX, textOffsetY;
	void (*func)();
	struct button_s *next;
} button_s;


typedef struct font_s {
	TTF_Font* data;
	int size;
	int w, h;
	char name[ STRING_LENGTH ];
} font_s;
font_s button_font;

typedef struct message_s {
	bool active;
	int time;
	int x1, y1, x2, y2;
	char text[ LONG_STRING_LENGTH ];
} message_s;
message_s ui_message;

// scrollbar orientation
enum {
	HORIZONTAL,
	VERTICAL,
};

typedef struct scrollbar_t {
	int orientation; // HORIZONTAL/VERTICAL 
	int x1, y1, x2, y2;
	int handle_max_w, handle_max_h;
	bool dragging_handle;
	button_s handle, arrow1, arrow2;
	float handle_pos;
	float ratio;
	int offset;
	int *element, *viewable_element;
} scrollbar_t;

typedef struct widget_list_box_t {
	int x1, y1, x2, y2, w, h;
	int viewable_element;
	struct string_list_t *list;
	int selected_row;
	scrollbar_t scrollbar;
} widget_list_box_t;

typedef struct widget_plain_text_t {
	int x1, y1;
	char *text1;
	char *text2;
	char *text3;
	colorRGB_t color;
} widget_plain_text_t;

typedef union widget_union {
	widget_list_box_t *list_box;
	widget_plain_text_t *plain_text;
} widget_union;

// widget type
enum {
	LIST_BOX,
	PLAIN_TEXT
};

typedef struct widget_t {
	int type;
	widget_union widget;
	struct widget_t *next;
} widget_t;

typedef struct window_s {
	int x1, y1, x2, y2, w, h;
	struct window_s *last_window;
	widget_t *widget;
	button_s *close_button;
	button_s *button;
	char notification[ LONG_STRING_LENGTH ];
} window_s;

typedef struct gui_surface_s {
	SDL_Surface 
		*arrow_up, 
		*arrow_down, 
		*arrow_left, 
		*arrow_right,
		*gradient;
} gui_surface_s;
gui_surface_s gui_surface;


char ui_language[ SMALL_STRING_LENGTH ];

button_s *ui_pressed_button;
button_s *ui_highlight_button;

button_s *button_editor;
button_s *button_game;

button_s *button_topbar;
button_s *button_dropmenu_main;
button_s *button_dropmenu_multiplayer;
button_s *active_dropmenu;
button_s *active_dropmenu_parent;

window_s host_window;
window_s client_window;
window_s *active_window;
// window_s *open_window[10]; // max opened windows TODO

scrollbar_t *ui_dragged_scrollbar;


void ui_display_window(void);
void ui_display_message(void);
void ui_new_message(char* fmt, ...);
void ui_button_function(void);
int ui_button_check_click(button_s **head_node);
void ui_button_close_window(void);
void ui_button_drag_window(void);
void ui_new_widget_list_box(int x1, int y1, int x2, int y2, 
		string_list_t *strlist, widget_t **widget_head_node);
void ui_new_widget_plain_text(char *text1, char *text2, char *text3, 
		int x1, int y1, colorRGB_t color, widget_t **widget_head_node);
void ui_scrollbar_update_size(int, int, scrollbar_t*);
button_s* ui_new_button(int x, int y, int w, int h, int min_w, int max_w, 
		int align, char *text, void func(void), int three_d, int gradient,
		button_s **node);
button_s* ui_button_check_pos(button_s **head_node);


#endif
