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
#include "net.h"
#include "ui.h"


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
	int turn;
	int player_n;
	char name[32];
	bool connected;
	Uint32 ack_packet_n;
	Uint32 recev_packet_n;
	bool recev_packet_ack_sent;
	unack_packet_s *unack_packet_mem;
	unack_packet_s *unack_packet_head;
	unack_packet_s *unack_packet_tail;
	unack_packet_s *unack_packet_next;
} local_player_s;
local_player_s local_player;

string_list_t host_list;


void cl_ui_button_open_window(void);
void cl_ui_button_close_window(void);
void cl_ui_init(void);

void cl_parse_srv_info(int byte_readed, UDPpacket *p);
void cl_request_connection(void);
void cl_request_lan_server(void);
int cl_init(void);
void cl_close(void);


#endif
