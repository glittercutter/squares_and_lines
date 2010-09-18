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


int net_test_packet_loss()
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



