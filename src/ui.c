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
// ui.c -

#include "ui.h"

#include "draw.h"
#include "input.h"


Button* ui_new_button(int x, int y, int w, int h, int min_w, int max_w, int align,
			char *text, void func(), Button **node)
{
	Button **node2 = node;

	DEBUG(int i = 1);

	int alfa = 255;
	int text_x = 0;
	int nor_w;
	
	DEBUG(printf("button: %s\n", text));

	if (w > max_w) {
		nor_w = max_w;
	} else if (w < min_w) {
		nor_w = min_w;
	} else nor_w = w;

	nor_w += (UI_BAR_PADDING * 2);

	Button *button;
	button = malloc(sizeof(Button));
	button->next = NULL;

	button->x1 = x;
	button->y1 = y;
	button->x2 = x + nor_w;
	button->y2 = y + h;
	button->w = nor_w;
	button->h = h;

	button->func = func;
	
	// surface
	button->surface = sdl_create_surface(nor_w, h);
	boxRGBA(button->surface, 0, 0, nor_w, h, color.topbar.r, 
			color.topbar.g, color.topbar.b, alfa);

	// text
	switch (align) {
		case ALIGN_LEFT:
			text_x = (UI_BAR_PADDING * 2);
		break;

		case ALIGN_CENTER:
			text_x = ((nor_w - w) / 2);
		break;

		case ALIGN_RIGHT:
			text_x = (nor_w - w) - (UI_BAR_PADDING * 2);
		break;
	}

	sdl_draw_text_blended2(button->surface, text_x, -1, text, button_font.data, 
			color.text);
	
	// adding to button list
	if (!*node2) {
		// first node
		*node2 = button;
		DEBUG(printf("adding first node, adress: %p\n", (void*)*node));
		DEBUG(printf("button no.0\n"));
		return *node2;
	}

	while (*node2) {
		DEBUG(++i);
		node2 = &(*node2)->next;
	}
	DEBUG(printf("adding node after, adress: %p\n", (void*)*node));
	// append in list
	*node2 = button;

	DEBUG(printf("new node, adress: %p\n", (void*)(*node)->next));
	DEBUG(printf("button no.%d\n", i));

	return *node2;
}


int ui_button_check_click(Button **button_type)
{
	Button *button = *button_type;

	while (button) {
		if ((input.mouse_y < button->y2) && (input.mouse_x < button->x2) && 
					(input.mouse_y > button->y1) && (input.mouse_x > button->x1)) {
			ui_pressed_button = button;
			input.mouse_button_left = FALSE;
			return 1;
		} else {
			button = button->next;
		}
	}
	return 0;
}


Button* ui_button_check_pos(Button **button_type)
{
	Button *button = *button_type;
	int offset = 1; // used to remove (crack) between buttons

	while (button) {
		if ((input.mouse_y < button->y2 + offset) && 
				(input.mouse_x < button->x2 +offset) && 
				(input.mouse_y > button->y1 - offset) && 
				(input.mouse_x > button->x1 + offset)) {
			return button;
		} else {
			button = button->next;
		}
	}
	return NULL;
}



/* 
====================
ui_button_function

Called by get_input() to execute the function of the button.
====================
*/
void ui_button_function()
{
	
	if (ui_pressed_button->func != NULL) {
		// check if the mouse is still on the button
		if ((input.mouse_y < ui_pressed_button->y2) && 
						(input.mouse_x < ui_pressed_button->x2) && 
						(input.mouse_y > ui_pressed_button->y1) && 
						(input.mouse_x > ui_pressed_button->x1)) {
			ui_pressed_button->func();
		}
	}
	ui_pressed_button = NULL;
}

#define MESSAGE_TIME 50
void ui_new_message(char* text)
{
	ui_message.active = TRUE;
	ui_message.time = MESSAGE_TIME;
	strncpy(ui_message.text, text, LONG_STRING_LENGTH - 1);
}


void ui_display_message()
{
	if (!ui_message.active) return;

	int pos_x = 0;

	// TODO find the right position, center text...
	switch (gamestate) {
		case GAME:
			pos_x = 100;
		break;

		case EDITOR:
			pos_x = 250;
		break;
	}

	sdl_draw_text_solid2(pos_x, -1, ui_message.text, button_font.data, color.text);
	--ui_message.time; 
	if (!ui_message.time) ui_message.active = FALSE;
}


