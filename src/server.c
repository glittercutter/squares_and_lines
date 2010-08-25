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
// server.c - 

#include "server.h"
#include "ui.h"


int lanhost_answer_client(IPaddress *client_ip)
{	
//  	Uint16 net_port = SDLNet_Write16(7788);
	UDPpacket *out_p;
	UDPsocket out_udpsd;
	
	/* Open a socket on random port */
	if (!(out_udpsd = SDLNet_UDP_Open(0)))
	{
		fprintf(stderr, "SDLNet_UDP_Open: %s\n", SDLNet_GetError());
		printf("lanhost_answer_client()\n");
		SDLNet_UDP_Close(out_udpsd);
		return 1;
	}
	
	/* Allocate memory for the packet */
	if (!(out_p = SDLNet_AllocPacket(512)))
	{
		fprintf(stderr, "SDLNet_AllocPacket: %s\n", SDLNet_GetError());
		exit(EXIT_FAILURE);
	}
	
	// fill the packet
	strcpy((char *)out_p->data, "54321/c_square/lan");
		
	out_p->address.host = client_ip->host;	/* Set the destination host */
	SDLNet_Write16(7788, &out_p->address.port); /* And destination port */

	out_p->len = strlen((char *)out_p->data) + 1;
	SDLNet_UDP_Send(out_udpsd, -1, out_p); /* This sets the p->channel */

// 	lanclient_wait_host_response();

	SDLNet_FreePacket(out_p);
	SDLNet_UDP_Close(out_udpsd);
	return 0;
}


void* lanhost_wait_client(void *is_a_thread)
{	
	sv_thread_active = TRUE;
	
	UDPsocket sd;       /* Socket descriptor */
	UDPpacket *p;       /* Pointer to packet memory */
	
	lan_wait_client = TRUE;
	
	/* Open a socket */
	if (!(sd = SDLNet_UDP_Open(2000)))
	{
		fprintf(stderr, "SDLNet_UDP_Open: %s\n", SDLNet_GetError());
		SDLNet_UDP_Close(sd);
		sv_thread_active = FALSE;
		pthread_exit(NULL);
	}
 
	/* Make space for the packet */
	if (!(p = SDLNet_AllocPacket(512)))
	{
		fprintf(stderr, "SDLNet_AllocPacket: %s\n", SDLNet_GetError());
		exit(EXIT_FAILURE);
	}

	/* Main loop */
	while (lan_wait_client)
	{
		SDL_Delay(10);
		/* Wait a packet. UDP_Recv returns != 0 if a packet is coming */
		if (SDLNet_UDP_Recv(sd, p))
		{
			printf("UDP Packet incoming\n");
			printf("\tChan:    %d\n", p->channel);
			printf("\tData:    %s\n", (char *)p->data);
			printf("\tLen:     %d\n", p->len);
			printf("\tMaxlen:  %d\n", p->maxlen);
			printf("\tStatus:  %d\n", p->status);
			printf("\tAddress: %x %x\n", p->address.host, p->address.port);
 
			/* Quit if packet contains "quit" */
			if (strcmp((char *)p->data, "12345/c_square/lan") == 0) {
				printf("received request\n");
				lanhost_answer_client(&p->address);
			}
		}		
	}

	SDLNet_FreePacket(p);
	SDLNet_UDP_Close(sd);

	sv_thread_active = FALSE;

	pthread_exit(NULL);
}

void lanhost_start_host()
{
	int rc = 0;
	
	if (lan_wait_client) {
		lan_wait_client = FALSE;
		while (42) {
			if (sv_thread_active) {
				SDL_Delay(10);
			} else break;
		}
	}
	active_window = &host_window;

	rc = pthread_create(&server_thread, NULL, lanhost_wait_client, (void*)rc);
	pthread_detach(server_thread);
	if (rc) {
		fprintf(stderr, "ERROR; return code from pthread_create() is %d\n", rc);
		exit(-1);
	}

}


void sv_button_close_window() 
{
	lan_wait_client = FALSE;
	ui_button_close_window();
}


void sv_init_ui()
{
	int x, y;
	int min_w, max_w;
	int h = button_font.size + UI_BAR_PADDING;
	int w;
	
	host_window.w = display_width * 0.7f;
	host_window.h = display_height * 0.7f;
	host_window.x1 = (display_width - host_window.w) / 2;
	host_window.y1 = ((display_height - button_topbar->h) - host_window.h) / 2 + button_topbar->h;
	host_window.x2 = host_window.x1 + host_window.w;
	host_window.y2 = host_window.y1 + host_window.h;
	
	// window title
	x = host_window.x1; y = host_window.y1;
	min_w = host_window.w - (UI_BAR_PADDING * 2); max_w = min_w;
	w = strlen(text.host_game) * button_font.w;
	ui_new_button(x, y, w, h, min_w, max_w, ALIGN_LEFT,
			text.host_game, *ui_button_drag_window, 1, 1, &host_window.button);
	// close window
	min_w = 1;
	w = strlen("x") * button_font.w + UI_BAR_PADDING;
	x = host_window.x2 - w - (UI_BAR_PADDING * 2); y = host_window.y1;
	ui_new_button(x, y, w, h, min_w, max_w, ALIGN_CENTER,
			"x", *sv_button_close_window, 1, 1, &host_window.close_button);
	
	x = host_window.x1 + (UI_BAR_PADDING * 2);
	y = host_window.y1 + (h * 2);

	ui_new_widget_list_box(host_window.x1 + (UI_BAR_PADDING * 2), 
			host_window.y1 + (h * 2) + (UI_BAR_PADDING * 2), host_window.x2 - (UI_BAR_PADDING * 2), 
			host_window.y2 - (UI_BAR_PADDING * 2), &peer_list, 
			&host_window.widget);
	
	peer_list.list = NULL;


}


// void sv_add_client(client_s *client)
// {
// 	if (!client) return;
// 
// 	client_t new_client*
// 	new_client = malloc(sizeof(client_t));
// 	
// 	
// 
// 	if (!*list_node) {
// 		*client_node = client
// 	while (!*client_node)
// }



