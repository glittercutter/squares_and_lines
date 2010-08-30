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
// draw.h -

#ifndef __DRAW_H__
#define __DRAW_H__

#include "shared.h"
#include "ui.h"

// Buffer for dinamic text drawing
char draw_txt_buf[200];


void sdl_draw_button();
SDL_Surface* sdl_create_surface(int, int);
void sdl_render();
void sdl_draw_text_solid(int, int, char*, TTF_Font*, int, int, int);
void sdl_draw_text_solid2(int, int, char*, TTF_Font*, ColorRGB);
void sdl_draw_text_blended(SDL_Surface*, int, int, char*, TTF_Font*, int, int, int);
void sdl_draw_text_blended2(SDL_Surface*, int, int, char*, TTF_Font*, ColorRGB);
void sdl_draw_line2(int, int, int, int, ColorRGB);
void sdl_draw_line3(SDL_Surface*, int, int, int, int, ColorRGB);
void sdl_draw_box2(int, int, int, int, ColorRGB);
void sdl_draw_rect2(SDL_Surface*, int, int, int, int, ColorRGB);
void sdl_draw_editor();
void sdl_draw_game();
void sdl_draw_main_fx();
void sdl_draw_menu();
void sdl_draw_widget_list_box(widget_list_box_t*);
void sdl_create_gui_graphic(void);
void sdl_create_button_gradient_effect(SDL_Surface*);


#endif
