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
// client.h -

#ifndef __CLIENT_H__
#define __CLIENT_H__

#include "shared.h"
#include "ui.h"



#define PACKET_SEND_RATE 2


typedef struct srv_list_s {
	int id;
	IPaddress address;
	char name[32];
	char ping[8];
	char player[16];
	string_list_s *list;
	struct srv_list_s *next;
} srv_list_s;
srv_list_s *srv_list;


typedef struct local_player_s {
	int id;
	char name[32];
} local_player_s;
local_player_s local_player;



// variable
pthread_t client_thread;
int cl_thread_active;
int lan_search_host;
string_list_t host_list;

// function
void lanclient_start_client(void);
void cl_init_ui(void);
void cl_add_lan_srv(int byte_readed, UDPpacket *p);

#endif
