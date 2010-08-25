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
// client.c - 

#include "client.h"

#include "common.h"
#include "ui.h"


void CL_add_lan_host(UDPpacket*);

int lanclient_wait_host_response()
{
	UDPsocket sd;       /* Socket descriptor */
	UDPpacket *p;       /* Pointer to packet memory */
	
	lan_search_host = TRUE;

	/* Open a socket */
	if (!(sd = SDLNet_UDP_Open(7788)))
	{
		fprintf(stderr, "SDLNet_UDP_Open: %s\n", SDLNet_GetError());
		printf("lanclient_wait_host_response()\n");
		SDLNet_UDP_Close(sd);
		return 1;
	}
 
	/* Make space for the packet */
	if (!(p = SDLNet_AllocPacket(512)))
	{
		fprintf(stderr, "SDLNet_AllocPacket: %s\n", SDLNet_GetError());
		exit(EXIT_FAILURE);
	}
 
	while (lan_search_host) {
		/* Wait a packet. UDP_Recv returns != 0 if a packet is coming */
		if (SDLNet_UDP_Recv(sd, p)) {
			printf("UDP Packet incoming\n");
			printf("\tChan:    %d\n", p->channel);
			printf("\tData:    %s\n", (char *)p->data);
			printf("\tLen:     %d\n", p->len);
			printf("\tMaxlen:  %d\n", p->maxlen);
			printf("\tStatus:  %d\n", p->status);
			printf("\tAddress: %x %x\n", p->address.host, p->address.port);
 		
			printf("%s\n", "client");

			/* Quit if packet contains "quit" */
			if (strcmp((char *)p->data, "54321/c_square/lan") == 0) {
				printf("host responded!\n");
				ui_new_message("host responded");
				CL_add_lan_host(p);
				break;
			}
		}	
		SDL_Delay(10);
	}

	SDLNet_FreePacket(p);
	SDLNet_UDP_Close(sd);

	return 0;
}


void* lanclient_search_host(void *is_a_thread)
{
	int net_port = 2000;
	UDPpacket *out_p;
	IPaddress out_ip;
	UDPsocket out_udpsd;
	char out_adressip[] = {"255.255.255."};
	char out_tmp_adressip[20];
	
	/* Open a socket on random port */
	if (!(out_udpsd = SDLNet_UDP_Open(0)))
	{
		fprintf(stderr, "SDLNet_UDP_Open: %s\n", SDLNet_GetError());
		printf("net_list_lan_host()\n");
		exit(EXIT_FAILURE);
	}
	
	/* Allocate memory for the packet */
	if (!(out_p = SDLNet_AllocPacket(512)))
	{
		fprintf(stderr, "SDLNet_AllocPacket: %s\n", SDLNet_GetError());
		exit(EXIT_FAILURE);
	}
	
	// fill the packet
	strcpy((char *)out_p->data, "12345/c_square/lan");
	
		
	snprintf(out_tmp_adressip, 19, "%s%d%s", out_adressip, 255, "\0");
	/* Resolve server name  */
	if (SDLNet_ResolveHost(&out_ip, out_tmp_adressip, net_port) == -1)
	{
		fprintf(stderr, "SDLNet_ResolveHost(%s %d): %s\n", out_tmp_adressip,
				net_port, SDLNet_GetError());
// 		return;
	}
	
	out_p->address.host = out_ip.host;	/* Set the destination host */
	out_p->address.port = out_ip.port;	/* And destination port */

	out_p->len = strlen((char *)out_p->data) + 1;
	SDLNet_UDP_Send(out_udpsd, -1, out_p); /* This sets the p->channel */

	printf("request server info at: %s\n", out_tmp_adressip);

	lanclient_wait_host_response();

	SDLNet_FreePacket(out_p);
	SDLNet_UDP_Close(out_udpsd);

	cl_thread_active = FALSE;

	pthread_exit(NULL);

}


void lanclient_start_client()
{
	int rc = 0;
	
	if (lan_search_host) {
		lan_search_host = FALSE;
		while (42) {
			if (cl_thread_active) {
				SDL_Delay(5);
			} else break;
		}
	}

	active_window = &client_window;

	rc = pthread_create(&client_thread, NULL, lanclient_search_host, (void*)rc);
	pthread_detach(client_thread);
	if (rc) {
		fprintf(stderr, "ERROR; return code from pthread_create() is %d\n", rc);
		exit(-1);
	}

}


void cl_button_close_window() 
{	
	lan_search_host = FALSE;
	ui_button_close_window();
}


void cl_init_ui()
{
	int x, y;
	int min_w, max_w;
	int h = button_font.size + UI_BAR_PADDING;
	int w;
	
	client_window.w = display_width * 0.7f;
	client_window.h = display_height * 0.7f;
	client_window.x1 = (display_width - client_window.w) / 2;
	client_window.y1 = ((display_height - button_topbar->h) - client_window.h) / 2 + button_topbar->h;
	client_window.x2 = client_window.x1 + client_window.w;
	client_window.y2 = client_window.y1 + client_window.h;
	
	// window title
	x = client_window.x1; y = client_window.y1;
	min_w = client_window.w - (UI_BAR_PADDING * 2); max_w = min_w;
	w = strlen(text.join_game) * button_font.w;
	ui_new_button(x, y, w, h, min_w, max_w, ALIGN_LEFT,
			text.join_game, *ui_button_drag_window, 1, 1, &client_window.button);
	// close window
	min_w = 1;
	w = strlen("x") * button_font.w + UI_BAR_PADDING;
	x = client_window.x2 - w - (UI_BAR_PADDING * 2); y = client_window.y1;
	ui_new_button(x, y, w, h, min_w, max_w, ALIGN_CENTER,
			"x", *cl_button_close_window, 1, 1, &client_window.close_button);
	
	x = client_window.x1 + (UI_BAR_PADDING * 2);
	y = client_window.y1 + (h * 2);

	ui_new_widget_list_box(client_window.x1 + (UI_BAR_PADDING * 2), 
			client_window.y1 + (h * 2) + (UI_BAR_PADDING * 2), client_window.x2 - (UI_BAR_PADDING * 2), 
			client_window.y2 - (UI_BAR_PADDING * 2), &host_list, 
			&client_window.widget);

	// game name
	host_list.col_position[0] = 0;
	// ping
	host_list.col_position[1] = (host_list.list_box->w - SCROLLBAR_SIZE) * 0.5f;
	// player
	host_list.col_position[2] = (host_list.list_box->w - SCROLLBAR_SIZE) * 0.75f;

	// collum name buttons
	w = strlen(text.lbox_server) * button_font.w + UI_BAR_PADDING;
	min_w = host_list.col_position[1] - host_list.col_position[0];
	ui_new_button(host_list.col_position[0] + host_list.list_box->x1, host_list.list_box->y1 - button_font.h, w, h,
			min_w, max_w, ALIGN_LEFT, text.lbox_server, NULL, 1, 1, &host_list.col_name);

	w = strlen(text.lbox_ping) * button_font.w + UI_BAR_PADDING;
	min_w = host_list.col_position[2] - host_list.col_position[1];
	ui_new_button(host_list.col_position[1] + host_list.list_box->x1, host_list.list_box->y1 - button_font.h, w, h,
			min_w, max_w, ALIGN_LEFT, text.lbox_ping, NULL, 1, 1, &host_list.col_name);

	w = strlen(text.lbox_player) * button_font.w + UI_BAR_PADDING;
	min_w = host_list.list_box->w - (SCROLLBAR_SIZE * 1.6f) - host_list.col_position[2];
	ui_new_button(host_list.col_position[2] + host_list.list_box->x1, host_list.list_box->y1 - button_font.h, w, h,
			min_w, max_w, ALIGN_LEFT, text.lbox_player, NULL, 1, 1, &host_list.col_name);

	host_list.list = NULL;

}


void CL_add_lan_host(UDPpacket *p)
{
// 	char ip[13];
// 	char host_info[STRING_LENGTH];
	char ip[] = {"12321"};
	char host_info[] = {"info"};
	
	static int count = 0;
	count++;
	char count_str[10];
	sprintf(count_str, "%d%c", count, '\0');

	com_add_string_node(&host_list, count_str, ip, host_info, NULL);
	ui_scrollbar_update_size(strlist_len(host_list.list), 
			host_list.max_element, &host_list.list_box->scrollbar);

// 	printf("element: %d\n", 
}



