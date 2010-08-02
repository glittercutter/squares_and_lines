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

#include "editor.h"
#include "draw.h"
#include "fx.h"
#include "game.h"
#include "init.h"
#include "input.h"
#include "menu.h"
#include "parse_public.h"
#include "ui.h"


int get_fps();


int main(int argc, char **argv)
{
	if(init()) {
		return 1;
	}

	while (gamestate != QUIT) {

		get_input();

		m_do_menu();
		fx_main();
		
		if (!active_window) {
			switch (gamestate) {
				case GAME:
					g_do_game();
				break;
				
				case EDITOR:
					ed_do_editor();
				break;
			}
		}

		// TODO frame skip
		switch (gamestate) {
			case GAME:
				sdl_draw_game();
			break;
			
			case EDITOR:
				sdl_draw_editor();
			break;
		}	
		
		ui_display_message();
		ui_display_window();
		sdl_draw_menu();
		sdl_draw_main_fx();

// 		printf("fps: %d\n", get_fps());
		sdl_render();

	}

	save_config();

	return 0;
}


void change_state_quit()
{
	gamestate = QUIT;
}


/*
==========
get_fps

fps is updated once per second
==========
*/
int get_fps()
{
	static int fps = 0, last_tick = 0, frame_counter = 0;
	int current_tick = SDL_GetTicks();
	// Update count every second
	if(current_tick >= (last_tick + 1000)) {
		fps = frame_counter;
		frame_counter = 0;
		last_tick = current_tick;
	} else ++frame_counter;
	
	return fps;
}



/* 
====================
get_random_number

result change even if called multiple time for a tick
====================
*/
int get_random_number(int max_number)
{
	static int last_tick;
	static int tick_instance = 0;

	int current_tick = SDL_GetTicks();
	if(last_tick == current_tick) {
		++tick_instance;
		current_tick *= tick_instance; 
	} else {
		tick_instance = 0;
		last_tick = current_tick;
	}
		
	srand(current_tick);
	int random_number = rand() % max_number;	
	
	return random_number;
}


/* 
====================
ipow

Exponentiation by squaring (fast)
====================
*/
int ipow(int base, int exp)
{
    int result = 1;
    while (exp)
    {
        if (exp & 1)
            result *= base;
        exp >>= 1;
        base *= base;
    }

    return result;
}


/* 
====================
longest_string

last argument must be a null pointer
====================
*/
int longest_string(char *str1, ...)
{
	va_list ap;
	char *tmp_str;
	int longest = 0;
	int len = 0;;

	va_start(ap, str1);
	
	tmp_str = str1;
	
	while(tmp_str != NULL) {
		len = strlen(tmp_str);
		if (len > longest) longest = len;
		tmp_str = va_arg(ap, char*);
	}

	va_end(ap);

	return longest;
}


