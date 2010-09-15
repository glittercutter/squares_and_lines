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
// net.c - 

#include "net.h"

#include "client.h"
#include "common.h"
#include "editor.h"
#include "game.h"
#include "fx.h"
#include "server.h"


#define INPUT_BUFFER_LENGTH 30
#define MAX_PORT 65535
#define BROADCAST_ADDRESS "255.255.255.255"
// #define BROADCAST_ADDRESS "modemcable003.44-200-24.mc.videotron.ca"
#define BROADCAST_PORT 2091
#define HEADER_SIZE 2


static TCPsocket main_tcp_socket;
static UDPsocket main_udp_socket;
static IPaddress main_tcp_ip;
static IPaddress main_udp_ip;

static SDLNet_SocketSet tcp_socket_set;
static SDLNet_SocketSet udp_socket_set;
/* Maximum number of connection in master server socket set */
static int srv_max_connection = 50;

static pthread_t udp_listen_th;
static pthread_t tcp_listen_th;

void msv_send_message(client_s*, char*);
void cl_send_message(char*);
void message_printf(char*, ...);
void* srv_udp_listen(void *is_a_thread);
void* srv_tcp_listen(void *is_a_thread);
void* cl_udp_listen(void *is_a_thread);
void* cl_tcp_listen(void *is_a_thread);
void cl_send_game_packet(void);
void srv_send_game_packet(void);
void close_server(void);
void close_client(void);
void srv_clear_client(void);
void srv_rm_acked_packet(Uint32 packet_n, client_s *cl);
void cl_rm_acked_packet(Uint32 packet_n);


int packet_loss_sim()
{
	static const int loss_rate = 1;
	static int i = 0;
	
	if (i++ == loss_rate) {
		i = 0;
		printf("packet losted!\n");
		return 1;
	}

	return 0;
}


int net_init_server()
{
	IPaddress server_tcp_ip;
	int server_port = BROADCAST_PORT;
	int rc = 0;
	unack_packet_s **tmp_unack_packet;

	if (net_is_server) return 0;
	if (net_is_client) close_client();
	
	// Generate ID
	if (!srv.id)
		srv.id = get_random_number(1000000);
	printf("srv id: %d\n", srv.id);	

	/* UDP - Open a socket on a known port */
	if (!(main_udp_socket = SDLNet_UDP_Open(server_port))) {
		fprintf(stderr, "SDLNet_UDP_Open: %s\n", SDLNet_GetError());
		fprintf(stderr, "Master server can't open port: %d.\n", server_port);
		SDLNet_UDP_Close(main_udp_socket);
		return 1;
	}
	/* TCP - Resolving the host using NULL make network interface to
	listen. Open on the same known port */
	if (SDLNet_ResolveHost(&server_tcp_ip, NULL, server_port) < 0) {
		fprintf(stderr, "SDLNet_ResolveHost: %s\n", SDLNet_GetError());
		SDLNet_UDP_Close(main_udp_socket);
		return 1;
	}
	/*  TCP - Open a connection with client */
	if (!(main_tcp_socket = SDLNet_TCP_Open(&server_tcp_ip))) {
		fprintf(stderr, "SDLNet_TCP_Open: %s\n", SDLNet_GetError());
		SDLNet_TCP_Close(main_tcp_socket);
		SDLNet_UDP_Close(main_udp_socket);
		return 1;
	}
	tcp_socket_set = SDLNet_AllocSocketSet(srv_max_connection);
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
	local_player.player_n = srv.nplayer;
	local_player.turn = srv.nplayer;

	net_is_server = true;
	net_game = true;

	rc = pthread_create(&udp_listen_th, NULL, srv_udp_listen, (void*)rc);
	pthread_detach(udp_listen_th);
	rc = pthread_create(&tcp_listen_th, NULL, srv_tcp_listen, (void*)rc);
	pthread_detach(tcp_listen_th);

	return 0;
}

int net_init_client()
{
	int tmp_port = BROADCAST_PORT;
	int rc = 0;
	unack_packet_s **tmp_unack_packet;
	
	if (net_is_client) return 0;
	if (net_is_server) close_server();
	
	while (42) {
		/* UDP - Open a socket on avalable port*/
		if ((main_udp_socket = SDLNet_UDP_Open(++tmp_port)))
			break; // sucess
		if (tmp_port == MAX_PORT) { // checked all ports
			fprintf(stderr, "SDLNet_UDP_Open: %s\n", SDLNet_GetError());
			SDLNet_UDP_Close(main_udp_socket);
			printf("Can't open any port !\n");
			return 1;
		}
	}
	tcp_socket_set = SDLNet_AllocSocketSet(1);
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
	rc = pthread_create(&tcp_listen_th, NULL, cl_tcp_listen, (void*)rc);
	pthread_detach(tcp_listen_th);

	return 0;
}


void close_server()
{
	if (!net_is_server) return;

	net_is_server = false;
	net_game = false;
	local_player.connected = false;
	SDL_Delay(500);

	srv_clear_client();
	SDLNet_FreeSocketSet(tcp_socket_set);
	SDLNet_FreeSocketSet(udp_socket_set);
	SDLNet_UDP_Close(main_udp_socket);
	SDLNet_TCP_Close(main_tcp_socket);
	free(local_player.unack_packet_mem);
}


void close_client()
{
	if (!net_is_client) return;

	net_is_client = false;
	net_game = false;
	SDL_Delay(500);

	SDLNet_FreeSocketSet(tcp_socket_set);
	SDLNet_FreeSocketSet(udp_socket_set);
	
	SDLNet_UDP_Close(main_udp_socket);	
	if (local_player.connected)
		SDLNet_TCP_Close(main_tcp_socket);
	
	local_player.connected = false;
	free(local_player.unack_packet_mem);
}

/* 
====================
srv_accept_request

Open a new UDP socket for the client.
Accept client request by replying with a UDP packet 
and wait for the client to initialise the TCP connection
====================
*/
int srv_accept_request(client_s *cl)
{	
#define MAX_ACK_PACKET 5
#define MAX_ACK_WAIT 10

	int port = BROADCAST_PORT + 1;
	int i_check = 0; // no of connection check for each packet sent
	int i_sent = 0; // no of ack packet sent
	int byte_writed = 0; // next write position
	
	// open a UDP socket for client
	while (42) {	
		if ((cl->udp_socket = SDLNet_UDP_Open(port)))
			break;// succes

		if (port == MAX_PORT) { // cant open any port
			fprintf(stderr, "SDLNet_UDP_Open: %s\n", SDLNet_GetError());
			fprintf(stderr, "Can't open any port !\n");
			return 1;
		}
		++port;
	}
	printf("Opened UDP port %d for client\n", port);

	/* Write packet */
	// write connection accepted bytes
	udp_out_p->data[byte_writed++] = 0x81;
	udp_out_p->data[byte_writed++] = 0x00;
	// Application ID
	strcpy((char *)&udp_out_p->data[byte_writed], "udp_sdltest");
	byte_writed += strlen((char *)&udp_out_p->data[byte_writed]);
	strcat((char *)&udp_out_p->data[byte_writed], "\0");
	byte_writed += 1;
	// Client username
	strcpy((char *)&udp_out_p->data[byte_writed], cl->username);
	byte_writed += strlen((char *)&udp_out_p->data[byte_writed]);
	strcat((char *)&udp_out_p->data[byte_writed], "\0");
	byte_writed += 1;
	// Port number we opened for the client
	SDLNet_Write16(port, &udp_out_p->data[byte_writed]);
	byte_writed += 2;
	// player no.
	SDLNet_Write32(cl->player_n, &udp_out_p->data[byte_writed]);
	byte_writed += 4;
	
	udp_out_p->data[byte_writed++] = 0x00;

	udp_out_p->address = cl->ip;
	udp_out_p->len = byte_writed;

	while (i_sent < MAX_ACK_PACKET) {
		if (!i_check) {
			if (!SDLNet_UDP_Send(main_udp_socket, -1, udp_out_p)) 
				fprintf(stderr, "error: SDLNet_UDP_Send: %s\n",
						SDLNet_GetError());
			printf("Ack sent to %s.\n", cl->username);
			++i_sent;
			i_check = MAX_ACK_WAIT;
		}
		SDL_Delay(10);

		if (!(cl->tcp_socket = SDLNet_TCP_Accept(main_tcp_socket))) {
			--i_check;
			continue;
		}
		if ((cl->tcp_ip = SDLNet_TCP_GetPeerAddress(cl->tcp_socket))) {
			SDLNet_TCP_AddSocket(tcp_socket_set, cl->tcp_socket);
			SDLNet_UDP_AddSocket(udp_socket_set, cl->udp_socket);
			cl->connected = true;
			printf("Connected.\n");	
			return 0;
		}
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
	// write connection refused bytes
	udp_out_p->data[byte_writed++] = 0x81;
	udp_out_p->data[byte_writed++] = 0x01;
	// Application ID
	strcpy((char *)&udp_out_p->data[byte_writed], "udp_sdltest");
	byte_writed += strlen((char *)&udp_out_p->data[byte_writed]);
	strcat((char *)&udp_out_p->data[byte_writed], "\0");
	byte_writed += 1;
	// refused reason string
	strcpy((char *)&udp_out_p->data[byte_writed], reason);
	byte_writed += strlen((char *)&udp_out_p->data[byte_writed]);
	strcat((char *)&udp_out_p->data[byte_writed], "\0");
	byte_writed += 1;

	udp_out_p->address = udp_in_p->address;
	udp_out_p->len = byte_writed;

	SDLNet_UDP_Send(main_udp_socket, -1, udp_out_p);
}


void srv_new_client(int byte_readed)
{
	int i = 0;
	char i_string[10];
	char tmp_username[20];
	client_s **tmp_client = &client;
	client_s *new_client;
	unack_packet_s **tmp_unack_packet;

	if (srv.nplayer >= srv.max_nplayer) {
		srv_refuse_request(text.srv_full);
		return;
	}

	// get username from the packet
	strncpy(tmp_username, (char *)&udp_in_p->data[byte_readed],
			sizeof(tmp_username) - 1);
	// check if the username is already in the list, if so, it is numbered
	while (*tmp_client) {
		if (!strcmp((*tmp_client)->username, tmp_username)) {
			++i;
			snprintf(i_string, sizeof(i_string), "[%d]", i);
			strncpy(tmp_username, (char *)&udp_in_p->data[byte_readed],
					sizeof(tmp_username) - strlen(i_string));
			strncat(tmp_username, i_string, sizeof(tmp_username) - 1);
			tmp_client = &client; // restart the search from the first element
		
		} else {
			tmp_client = &(*tmp_client)->next;
		}
	}
	printf("Connection request\n");
	printf("Username: %s\n", tmp_username);

	new_client = malloc(sizeof(client_s));
	new_client->next = NULL;
	new_client->connected = false;
	new_client->ip = udp_in_p->address;
	strncpy(new_client->username, tmp_username, sizeof(new_client->username));
	new_client->recev_packet_n = 0;
	new_client->recev_packet_ack_sent = false;
	new_client->new_packet_buffer_size = 0;
	new_client->player_n = srv.nplayer + 1;

	if (srv_accept_request(new_client)) {
		printf("Error connecting to client\n");
		SDLNet_UDP_Close(new_client->udp_socket);
		free(new_client);
		return;
	}

	tmp_client = &client;
	// add client to the end of the list
	while (*tmp_client) {
		tmp_client = &(*tmp_client)->next;
	}
	*tmp_client = new_client;

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

	if (pthread_mutex_init(&new_client->new_packet_buffer_mutex, NULL))
		eprint("pthread_mutex_init\n");
	
	++srv.nplayer;
	printf("player no:%d\n", new_client->player_n);
}


void srv_rm_client(client_s *cl)
{	
	client_s **tmp_cl = &client;
	printf("%s: disconnected\n", cl->username);

	SDLNet_TCP_DelSocket(tcp_socket_set, cl->tcp_socket);
	SDLNet_UDP_DelSocket(udp_socket_set, cl->udp_socket);	
	SDLNet_TCP_Close(cl->tcp_socket);
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
		SDLNet_TCP_DelSocket(tcp_socket_set, cl->tcp_socket);
		SDLNet_UDP_DelSocket(udp_socket_set, cl->udp_socket);	
		SDLNet_TCP_Close(cl->tcp_socket);
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


void cl_make_connection(int byte_readed)
{
	if (local_player.connected) return;

	// read username (in case the host changed it)
	strncpy(local_player.name, (char *)&udp_in_p->data[byte_readed],
			sizeof(local_player.name));
	byte_readed += strlen((char *)&udp_out_p->data[byte_readed]) + 1;

	// Read the port number the host opened for UDP communication
	// The port number is already in network byte order
	memcpy(&main_udp_ip.port, &udp_in_p->data[byte_readed], sizeof(short));
	byte_readed += sizeof(short);
	// use the address we received the packet from
	main_udp_ip.host = udp_in_p->address.host;

	// TCP
	main_tcp_ip.host = udp_in_p->address.host;
	SDLNet_Write16(BROADCAST_PORT, &main_tcp_ip.port);
	if (!(main_tcp_socket = SDLNet_TCP_Open(&main_tcp_ip))) {
		fprintf(stderr, "SDLNet_TCP_Open: %s\n", SDLNet_GetError());
		SDLNet_TCP_Close(main_tcp_socket);
		return;
	}
	SDLNet_TCP_AddSocket(tcp_socket_set, main_tcp_socket);
	
	// read player number the server gave us
	local_player.player_n = SDLNet_Read32(&udp_in_p->data[byte_readed]);
	byte_readed += 4;

	printf("Connected\n");
	printf("Username: %s\n", local_player.name);
	local_player.connected = true;
	cl_button_close_window();
	ui_new_message(text.cl_connected);
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
		printf("no server selected\n");
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
			printf("found selected\n");
			break;
		}
		tmp_srv_list = &(*tmp_srv_list)->next;
	}

	// write connection request bytes
	udp_out_p->data[byte_writed++] = 0x01;
	udp_out_p->data[byte_writed++] = 0x00;
	
	strcpy((char *)&udp_out_p->data[byte_writed], "udp_sdltest");
	 /* set position for next write */
	byte_writed += strlen((char *)&udp_out_p->data[byte_writed]);
	strcat((char *)&udp_out_p->data[byte_writed], "\0");
	byte_writed += 1;

	if (!local_player.name[0]) strncpy(local_player.name, "noname",
			sizeof(local_player.name));
	strcpy((char *)&udp_out_p->data[byte_writed], local_player.name);
	byte_writed += strlen((char *)&udp_out_p->data[byte_writed]);
	strcat((char *)&udp_out_p->data[byte_writed], "\0");
	byte_writed += 1;

	udp_out_p->address = (*tmp_srv_list)->address;
	udp_out_p->len = byte_writed;

	printf("ip: %x\n", SDLNet_Read32(&udp_out_p->address.host));
	printf("port: %x\n", SDLNet_Read16(&udp_out_p->address.port));

	SDLNet_UDP_Send(main_udp_socket, -1, udp_out_p);
	
	printf("Waiting host response\n");
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
	// write info answer bytes
	udp_out_p->data[byte_writed++] = 0x82;
	udp_out_p->data[byte_writed++] = 0x00;	
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
	// Tick for ping latency
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

	while (udp_in_p->data[byte_readed] != 0x00) {
		printf("byte_readed: %d\n", byte_readed);
		switch (udp_in_p->data[byte_readed++]) {
		case RESENT_BYTE:
			if (SDLNet_Read32(&udp_in_p->data[byte_readed]) <= 
					cl->recev_packet_n) {
				byte_readed += 4;
				/* packet was already received,
				// skip to the end of the packet */
				packet_size = SDLNet_Read32(&udp_in_p->data[byte_readed]);
				byte_readed += packet_size;
				printf("resent packet size: %d\n", packet_size);
				byte_readed += 4;
				printf("packet received already\n");
				break;
			}
			printf("resent packet\n");
			byte_readed += 8; // (packet number and lenght)
			break;

		/* GAME */
		case G_SEG_GLOW_BYTE:
			x = SDLNet_Read32(&udp_in_p->data[byte_readed]);
			byte_readed += 4;
			y = SDLNet_Read32(&udp_in_p->data[byte_readed]);
			byte_readed += 4;
			seg_glow_current.pos = SDLNet_Read32(&udp_in_p->data[byte_readed]);
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


		case PACKET_ACK_BYTE:
			srv_rm_acked_packet(SDLNet_Read32(&udp_in_p->data[byte_readed]), 
					cl);
			byte_readed += 4;
			// don't send ack if the received packet contain only an ack
			if (udp_in_p->data[byte_readed] == 0x00) {
				cl->recev_packet_ack_sent = true;
			}
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

		default:
			printf("packet parsing: header not recognized\n");
			return;
		}
	}
}

void cl_parse_game_packet(int byte_readed)
{
	int packet_size;
	int state;
	int x, y;
	int side;
	
	while (udp_in_p->data[byte_readed] != 0x00) {
		switch (udp_in_p->data[byte_readed++]) {
		case RESENT_BYTE:
			if (SDLNet_Read32(&udp_in_p->data[byte_readed]) <= 
					local_player.recev_packet_n) {
				byte_readed += 4;
				/* packet was already received,
				// skip to the end of the packet */
				packet_size = SDLNet_Read32(&udp_in_p->data[byte_readed]);
				byte_readed += packet_size;
				printf("resent packet size: %d\n", packet_size);
				byte_readed += 4;
				printf("packet received already\n");
				break;
			}
			printf("resent packet\n");
			byte_readed += 8; // (packet number and lenght)
			break;

		case PACKET_ACK_BYTE:
			cl_rm_acked_packet(SDLNet_Read32(&udp_in_p->data[byte_readed]));
			byte_readed += 4;
			// don't send ack if the received packet contain only an ack
			if (udp_in_p->data[byte_readed] == 0x00) {
				local_player.recev_packet_ack_sent = true;
			}
			break;
		
		/* GAME */
		case G_SEG_GLOW_BYTE:
			x = SDLNet_Read32(&udp_in_p->data[byte_readed]);
			byte_readed += 4;
			y = SDLNet_Read32(&udp_in_p->data[byte_readed]);
			byte_readed += 4;
			seg_glow_current.pos = SDLNet_Read32(&udp_in_p->data[byte_readed]);
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

		case G_PLAYER_TURN_BYTE:
			local_player.turn = SDLNet_Read32(&udp_in_p->data[byte_readed]);
			byte_readed += 4;
			printf("player no:%d, player turn:%d\n",local_player.player_n, local_player.turn);
			fx_new_transition(NULL, 5, FX_PLAYER_CHANGE);
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

		case STATE_CHANGE_BYTE:
			state = SDLNet_Read32(&udp_in_p->data[byte_readed]);
			byte_readed += 4;
			switch (state) {
			case GAME:
				g_change_state();
				break;
			case EDITOR:
				ed_change_state();
				break;
			}
			break;

		default:
			printf("packet parsing: header not recognized\n");
			return;
		}
	}
}

int srv_parse_udp_packet(client_s *cl)
{
	int byte_readed = 0;
	int packet_n;
	
	switch (udp_in_p->data[byte_readed++]) {
	// game packet from client
	case 0x04:
		if ((udp_in_p->data[byte_readed++]) != 0x00) break;
		packet_n = SDLNet_Read32(&udp_in_p->data[byte_readed]);
		if (packet_n < cl->recev_packet_n) {
			printf("unordered packet\n");
			return 0;
		}
		byte_readed += 4;

// if (packet_loss_sim()) break;

		cl->recev_packet_ack_sent = false;
		srv_parse_game_packet(byte_readed, cl);
		cl->recev_packet_n = packet_n;
		return 0;

	case 0x03:
		if ((udp_in_p->data[byte_readed++]) != 0x00) break;
		printf("\nMessage from UDP connection\n");
// 		message_printf("%s\n", (char *)&udp_in_p->data[byte_readed]);
		return 0;
	
	// Info request
	case 0x02:
		if ((udp_in_p->data[byte_readed++]) != 0x00) break;
		if (strcmp((char *)&udp_in_p->data[byte_readed], "udp_sdltest"))
			break;
		byte_readed += strlen((char *)&udp_in_p->data[byte_readed]) + 1;
		srv_send_info(byte_readed);
		return 0;

	// connection request
	case 0x01:
		if ((udp_in_p->data[byte_readed++]) != 0x00) break;
		if (strcmp((char *)&udp_in_p->data[byte_readed], "udp_sdltest"))
			break;
		byte_readed += strlen((char *)&udp_in_p->data[byte_readed]) + 1;
		srv_new_client(byte_readed);
		return 0;
	}
	// no header recognised
	return 1;
}

void cl_parse_udp_packet()
{	
	int byte_readed = 0;
	int packet_n;

	switch (udp_in_p->data[byte_readed++]) {
	// game packet from server
	case 0x84:
		if ((udp_in_p->data[byte_readed++]) != 0x00) break;
		packet_n = SDLNet_Read32(&udp_in_p->data[byte_readed]);
		if (packet_n < local_player.recev_packet_n) {
			printf("unordered packet\n");
			break; 
		}
		byte_readed += 4;

// if (packet_loss_sim()) break;

		local_player.recev_packet_ack_sent = false;
		cl_parse_game_packet(byte_readed);
		local_player.recev_packet_n = packet_n;
		break;

	// message from server
	case 0x83:
		if ((udp_in_p->data[byte_readed++]) != 0x00) 
			break;
		// display message
		break;
	
	// server info
	case 0x82:
		if ((udp_in_p->data[byte_readed++]) != 0x00)
			break;
		if (strcmp((char *)&udp_in_p->data[byte_readed], "udp_sdltest"))
			break;
		byte_readed += strlen((char *)&udp_in_p->data[byte_readed]) + 1;	
		cl_add_lan_srv(byte_readed, udp_in_p);
		break;
	
	// server connection request response
	case 0x81:
		if ((udp_in_p->data[byte_readed]) == 0x01) {
			++byte_readed;
			if (strcmp((char *)&udp_in_p->data[byte_readed], "udp_sdltest"))
				break;
			byte_readed += strlen((char *)&udp_in_p->data[byte_readed]) + 1;
			ui_new_message((char *)&udp_in_p->data[byte_readed]);
			break;
		}
		if ((udp_in_p->data[byte_readed]) != 0x00)
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


void* srv_udp_listen(void *is_a_thread)
{
	client_s *checked_client;
	int numready = 0;
	Uint32 sent_tick = 0;
	Uint32 current_tick;

	while (net_is_server) {
		if ((numready = SDLNet_CheckSockets(udp_socket_set, 40))) {
			if (numready == -1) {
				printf("SDLNet_CheckSockets: %s\n", SDLNet_GetError());
				/* Most of the time this is a system error, where perror 
				   might help you. */
				perror("SDLNet_CheckSockets");
			}

			/* Check all sockets with SDLNet_SocketReady and handle 
			   the active ones. */
			checked_client = client;
			while (checked_client) {
				if (SDLNet_SocketReady(checked_client->udp_socket)) {
					udp_in_p->data[udp_in_p->len] = 0;
					if (SDLNet_UDP_Recv(checked_client->udp_socket, 
							udp_in_p) > 0) {
						if (srv_parse_udp_packet(checked_client)) {
							// server parsing did not recognized the header
							cl_parse_udp_packet(checked_client);
						}
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

			// send packet only each 40 milliseconds
			current_tick = SDL_GetTicks();
			if (sent_tick <= current_tick - 40) {
				srv_send_game_packet();
				sent_tick = current_tick;
			}
		} else { 
			// nothing was received for 40 milliseconds
			sent_tick = SDL_GetTicks();
			srv_send_game_packet();
		}
	}
	printf("udp thread exited\n");
	pthread_exit(NULL);
}


void* cl_udp_listen(void *is_a_thread)
{
	Uint32 sent_tick = 0;
	Uint32 current_tick;

	while (net_is_client) {
		if (SDLNet_CheckSockets(udp_socket_set, 40)) {
			if (SDLNet_UDP_Recv(main_udp_socket, udp_in_p)) {		
				// null terminate data to the received length
				udp_in_p->data[udp_in_p->len] = 0;
				cl_parse_udp_packet();
			}
			/* Dont send game packet if one was sent less 
			   than 40 milliseconds ago */
			current_tick = SDL_GetTicks();
			if (sent_tick <= current_tick - 40) {
				cl_send_game_packet();
				sent_tick = current_tick;
			}
		} else { 
			// nothing was received in 40 milliseconds, send game packet
			sent_tick = SDL_GetTicks();
			cl_send_game_packet();
		}
	}
	printf("udp thread exited\n");
	pthread_exit(NULL);
}


void srv_parse_tcp_packet(client_s *cl, byte *buffer)
{
	switch (buffer[0]) {
	// chat message from client "0300"
	case 0x03:
		if (buffer[1] != 0x00) break;
// 		message_printf("%s: %s\n", cl->username, (char *)&buffer[HEADER_SIZE]);
		msv_send_message(cl, (char *)&buffer[HEADER_SIZE]);
		break;
	
	// client is disconnecting "0000"
	case 0x00:
		if (buffer[1] != 0x00) break;
		srv_rm_client(cl);
		break;

	default:
		break;
	}
}


void cl_parse_tcp_packet(byte *buffer)
{
	switch (buffer[0]) {
	// chat message from server "8300"
	case 0x83:
		if (buffer[1] != 0x00) break;
// 		message_printf("%s\n", (char *)&buffer[HEADER_SIZE]);
		break;
	
	// server is disconnecting "8000"
	case 0x80:
		if (buffer[1] != 0x00) break;
		printf("Server disconnected.\n");
		close_client();
		break;

	}
}


void* srv_tcp_listen(void *is_a_thread)
{
	int numready;
	client_s *checked_client;
	byte buffer[512];

	while (net_is_server) {
		numready = SDLNet_CheckSockets(tcp_socket_set, 1000);
		if (numready) {
			printf("received TCP packet\n");
		}
		if (numready == -1) {
			printf("SDLNet_CheckSockets: %s\n", SDLNet_GetError());
			//most of the time this is a system error, where perror might help you.
			perror("SDLNet_CheckSockets");

		} else if (numready && client) {
			// check all sockets with SDLNet_SocketReady and handle the active ones.
			checked_client = client;
			while (checked_client) {
				if (SDLNet_SocketReady(checked_client->tcp_socket)) {
					if (checked_client) {
						if (SDLNet_TCP_Recv(checked_client->tcp_socket, buffer, 512) >
								0) {
							srv_parse_tcp_packet(checked_client, buffer);
							--numready;
						} else { 
							/* client probably dislocal_player.connected */
							srv_rm_client(checked_client);
							--numready;
						}
					}
				}
				if (numready) { 
					checked_client = checked_client->next;
				} else {
					break; // dont check other socket
				}
			}

			if (numready) {	
				printf("Didnt expected the unexpected!\n");
			}
			memset(buffer, 0, sizeof(buffer)); // NEEDED?
		}
	}
	printf("tcp thread exited\n");
	pthread_exit(NULL);
}

void* cl_tcp_listen(void *is_a_thread)
{
	byte buffer[512];

	while (net_is_client) {
		if (SDLNet_CheckSockets(tcp_socket_set, 1000)) {
			printf("received TCP packet!\n");
			if (SDLNet_TCP_Recv(main_tcp_socket, buffer, 512) >
					0) {
				cl_parse_tcp_packet(buffer);

			} else { 
				/* server probably dislocal_player.connected */
				close_client();
				printf("Server disconnected\n");
				break;
			}
			memset(buffer, 0, sizeof(buffer)); // NEEDED?
		}
	}
	printf("tcp thread exited\n");
	pthread_exit(NULL);
}

void cl_send_game_packet(void)
{
	static Uint32 packet_n = 0;
	int byte_writed = 0;
	unack_packet_s **tmp_unack_packet;

	// check if we have something to send
	if ((local_player.recev_packet_ack_sent) && 
			(!local_player.unack_packet_head->active) &&
			(udp_new_buffer_writed == 0)) {
		return;
	}

	// write game header
	udp_out_p->data[byte_writed++] = 0x04;
	udp_out_p->data[byte_writed++] = 0x00;
	
	++packet_n;
	// write packet number
	SDLNet_Write32(packet_n, &udp_out_p->data[byte_writed]);
	byte_writed += 4;
	
	if (!local_player.recev_packet_ack_sent) {
		printf("sent ack, packet: %hd\n", local_player.recev_packet_n);
		udp_out_p->data[byte_writed++] = PACKET_ACK_BYTE;
		SDLNet_Write32(local_player.recev_packet_n, &udp_out_p->data[byte_writed]);
		byte_writed += 4;
		local_player.recev_packet_ack_sent = true;
	}

// --------------------------------
	// resend unack packet
	if (local_player.unack_packet_head->active) {
		tmp_unack_packet = &local_player.unack_packet_head;
		while ((*tmp_unack_packet)->active) {
			printf("resent no.%hd\n", (*tmp_unack_packet)->packet_n);
			if (!(*tmp_unack_packet)->data[0]) {
				printf("error: unack packet empty\n");
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
				DEBUG(printf("unack packet overflow! (send)\n"));
				break;
			}
		}
	}
// ---------------------------------END

	if (udp_new_buffer_writed != 0) {
		printf("out buffer size: %d, pos: %d\n", udp_new_buffer_writed, byte_writed);
		pthread_mutex_lock(&udp_new_buffer_mutex);
		memcpy(&udp_out_p->data[byte_writed], udp_new_buffer, 
				udp_new_buffer_writed);
		byte_writed += udp_new_buffer_writed;

// ------------------------------------
		// copy packet to resend it if non-acked
		memcpy(&local_player.unack_packet_next->data,
				udp_new_buffer, udp_new_buffer_writed);

		local_player.unack_packet_next->active = true;
		local_player.unack_packet_next->len = udp_new_buffer_writed;
		local_player.unack_packet_next->packet_n = packet_n;
		if (local_player.unack_packet_next->next == NULL) {
			DEBUG(printf("unack packet overflow! (save)\n"));
		} else {
			local_player.unack_packet_next = local_player.unack_packet_next->next;
		}
// ------------------------------------END
		
		udp_new_buffer_writed = 0;
		pthread_mutex_unlock(&udp_new_buffer_mutex);
	}
	// terminate packet
	udp_out_p->data[byte_writed++] = 0x00;

	DEBUG(printf("sent game packet, len: %d\n", byte_writed));

	udp_out_p->len = byte_writed;
	udp_out_p->address = main_udp_ip;
	if (!SDLNet_UDP_Send(main_udp_socket, -1, udp_out_p))
		fprintf(stderr, "SDLNet_UDP_Send: %s\n", SDLNet_GetError());

}

#define GAME_PACKET_HEADER_LEN 2
void srv_send_game_packet()
{	
	int byte_writed = 0;
	client_s **cl = &client;	
	static Uint32 packet_n = 0;
	unack_packet_s **tmp_unack_packet;

	// write game header	
	udp_out_p->data[byte_writed++] = 0x84;
	udp_out_p->data[byte_writed++] = 0x00;

	++packet_n;
	cl = &client;
	
	while (*cl) {
		if (((*cl)->recev_packet_ack_sent) && 
				(!(*cl)->new_packet_buffer_size) &&
				(!(*cl)->unack_packet_head->active) &&
				(!udp_new_buffer_writed)) {
			cl = &(*cl)->next;
			continue;
		}
		printf("buff_writed: %d\n", udp_new_buffer_writed);

		byte_writed = GAME_PACKET_HEADER_LEN;
		SDLNet_Write32(packet_n, &udp_out_p->data[byte_writed]);
		byte_writed += 4;

		if (!(*cl)->recev_packet_ack_sent) {
			udp_out_p->data[byte_writed++] = PACKET_ACK_BYTE;
			SDLNet_Write32((*cl)->recev_packet_n, &udp_out_p->data[byte_writed]);
			byte_writed += 4;
			printf("sent ack, packet: %hd\n", (*cl)->recev_packet_n);
			(*cl)->recev_packet_ack_sent = true;
		}

// --------------------------------
		// resend unack packet
		if ((*cl)->unack_packet_head->active) {
			tmp_unack_packet = &(*cl)->unack_packet_head;
			while ((*tmp_unack_packet)->active) {
				printf("resent no.%hd\n", (*tmp_unack_packet)->packet_n);
				if (!(*tmp_unack_packet)->data[0]) {
					printf("error: unack packet empty\n");
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
					DEBUG(printf("unack packet overflow! (send)\n"));
					break;
				}
			}
		}
// ---------------------------------END

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

// ------------------------------------
		// copy packet to resend it if non-acked
		memcpy(&(*cl)->unack_packet_next->data,
				udp_new_buffer, udp_new_buffer_writed);

		(*cl)->unack_packet_next->active = true;
		(*cl)->unack_packet_next->len = udp_new_buffer_writed;
		(*cl)->unack_packet_next->packet_n = packet_n;
		if ((*cl)->unack_packet_next->next == NULL) {
			DEBUG(printf("unack packet overflow! (save)\n"));
		} else {
			(*cl)->unack_packet_next = (*cl)->unack_packet_next->next;
		}
// ------------------------------------END

			udp_new_buffer_writed = 0;
			pthread_mutex_unlock(&udp_new_buffer_mutex);
		}
		// terminate packet
		udp_out_p->data[byte_writed++] = 0x00;
		
		DEBUG(printf("sent game packet, len: %d\n", byte_writed));

		udp_out_p->len = byte_writed;
		udp_out_p->address = (*cl)->ip;
		if (!SDLNet_UDP_Send((*cl)->udp_socket, -1, udp_out_p))
			fprintf(stderr, "SDLNet_UDP_Send: %s\n",
					SDLNet_GetError());

		cl = &(*cl)->next;
	}
}


/* 
====================
msv_send_message

Used by server to send message to all client exept the sender.
Message with the "UDP" prefix are send using the UDP connection,
otherwise with TCP.
====================
*/
void msv_send_message(client_s *from_cl, char *buffer)
{
	byte data[512];
	int len = 0;
	client_s *tmp_cl = client;
	
	// send using udp
	if (!strncmp(buffer, "UDP", 3)) {
			udp_out_p->data[0] = 0x83;
			udp_out_p->data[1] = 0x00;
			strncpy((char *)&udp_out_p->data[HEADER_SIZE], buffer, 
					sizeof(data) - HEADER_SIZE);
			udp_out_p->len = strlen(buffer) + HEADER_SIZE + 1;

		while (tmp_cl) {
			if (tmp_cl != from_cl) { // don't send to authors
				udp_out_p->address = tmp_cl->ip;
				if (SDLNet_UDP_Send(tmp_cl->udp_socket, -1, udp_out_p) < len)
					fprintf(stderr, "SDLNet_UDP_Send: %s\n", SDLNet_GetError());
			}
			tmp_cl = tmp_cl->next;
		}
	
	// send using TCP
	} else {
		
		data[0] = 0x83;
		data[1] = 0x00;
		strncpy((char *)&data[HEADER_SIZE], buffer, sizeof(data) - HEADER_SIZE);
		len = strlen((char *)&data[HEADER_SIZE]) + HEADER_SIZE + 1;
		
		while (tmp_cl) {
			if (tmp_cl != from_cl) { // don't send to authors
				if (SDLNet_TCP_Send(tmp_cl->tcp_socket, (void *)data, len) < len)
					fprintf(stderr, "SDLNet_TCP_Send: %s\n", SDLNet_GetError());
			}
			tmp_cl = tmp_cl->next;
		}
	}
}


/* 
====================
cl_send_message

Send message to the server.
Message with the "UDP" prefix are send using the UDP connection,
otherwise with TCP.
====================
*/
void cl_send_message(char *buffer)
{
	byte data[512]; // only needed for TCP
	int len = 0;

	// send using udp
	if (!strncmp(buffer, "UDP", 3)) {
		udp_out_p->data[0] = 0x03;
		udp_out_p->data[1] = 0x00;
		strncpy((char *)&udp_out_p->data[HEADER_SIZE], buffer, 
				sizeof(data) - HEADER_SIZE);
		udp_out_p->len = strlen(buffer) + HEADER_SIZE + 1;

		udp_out_p->address = main_udp_ip;
		printf("sending UDP packet.\naddress: %d, port %hd.\n", 
				udp_out_p->address.host, SDLNet_Read16(&udp_out_p->address.port));
		if (SDLNet_UDP_Send(main_udp_socket, -1, udp_out_p) < len)
			fprintf(stderr, "SDLNet_UDP_Send: %s\n", SDLNet_GetError());

	} else {
		data[0] = 0x03;
		data[1] = 0x00;
		strncpy((char *)&data[HEADER_SIZE], buffer, sizeof(data) - HEADER_SIZE);
		len = strlen((char *)&data[HEADER_SIZE]) + HEADER_SIZE;

		if (SDLNet_TCP_Send(main_tcp_socket, (void *)data, len) < len)
			fprintf(stderr, "SDLNet_TCP_Send: %s\n", SDLNet_GetError());
	}
}


/* 
====================
disconnect

Used both the client end server.
The server send a disconnect packet to all client "8000".
Client send it to the server "0000".
====================
*/
void disconnect()
{
	byte buffer[HEADER_SIZE];
	int len = HEADER_SIZE;
	client_s *tmp_cl = client;

	if (net_is_server) {
		buffer[0] = 0x80;
		buffer[1] = 0x00;

		while (tmp_cl)	{
			if (SDLNet_TCP_Send(tmp_cl->tcp_socket, (void *)buffer, len) < len)
				fprintf(stderr, "SDLNet_TCP_Send: %s\n", SDLNet_GetError());
			tmp_cl = tmp_cl->next;
		}

	} else {
		buffer[0] = 0x00;
		buffer[1] = 0x00;

		if (SDLNet_TCP_Send(main_tcp_socket, (void *)buffer, len) < len)
			fprintf(stderr, "SDLNet_TCP_Send: %s\n", SDLNet_GetError());
	}
	
	local_player.connected = false;
	net_is_server = false;
}


void request_local_srv()
{
	int byte_writed = 0;
	IPaddress udp_ip;
	
	// write server request info bytes
	udp_out_p->data[byte_writed++] = 0x02;
	udp_out_p->data[byte_writed++] = 0x00;
	// app name
	strcpy((char *)&udp_out_p->data[byte_writed], "udp_sdltest");
	byte_writed += strlen((char *)&udp_out_p->data[byte_writed]);
	strcat((char *)&udp_out_p->data[byte_writed], "\0");
	byte_writed += 1;
	// ping
	SDLNet_Write32(SDL_GetTicks(), &udp_out_p->data[byte_writed]);
	byte_writed += 4;

	if (SDLNet_ResolveHost(&udp_ip, BROADCAST_ADDRESS, BROADCAST_PORT) == -1) {
		fprintf(stderr, "SDLNet_ResolveHost(%s:%d) %s\n", BROADCAST_ADDRESS,
				BROADCAST_PORT, SDLNet_GetError());
	}
	udp_out_p->address = udp_ip;
	udp_out_p->len = byte_writed;
	SDLNet_UDP_Send(main_udp_socket, -1, udp_out_p);
}


void cl_rm_acked_packet(Uint32 packet_n)
{
	DEBUG(printf("packet acked: %hd\n", packet_n));
	
	while (local_player.unack_packet_head->active && 
			local_player.unack_packet_head->packet_n <= packet_n) {
		if (local_player.unack_packet_next == local_player.unack_packet_head)
			local_player.unack_packet_next = local_player.unack_packet_head->next;
		local_player.unack_packet_head->active = false;
		local_player.unack_packet_tail->next = local_player.unack_packet_head;
		local_player.unack_packet_head = local_player.unack_packet_head->next;
		local_player.unack_packet_tail = local_player.unack_packet_tail->next;
		local_player.unack_packet_tail->next = NULL;
	}
}

void srv_rm_acked_packet(Uint32 packet_n, client_s *cl)
{
	printf("packet acked: %hd\n", packet_n);

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


void net_write_int(byte id_byte, int count, ...)
{
	int arg;
	va_list ap;
	va_start(ap, count);
	arg = va_arg(ap, int);
	
	pthread_mutex_lock(&udp_new_buffer_mutex);

	udp_new_buffer[udp_new_buffer_writed++] = id_byte;
	for (int i = 0; i < count; arg = va_arg(ap, int), ++i) {
		SDLNet_Write32(arg, &udp_new_buffer[udp_new_buffer_writed]);
		udp_new_buffer_writed += 4;
	}

	pthread_mutex_unlock(&udp_new_buffer_mutex);
}


void net_write_sync_square()
{
	
}



