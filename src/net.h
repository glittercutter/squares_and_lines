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

#define PACKET_LENGHT 512
#define UNACK_PACKET_STORAGE_SIZE 60

// net.c
#define ED_ADD_SQUARE_BYTE 0x51
#define ED_RM_SQUARE_BYTE 0x52
#define PACKET_ACK_BYTE 0x11
#define RESENT_BYTE 0x12


typedef struct unack_packet_s {
	bool active;
	Uint32 packet_n;
	int len;
	byte data[PACKET_LENGHT];
	struct unack_packet_s *next;
} unack_packet_s;


int net_game;
UDPpacket *udp_out_p;
UDPpacket *udp_in_p;
byte udp_new_buffer[PACKET_LENGHT];
int udp_new_buffer_writed;
pthread_mutex_t udp_new_buffer_mutex;

void cl_request_connection(void);
void request_local_srv(void);
int net_init_server(void);
int net_init_client(void);
void net_write_vector(byte id_byte, int x, int y);

// net_z.c
int zdeflate(char *source, unsigned char *out, int *size);
int zinflate(char *source, char *out, int size);


#endif
