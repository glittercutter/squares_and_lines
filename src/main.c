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
// main.c - 

#include "main.h"

#include "common.h"
#include "editor.h"
#include "draw.h"
#include "fx.h"
#include "game.h"
#include "init.h"
#include "input.h"
#include "menu.h"
#include "parse_public.h"
#include "ui.h"


int main(int argc, char **argv)
{
	if (init()) {
		return 1;
	}

	while (gamestate != QUIT) {

		get_input();
		menu_main();
		fx_main();
		
		if (!active_window) {
			switch (gamestate) {
			case GAME:
				game_main();
				break;
			
			case EDITOR:
				editor_main();
			}
		}

		// Dont draw anything if we dont have focus.
		if (SDL_GetAppState() != 
				(SDL_APPACTIVE | SDL_APPMOUSEFOCUS | SDL_APPINPUTFOCUS)) {
			// idle
			SDL_Delay(MAX_FPS);

		} else {
			// Draw
			// TODO frame skip
			switch (gamestate) {
			case GAME:
				sdl_draw_game();
				break;
			
			case EDITOR:
				sdl_draw_editor();
			}
			
			ui_display_window();
			ui_display_message();
			sdl_draw_menu();
			sdl_draw_main_fx();

// 			printf("\rfps: %d", get_fps()); fflush(stdout);
			
			sdl_render();	
		}

	}

	save_config();
	return 0;
}


void change_state_quit()
{
	gamestate = QUIT;
}



