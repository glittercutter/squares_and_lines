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
// editor.h - 

#ifndef __EDITOR_H__
#define __EDITOR_H__

#include "shared.h"


int ed_grid_w;
int ed_grid_h;
int ed_square_size;
int ed_start_x;
int ed_start_y;


void ed_change_state(void);
void editor_main(void);
void ed_init_ui(void);
int ed_init(void);
void ed_net_add_square(int x, int y);
void ed_net_rm_square(int x, int y);


#endif
