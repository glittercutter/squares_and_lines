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
#include "editor.h"
#include "fx.h"
#include "game.h"
#include "net.h"
#include "server.h"
#include "ui.h"


void cl_update_srvlist(void);
void cl_clear_srvlist(void);

void cl_close();
int cl_init();
void cl_send_game_packet(void);
void cl_parse_udp_packet(void);
void cl_parse_game_packet(int byte_readed);
void cl_request_connection();
void cl_request_lan_server();
void cl_make_connection(int byte_readed);
void* cl_udp_listen(void *is_a_thread);
void cl_rm_acked_packet(Uint32 packet_n);
int cl_get_player_name(char* buf);
void cl_send_pong(void);
void cl_update_last_packet_tick(void);


Uint32 cl_last_packet_tick = 0;


void cl_ui_button_open_window()
{
	if (!cl_init())
		active_window = &client_window;
	cl_request_lan_server();
}


void cl_ui_button_close_window() 
{	
	ui_button_close_window();
}


void cl_ui_init()
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
			*cl_ui_button_close_window, 1, 1, &client_window.close_button);

	// player name
	x = client_window.x1 + (UI_BAR_PADDING * 3);
	y = client_window.y1 + h + (UI_BAR_PADDING * 2);
	ui_new_widget_plain_text(text.txt_player_name_is, local_player.name, "", x, y,
			color.text, &client_window.widget);	
	
	// buttons
	min_w = 80;
// 	x = host_window.x1 + (UI_BAR_PADDING * 4);
// 	y = host_window.y1 + h + (UI_BAR_PADDING * 2);

	y += h * 2;

// 	// configure button
// 	w = strlen(text.configure) * button_font.w + UI_BAR_PADDING;
// 	ui_new_button(x, y, w, h, min_w, max_w, ALIGN_CENTER,
// 			text.configure, *cl_ui_button_close_window, 1, 1, 
// 			&client_window.button);

	// join button
	w = strlen(text.join) * button_font.w + UI_BAR_PADDING;
	y += h + (UI_BAR_PADDING);
	ui_new_button(x, y, w, h, min_w, max_w, ALIGN_CENTER,
			text.join, *cl_request_connection, 1, 1, 
			&client_window.button);	
	// update button
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
	local_player.connected = false;

}


void cl_parse_srv_info(int byte_readed, UDPpacket *p)
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

	while (*tmp_srv) {
		if ((p->address.host == (*tmp_srv)->address.host) && 
				(id == (*tmp_srv)->id)) {

			/* We have this one already, only update information */
			goto update_info_only;

		}
		tmp_srv = &(*tmp_srv)->next;
	}
	
	new_srv = malloc(sizeof(srv_list_s));
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
	cl_request_lan_server();
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


void* cl_udp_listen(void *is_a_thread)
{
	Uint32 sent_tick = 0;
	Uint32 current_tick;

	while (net_is_client) {
		if (SDLNet_CheckSockets(udp_socket_set, PACKET_SEND_RATE)) {

			/* ====================================== */
			/* Uncomment this to simulate packet loss */
// 			if (net_test_packet_loss()) continue;
			/* ====================================== */

			if (SDLNet_UDP_Recv(main_udp_socket, udp_in_p)) {		
				// null terminate data to the received length
				udp_in_p->data[udp_in_p->len] = 0;
				cl_parse_udp_packet();
			}
			/* Dont send game packet if one was sent less 
			   than PACKET_SEND_RATE */
			current_tick = SDL_GetTicks();
			if (sent_tick <= current_tick - PACKET_SEND_RATE) {
				cl_send_game_packet();
				sent_tick = current_tick;
			}
		} else { 
			// "PACKET_SEND_RATE" timeout, send game packet
			if (!local_player.connected) continue;
			sent_tick = SDL_GetTicks();
			cl_send_game_packet();
		}
	}
	pthread_exit(NULL);
	return NULL; // GCC complain on Windows if we dont return anything
}


int cl_init()
{
	int tmp_port = BROADCAST_PORT;
	int rc = 0;
	unack_packet_s **tmp_unack_packet;
#ifdef WINDOWS
	DWORD name_len = sizeof local_player.name;
#endif
	
	if (net_is_client) return 0;
	if (net_is_server) srv_close();

	// set player name
	if (!local_player.name[0]) {

#ifdef WINDOWS
		GetUserName(local_player.name, &name_len);
#else
		strncpy(local_player.name, getenv("USER"), sizeof local_player.name);
#endif

	}

	while (42) {
		/* UDP - Open a socket on avalable port*/
		if ((main_udp_socket = SDLNet_UDP_Open(++tmp_port)))
			break; // sucess
		if (tmp_port == MAX_PORT) { // checked all ports
			fprintf(stderr, "cl: SDLNet_UDP_Open: %s\n", SDLNet_GetError());
			SDLNet_UDP_Close(main_udp_socket);
			printf("cl: Can't open any port !\n");
			return 1;
		}
	}
	udp_socket_set = SDLNet_AllocSocketSet(1);
	SDLNet_UDP_AddSocket(udp_socket_set, main_udp_socket);

	// init local player packet
	local_player.unack_packet_mem = malloc(sizeof(unack_packet_s) *
			UNACK_PACKET_STORAGE_SIZE);
	local_player.unack_packet_head = local_player.unack_packet_mem;
	tmp_unack_packet = &local_player.unack_packet_mem;
	for (int i = 1; i < UNACK_PACKET_STORAGE_SIZE; i++) {
		(*tmp_unack_packet)->next = (unack_packet_s *)local_player.unack_packet_head + i;
		(*tmp_unack_packet)->active = false;
		tmp_unack_packet = &(*tmp_unack_packet)->next;
	}
	(*tmp_unack_packet)->next = NULL;
	(*tmp_unack_packet)->active = false;
	local_player.unack_packet_tail = *tmp_unack_packet;
	local_player.unack_packet_next = local_player.unack_packet_head;

	net_is_client = true;
	net_game = true;

	rc = pthread_create(&udp_listen_th, NULL, cl_udp_listen, (void*)rc);
	pthread_detach(udp_listen_th);

	cl_last_packet_tick = SDL_GetTicks();

	return 0;
}


void cl_close()
{
	if (!net_is_client) return;

	net_is_client = false;
	net_game = false;
	SDL_Delay(500);

	SDLNet_FreeSocketSet(udp_socket_set);
	
	SDLNet_UDP_Close(main_udp_socket);	
	
	local_player.connected = false;
	free(local_player.unack_packet_mem);
}


void cl_make_connection(int byte_readed)
{
	if (local_player.connected) return;

	// read username (in case the host changed it)
	strncpy(local_player.name, (char *)&udp_in_p->data[byte_readed],
			sizeof(local_player.name));
	byte_readed += strlen(local_player.name) + 1;
	
	// Read the port number the host opened for UDP communication
	// The port number is already in network byte order
	memcpy(&main_udp_ip.port, &udp_in_p->data[byte_readed], sizeof(short));
	byte_readed += sizeof(short);
	// use the address we received the packet from
	main_udp_ip.host = udp_in_p->address.host;

	// read player number the server gave us
	local_player.player_n = SDLNet_Read32(&udp_in_p->data[byte_readed]);
	byte_readed += 4;
	
	DEBUG(printf("cl: Connected\n"));
	DEBUG(printf("cl: Username: %s\n", local_player.name));
	set_gamestate_EDITOR();
	local_player.connected = true;
	cl_ui_button_close_window();
	ui_new_message(text.cl_connected);

	cl_last_packet_tick = SDL_GetTicks();
}


/* 
====================
cl_request_connection

Send a UDP packet with a request header "0100", 
program name ID "udp_sdltest",
client username.
====================
*/
void cl_request_connection()
{
	if (host_list.list_box->selected_row == -1) {
		ui_new_message("no server selected");
		return;
	}
	
	int byte_writed = 0;
	srv_list_s **tmp_srv_list = &srv_list;

	int offset_viewable_element = host_list.list_box->scrollbar.offset + 
			host_list.list_box->viewable_element;
	// address from selected server
	for (int i = host_list.list_box->scrollbar.offset;
			(i < offset_viewable_element); i++) {
		if ((*tmp_srv_list == NULL)) return;
		if (host_list.list_box->selected_row == i) {
			break;
		}
		tmp_srv_list = &(*tmp_srv_list)->next;
	}

	// write id header
	udp_out_p->data[byte_writed++] = NET_GLOBAL_HEADER;
	// write connection request bytes
	udp_out_p->data[byte_writed++] = NET_CL_CONNECT;
	
	strcpy((char *)&udp_out_p->data[byte_writed], "udp_sdltest");
	 /* set position for next write */
	byte_writed += strlen((char *)&udp_out_p->data[byte_writed]);
	strcat((char *)&udp_out_p->data[byte_writed], "\0");
	byte_writed += 1;
	
	// sent username to the client
	// set default name if none is present
	if (!local_player.name[0]) strncpy(local_player.name, "noname",
			sizeof(local_player.name));
	strcpy((char *)&udp_out_p->data[byte_writed], local_player.name);
	byte_writed += strlen((char *)&udp_out_p->data[byte_writed]);
	strcat((char *)&udp_out_p->data[byte_writed], "\0");
	byte_writed += 1;

	udp_out_p->address = (*tmp_srv_list)->address;
	udp_out_p->len = byte_writed;

	if (!SDLNet_UDP_Send(main_udp_socket, -1, udp_out_p))
			fprintf(stderr, "cl: SDLNet_UDP_Send: %s\n",
			SDLNet_GetError());
	
	DEBUG(printf("cl: ip: %x\n", SDLNet_Read32(&udp_out_p->address.host)));
	DEBUG(printf("cl: port: %x\n", SDLNet_Read16(&udp_out_p->address.port)));
	DEBUG(printf("cl: Waiting host response\n"));
}


void cl_request_lan_server()
{
	int byte_writed = 0;
	IPaddress udp_ip;

	// write id header
	udp_out_p->data[byte_writed++] = NET_GLOBAL_HEADER;	
	// write server request info bytes
	udp_out_p->data[byte_writed++] = NET_CL_INFO;
	
	// app name
	strcpy((char *)&udp_out_p->data[byte_writed], "udp_sdltest");
	byte_writed += strlen((char *)&udp_out_p->data[byte_writed]);
	strcat((char *)&udp_out_p->data[byte_writed], "\0");
	byte_writed += 1;
	
	// ping
	SDLNet_Write32(SDL_GetTicks(), &udp_out_p->data[byte_writed]);
	byte_writed += 4;


	if (SDLNet_ResolveHost(&udp_ip, BROADCAST_ADDRESS, BROADCAST_PORT) == -1) {
		fprintf(stderr, "cl: SDLNet_ResolveHost(%s:%d) %s\n", BROADCAST_ADDRESS,
				BROADCAST_PORT, SDLNet_GetError());
	}
	udp_out_p->address = udp_ip;
	udp_out_p->len = byte_writed;
	SDLNet_UDP_Send(main_udp_socket, -1, udp_out_p);
}


void cl_parse_game_packet(int byte_readed)
{
	// TODO really only need 2 integer...
	int packet_size;
	int state;
	int x, y;
	int side;
	int packet_n;

	cl_last_packet_tick = SDL_GetTicks();
	
	if (byte_readed >= PACKET_LENGHT) return;

	while (udp_in_p->data[byte_readed] != NET_NULL) {
		switch (udp_in_p->data[byte_readed++]) {
		
		case RESENT_BYTE:
			packet_n = net_read_32(&byte_readed);
			packet_size = net_read_32(&byte_readed);
			if (packet_n <= local_player.recev_packet_n) {
				/* packet was already received,
				// skip to the end of the packet */
				byte_readed += packet_size;
				break;
			}
			DEBUG(printf("cl: resent packet\n"));
			break;
		
		case NET_PING:
			cl_send_pong();
			break;

		case NET_PONG:
			break;

		case PACKET_ACK_BYTE:
			cl_rm_acked_packet(net_read_32(&byte_readed));
			// don't send ack if the received packet contain only an ack
			if (udp_in_p->data[byte_readed] == NET_NULL) {
				local_player.recev_packet_ack_sent = true;
			}
			break;
		
		/* GAME */
		case G_SEG_GLOW_BYTE:
			x = net_read_32(&byte_readed);
			y = net_read_32(&byte_readed);
			seg_glow_current.side = net_read_32(&byte_readed);
			fx_net_glow(x, y);
			break;

		case G_ADD_SEG_BYTE:
			x = net_read_32(&byte_readed);
			y = net_read_32(&byte_readed);
			side = net_read_32(&byte_readed);
			g_add_segment(x, y, side);
			break;

		case G_PLAYER_TURN_BYTE:
			local_player.turn = net_read_32(&byte_readed);
			fx_new_transition(NULL, 5, FX_PLAYER_CHANGE);
			break;


		/* EDITOR */
		case ED_ADD_SQUARE_BYTE:
			x = net_read_32(&byte_readed);
			y = net_read_32(&byte_readed);
			ed_net_add_square(x, y);
			break;
		case ED_RM_SQUARE_BYTE:
			x = net_read_32(&byte_readed);
			y = net_read_32(&byte_readed);
			ed_net_rm_square(x, y);
			break;

		case STATE_CHANGE_BYTE:
			state = net_read_32(&byte_readed);
			switch (state) {
			case GAME:
				set_gamestate_GAME();
				break;
			case EDITOR:
				set_gamestate_EDITOR();
				break;
			}
			break;

		case NET_SYNC_SQUARES:
			ed_clear_squares();
			break;

		case NET_SYNC_PLAYER_NAME:
			byte_readed += 
				cl_get_player_name((char*)&udp_in_p->data[byte_readed]);
			break;
		
		default:
			return;
		}
	}
}


void cl_parse_udp_packet()
{	
	int byte_readed = 0;
	int packet_n;
	
	if (udp_in_p->data[byte_readed++] != NET_GLOBAL_HEADER) return;

	switch (udp_in_p->data[byte_readed++]) {
	// game packet from server
	case NET_SRV_GAME:
		packet_n = net_read_32(&byte_readed);
		if (packet_n < local_player.recev_packet_n) {
			DEBUG(printf("cl: unordered packet\n"));
			break; 
		}

		local_player.recev_packet_ack_sent = false;
		cl_parse_game_packet(byte_readed);
		local_player.recev_packet_n = packet_n;
		break;

	// message from server
	case NET_SRV_MESSAGE:
		// display message
		break;
	
	// server info
	case NET_SRV_INFO:
		if (strcmp((char *)&udp_in_p->data[byte_readed], "udp_sdltest"))
			break;
		byte_readed += strlen((char *)&udp_in_p->data[byte_readed]) + 1;	
		cl_parse_srv_info(byte_readed, udp_in_p);
		break;
	
	// server connection request response
	case NET_SRV_CONNECT:
		if ((udp_in_p->data[byte_readed]) == NET_REFUSE) {
			++byte_readed;
			if (strcmp((char *)&udp_in_p->data[byte_readed], "udp_sdltest"))
				break;
			byte_readed += strlen((char *)&udp_in_p->data[byte_readed]) + 1;
			// display reason for being refused
			ui_new_message((char *)&udp_in_p->data[byte_readed]);
			break;
		}
		if ((udp_in_p->data[byte_readed]) != NET_ACCEPT)
			break;
		++byte_readed;
		if (strcmp((char *)&udp_in_p->data[byte_readed], "udp_sdltest"))
			break;
		byte_readed += strlen((char *)&udp_in_p->data[byte_readed]) + 1;
		cl_make_connection(byte_readed);
		break;
	
	default:
		break;
	}
}


void cl_send_game_packet(void)
{
	static Uint32 packet_n = 0;
	int byte_writed = 0;
	unack_packet_s **tmp_unack_packet;

	// check if we have something to send
	if ((local_player.recev_packet_ack_sent) && 
			(!local_player.unack_packet_head->active) &&
			(udp_new_buffer_writed == 0) &&
			(SDL_GetTicks() - cl_last_packet_tick < NET_PING_RATE)) {
		return;
	}
	
	// write id header
	udp_out_p->data[byte_writed++] = NET_GLOBAL_HEADER;
	// write game header
	udp_out_p->data[byte_writed++] = NET_CL_GAME;
	
	++packet_n;
	// write packet number
	SDLNet_Write32(packet_n, &udp_out_p->data[byte_writed]);
	byte_writed += 4;
	
	if (!local_player.recev_packet_ack_sent) {
		udp_out_p->data[byte_writed++] = PACKET_ACK_BYTE;
		SDLNet_Write32(local_player.recev_packet_n, &udp_out_p->data[byte_writed]);
		byte_writed += 4;
		local_player.recev_packet_ack_sent = true;
	}

	// resend unack packet
	if (local_player.unack_packet_head->active) {
		tmp_unack_packet = &local_player.unack_packet_head;
		while ((*tmp_unack_packet)->active) {
			DEBUG(printf("cl: resent no.%hd\n", (*tmp_unack_packet)->packet_n));
			if (!(*tmp_unack_packet)->data[0]) {
				// packet is empty
				tmp_unack_packet = &(*tmp_unack_packet)->next;
				continue;
			}
			udp_out_p->data[byte_writed++] = RESENT_BYTE;
			// write packet number
			SDLNet_Write32((*tmp_unack_packet)->packet_n,
					&udp_out_p->data[byte_writed]);
			byte_writed += 4;
			// write packet lenght
			SDLNet_Write32((*tmp_unack_packet)->len,
					&udp_out_p->data[byte_writed]);
			byte_writed += 4;
			// write packet data
			memcpy(&udp_out_p->data[byte_writed], (*tmp_unack_packet)->data,
					(*tmp_unack_packet)->len);
			byte_writed += (*tmp_unack_packet)->len;

			if ((*tmp_unack_packet)->next) {
				tmp_unack_packet = &(*tmp_unack_packet)->next;
			} else {
				DEBUG(printf("cl: unack packet overflow! (send)\n"));
				break;
			}
		}
	}

	if (udp_new_buffer_writed != 0) {
		pthread_mutex_lock(&udp_new_buffer_mutex);
		memcpy(&udp_out_p->data[byte_writed], udp_new_buffer, 
				udp_new_buffer_writed);
		byte_writed += udp_new_buffer_writed;

		// copy packet to resend it if non-acked
		memcpy(&local_player.unack_packet_next->data,
				udp_new_buffer, udp_new_buffer_writed);

		local_player.unack_packet_next->active = true;
		local_player.unack_packet_next->len = udp_new_buffer_writed;
		local_player.unack_packet_next->packet_n = packet_n;
		
		if (local_player.unack_packet_next->next == NULL) {
			DEBUG(printf("cl: unack packet overflow! (save)\n"));
		} else {
			local_player.unack_packet_next = local_player.unack_packet_next->next;
		}
		
		udp_new_buffer_writed = 0;
		pthread_mutex_unlock(&udp_new_buffer_mutex);
	}

	if ((SDL_GetTicks() - cl_last_packet_tick) > NET_PING_RATE) {
		if ((SDL_GetTicks() - cl_last_packet_tick) > NET_PING_TIMEOUT) {
			ui_new_message("Can't reach server, disconnecting\n");
			cl_close();
			set_gamestate_EDITOR();
			return;
		}
		printf("sending ping\n");
		udp_out_p->data[byte_writed++] = NET_PING;
	}

	// terminate packet
	udp_out_p->data[byte_writed++] = NET_NULL;

	udp_out_p->len = byte_writed;
	udp_out_p->address = main_udp_ip;

	if (!SDLNet_UDP_Send(main_udp_socket, -1, udp_out_p))
		fprintf(stderr, "cl: SDLNet_UDP_Send: %s\n", SDLNet_GetError());
}


void cl_rm_acked_packet(Uint32 packet_n)
{
	while (local_player.unack_packet_head->active && 
			local_player.unack_packet_head->packet_n <= packet_n) {

		if (local_player.unack_packet_next == local_player.unack_packet_head) {
			local_player.unack_packet_next = local_player.unack_packet_head->next;
		}

		local_player.unack_packet_head->active = false;
		local_player.unack_packet_tail->next = local_player.unack_packet_head;
		local_player.unack_packet_head = local_player.unack_packet_head->next;
		local_player.unack_packet_tail = local_player.unack_packet_tail->next;
		local_player.unack_packet_tail->next = NULL;
	}
}


/* 
====================
cl_get_player_list

Receive player name from the server
Return the lenght of 'buf'
====================
*/
int cl_get_player_name(char* buf)
{
	int buflen = strlen(buf);
	int player_n = NONE;
	char name[32];

	if (sscanf(buf, "%d %s", &player_n, name) == 2) {
		strncpy(player[player_n].name, name, sizeof player[player_n].name);
	}

	return buflen + 1;
}


void cl_send_pong()
{
	net_write_int(NET_PONG, 0);
}


void cl_update_last_packet_tick()
{
	cl_last_packet_tick = SDL_GetTicks();
}


