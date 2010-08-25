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
// init.c - 

#include "init.h"

#include "common.h"
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


static void sdl_init()
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0 )
		eprint("Unable to initialize SDL: %s\n", SDL_GetError());
}


static void sdl_init_video()
{
	if (display_fullscreen) {
		screen = SDL_SetVideoMode(display_width, display_height, 0, //
				SDL_ANYFORMAT|SDL_SWSURFACE|SDL_FULLSCREEN);
	} else {
		screen = SDL_SetVideoMode(display_width, display_height, 0, //
				SDL_ANYFORMAT|SDL_SWSURFACE);
	}

	if (SDL_Init(SDL_INIT_VIDEO) < 0 )
		eprint("Unable to initialize video: %s\n", SDL_GetError());

	SDL_WM_SetCaption(WINDOW_TITLE, 0 ); // set window name
}


static void sdl_load_font(Font *font)
{
	font->data = (TTF_Font *)TTF_OpenFont(font->name, font->size);
	if (!font->data) {
   		eprint("TTF_OpenFont: %s\n", TTF_GetError());
		// FIXME crash later in program when font is used but not loaded
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
	
	sdl_init();
	sdl_init_video();
	
	// fonts
	if (TTF_Init()==-1) 
		eprint("TTF_Init: %s\n", TTF_GetError());
	sdl_load_font(&button_font);

	/* Initialize SDL_net */
	if (SDLNet_Init() < 0) 
		eprint("SDLNet_Init: %s\n", SDLNet_GetError());
	
	ui_init();

	/* We start in the editor */
	ed_init();
	gamestate = EDITOR;

	/* sdl call this function at exit */
	atexit(sdl_cleanup);

	return 0;
}


