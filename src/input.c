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
// g_input.c - 

#include "input.h"

#include "ui.h"


void get_input()
{
	SDL_Event event;
		// loop through waiting messages and process them
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{	
				// closing the window exit the program
				case SDL_MOUSEBUTTONDOWN:
					switch(event.button.button) 
					{
						case SDL_BUTTON_LEFT:
							input.mouse_button_left = TRUE;
							return;
						break;

						case SDL_BUTTON_RIGHT:
							input.mouse_button_right = TRUE;
						break;

						case SDL_BUTTON_MIDDLE:
							input.mouse_button_middle = TRUE;
						break;
					}
				break;
				
				case SDL_MOUSEMOTION:
					input.mouse_x = event.motion.x;
					input.mouse_y = event.motion.y;
				break;
				
				case SDL_MOUSEBUTTONUP:
					switch(event.button.button) 
					{
						case SDL_BUTTON_LEFT:
							if (ui_pressed_button) {
								ui_button_function();
							}
							input.mouse_button_left = FALSE;
						break;

						case SDL_BUTTON_RIGHT:
							input.mouse_button_right = FALSE;
						break;

						case SDL_BUTTON_MIDDLE:
							input.mouse_button_middle = FALSE;
						break;
					}
				break;
				
				case SDL_QUIT:
					gamestate = QUIT;
				break;
						
				default:
				break;
			}	
		break;
	}
}


