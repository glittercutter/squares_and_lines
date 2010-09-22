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
#define PACKET_SEND_RATE 40 // milliseconds

#define NET_GLOBAL_HEADER 0x2a
#define NET_SYNC_SQUARES 0x2b
#define NET_ACCEPT 0xa0
#define NET_REFUSE 0xb0
#define NET_NULL 0x00

#define NET_SRV_DISCONNECT 0x89
#define NET_SRV_GAME 0x84
#define NET_SRV_MESSAGE 0x83
#define NET_SRV_INFO 0x82
#define NET_SRV_CONNECT 0x81

#define NET_CL_DISCONNECT 0x09
#define NET_CL_GAME 0x04
#define NET_CL_MESSAGE 0x03
#define NET_CL_INFO 0x02
#define NET_CL_CONNECT 0x01


#define PACKET_ACK_BYTE 0x11
#define RESENT_BYTE 0x12
#define STATE_CHANGE_BYTE 0x21
#define ED_ADD_SQUARE_BYTE 0x51
#define ED_RM_SQUARE_BYTE 0x52
#define G_SEG_GLOW_BYTE 0x71
#define G_ADD_SEG_BYTE 0x72
#define G_PLAYER_TURN_BYTE 0x73

#define MAX_PORT 65535
#define BROADCAST_ADDRESS "255.255.255.255"
// #define BROADCAST_ADDRESS "modemcable003.44-200-24.mc.videotron.ca"
#define BROADCAST_PORT 2091
#define HEADER_SIZE 2


//
// net.c
//
typedef struct unack_packet_s {
	bool active;
	Uint32 packet_n;
	int len;
	byte data[PACKET_LENGHT];
	struct unack_packet_s *next;
} unack_packet_s;

bool net_is_server;
bool net_is_client;
bool net_game;

UDPpacket *udp_out_p;
UDPpacket *udp_in_p;
pthread_t udp_listen_th;
pthread_t tcp_listen_th;
TCPsocket main_tcp_socket;
UDPsocket main_udp_socket;
IPaddress main_tcp_ip;
IPaddress main_udp_ip;
SDLNet_SocketSet tcp_socket_set;
SDLNet_SocketSet udp_socket_set;

byte udp_new_buffer[PACKET_LENGHT];
int udp_new_buffer_writed;
pthread_mutex_t udp_new_buffer_mutex;


int net_test_packet_loss(void);
void net_write_int(byte id_byte, int count, ...);
void net_write_sync_square(void);
void disconnect(void);


//
// net_z.c
//
int zdeflate(char *source, unsigned char *out, int *size);
int zinflate(char *source, char *out, int size);


#endif
