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
// server.h.h -

#ifndef __SERVER_H_
#define __SERVER_H_

#include "shared.h"
#include "ui.h"


// variable
pthread_t server_thread;
int sv_thread_active;
int lan_wait_client;
string_list_t peer_list;


// function
void lanhost_start_host(void);
void sv_init_ui(void);


#endif
