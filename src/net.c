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
	if (net_is_server) srv_close();
	else if (net_is_client) cl_close();
	else return; // wasn't connected 
	
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


void net_write_string(byte id_byte, char* fmt, ...)
{	
	int tmp_writed;
	va_list ap;
	char buf[512];
	va_start(ap, fmt);
	vsnprintf(buf, sizeof buf, fmt, ap);
	va_end(ap);
	
	int buflen = strlen(buf);

	// prevent overflow
	while (42) {
		tmp_writed = udp_new_buffer_writed;
		if (tmp_writed + 1 + buflen < PACKET_LENGHT - 1) {
			break;
		} else {
			SDL_Delay(100);
		}
	}

	pthread_mutex_lock(&udp_new_buffer_mutex);

	udp_new_buffer[udp_new_buffer_writed++] = id_byte;
	strcpy((char*)&udp_new_buffer[udp_new_buffer_writed], buf);
	udp_new_buffer_writed += buflen + 1;

	pthread_mutex_unlock(&udp_new_buffer_mutex);
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


int net_read_32(int *start_byte)
{
	int readed = SDLNet_Read32(&udp_in_p->data[*start_byte]);
	*start_byte += 4;
	return readed;
}


short net_read_16(int *start_byte)
{
	short readed = SDLNet_Read16(&udp_in_p->data[*start_byte]);
	*start_byte += 2;
	return readed;
}


