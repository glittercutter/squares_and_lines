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
// server.h.h -

#ifndef __SERVER_H_
#define __SERVER_H_

#include "shared.h"
#include "net.h"
#include "ui.h"


typedef struct srv_s {
	short nplayer, max_nplayer;
	int id, last_ack_packet;
	char name[32];
} srv_s;
srv_s srv;


typedef struct client_s {
	IPaddress ip;
	IPaddress *tcp_ip;
	TCPsocket tcp_socket;
	UDPsocket udp_socket;

	char username[32];
	int player_n;
	
	bool connected;
	
	string_list_s *list;
	Uint32 ack_packet_n;
	Uint32 recev_packet_n;
	bool recev_packet_ack_sent;
	
	pthread_mutex_t new_packet_buffer_mutex;
	int new_packet_buffer_size;
	byte new_packet_buffer[PACKET_LENGHT];
	
	unack_packet_s *unack_packet_mem;
	unack_packet_s *unack_packet_head;
	unack_packet_s *unack_packet_tail;
	unack_packet_s *unack_packet_next;
	
	struct client_s *next;
} client_s;
client_s *client;


string_list_t client_list;


void lanhost_start_host(void);
void sv_init_ui(void);


#endif
