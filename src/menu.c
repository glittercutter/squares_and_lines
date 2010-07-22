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
// menu.c -

#include "menu.h"

#include "editor.h"
#include "draw.h"
#include "fx.h"
#include "main.h"
#include "input.h"
#include "ui.h"



void m_open_main()
{
	active_dropmenu = button_dropmenu_main;
	active_dropmenu_parent = button_topbar;
}

void m_close_main()
{
	active_dropmenu = NULL;
}

void m_open_option()
{
	char string[ LONG_STRING_LENGTH ];
	snprintf(string, LONG_STRING_LENGTH - 1, 
			"edit out the file %s with a text editor !", CONFIG_FILENAME);
	ui_new_message(string);
}


void m_button_new_game()
{
	fx_new_transition(*ed_change_state, 3, FX_FADE);
}

void m_button_quit()
{
	fx_new_transition(*change_state_quit, 3, FX_FADE);
}


void m_init_ui()
{	
	// topbar
	// ================================================================
	int min_w = 40;
	int max_w = 100;
	int w;
	int h = button_font.size + UI_BAR_PADDING;
	int x, y;
	Button *last_button;

	DEBUG(printf("topbar button\n"));

	x = 0; y = 0;
	w = strlen(text.main_menu) * button_font.w + UI_BAR_PADDING;
	last_button = ui_new_button(x, y, w, h, min_w, max_w, ALIGN_CENTER, text.main_menu,
			*m_open_main, &button_topbar);

	// main
	// ================================================================
	min_w = 110;
	x = 0; y = last_button->y2;
	w = strlen(text.new_game) * button_font.w + UI_BAR_PADDING;
	last_button = ui_new_button(x, y, w, h, min_w, max_w, ALIGN_LEFT, text.new_game,
			*m_button_new_game, &button_dropmenu_main);

	x = last_button->x1; y = last_button->y2;
	w = strlen(text.option) * button_font.w + UI_BAR_PADDING;
	last_button = ui_new_button(x, y, w, h, min_w, max_w, ALIGN_LEFT, text.option,
			*m_open_option, &button_dropmenu_main);
	
	x = last_button->x1; y = last_button->y2;
	w = strlen(text.quit) * button_font.w + UI_BAR_PADDING;
	last_button = ui_new_button(x, y, w, h, min_w, max_w, ALIGN_LEFT, text.quit,
			*m_button_quit, &button_dropmenu_main);

	// language
	// ================================================================
	/* TODO check how many (valid?)file are in the ./lang dir, create button in loop */

}


void m_ui()
{
	if (active_dropmenu) {
		ui_highlight_button = ui_button_check_pos(&active_dropmenu);
		if ((ui_button_check_pos(&active_dropmenu) == NULL) && 
				(ui_button_check_pos(&active_dropmenu_parent) == NULL)) {
			active_dropmenu = NULL;
		}
	}
	if (input.mouse_button_left) {
		ui_button_check_click(&button_topbar);
		if (active_dropmenu) {
			ui_button_check_click(&active_dropmenu);
		}
	}		
}


void m_do_menu()
{
	m_ui();
}




