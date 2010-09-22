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
	int byte_writed = 0;
	client_s *tmp_cl;
	
	buffer[byte_writed++] = NET_GLOBAL_HEADER;
	
	if (net_is_server) {
		buffer[byte_writed++] = NET_SRV_DISCONNECT;
		buffer[byte_writed++] = NET_NULL;
		
		tmp_cl = client;
		while (tmp_cl)	{
			if (SDLNet_TCP_Send(tmp_cl->tcp_socket, (void *)buffer, byte_writed) < byte_writed)
				fprintf(stderr, "SDLNet_TCP_Send: %s\n", SDLNet_GetError());
			tmp_cl = tmp_cl->next;
		}

	} else if (net_is_client) {
		buffer[byte_writed++] = NET_CL_DISCONNECT;
		buffer[byte_writed++] = NET_NULL;

		if (SDLNet_TCP_Send(main_tcp_socket, (void *)buffer, byte_writed) < byte_writed)
			fprintf(stderr, "SDLNet_TCP_Send: %s\n", SDLNet_GetError());
	} else {
		// wasn't connected
		return;
	}
	
	local_player.connected = false;
	net_is_server = false;
	net_is_client = false;
	net_game = false;
	
	if (active_window == &host_window) active_window = NULL;
	if (active_window == &client_window) active_window = NULL;

}


void net_write_int(byte id_byte, int count, ...)
{
	int arg;
	int tmp_writed;
	va_list ap;
	va_start(ap, count);
	arg = va_arg(ap, int);

	// prevent overflow
	while (42) {
		tmp_writed = udp_new_buffer_writed;
		if (tmp_writed + 1 + (4 * count) < PACKET_LENGHT - 1) {
			break;
		} else {
			SDL_Delay(100);
		}
	}

	pthread_mutex_lock(&udp_new_buffer_mutex);

	udp_new_buffer[udp_new_buffer_writed++] = id_byte;
	for (int i = 0; i < count; arg = va_arg(ap, int), ++i) {
		SDLNet_Write32(arg, &udp_new_buffer[udp_new_buffer_writed]);
		udp_new_buffer_writed += 4;
	}

	pthread_mutex_unlock(&udp_new_buffer_mutex);
	DEBUG(printf("new packet buffer writed = %d\n", udp_new_buffer_writed));
}


void net_write_sync_square()
{
	int i = 0;
	
	net_write_int(NET_SYNC_SQUARES, 0);

	for (; i < ed_grid_h; i++) {
		for (int j = 0; j < ed_grid_w; j++) {
			if (squares[i][j].active) {
				net_write_int(ED_ADD_SQUARE_BYTE, 2, j, i);
			}
		}
	}

}



