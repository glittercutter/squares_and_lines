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
// net.h -

#ifndef __NET_H__
#define __NET_H__

#include "shared.h"

// net.c
typedef struct client_s {
	IPaddress ip;
	IPaddress *tcp_ip;
	TCPsocket tcp_socket;
	UDPsocket udp_socket;
	char username[20];
	bool connected;
	struct client_s *next;
} client_s;
client_s *client;

UDPpacket *udp_out_p;
UDPpacket *udp_in_p;

void cl_request_connection(void);
void request_local_srv(void);
int start_srv(void);

// net_z.c
int zdeflate(char *source, unsigned char *out, int *size);
int zinflate(char *source, char *out, int size);


#endif
