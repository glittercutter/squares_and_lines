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

#include "common.h"
#include "client.h"
#include "editor.h"
#include "draw.h"
#include "fx.h"
#include "main.h"
#include "input.h"
#include "server.h"
#include "ui.h"


// game
void m_open_main()
{
	active_dropmenu = button_dropmenu_main;
	active_dropmenu_parent = button_topbar;
}

void m_open_option()
{
	char string[ LONG_STRING_LENGTH ];
	snprintf(string, LONG_STRING_LENGTH - 1, 
			"open the file %s with a text editor !", CONFIG_FILENAME);
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

// multiplayer
void m_open_multiplayer()
{
	active_dropmenu = button_dropmenu_multiplayer;
	active_dropmenu_parent = button_topbar;
}

void m_button_host_game()
{
	lanhost_start_host();
}

void m_button_join_game()
{
	lanclient_start_client();
}


void m_init_ui()
{	
	int min_w = 47;
	int max_w = 180;
	int w;
	int h = button_font.size + UI_BAR_PADDING;
	int x, y;
	// buttons position are based previously created button
	button_s *last_button;
	button_s *topbar;

	// ================================================================
	// topbar
	// ================================================================

	// game
	// ================================================================
	x = 0; y = 0;
	w = strlen(text.main_menu) * button_font.w;
	last_button = ui_new_button(x, y, w, h, min_w, max_w, ALIGN_CENTER,
			text.main_menu, *m_open_main, 0, 1, &button_topbar);
	topbar = last_button;
	
	// dropmenu
	// find the longest string for the dropmenu buttons
	min_w = longest_string(text.new_game, text.option, text.quit, NULL) * 
			button_font.w + (UI_BAR_PADDING * 4);

	x = 0; y = last_button->y2;
	w = strlen(text.new_game) * button_font.w + UI_BAR_PADDING;
	last_button = ui_new_button(x, y, w, h, min_w, max_w, ALIGN_LEFT,
			text.new_game, *m_button_new_game, 0, 0, &button_dropmenu_main);

	x = last_button->x1; y = last_button->y2;
	w = strlen(text.option) * button_font.w + UI_BAR_PADDING;
	last_button = ui_new_button(x, y, w, h, min_w, max_w, ALIGN_LEFT, 
			text.option, *m_open_option, 0, 0, &button_dropmenu_main);
	// TODO actual option window ;)

	x = last_button->x1; y = last_button->y2;
	w = strlen(text.quit) * button_font.w + UI_BAR_PADDING;
	last_button = ui_new_button(x, y, w, h, min_w, max_w, ALIGN_LEFT, 
			text.quit, *m_button_quit, 0, 0, &button_dropmenu_main);

	// multiplayer
	// ================================================================
	min_w = 47;
	x = topbar->x2; y = topbar->y1;
	w = strlen(text.multiplayer) * button_font.w;
	last_button = ui_new_button(x, y, w, h, min_w, max_w, ALIGN_CENTER,
			text.multiplayer, *m_open_multiplayer, 0, 1, &button_topbar);
	topbar = last_button;
	
	// dropmenu
	// find the longest string for the dropmenu buttons
	min_w = longest_string(text.host_game, text.join_game, NULL) * 
			button_font.w + (UI_BAR_PADDING * 4);

	x = last_button->x1; y = last_button->y2;
	w = strlen(text.host_game) * button_font.w + UI_BAR_PADDING;
	last_button = ui_new_button(x, y, w, h, min_w, max_w, ALIGN_LEFT,
			text.host_game, *m_button_host_game, 0, 0, &button_dropmenu_multiplayer);

	x = last_button->x1; y = last_button->y2;
	w = strlen(text.join_game) * button_font.w + UI_BAR_PADDING;
	last_button = ui_new_button(x, y, w, h, min_w, max_w, ALIGN_LEFT,
			text.join_game, *m_button_join_game, 0, 0, &button_dropmenu_multiplayer);

}


static void m_ui()
{
	if (active_dropmenu) {
		ui_highlight_button = ui_button_check_pos(&active_dropmenu);
		if ((ui_button_check_pos(&active_dropmenu) == NULL) && 
				(ui_button_check_pos(&active_dropmenu_parent) == NULL))
			active_dropmenu = NULL;
	}
	if (input.mouse_button_left) {
		// check if the mouse is on the topbar before checking buttons
		if (input.mouse_y <= button_topbar->y2)
			ui_button_check_click(&button_topbar);
		if (active_dropmenu)
			// active_dropmenu is a pointer to button type
			ui_button_check_click(&active_dropmenu);
	}		
}


void menu_main()
{
	m_ui();
}


