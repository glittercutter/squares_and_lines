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
// client.c - 

#include "client.h"

#include "common.h"
#include "net.h"
#include "ui.h"


void cl_update_srvlist();
void cl_clear_srvlist();


void lanclient_start_client()
{
	if (!net_init_client())
		active_window = &client_window;
	request_local_srv();
}


void cl_button_close_window() 
{	
	ui_button_close_window();
}


void cl_init_ui()
{
	int x, y;
	int min_w, max_w;
	int h = button_font.size + UI_BAR_PADDING;
	int w;
	
	client_window.w = display_width * 0.7f;
	client_window.h = display_height * 0.7f;
	client_window.x1 = (display_width - client_window.w) / 2;
	client_window.y1 = ((display_height - button_topbar->h) - client_window.h) / 2 + button_topbar->h;
	client_window.x2 = client_window.x1 + client_window.w;
	client_window.y2 = client_window.y1 + client_window.h;
	
	// window title
	x = client_window.x1; y = client_window.y1;
	min_w = client_window.w - (UI_BAR_PADDING * 2); max_w = min_w;
	w = strlen(text.join_game) * button_font.w;
	ui_new_button(x, y, w, h, min_w, max_w, ALIGN_LEFT,
			text.join_game, *ui_button_drag_window, 1, 1, &client_window.button);
	// close window
	min_w = 1;
	w = strlen("x") * button_font.w + UI_BAR_PADDING;
	x = client_window.x2 - w - (UI_BAR_PADDING * 2); y = client_window.y1;
	ui_new_button(x, y, w, h, min_w, max_w, ALIGN_CENTER, "x", 
			*cl_button_close_window, 1, 1, &client_window.close_button);

	// player name
	x = client_window.x1 + (UI_BAR_PADDING * 3);
	y = client_window.y1 + h + (UI_BAR_PADDING * 2);
	ui_new_widget_plain_text(text.txt_player_name_is, local_player.name, "", x, y,
			color.text, &client_window.widget);	
	
	// configure button
	min_w = 80;
	w = strlen(text.configure) * button_font.w + UI_BAR_PADDING;
	x = host_window.x2 - min_w - (UI_BAR_PADDING * 4);
	y = host_window.y1 + h + (UI_BAR_PADDING * 2);
	ui_new_button(x, y, w, h, min_w, max_w, ALIGN_CENTER,
			text.configure, *cl_button_close_window, 1, 1, 
			&client_window.button);
	// join button
	w = strlen(text.join) * button_font.w + UI_BAR_PADDING;
	y += h + (UI_BAR_PADDING);
	ui_new_button(x, y, w, h, min_w, max_w, ALIGN_CENTER,
			text.join, *cl_request_connection, 1, 1, 
			&client_window.button);	
	// update
	w = strlen(text.update) * button_font.w + UI_BAR_PADDING;
	y += h + (UI_BAR_PADDING);
	ui_new_button(x, y, w, h, min_w, max_w, ALIGN_CENTER,
			text.update, *cl_update_srvlist, 1, 1, 
			&client_window.button);

	ui_new_widget_list_box(
			client_window.x1 + (UI_BAR_PADDING * 2), 
			client_window.y1 +  (client_window.h / 2), 
			client_window.x2 - (UI_BAR_PADDING * 2), 
			client_window.y2 - (UI_BAR_PADDING * 2), &host_list, 
			&client_window.widget);

	// game name
	host_list.col_position[0] = 0;
	// ping
	host_list.col_position[1] = (host_list.list_box->w - SCROLLBAR_SIZE) * 0.7f;
	// player
	host_list.col_position[2] = (host_list.list_box->w - SCROLLBAR_SIZE) * 0.8f;

	// collum name buttons
	w = strlen(text.lbox_server) * button_font.w + UI_BAR_PADDING;
	min_w = host_list.col_position[1] - host_list.col_position[0];
	ui_new_button(host_list.col_position[0] + host_list.list_box->x1, 
			host_list.list_box->y1 - button_font.h, w, h, min_w, max_w,
			ALIGN_LEFT, text.lbox_server, NULL, 1, 0, &host_list.col_name);

	w = strlen(text.lbox_ping) * button_font.w + UI_BAR_PADDING;
	min_w = host_list.col_position[2] - host_list.col_position[1];
	ui_new_button(host_list.col_position[1] + host_list.list_box->x1, 
			host_list.list_box->y1 - button_font.h, w, h, min_w, max_w,
			ALIGN_LEFT, text.lbox_ping, NULL, 1, 0, &host_list.col_name);

	w = strlen(text.lbox_player) * button_font.w + UI_BAR_PADDING;
	min_w = host_list.list_box->w - (SCROLLBAR_SIZE * 1.6f) - 
			host_list.col_position[2];
	ui_new_button(host_list.col_position[2] + host_list.list_box->x1, 
			host_list.list_box->y1 - button_font.h, w, h, min_w, max_w,
			ALIGN_LEFT, text.lbox_player, NULL, 1, 0, &host_list.col_name);

	host_list.list = NULL;

}


void cl_add_lan_srv(int byte_readed, UDPpacket *p)
{
	srv_list_s **tmp_srv = &srv_list;
	srv_list_s *new_srv;
	char *name = (char*)&p->data[byte_readed];
	byte_readed += strlen((char*)&p->data[byte_readed]) + 1;

	int id = SDLNet_Read32(&p->data[byte_readed]);
	byte_readed += 4;

	short nplayer = SDLNet_Read16(&p->data[byte_readed]);
	byte_readed += 2;
	short max_nplayer = SDLNet_Read16(&p->data[byte_readed]);
	byte_readed += 2;

	Uint32 ping = SDLNet_Read32(&p->data[byte_readed]);
	byte_readed += 4;

	printf("recv tick: %u", ping);		

	while (*tmp_srv) {
		if ((p->address.host == (*tmp_srv)->address.host) && 
				(id == (*tmp_srv)->id)) {

			/* We have this one already, dont create a new instance */
			goto update_info_only;

		}
		printf("node\n");	
		tmp_srv = &(*tmp_srv)->next;
	}
	
	new_srv = malloc(sizeof(srv_list_s));
	printf("address: %p\n", (void*)new_srv);
	new_srv->next = NULL;
	new_srv->address = p->address;
	new_srv->id = id;
	
	tmp_srv = &srv_list;
	if (!*tmp_srv) {
		// first node
		*tmp_srv = new_srv;
	} else {
		while (*tmp_srv) {
			tmp_srv = &(*tmp_srv)->next;
		}
		// append to list
		*tmp_srv = new_srv;
	}

	new_srv->list = add_string_node(&host_list);	
	new_srv->list->string[0] = new_srv->name;
	new_srv->list->string[1] = new_srv->ping;
	new_srv->list->string[2] = new_srv->player;

	ui_scrollbar_update_size(strlist_len(host_list.list), 
			host_list.max_element, &host_list.list_box->scrollbar);

update_info_only:

	// server name
	strncpy((*tmp_srv)->name, name, sizeof((*tmp_srv)->name));
	// ping
	snprintf((*tmp_srv)->ping, sizeof((*tmp_srv)->ping), 
			"%d", (SDL_GetTicks() - ping));
	// player
	snprintf((*tmp_srv)->player, sizeof((*tmp_srv)->player), 
			"%hd / %hd", nplayer, max_nplayer);

}


void cl_update_srvlist()
{
	cl_clear_srvlist();
	request_local_srv();
}


void cl_clear_srvlist()
{
	srv_list_s *tmp_node = srv_list;
	srv_list_s *next_node;
	
	while (tmp_node) {
		next_node = tmp_node->next;
		free(tmp_node);
		tmp_node = next_node;
	}
	srv_list = NULL;

	clear_strlist(&host_list);
}




