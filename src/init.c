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
// g_init.c - 

#include "init.h"

#include "client.h"
#include "draw.h"
#include "editor.h"
#include "game.h"
#include "menu.h"
#include "parse_public.h"
#include "server.h"
#include "ui.h"



/* 
====================
sdl_cleanup

Called by SDL at the end of the program.
====================
*/
void 
sdl_cleanup()
{
	TTF_Quit();
	SDL_Quit();
}


static int sdl_init()
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0 ) {
		printf("Unable to initialize SDL: %s\n", SDL_GetError());
		// TODO handle error...
		return 1;
	}
	return 0;
}


static int sdl_init_video()
{
	if (display_fullscreen) {
		screen = SDL_SetVideoMode(display_width, display_height, 0, //
				SDL_ANYFORMAT|SDL_SWSURFACE|SDL_FULLSCREEN);
	} else {
		screen = SDL_SetVideoMode(display_width, display_height, 0, //
				SDL_ANYFORMAT|SDL_SWSURFACE);
	}

	if (SDL_Init(SDL_INIT_VIDEO) < 0 ) {
		SDL_Quit();
		return 1;
	}
	// set window name
	SDL_WM_SetCaption(WINDOW_TITLE, 0 );
	return 0;
}


static void sdl_load_font(Font *font)
{
	font->data = (TTF_Font *)TTF_OpenFont(font->name, font->size);
	if (!font->data) {
   		printf("TTF_OpenFont: %s\n", TTF_GetError());
		// FIXME crash later in program
    	// TODO try loading a backup font ? maybe precompiled one ?
	}
	TTF_SizeText(font->data, "0", &font->w, &font->h);
}


void ui_init()
{
	sdl_create_gui_graphic();
	m_init_ui(); // other ui are based on this one
	ed_init_ui();
	g_init_ui();
	sv_init_ui();
	cl_init_ui();
}


int init()
{
	load_config();
	load_lang();

	if (sdl_init()) {
		return 1;
	}

	if (sdl_init_video()) {
		return 1;
	}
	
	// font
	if (TTF_Init()==-1) {
    	printf("TTF_Init: %s\n", TTF_GetError());
	}

	sdl_load_font(&button_font);

	/* Initialize SDL_net */
	if (SDLNet_Init() < 0)
	{
		fprintf(stderr, "SDLNet_Init: %s\n", SDLNet_GetError());
		exit(EXIT_FAILURE);
	}
	
	ui_init();
	ed_init();

	gamestate = EDITOR;

	// sdl call this function at exit
	atexit(sdl_cleanup);

	return 0;
}


