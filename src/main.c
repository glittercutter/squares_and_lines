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

#include "client.h"
#include "common.h"
#include "editor.h"
#include "draw.h"
#include "fx.h"
#include "game.h"
#include "input.h"
#include "menu.h"
#include "parser_public.h"
#include "server.h"
#include "ui.h"


void do_at_exit()
{
	TTF_Quit();
	SDL_Quit();
}

static void sdl_init()
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0 )
		eprint("Unable to initialize SDL: %s\n", SDL_GetError());

	// set icon
	SDL_Surface *image = SDL_LoadBMP("icon.bmp");
	Uint32 colorkey = SDL_MapRGB(image->format, 0, 0, 0);
	SDL_SetColorKey(image, SDL_SRCCOLORKEY, colorkey);
	SDL_WM_SetIcon(image,NULL);

	if (display_fullscreen) {
		screen = SDL_SetVideoMode(display_width, display_height, 0, //
				SDL_ANYFORMAT | SDL_SWSURFACE | SDL_FULLSCREEN);
	} else {
		screen = SDL_SetVideoMode(display_width, display_height, 0, //
				SDL_ANYFORMAT | SDL_SWSURFACE);
	}

	SDL_WM_SetCaption(WINDOW_TITLE, 0 ); // set window name
}

static void load_font(font_s *font)
{
	font->data = (TTF_Font *)TTF_OpenFont(font->name, font->size);
	if (!font->data) {
   		eprint("TTF_OpenFont: %s\n", TTF_GetError());
		// FIXME crash later in program when font is used but not loaded
    	// TODO try loading a backup font ? maybe precompiled one ?
	}
	// for monospace fonts
	TTF_SizeText(font->data, "0", &font->w, &font->h);
}

void ui_init()
{
	sdl_create_gui_graphic();

	m_init_ui(); // other ui are based on this one
	ed_init_ui();
	g_init_ui();
	srv_ui_init();
	cl_ui_init();
}

int init()
{	
	load_config();
	load_lang();
	
	sdl_init();

	/* Initialize SDL_net */
	if (SDLNet_Init()) 
		eprint("SDLNet_Init: %s\n", SDLNet_GetError());
	
	/* Allocate memory for packet */
	if (!(udp_in_p = SDLNet_AllocPacket(PACKET_LENGHT)))
		eprint("SDLNet_AllocPacket: %s\n", SDLNet_GetError());
	if (!(udp_out_p = SDLNet_AllocPacket(PACKET_LENGHT)))
		eprint("SDLNet_AllocPacket: %s\n", SDLNet_GetError());
	
	/* Networking mutex */
	if (pthread_mutex_init(&list_box_mutex, NULL))
		eprint("pthread_mutex_init\n");
	if (pthread_mutex_init(&udp_new_buffer_mutex, NULL))
		eprint("pthread_mutex_init\n");

	// fonts
	if (TTF_Init()) 
		eprint("TTF_Init: %s\n", TTF_GetError());
	load_font(&button_font);

	ui_init();
	
	/* We start in the editor */
	ed_init();
	set_gamestate_EDITOR();

	/* Function called at exit */
	atexit(do_at_exit);

	return 0;
}

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
		if (!(SDL_GetAppState() & ~SDL_APPACTIVE) &&
				display_allow_idle) {
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
	
	if (net_game) {
		if (net_is_client)
			cl_close();
		else
			srv_close();
	}

	save_config();
	return 0;
}

void set_gamestate_QUIT()
{
	gamestate = QUIT;
}
