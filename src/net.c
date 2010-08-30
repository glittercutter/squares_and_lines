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
#include "server.h"


#define INPUT_BUFFER_LENGTH 30
#define MAX_PORT 65535
#define BROADCAST_ADDRESS "255.255.255.255"
// #define BROADCAST_ADDRESS "modemcable003.44-200-24.mc.videotron.ca"
#define BROADCAST_PORT 2091
#define HEADER_SIZE 2

static bool connected = false;
static bool master_server = false;

static int server_port = BROADCAST_PORT;

static char username[20]; // username of the local user

static TCPsocket main_tcp_socket;
static UDPsocket main_udp_socket;
static IPaddress host_udp_ip;
static IPaddress host_tcp_ip;

static SDLNet_SocketSet tcp_socket_set;
static SDLNet_SocketSet udp_socket_set;
/* Maximum number of connection in master server socket set */
static int msv_max_connection = 50;

static pthread_t udp_listen_th;
static pthread_t tcp_listen_th;


void msv_send_message(client_s*, char*);
void cl_send_message(char*);
void message_printf(char*, ...);
void* udp_listen_main(void *is_a_thread);
void* tcp_listen_main(void *is_a_thread);


int net_init()
{
	static bool inited = false;
	if (inited) return 0;

	IPaddress server_tcp_ip;
	int tmp_port = server_port;
	int rc = 0;

	if (master_server) {
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
			return 1;
		}
		tcp_socket_set = SDLNet_AllocSocketSet(msv_max_connection);
		udp_socket_set = SDLNet_AllocSocketSet(msv_max_connection);
		SDLNet_UDP_AddSocket(udp_socket_set, main_udp_socket);

		srv.max_nplayer = 2;
		srv.nplayer = 1;

	} else { /* We are a client */
		while (42) {
			/* UDP - Open a socket on avalable port*/
			if ((main_udp_socket = SDLNet_UDP_Open(++tmp_port)))
				break; // sucess
			if (server_port == MAX_PORT) { // checked all ports
				fprintf(stderr, "SDLNet_UDP_Open: %s\n", SDLNet_GetError());
				SDLNet_UDP_Close(main_udp_socket);
				printf("Can't open any port !\n");
				return 1;
			}
		}
		tcp_socket_set = SDLNet_AllocSocketSet(1);
		udp_socket_set = SDLNet_AllocSocketSet(1);
		SDLNet_UDP_AddSocket(udp_socket_set, main_udp_socket);
	}

	rc = pthread_create(&udp_listen_th, NULL, udp_listen_main, (void*)rc);
	pthread_detach(udp_listen_th);
	rc = pthread_create(&tcp_listen_th, NULL, tcp_listen_main, (void*)rc);
	pthread_detach(tcp_listen_th);

	inited = true;
	return 0;
}


// TODO better
void close_master_server()
{
	master_server = false;
	connected = false;
	SDL_Delay(2000);
}


/* 
====================
msv_accept_request

Open a new UDP socket for the client.
Accept client request by replying with a UDP packet 
and wait for the client to initialise the TCP connection
====================
*/
int msv_accept_request(client_s *cl)
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
			break;	// opened a port

		if (port == MAX_PORT) { // cant open any port
			fprintf(stderr, "SDLNet_UDP_Open: %s\n", SDLNet_GetError());
			fprintf(stderr, "Can't open any port !\n");
			return 1;
		}
		++port;
	}
	printf("Opened UDP port %d for client\n", port);

	/* Write packet */
	// write connection request bytes
	udp_out_p->data[byte_writed++] = 0x81;
	udp_out_p->data[byte_writed++] = 0x00;
	// Application ID
	strcpy((char *)&udp_out_p->data[byte_writed], "udp_sdltest");
	byte_writed += strlen((char *)&udp_out_p->data[byte_writed]);
	strcat((char *)&udp_out_p->data[byte_writed], "\0"); // terminate the string
	byte_writed += 1; // count null character
	// Client username
	strcpy((char *)&udp_out_p->data[byte_writed], cl->username);
	byte_writed += strlen((char *)&udp_out_p->data[byte_writed]);
	strcat((char *)&udp_out_p->data[byte_writed], "\0");
	byte_writed += 1;
	// Port number we opened for the client
	SDLNet_Write16(port, &udp_out_p->data[byte_writed]);
	byte_writed += sizeof(short) + 1;

	udp_out_p->address.host = cl->ip.host;
	udp_out_p->address.port = cl->ip.port;
	udp_out_p->len = byte_writed;

	while (i_sent < MAX_ACK_PACKET) {
		if (!i_check) {
			SDLNet_UDP_Send(main_udp_socket, -1, udp_out_p); /* This sets the p->channel */
			printf("Ack sent to %s.\n", cl->username);
			++i_sent;
			i_check = MAX_ACK_WAIT;
		}
		SDL_Delay(10);

		if ((cl->tcp_socket = SDLNet_TCP_Accept(main_tcp_socket))) {
			/* Get the remote address */
			if ((cl->tcp_ip = SDLNet_TCP_GetPeerAddress(cl->tcp_socket))) {
				SDLNet_TCP_AddSocket(tcp_socket_set, cl->tcp_socket);
				SDLNet_UDP_AddSocket(udp_socket_set, cl->udp_socket);
				cl->connected = true;
				printf("Connected.\n");
				return 0;
			}
		}
		--i_check;
	}
	return 1; // error
}


void msv_new_client(int byte_readed)
{
	int i = 0;
	char i_string[10];
	char tmp_username[20];
	client_s **tmp_client = &client;
	client_s *new_client;

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
		
	if (!msv_accept_request(new_client)) {
		tmp_client = &client;
		// add client to the end of the list
		while (*tmp_client) {
			tmp_client = &(*tmp_client)->next;
		}
		*tmp_client = new_client;
	} else {
		printf("Error connecting to client\n");
		SDLNet_UDP_Close(new_client->udp_socket);
		free(new_client);
	}
}


void msv_rm_client(client_s *cl)
{	
	client_s *tmp_cl = client;
	printf("%s: disconnected\n", cl->username);

	SDLNet_TCP_DelSocket(tcp_socket_set, cl->tcp_socket);
	SDLNet_UDP_DelSocket(udp_socket_set, cl->udp_socket);	
	SDLNet_TCP_Close(cl->tcp_socket);
	SDLNet_UDP_Close(cl->udp_socket);
	
	while (tmp_cl) {
		if (tmp_cl->next == cl) {
			// replace with the next node
			tmp_cl->next = cl->next;
			break;
		}
		tmp_cl = tmp_cl->next;
	}
	free(cl);
}


void cl_make_connection(int byte_readed)
{
	if (connected) return;

	// read username (in case the host changed it)
	strncpy(username, (char *)&udp_in_p->data[byte_readed], sizeof(username));
	byte_readed += strlen((char *)&udp_out_p->data[byte_readed]) + 1;

	// Read the port number the host opened for UDP communication
	// The port number is already in network byte order
	memcpy(&host_udp_ip.port, &udp_in_p->data[byte_readed], sizeof(short));
	byte_readed += sizeof(short);
	host_udp_ip.host = udp_in_p->address.host; // use the address we received from
	
	host_tcp_ip.host = udp_in_p->address.host;
	SDLNet_Write16(BROADCAST_PORT, &host_tcp_ip.port);
	
	if (!(main_tcp_socket = SDLNet_TCP_Open(&host_tcp_ip))) {
		fprintf(stderr, "SDLNet_TCP_Open: %s\n", SDLNet_GetError());
		SDLNet_TCP_Close(main_tcp_socket);
		return;
	}
	SDLNet_TCP_AddSocket(tcp_socket_set, main_tcp_socket);

	printf("Connected\n");
	printf("Username: %s\n", username);
	connected = true;
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
	
	int byte_writed = 0; // last byte writed position
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
	strcat((char *)&udp_out_p->data[byte_writed], "\0"); // terminate the string
	byte_writed += 1; // count null character

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
	
	printf("Waiting host response.\n");
}


void msv_send_info(int byte_readed)
{
	int byte_writed = 0;

	/* Write packet */
	// write connection request bytes
	udp_out_p->data[byte_writed++] = 0x82;
	udp_out_p->data[byte_writed++] = 0x00;
	
	// Program name
	strcpy((char *)&udp_out_p->data[byte_writed], "udp_sdltest");
	byte_writed += strlen((char *)&udp_out_p->data[byte_writed]);
	strcat((char *)&udp_out_p->data[byte_writed], "\0"); // terminate the string
	byte_writed += 1; // count null character
	
	// Server name
	if (!srv.name[0])
		strncpy(srv.name, "default name", sizeof(srv.name));
	strcpy((char *)&udp_out_p->data[byte_writed], srv.name);
	byte_writed += strlen((char *)&udp_out_p->data[byte_writed]);
	strcat((char *)&udp_out_p->data[byte_writed], "\0");
	byte_writed += 1;
	
	printf("id is at byte: %d\n", byte_writed);	

	// Server ID
	SDLNet_Write32(srv.id, &udp_out_p->data[byte_writed]);
	byte_writed += 4;

	// Current/Max players
	SDLNet_Write16(srv.nplayer, &udp_out_p->data[byte_writed]);
	byte_writed += 2;
	SDLNet_Write16(srv.max_nplayer, &udp_out_p->data[byte_writed]);
	byte_writed += 2;

	// Tick for ping
	memcpy(&udp_out_p->data[byte_writed], &udp_in_p->data[byte_readed], 4);
	printf("recv tick: %u\n", SDLNet_Read32(&udp_in_p->data[byte_readed]));
	byte_writed += 4;

	udp_out_p->address.host = udp_in_p->address.host;
	udp_out_p->address.port = udp_in_p->address.port;
	udp_out_p->len = byte_writed;
	SDLNet_UDP_Send(main_udp_socket, -1, udp_out_p);
}


int msv_parse_udp_packet()
{
	int byte_readed = 0;
	printf("receive UDP packet\n");
	switch (udp_in_p->data[byte_readed++]) {
	case 0x03:
		if ((udp_in_p->data[byte_readed++]) != 0x00) break;
		printf("\nMessage from UDP connection\n");
// 		message_printf("%s\n", (char *)&udp_in_p->data[byte_readed]);
		return 0;
	
	// Info request
	case 0x02:
		if ((udp_in_p->data[byte_readed++]) != 0x00) break;
		if (strcmp((char *)&udp_in_p->data[byte_readed], "udp_sdltest")) break;
		byte_readed += strlen((char *)&udp_in_p->data[byte_readed]) + 1;
		msv_send_info(byte_readed);
		return 0;

	case 0x01:
		if ((udp_in_p->data[byte_readed++]) != 0x00) break;
		if (strcmp((char *)&udp_in_p->data[byte_readed], "udp_sdltest")) break;
		byte_readed += strlen((char *)&udp_in_p->data[byte_readed]) + 1;
		msv_new_client(byte_readed);
		return 0;
	}
	// no header recognised
	return 1;
}

void cl_parse_udp_packet()
{	
	int byte_readed = 0;

	switch (udp_in_p->data[byte_readed++]) {
	// message from server
	case 0x83:
		if ((udp_in_p->data[byte_readed++]) != 0x00) 
			break;
		printf("\nMessage from UDP connection\n");
// 		message_printf("%s\n", (char *)&udp_in_p->data[byte_readed]);
		break;

	case 0x82:
		if ((udp_in_p->data[byte_readed++]) != 0x00)
			break;
		if (strcmp((char *)&udp_in_p->data[byte_readed], "udp_sdltest"))
			break;
		printf("byte_readed before: %d\n", byte_readed);
		byte_readed += strlen((char *)&udp_in_p->data[byte_readed]) + 1;	
		printf("byte_readed after: %d\n", byte_readed);
		cl_add_lan_srv(byte_readed, udp_in_p);
		break;
	
	// server accepted connection
	case 0x81:
		if ((udp_in_p->data[byte_readed++]) != 0x00) break;	
		if (strcmp((char *)&udp_in_p->data[byte_readed], "udp_sdltest"))
			break;
		printf("client: connection accepted\n");
		byte_readed += strlen((char *)&udp_in_p->data[byte_readed]) + 1;
		cl_make_connection(byte_readed);
		break;
	}
}


void msv_udp_listen()
{
	client_s *checked_client;
	int numready = 0;

	while (master_server) {
		numready = SDLNet_CheckSockets(udp_socket_set, 1000);
		if (numready == -1) {
			printf("SDLNet_CheckSockets: %s\n", SDLNet_GetError());
			//most of the time this is a system error, where perror might help you.
			perror("SDLNet_CheckSockets");
		}
		
		if (numready) {
			// check all sockets with SDLNet_SocketReady and handle the active ones.
			checked_client = client;
			while (checked_client) {
				if (SDLNet_SocketReady(checked_client->udp_socket)) {
					udp_in_p->data[udp_in_p->len] = 0;
					if (SDLNet_UDP_Recv(checked_client->udp_socket, udp_in_p) >
							0) {
						if (msv_parse_udp_packet(checked_client)) {
							// not a packet destined to the server
							cl_parse_udp_packet(checked_client);
						}
						--numready;
					}
				}

				if (numready) { 
					checked_client = checked_client->next;
				} else {
					// dont check other socket
					break;
				}
			}

			if (numready) {
				// not a packet from any known client, ckeck main socket
				if (SDLNet_UDP_Recv(main_udp_socket, udp_in_p)) {		
					// null terminate data to the received length
					msv_parse_udp_packet();
				}
			}
		}
	}

}


void cl_udp_listen()
{
	while (!master_server) {
		if (SDLNet_CheckSockets(udp_socket_set, 1000)) {
			if (SDLNet_UDP_Recv(main_udp_socket, udp_in_p)) {		
				// null terminate data to the received length
				udp_in_p->data[udp_in_p->len] = 0;
				printf("client: received UDP packet\n");	
				cl_parse_udp_packet();
			}
		}
	}

}


void* udp_listen_main(void *is_a_thread)
{
	if (master_server) {
		msv_udp_listen();
	} else {
		cl_udp_listen();
	}

	pthread_exit(NULL);
}


#define MESSAGE_HEADER_LEN 2
void msv_parse_tcp_packet(client_s *cl, byte *buffer)
{
	switch (buffer[0]) {
	// chat message from client "0300"
	case 0x03:
		if (buffer[1] != 0x00) break;
// 		message_printf("%s: %s\n", cl->username, (char *)&buffer[MESSAGE_HEADER_LEN]);
		msv_send_message(cl, (char *)&buffer[MESSAGE_HEADER_LEN]);
		break;
	
	// client is disconnecting "0000"
	case 0x00:
		if (buffer[1] != 0x00) break;
		msv_rm_client(cl);
		break;

	}

}


void cl_parse_tcp_packet(byte *buffer)
{
	switch (buffer[0]) {
	// chat message from server "8300"
	case 0x83:
		if (buffer[1] != 0x00) break;
// 		message_printf("%s\n", (char *)&buffer[MESSAGE_HEADER_LEN]);
		break;
	
	// server is disconnecting "8000"
	case 0x80:
		if (buffer[1] != 0x00) break;
		printf("Server disconnected.\n");
		connected = false;
		break;

	}

}


void msv_tcp_listen()
{
	int numready;
	client_s *checked_client;
	byte buffer[512];

	while (master_server) {
		numready = SDLNet_CheckSockets(tcp_socket_set, 1000);
		if (numready) {
			printf("received TCP packet!\n");
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
							msv_parse_tcp_packet(checked_client, buffer);
							--numready;
						} else { 
							/* client probably disconnected */
							msv_rm_client(checked_client);
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

}

void cl_tcp_listen()
{
	byte buffer[512];

	while (!master_server) {
		if (SDLNet_CheckSockets(tcp_socket_set, 1000)) {
			printf("received TCP packet!\n");
			if (SDLNet_TCP_Recv(main_tcp_socket, buffer, 512) >
					0) {
				cl_parse_tcp_packet(buffer);

			} else { 
				/* server probably disconnected */
				connected = false;
				SDLNet_TCP_DelSocket(tcp_socket_set, main_tcp_socket);
				printf("Server disconnected\n");
				return;
			}
			memset(buffer, 0, sizeof(buffer)); // NEEDED?
		}
	}

}



/* 
====================
tcp_listen_main

Maun function for the TCP thread.
Listen directly if we are the server, else
wait until the client is connected.
====================
*/
void* tcp_listen_main(void *is_a_thread)
{
	while (42) {
		if (master_server) {
			msv_tcp_listen();
		} else {
			if (connected) {
				cl_tcp_listen();
			}
		}
		SDL_Delay(10);
	}

	pthread_exit(NULL);
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

		udp_out_p->address = host_udp_ip;
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

	if (master_server) {
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
	
	connected = false;
	master_server = false;
}


void request_local_srv()
{
	int byte_writed = 0; // last byte writed position
	IPaddress udp_ip;

	if (net_init()) {
		// TODO handle error
	}
	
	// write server request info bytes
	udp_out_p->data[byte_writed++] = 0x02;
	udp_out_p->data[byte_writed++] = 0x00;
	
	strcpy((char *)&udp_out_p->data[byte_writed], "udp_sdltest");
	byte_writed += strlen((char *)&udp_out_p->data[byte_writed]);
	strcat((char *)&udp_out_p->data[byte_writed], "\0"); // terminate the string
	byte_writed += 1; // count null character

	// Ping
	SDLNet_Write32(SDL_GetTicks(), &udp_out_p->data[byte_writed]);
	printf("tick send: %u\n", SDLNet_Read32(&udp_out_p->data[byte_writed]));
	byte_writed += 4;


	if (SDLNet_ResolveHost(&udp_ip, BROADCAST_ADDRESS, BROADCAST_PORT) == -1) {
		fprintf(stderr, "SDLNet_ResolveHost(%s:%d) %s\n", BROADCAST_ADDRESS,
				BROADCAST_PORT, SDLNet_GetError());
	}
	
	udp_out_p->address = udp_ip;
	udp_out_p->len = byte_writed;
	SDLNet_UDP_Send(main_udp_socket, -1, udp_out_p);
}


int start_srv()
{
	master_server = true;
	if (net_init()) {
		return 1;
	}

	// Generate ID only once
	if (!srv.id)
		srv.id = get_random_number(1000000);
	printf("srv id: %d\n", srv.id);	

	return 0;
}


