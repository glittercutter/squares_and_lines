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
// server.c - 

#include "server.h"

#include "net.h"
#include "ui.h"


void sv_button_close_window() 
{
	ui_button_close_window();
}

void lanhost_start_host()
{
	if (!net_init_server())
		active_window = &host_window;
}

void sv_init_ui()
{
	int x, y;
	int min_w, max_w;
	int h = button_font.size + UI_BAR_PADDING;
	int w;
	
	host_window.w = display_width * 0.7f;
	host_window.h = display_height * 0.7f;
	host_window.x1 = (display_width - host_window.w) / 2;
	host_window.y1 = ((display_height - button_topbar->h) - 
			host_window.h) / 2 + button_topbar->h;
	host_window.x2 = host_window.x1 + host_window.w;
	host_window.y2 = host_window.y1 + host_window.h;
	
	// window title
	x = host_window.x1; y = host_window.y1;
	min_w = host_window.w - (UI_BAR_PADDING * 2); max_w = min_w;
	w = strlen(text.host_game) * button_font.w;
	ui_new_button(x, y, w, h, min_w, max_w, ALIGN_LEFT,
			text.host_game, *ui_button_drag_window, 1, 1, &host_window.button);
	// close window
	min_w = 1;
	w = strlen("x") * button_font.w + UI_BAR_PADDING;
	x = host_window.x2 - w - (UI_BAR_PADDING * 2); y = host_window.y1;
	ui_new_button(x, y, w, h, min_w, max_w, ALIGN_CENTER,
			"x", *sv_button_close_window, 1, 1, &host_window.close_button);
	
	// server name
	x = host_window.x1 + (UI_BAR_PADDING * 3);
	y = host_window.y1 + h + (UI_BAR_PADDING * 2);
	ui_new_widget_plain_text(text.txt_srv_name_is, srv.name, "", x, y,
			color.text, &host_window.widget);
	
	// configure button
	min_w = 80;
	w = strlen(text.configure) * button_font.w + UI_BAR_PADDING;
	x = host_window.x2 - min_w - (UI_BAR_PADDING * 4);
	y = host_window.y1 + h + (UI_BAR_PADDING * 2);
	ui_new_button(x, y, w, h, min_w, max_w, ALIGN_CENTER, text.configure, 
			*sv_button_close_window, 1, 1, &host_window.close_button);


	ui_new_widget_list_box(host_window.x1 + (UI_BAR_PADDING * 2), 
			host_window.y1 + (host_window.h / 2),
			host_window.x2 - (UI_BAR_PADDING * 2), 
			host_window.y2 - (UI_BAR_PADDING * 2), &peer_list, 
			&host_window.widget);

	// client name
	peer_list.col_position[0] = 0;
	// ping
	peer_list.col_position[1] = (peer_list.list_box->w - SCROLLBAR_SIZE) * 0.7f;

	// collum name buttons
	w = strlen(text.lbox_player) * button_font.w + UI_BAR_PADDING;
	min_w = peer_list.col_position[1] - peer_list.col_position[0];
	ui_new_button(peer_list.col_position[0] + peer_list.list_box->x1, 
			peer_list.list_box->y1 - button_font.h, w, h, min_w, max_w,
			ALIGN_LEFT, text.lbox_player, NULL, 1, 0, &peer_list.col_name);

	w = strlen(text.lbox_ping) * button_font.w + UI_BAR_PADDING;
	min_w = peer_list.list_box->w - (SCROLLBAR_SIZE * 1.6f) - 
			peer_list.col_position[1];	
	ui_new_button(peer_list.col_position[1] + peer_list.list_box->x1, 
			peer_list.list_box->y1 - button_font.h, w, h, min_w, max_w,
			ALIGN_LEFT, text.lbox_ping, NULL, 1, 0, &peer_list.col_name);

	peer_list.list = NULL;

}


