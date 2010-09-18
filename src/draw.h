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


void sdl_draw_button(button_s *button);
SDL_Surface* sdl_create_surface(int w, int h);
void sdl_render(void);
void sdl_draw_text_solid2(int, int, char*, TTF_Font*, colorRGB_t);
void sdl_draw_text_blended2(SDL_Surface*, int, int, char*, TTF_Font*, colorRGB_t);
void sdl_draw_line_screen(int, int, int, int, colorRGB_t);
void sdl_draw_line_surface(SDL_Surface*, int, int, int, int, colorRGB_t);
void sdl_draw_box_screen(int, int, int, int, colorRGB_t);
void sdl_draw_rect_surface(SDL_Surface*, int, int, int, int, colorRGB_t);
void sdl_draw_editor(void);
void sdl_draw_game(void);
void sdl_draw_main_fx(void);
void sdl_draw_menu(void);
void sdl_draw_widget_list_box(widget_list_box_t*);
void sdl_draw_fx_3d(int x1, int y1, int x2, int y2);
void sdl_create_gui_graphic(void);
void sdl_draw_gradient_effect_surface(SDL_Surface*);


#endif
