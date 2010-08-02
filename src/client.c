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

#include "ui.h"


int lanclient_wait_host_response()
{
	UDPsocket sd;       /* Socket descriptor */
	UDPpacket *p;       /* Pointer to packet memory */
 
	/* Open a socket */
	if (!(sd = SDLNet_UDP_Open(7788)))
	{
		fprintf(stderr, "SDLNet_UDP_Open: %s\n", SDLNet_GetError());
		printf("lanclient_wait_host_response()\n");
		return 1;
	}
 
	/* Make space for the packet */
	if (!(p = SDLNet_AllocPacket(512)))
	{
		fprintf(stderr, "SDLNet_AllocPacket: %s\n", SDLNet_GetError());
		exit(EXIT_FAILURE);
	}
 
	for (int i = 0; i < 100; i++)
	{	
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
 		
			printf("%s\n", "client");

			/* Quit if packet contains "quit" */
			if (strcmp((char *)p->data, "54321/c_square/lan") == 0) {
				printf("host responded!\n");
				ui_new_message("host responded");
				break;
			}
		}	
		SDL_Delay(10);
	}

	SDLNet_FreePacket(p);
	SDLNet_UDP_Close(sd);

	return 0;
}


int lanclient_search_host()
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
		return 1;
	}
	
	out_p->address.host = out_ip.host;	/* Set the destination host */
	out_p->address.port = out_ip.port;	/* And destination port */

	out_p->len = strlen((char *)out_p->data) + 1;
	SDLNet_UDP_Send(out_udpsd, -1, out_p); /* This sets the p->channel */

	printf("send to: %s\n", out_tmp_adressip);

	lanclient_wait_host_response();

	SDLNet_FreePacket(out_p);
	SDLNet_UDP_Close(out_udpsd);

	return 0;
}



