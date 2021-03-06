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
// input.h - 

#ifndef __INPUT_H__
#define __INPUT_H__

#include "shared.h"


typedef struct input_s {
	int mouse_x;
	int mouse_y;
	bool mouse_button_left;
	bool mouse_button_middle;
	bool mouse_button_right;
} input_s;
input_s input;


void get_input(void);


#endif
