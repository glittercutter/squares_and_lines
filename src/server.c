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

#include "client.h"
#include "common.h"
#include "editor.h"
#include "fx.h"
#include "game.h"
#include "net.h"
#include "ui.h"


/* Maximum number of connection in server socket set */
static int srv_max_connection = 50; // TODO dynamic expension


int srv_init();
void srv_close();
int srv_accept_request(client_s *cl);
void srv_refuse_request(char *reason);
void srv_new_client(int byte_readed);
void srv_rm_client(client_s *cl);
void srv_clear_client();
void srv_send_info(int byte_readed);
void srv_parse_game_packet(int byte_readed, client_s *cl);
void srv_parse_udp_packet(client_s *cl);
void* srv_udp_listen(void *is_a_thread);
void srv_send_game_packet();
void srv_rm_acked_packet(Uint32 packet_n, client_s *cl);
void srv_send_pong(client_s *cl);
void srv_update_last_packet_tick(client_s *cl);



void srv_ui_button_close_window() 
{
	ui_button_close_window();
}


void srv_ui_button_open_window()
{
	if (!srv_init())
		active_window = &host_window;
}


void srv_ui_init()
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
			"x", *srv_ui_button_close_window, 1, 1, &host_window.close_button);
	
	// server name
	x = host_window.x1 + (UI_BAR_PADDING * 3);
	y = host_window.y1 + h + (UI_BAR_PADDING * 2);
	ui_new_widget_plain_text(text.txt_srv_name_is, srv.name, "", x, y,
			color.text, &host_window.widget);
	
// 	// configure button
// 	min_w = 80;
// 	w = strlen(text.configure) * button_font.w + UI_BAR_PADDING;
// 	x = host_window.x2 - min_w - (UI_BAR_PADDING * 4);
// 	y = host_window.y1 + h + (UI_BAR_PADDING * 2);
// 	ui_new_button(x, y, w, h, min_w, max_w, ALIGN_CENTER, text.configure, 
// 			*srv_ui_button_close_window, 1, 1, &host_window.close_button);


	ui_new_widget_list_box(host_window.x1 + (UI_BAR_PADDING * 2), 
			host_window.y1 + (host_window.h / 2),
			host_window.x2 - (UI_BAR_PADDING * 2), 
			host_window.y2 - (UI_BAR_PADDING * 2), &client_list, 
			&host_window.widget);

	// client name
	client_list.col_position[0] = 0;
	// ping
	client_list.col_position[1] = (client_list.list_box->w - SCROLLBAR_SIZE) * 0.7f;

	// collum name buttons
	w = strlen(text.lbox_player) * button_font.w + UI_BAR_PADDING;
	min_w = client_list.col_position[1] - client_list.col_position[0];
	ui_new_button(client_list.col_position[0] + client_list.list_box->x1, 
			client_list.list_box->y1 - button_font.h, w, h, min_w, max_w,
			ALIGN_LEFT, text.lbox_player, NULL, 1, 0, &client_list.col_name);

	w = strlen(text.lbox_ping) * button_font.w + UI_BAR_PADDING;
	min_w = client_list.list_box->w - (SCROLLBAR_SIZE * 1.6f) - 
			client_list.col_position[1];	
	ui_new_button(client_list.col_position[1] + client_list.list_box->x1, 
			client_list.list_box->y1 - button_font.h, w, h, min_w, max_w,
			ALIGN_LEFT, text.lbox_ping, NULL, 1, 0, &client_list.col_name);

	client_list.list = NULL;

}


/* 
====================
srv_init

initialize server,
start listening thread
====================
*/
int srv_init()
{
	int server_port = BROADCAST_PORT;
	int rc = 0;
	unack_packet_s **tmp_unack_packet;

	if (net_is_server) return 0;
	if (net_is_client) cl_close();
	
	// set player name
	if (!local_player.name[0])
		strncpy(local_player.name, getenv("USER"), sizeof local_player.name);
	
	// insert name
	strncpy(player[local_player.player_n].name, local_player.name, 
		sizeof player[local_player.player_n].name);
	
	// set server name
	if (!srv.name[0]) {
		strncpy(srv.name, local_player.name, sizeof local_player.name);
		strncat(srv.name, "'s server", sizeof local_player.name);
	}

	// Generate server ID to allow multiple server on the same IP
	if (!srv.id)
		srv.id = get_random_number(1000000);

	/* UDP - Open a socket on a known port */
	if (!(main_udp_socket = SDLNet_UDP_Open(server_port))) {
		fprintf(stderr, "SDLNet_UDP_Open: %s\n", SDLNet_GetError());
		ui_new_message("Server can't open port: %d.\n", server_port);
		SDLNet_UDP_Close(main_udp_socket);
		return 1;
	}

	udp_socket_set = SDLNet_AllocSocketSet(srv_max_connection);
	SDLNet_UDP_AddSocket(udp_socket_set, main_udp_socket);

	// init local player packet
	local_player.unack_packet_mem = malloc(sizeof(unack_packet_s) *
			UNACK_PACKET_STORAGE_SIZE);
	local_player.unack_packet_head = local_player.unack_packet_mem;
	tmp_unack_packet = &local_player.unack_packet_mem;
	for (int i = 1; i < UNACK_PACKET_STORAGE_SIZE; i++) {
		(*tmp_unack_packet)->next = (unack_packet_s *)local_player.unack_packet_head + i;
		tmp_unack_packet = &(*tmp_unack_packet)->next;
	}
	(*tmp_unack_packet)->next = NULL;
	(*tmp_unack_packet)->active = false;
	local_player.unack_packet_tail = *tmp_unack_packet;
	local_player.unack_packet_next = local_player.unack_packet_head;

	srv.max_nplayer = 2;
	srv.nplayer = 1;
	local_player.player_n = srv.nplayer - 1;
	local_player.turn = srv.nplayer;

	net_is_server = true;
	net_game = true;

	rc = pthread_create(&udp_listen_th, NULL, srv_udp_listen, (void*)rc);
	pthread_detach(udp_listen_th);

	return 0;
}


/* 
====================
srv_close

destroy everything create by "srv_init"
====================
*/
void srv_close()
{
	if (!net_is_server) return;

	net_is_server = false;
	net_game = false;
	local_player.connected = false;
	SDL_Delay(500);

	srv_clear_client();
	SDLNet_FreeSocketSet(udp_socket_set);
	SDLNet_UDP_Close(main_udp_socket);
	free(local_player.unack_packet_mem);
}


/* 
====================
srv_accept_request

Open a new UDP socket for the client.
Accept client request by replying with a UDP packet 
====================
*/
int srv_accept_request(client_s *cl)
{	
	const int max_ack_send = 5;
	const int max_ack_wait = 10;

	int port = BROADCAST_PORT + 1;
	int i_check = 0; // no of connection check for each packet sent
	int i_sent = 0; // no of ack packet sent
	int byte_writed = 0; // next write position
	
	// open a UDP socket for client
	while (42) {	
		if ((cl->udp_socket = SDLNet_UDP_Open(port)))
			break;// succes

		if (port == MAX_PORT) { // cant open any port
			fprintf(stderr, "srv: SDLNet_UDP_Open: %s\n", SDLNet_GetError());
			fprintf(stderr, "srv: Can't open any port !\n");
			return 1;
		}
		++port;
	}

	/* Write packet */
	
	// write id header
	udp_out_p->data[byte_writed++] = NET_GLOBAL_HEADER;
	// write connection answer byte
	udp_out_p->data[byte_writed++] = NET_SRV_CONNECT;
	// write connection accepted byte
	udp_out_p->data[byte_writed++] = NET_ACCEPT;

	// Application ID
	strcpy((char *)&udp_out_p->data[byte_writed], "udp_sdltest");
	byte_writed += strlen("udp_sdltest");
	udp_out_p->data[byte_writed++] = '\0';
	
	// Client username
	strcpy((char *)&udp_out_p->data[byte_writed], cl->username);
	byte_writed += strlen(cl->username);
	udp_out_p->data[byte_writed++] = '\0';

	// Port number we opened for the client
	SDLNet_Write16(port, &udp_out_p->data[byte_writed]);
	byte_writed += 2;
	
	// player no.
	SDLNet_Write32(cl->player_n, &udp_out_p->data[byte_writed]);
	byte_writed += 4;
	
	// terminate packet
	udp_out_p->data[byte_writed++] = NET_NULL;

	udp_out_p->address = cl->ip;
	udp_out_p->len = byte_writed;

	while (i_sent < max_ack_send) {
		if (!i_check) {
			if (!SDLNet_UDP_Send(main_udp_socket, -1, udp_out_p)) 
				fprintf(stderr, "srv: SDLNet_UDP_Send: %s\n",
						SDLNet_GetError());
			DEBUG(printf("srv: Accept ack sent to %s.\n", cl->username));
			++i_sent;
			i_check = max_ack_wait;
		}
		SDL_Delay(10);

		SDLNet_UDP_AddSocket(udp_socket_set, cl->udp_socket);
		cl->connected = true;
		DEBUG(printf("srv: Connected.\n"));	
		net_write_sync_square();
		return 0;
	}

	return 1; // error
}


/* 
====================
srv_refuse_request

====================
*/
void srv_refuse_request(char *reason)
{	
	int byte_writed = 0;

	/* Write packet */
	
	// write id header
	udp_out_p->data[byte_writed++] = NET_GLOBAL_HEADER;
	// write connection answer byte
	udp_out_p->data[byte_writed++] = NET_SRV_CONNECT;
	// write connection refused byte
	udp_out_p->data[byte_writed++] = NET_REFUSE;

	// Application ID
	strcpy((char *)&udp_out_p->data[byte_writed], "udp_sdltest");
	byte_writed += strlen((char *)&udp_out_p->data[byte_writed]);
	strcat((char *)&udp_out_p->data[byte_writed++], "\0");
	
	// refused reason string
	strcpy((char *)&udp_out_p->data[byte_writed], reason);
	byte_writed += strlen((char *)&udp_out_p->data[byte_writed]);
	strcat((char *)&udp_out_p->data[byte_writed++], "\0");

	udp_out_p->address = udp_in_p->address;
	udp_out_p->len = byte_writed;

	SDLNet_UDP_Send(main_udp_socket, -1, udp_out_p);
}


void srv_new_client(int byte_readed)
{
	int i = 0;
	char i_string[10];
	char tmp_username[32];
	client_s **tmp_client = &client;
	client_s *new_client;
	unack_packet_s **tmp_unack_packet;
	
	// refuse connection
	// server is full
	if (srv.nplayer >= srv.max_nplayer) {
		srv_refuse_request(text.srv_full);
		return;
	}
	// game in progress
	if (gamestate == GAME) {
		srv_refuse_request("game in progress");
		return;
	}

	// get client name from the packet
	strncpy(tmp_username, (char *)&udp_in_p->data[byte_readed],
			sizeof(tmp_username));

	// check if name is the client as the same name as ours
	if (!strncmp(local_player.name, tmp_username, sizeof local_player.name - 1)) {
		++i;
		snprintf(i_string, sizeof(i_string) - 1, "[%d]", i);
		strncat(tmp_username, i_string, sizeof(tmp_username));
	}

	/* check if the username is is the same as another player, 
		if so, it is numbered */
	while (*tmp_client) {
		if (!strncmp((*tmp_client)->username, tmp_username, 
				sizeof local_player.name)) {
			++i;
			snprintf(i_string, sizeof(i_string), "[%d]", i);
			strncpy(tmp_username, (char *)&udp_in_p->data[byte_readed],
					sizeof(tmp_username) - strlen(i_string));
			strncat(tmp_username, i_string, sizeof(tmp_username) - 1);
			// restart the search from the first element
			tmp_client = &client;

		} else {
			tmp_client = &(*tmp_client)->next;
		}
	}

	DEBUG(printf("srv: Connection request\n"));
	DEBUG(printf("srv: Username: %s\n", tmp_username));

	// allocate & init client variables
	new_client = malloc(sizeof(client_s));
	new_client->next = NULL;
	new_client->connected = false;
	new_client->ip = udp_in_p->address;
	strncpy(new_client->username, tmp_username, sizeof(new_client->username));
	new_client->recev_packet_n = 0;
	new_client->recev_packet_ack_sent = false;
	new_client->new_packet_buffer_size = 0;
	new_client->last_packet_tick = SDL_GetTicks();
	new_client->player_n = srv.nplayer;

	if (srv_accept_request(new_client)) {
		printf("srv: Error connecting to client\n");
		SDLNet_UDP_Close(new_client->udp_socket);
		free(new_client);
		return;
	}

	// insert name in player list
	strncpy(player[new_client->player_n].name, new_client->username,
			sizeof player[new_client->player_n].name);

	// add client to the end of the list
	// find the end of list
	tmp_client = &client;
	while (*tmp_client) {
		tmp_client = &(*tmp_client)->next;
	}
	*tmp_client = new_client;
	// add node for client
	new_client->list = add_string_node(&client_list);	
	new_client->list->string[0] = new_client->username;
	ui_scrollbar_update_size(strlist_len(client_list.list), 
			client_list.max_element, &client_list.list_box->scrollbar);

	// alloc space to store client unack packets
	new_client->unack_packet_mem = calloc(UNACK_PACKET_STORAGE_SIZE,
			sizeof(unack_packet_s));
	new_client->unack_packet_head = new_client->unack_packet_mem;
	tmp_unack_packet = &new_client->unack_packet_head;
	for (int i = 1; i < UNACK_PACKET_STORAGE_SIZE; i++) {
		(*tmp_unack_packet)->next = 
				(unack_packet_s *)new_client->unack_packet_head + i;
		tmp_unack_packet = &(*tmp_unack_packet)->next;
	}
	(*tmp_unack_packet)->next = NULL;
	(*tmp_unack_packet)->active = false;
	new_client->unack_packet_tail = *tmp_unack_packet;
	new_client->unack_packet_next = new_client->unack_packet_head;

	// mutex for client packets
	if (pthread_mutex_init(&new_client->new_packet_buffer_mutex, NULL))
		eprint("srv: pthread_mutex_init\n");
	
	// the server got a new client
	++srv.nplayer;
}


void srv_rm_client(client_s *cl)
{	
	client_s **tmp_cl = &client;
	DEBUG(printf("srv: %s: disconnected\n", cl->username));

	SDLNet_UDP_DelSocket(udp_socket_set, cl->udp_socket);	
	SDLNet_UDP_Close(cl->udp_socket);
	
	rm_string_node(&client_list, cl->list);
	free(cl->unack_packet_mem);

	if (client == cl) {
		// first node
		if (cl->next) {
			client = cl->next;
		} else {
			client = NULL;
		}
		free(cl);
	} else { 
		while (*tmp_cl) {
			if ((*tmp_cl)->next == cl) {
				// replace with the next node
				(*tmp_cl)->next = cl->next;
				break;
			}
			tmp_cl = &(*tmp_cl)->next;
		}
		free(cl);
	}
	
	ui_scrollbar_update_size(strlist_len(client_list.list), 
			client_list.max_element, &client_list.list_box->scrollbar);
	--srv.nplayer;
}


void srv_clear_client()
{	
	client_s *next_cl;
	client_s *cl = client;

	while (cl) {
		SDLNet_UDP_DelSocket(udp_socket_set, cl->udp_socket);	
		SDLNet_UDP_Close(cl->udp_socket);

		next_cl = cl->next;
		free(cl);
		cl = next_cl;
	}

	client = NULL;
	clear_strlist(&client_list);
	ui_scrollbar_update_size(strlist_len(client_list.list), 
			client_list.max_element, &client_list.list_box->scrollbar);
}


/* 
====================
srv_send_info

Send info about our server.
====================
*/
void srv_send_info(int byte_readed)
{
	int byte_writed = 0;

	/* Write packet */
	
	// write id header
	udp_out_p->data[byte_writed++] = NET_GLOBAL_HEADER;
	// write info answer bytes
	udp_out_p->data[byte_writed++] = NET_SRV_INFO;
	
	// Program name
	strcpy((char *)&udp_out_p->data[byte_writed], "udp_sdltest");
	byte_writed += strlen((char *)&udp_out_p->data[byte_writed]);
	strcat((char *)&udp_out_p->data[byte_writed], "\0");
	byte_writed += 1;
	
	// Server name
	if (!srv.name[0])
		strncpy(srv.name, "default name", sizeof(srv.name));
	strcpy((char *)&udp_out_p->data[byte_writed], srv.name);
	byte_writed += strlen((char *)&udp_out_p->data[byte_writed]);
	strcat((char *)&udp_out_p->data[byte_writed], "\0");
	byte_writed += 1;
	
	// Server ID
	SDLNet_Write32(srv.id, &udp_out_p->data[byte_writed]);
	byte_writed += 4;
	
	// Current/Max players
	SDLNet_Write16(srv.nplayer, &udp_out_p->data[byte_writed]);
	byte_writed += 2;
	SDLNet_Write16(srv.max_nplayer, &udp_out_p->data[byte_writed]);
	byte_writed += 2;
	
	// return tick for latency (ping)
	memcpy(&udp_out_p->data[byte_writed], &udp_in_p->data[byte_readed], 4);
	byte_writed += 4;

	udp_out_p->address.host = udp_in_p->address.host;
	udp_out_p->address.port = udp_in_p->address.port;
	udp_out_p->len = byte_writed;
	SDLNet_UDP_Send(main_udp_socket, -1, udp_out_p);
}


void srv_parse_game_packet(int byte_readed, client_s *cl)
{
	int packet_size;
	int x, y;
	int side;
	
	cl->last_packet_tick = SDL_GetTicks();

	while (udp_in_p->data[byte_readed] != NET_NULL) {
		switch (udp_in_p->data[byte_readed++]) {

		case RESENT_BYTE:
			if (SDLNet_Read32(&udp_in_p->data[byte_readed]) <= 
					cl->recev_packet_n) {
				byte_readed += 4;
				/* packet was already received,
				// skip to the end of the packet */
				packet_size = SDLNet_Read32(&udp_in_p->data[byte_readed]);
				byte_readed += packet_size;
				byte_readed += 4;
				DEBUG(printf("srv: packet received already\n"));
				break;
			}
			DEBUG(printf("srv: resent packet\n"));
			byte_readed += 8; // (packet number and lenght)
			break;

		case NET_PING:
			srv_send_pong(cl);
			break;

		case NET_PONG:
			break;

		case PACKET_ACK_BYTE:
			srv_rm_acked_packet(SDLNet_Read32(&udp_in_p->data[byte_readed]), 
					cl);
			byte_readed += 4;
			// don't send ack if the received packet contain only an ack
			if (udp_in_p->data[byte_readed] == NET_NULL) {
				cl->recev_packet_ack_sent = true;
			}
			break;

		/* GAME */
		case G_SEG_GLOW_BYTE:
			x = SDLNet_Read32(&udp_in_p->data[byte_readed]);
			byte_readed += 4;
			y = SDLNet_Read32(&udp_in_p->data[byte_readed]);
			byte_readed += 4;
			seg_glow_current.side = SDLNet_Read32(&udp_in_p->data[byte_readed]);
			byte_readed += 4;
			fx_net_glow(x, y);
			break;

		case G_ADD_SEG_BYTE:
			x = SDLNet_Read32(&udp_in_p->data[byte_readed]);
			byte_readed += 4;
			y = SDLNet_Read32(&udp_in_p->data[byte_readed]);
			byte_readed += 4;
			side = SDLNet_Read32(&udp_in_p->data[byte_readed]);
			byte_readed += 4;
			g_add_segment(x, y, side);
			break;

		/* EDITOR */
		case ED_ADD_SQUARE_BYTE:
			ed_net_add_square(SDLNet_Read32(&udp_in_p->data[byte_readed]),
					SDLNet_Read32(&udp_in_p->data[byte_readed + 4]));
			byte_readed += 8;
			break;
		case ED_RM_SQUARE_BYTE:
			ed_net_rm_square(SDLNet_Read32(&udp_in_p->data[byte_readed]),
					SDLNet_Read32(&udp_in_p->data[byte_readed + 4]));
			byte_readed += 8;
			break;
		
		case NET_SYNC_SQUARES:
			ed_clear_squares();
			break;
		
		default:
			return;
		}
	}
}


void srv_parse_udp_packet(client_s *cl)
{
	int byte_readed = 0;
	int packet_n;
	
	// global header
	if (udp_in_p->data[byte_readed++] != NET_GLOBAL_HEADER) return;

	switch (udp_in_p->data[byte_readed++]) {
	// game packet from client
	case NET_CL_GAME:
		packet_n = SDLNet_Read32(&udp_in_p->data[byte_readed]);
		if (!cl) return; // filter old packet on connection
		if (packet_n < cl->recev_packet_n) {
			DEBUG(printf("srv: unordered packet\n"));
			break;
		}
		byte_readed += 4;

		cl->recev_packet_ack_sent = false;
		srv_parse_game_packet(byte_readed, cl);
		cl->recev_packet_n = packet_n;
		break;

	case NET_CL_MESSAGE:
		// message
		break;
	
	// Info request
	case NET_CL_INFO:
		if (strcmp((char *)&udp_in_p->data[byte_readed], "udp_sdltest"))
			break;
		byte_readed += strlen((char *)&udp_in_p->data[byte_readed]) + 1;
		srv_send_info(byte_readed);
		break;

	// connection request
	case NET_CL_CONNECT:
		if (strcmp((char *)&udp_in_p->data[byte_readed], "udp_sdltest"))
			break;
		byte_readed += strlen((char *)&udp_in_p->data[byte_readed]) + 1;
		srv_new_client(byte_readed);
		break;
	}
}


void* srv_udp_listen(void *is_a_thread)
{
	client_s *checked_client;
	int numready = 0;
	Uint32 sent_tick = 0;
	Uint32 current_tick;


	while (net_is_server) {
		if ((numready = SDLNet_CheckSockets(udp_socket_set, PACKET_SEND_RATE))) {

			/* ====================================== */
			/* Uncomment this to simulate packet loss */
// 			if (net_test_packet_loss()) continue;
			/* ====================================== */

			if (numready == -1) {
				printf("srv: SDLNet_CheckSockets: %s\n", SDLNet_GetError());
				/* Most of the time this is a system error, where perror 
				   might help you. */
				perror("srv: SDLNet_CheckSockets");
				continue;
			}
			
			/* Check all sockets with SDLNet_SocketReady and handle 
			   the active ones. */
			checked_client = client;
			while (checked_client) {
				if (SDLNet_SocketReady(checked_client->udp_socket)) {
					udp_in_p->data[udp_in_p->len] = 0;
					if (SDLNet_UDP_Recv(checked_client->udp_socket, 
							udp_in_p) > 0) {
						srv_parse_udp_packet(checked_client);
						--numready;
					}
				}
				if (numready) { // packets in the queue
					checked_client = checked_client->next;
				} else {
					break; // dont check other socket
				}
			}
			if (numready) { // still packets in the queue
				// not a packet from any known client, ckeck main socket
				if (SDLNet_UDP_Recv(main_udp_socket, udp_in_p))
					srv_parse_udp_packet(NULL);
			}

			// dont exceed "PACKET_SEND_RATE"
			current_tick = SDL_GetTicks();
			if (sent_tick <= current_tick - PACKET_SEND_RATE) {
				srv_send_game_packet();
				sent_tick = current_tick;
			}
		} else { 
			// "PACKET_SEND_RATE" timeout
			sent_tick = SDL_GetTicks();
			srv_send_game_packet();
		}
	}
	pthread_exit(NULL);
	return NULL; // GCC complain on Windows if we dont return anything
}


#define GAME_PACKET_HEADER_LEN 2
void srv_send_game_packet()
{	
	int byte_writed = 0;
	client_s **cl = &client;	
	static Uint32 packet_n = 0;
	unack_packet_s **tmp_unack_packet;

	// write id header
	udp_out_p->data[byte_writed++] = NET_GLOBAL_HEADER;
	// write game header	
	udp_out_p->data[byte_writed++] = NET_SRV_GAME;

	++packet_n;
	
	if (!(*cl)) {
		udp_new_buffer_writed = 0;
		return;
	}
	
	while (*cl) {
		if (((*cl)->recev_packet_ack_sent) && 
				(!(*cl)->new_packet_buffer_size) &&
				(!(*cl)->unack_packet_head->active) &&
				(!udp_new_buffer_writed) &&
				((SDL_GetTicks() - (*cl)->last_packet_tick) < NET_PING_RATE)) {
			cl = &(*cl)->next;
			continue;
		}
		
		byte_writed = GAME_PACKET_HEADER_LEN;
		SDLNet_Write32(packet_n, &udp_out_p->data[byte_writed]);
		byte_writed += 4;

		if (!(*cl)->recev_packet_ack_sent) {
			udp_out_p->data[byte_writed++] = PACKET_ACK_BYTE;
			SDLNet_Write32((*cl)->recev_packet_n, &udp_out_p->data[byte_writed]);
			byte_writed += 4;
			(*cl)->recev_packet_ack_sent = true;
		}

		// resend unack packet
		if ((*cl)->unack_packet_head->active) {
			tmp_unack_packet = &(*cl)->unack_packet_head;
			while ((*tmp_unack_packet)->active) {
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
					DEBUG(printf("srv: unack packet overflow! (send)\n"));
					break;
				}
			}
		}

		if ((*cl)->new_packet_buffer_size) {
			pthread_mutex_lock(&(*cl)->new_packet_buffer_mutex);
			memcpy(&udp_out_p->data[byte_writed], (*cl)->new_packet_buffer,
					(*cl)->new_packet_buffer_size);
			byte_writed += (*cl)->new_packet_buffer_size;
			(*cl)->new_packet_buffer_size = 0;
			pthread_mutex_unlock(&(*cl)->new_packet_buffer_mutex);
		}

		if (udp_new_buffer_writed) {
			pthread_mutex_lock(&udp_new_buffer_mutex);
			memcpy(&udp_out_p->data[byte_writed], udp_new_buffer, 
					udp_new_buffer_writed);
			byte_writed += udp_new_buffer_writed;

			// copy packet to resend it if non-acked
			memcpy(&(*cl)->unack_packet_next->data,
					udp_new_buffer, udp_new_buffer_writed);
			(*cl)->unack_packet_next->active = true;
			(*cl)->unack_packet_next->len = udp_new_buffer_writed;
			(*cl)->unack_packet_next->packet_n = packet_n;
			if ((*cl)->unack_packet_next->next == NULL) {
				DEBUG(printf("srv: unack packet overflow! (save)\n"));
			} else {
				(*cl)->unack_packet_next = (*cl)->unack_packet_next->next;
			}
				udp_new_buffer_writed = 0;
				pthread_mutex_unlock(&udp_new_buffer_mutex);
		}

		if ((SDL_GetTicks() - (*cl)->last_packet_tick) > NET_PING_RATE) {
			if ((SDL_GetTicks() - (*cl)->last_packet_tick) > NET_PING_TIMEOUT) {
				ui_new_message("%s timeout", (*cl)->username);
				srv_rm_client(*cl);
				continue;
			}
			udp_out_p->data[byte_writed++] = NET_PING;
		}

		// terminate packet
		udp_out_p->data[byte_writed++] = NET_NULL;
		
		udp_out_p->len = byte_writed;
		udp_out_p->address = (*cl)->ip;
		if (!SDLNet_UDP_Send((*cl)->udp_socket, -1, udp_out_p))
			fprintf(stderr, "srv: SDLNet_UDP_Send: %s\n",
					SDLNet_GetError());

		cl = &(*cl)->next;
	}
}


void srv_rm_acked_packet(Uint32 packet_n, client_s *cl)
{
	if (!cl)
		return;

	while (cl->unack_packet_head->active && 
			cl->unack_packet_head->packet_n <= packet_n) {
		if (cl->unack_packet_next == cl->unack_packet_head)
			cl->unack_packet_next = cl->unack_packet_head->next;

		cl->unack_packet_head->active = false;
		cl->unack_packet_tail->next = cl->unack_packet_head;
		cl->unack_packet_head = cl->unack_packet_head->next;
		cl->unack_packet_tail = cl->unack_packet_tail->next;
		cl->unack_packet_tail->next = NULL;
	}
}

void srv_sync_player_name()
{
	for (int i = 0; i < srv.nplayer; i++) {
		net_write_string(NET_SYNC_PLAYER_NAME, "%d %s", i, player[i].name);
	}
}


void srv_send_pong(client_s *cl)
{
	net_write_int(NET_PONG, 0);
}


void srv_update_last_packet_tick(client_s *cl)
{
	cl->last_packet_tick = SDL_GetTicks();
}




