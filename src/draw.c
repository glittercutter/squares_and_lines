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
// draw.c - 

#include "draw.h"
#include "editor.h"
#include "fx.h"
#include "game.h"
#include "ui.h"


SDL_Surface* sdl_create_surface(int w, int h)
{
	SDL_Surface *surface;
	surface = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 32, 
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	   					0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
#else
						0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
#endif

	return surface;
}


void sdl_draw_surface(SDL_Surface *srcimg, int sx, int sy, int sw, int sh,
		SDL_Surface *dstimg, int dx, int dy, int alpha) 
{
	if ((srcimg == NULL) || (alpha == 0)) {
		return; 
	}

	SDL_Rect src, dst;
	src.x = sx;  src.y = sy;  src.w = sw;  src.h = sh;
	dst.x = dx;  dst.y = dy;  dst.w = src.w;  dst.h = src.h;
	
	if(alpha != 255) {
		SDL_SetAlpha(srcimg, SDL_SRCALPHA, alpha);
	}

	SDL_BlitSurface(srcimg, &src, dstimg, &dst);
}


/*
==============
sdl_draw_text_solid

Quick and dirty.
Text with transparent background. 
Draw to screen.
==============
*/
void sdl_draw_text_solid(int x, int y, char *text, TTF_Font *font, int r, int g, int b)
{
	SDL_Color color = { r, g, b };
	SDL_Surface* textSurface = TTF_RenderText_Solid(font, text, color);

	SDL_Rect textLocation = { x, y, 0, 0 };
	SDL_BlitSurface(textSurface, NULL, screen, &textLocation);
	
	SDL_FreeSurface(textSurface);
}
void sdl_draw_text_solid2(int x, int y, char *text, TTF_Font *font, ColorRGB color)
{
	SDL_Color sdl_color = { color.r, color.g, color.b };
	SDL_Surface* textSurface = TTF_RenderText_Solid(font, text, sdl_color);

	SDL_Rect textLocation = { x, y, 0, 0 };
	SDL_BlitSurface(textSurface, NULL, screen, &textLocation);
	
	SDL_FreeSurface(textSurface);
}


/*
==============
sdl_draw_text_blended

Slow,
text is antialiased with transparent background.
Draw to specified SDL_Surface.
==============
*/
void sdl_draw_text_blended(SDL_Surface *surface, int x, int y, char *text,
			TTF_Font *font, int r, int g, int b)
{
	SDL_Color color = { r, g, b };
	SDL_Surface* textSurface = TTF_RenderText_Blended(font, text, color);

	SDL_Rect textLocation = { x, y, 0, 0 };
	SDL_BlitSurface(textSurface, NULL, surface, &textLocation);
	
	SDL_FreeSurface(textSurface);
}
void sdl_draw_text_blended2(SDL_Surface *surface, int x, int y, char *text,
			TTF_Font *font, ColorRGB color)
{
	SDL_Color sdl_color = { color.r, color.g, color.b };
	SDL_Surface* textSurface = TTF_RenderText_Blended(font, text, sdl_color);

	SDL_Rect textLocation = { x, y, 0, 0 };
	SDL_BlitSurface(textSurface, NULL, surface, &textLocation);
	
	SDL_FreeSurface(textSurface);
}

/* Primitive
==============
Draw to screen
*/
void sdl_draw_line(int x1, int y1, int x2, int y2, int r, int g, int b)
{
	int a = 255;
	lineRGBA(screen, x1, y1, x2, y2, r, g, b, a);
}
void sdl_draw_line2(int x1, int y1, int x2, int y2, ColorRGB color)
{
	int a = 255;
	lineRGBA(screen, x1, y1, x2, y2, color.r, color.g, color.b, a);
}
void sdl_draw_line3(SDL_Surface *surface, int x1, int y1, int x2, int y2, ColorRGB color)
{
	int a = 200;
	lineRGBA(surface, x1, y1, x2, y2, color.r, color.g, color.b, a);
}
void sdl_draw_glow_line(int x1, int y1, int x2, int y2, ColorRGB color, int level)
{
	lineRGBA(screen, x1, y1, x2, y2, color.r, color.g, color.b, level);
}

void sdl_draw_rect(int x1, int y1, int x2, int y2, int r, int g, int b)
{
	int a = 255;
	rectangleRGBA(screen, x1, y1, x2, y2, r, g, b, a);
}
void sdl_draw_rect2(SDL_Surface *surface, int x1, int y1, int x2, int y2, ColorRGB color)
{
	int a = 255;
	rectangleRGBA(surface, x1, y1, x2, y2, color.r, color.g, color.b, a);
}

void sdl_draw_filled_circle(int x, int y, int radius, int r, int g, int b)
{
	int a = 255;
	filledEllipseRGBA(screen, x, y, radius, radius, r, g, b, a);
}

void sdl_draw_box(int x1, int y1, int x2, int y2, int r, int g, int b)
{
	int a = 255;
	boxRGBA(screen, x1, y1, x2, y2, r, g, b, a);
}
void sdl_draw_box2(int x1, int y1, int x2, int y2, ColorRGB color)
{
	int a = 255;
	boxRGBA(screen, x1, y1, x2, y2, color.r, color.g, color.b, a);
}

void sdl_draw_triangle(SDL_Surface *surface, int x1, int y1, int x2, int y2,
	   	int x3, int y3, ColorRGB color)
{
	int a = 255;
	filledTrigonRGBA(surface, x1, y1, x2, y2, x3, y3, color.r, color.g, color.b, a);
}
/*
==============
*/


static void sdl_draw_grid()
{
	int x1, y1;
	int x2, y2;

	x1 = ed_start_x;
	x2 = ed_start_x + (ed_grid_w * min_square_size);
	y1 = ed_start_y;

	for (int i = 0; i <= ed_grid_h; i++) {
		sdl_draw_line2(x1, y1, x2, y1, color.ed_grid);
		y1 += min_square_size;
	}

	y1 = ed_start_y;
	y2 = ed_start_y + (ed_grid_h * min_square_size);

	for (int i = 0; i <= ed_grid_w; i++) {
		sdl_draw_line2(x1, y1, x1, y2, color.ed_grid);
		x1 += min_square_size;
	}
}


static void sdl_draw_game_info()
{
	char string[30];
	int offset = display_width - TEXT_MARGIN;
	ColorRGB tmp_color;

	if (player_turn == PLAYER_1) {
		tmp_color.r = 0;
		tmp_color.g = 0;
		tmp_color.b = 0;
	} else {
		tmp_color.r = color.square_owner[PLAYER_1].r;
		tmp_color.g = color.square_owner[PLAYER_1].g;
		tmp_color.b = color.square_owner[PLAYER_1].b;
	}

	sprintf(string, "%s 2 : %d", text.player, score[1]);
	offset -= button_font.w * (strlen(string));
	sdl_draw_text_solid2(offset, -1, string, button_font.data, tmp_color);
	player[1].score_text_pos_x = offset;

	if (player_turn == PLAYER_0) {
		tmp_color.r = 0;
		tmp_color.g = 0;
		tmp_color.b = 0;
	} else {
		tmp_color.r = color.square_owner[PLAYER_0].r;
		tmp_color.g = color.square_owner[PLAYER_0].g;
		tmp_color.b = color.square_owner[PLAYER_0].b;
	}

	sprintf(string, "%s 1 : %d", text.player, score[0]);
	offset -= button_font.w * (strlen(string)) + TEXT_MARGIN + TEXT_MARGIN;
	sdl_draw_text_solid2(offset, -1, string, button_font.data, tmp_color);
	player[0].score_text_pos_x = offset;


	sprintf(string, "%s -", text.score);
	offset -= button_font.w * (strlen(string)) + TEXT_MARGIN + TEXT_MARGIN;
	sdl_draw_text_solid2(offset, -1, string, button_font.data, color.text);
}


static void sdl_draw_game_squares()
{
	for (int i = 0; i < ed_grid_h; i++) {		
		for (int j = 0; j < ed_grid_w; j++) {
			if (squares[i][j].active) {
				// top
				sdl_draw_line2(squares[i][j].x1, squares[i][j].y1,
						squares[i][j].x2, squares[i][j].y1, 
						color.square_owner[squares[i][j].owner_up]);
				// right
				sdl_draw_line2(squares[i][j].x2, squares[i][j].y1,
						squares[i][j].x2, squares[i][j].y2, 
						color.square_owner[squares[i][j].owner_right]);
				
				// bottom
				sdl_draw_line2(squares[i][j].x1, squares[i][j].y2,
						squares[i][j].x2, squares[i][j].y2,
						color.square_owner[squares[i][j].owner_down]);
				// left
				sdl_draw_line2(squares[i][j].x1, squares[i][j].y1,
						squares[i][j].x1, squares[i][j].y2, 
						color.square_owner[squares[i][j].owner_left]);

				if (squares[i][j].owner) {
					sdl_draw_box2(squares[i][j].x1, squares[i][j].y1,
						squares[i][j].x2, squares[i][j].y2, 
							color.square_owner[squares[i][j].owner]);
				}
				
			}
		}
	}
}



static void sdl_draw_ed_squares()
{
	for (int i = 0; i < ed_grid_h; i++) {		
		for (int j = 0; j < ed_grid_w; j++) {
			if (squares[i][j].active) {
				// outline
				// top
				if ((i == 0) || (!squares[i - 1][j].active)) {
					sdl_draw_line2(squares[i][j].x1, squares[i][j].y1,
						squares[i][j].x2, squares[i][j].y1, color.ed_outline);
				}
				// right
				if ((j == ed_grid_w - 1) || (!squares[i][j + 1].active)) {	
					sdl_draw_line2(squares[i][j].x2, squares[i][j].y1,
						squares[i][j].x2, squares[i][j].y2, color.ed_outline);
				}
				
				// bottom
				if ((i == ed_grid_h - 1) || (!squares[i + 1][j].active)) {
					sdl_draw_line2(squares[i][j].x1, squares[i][j].y2,
						squares[i][j].x2, squares[i][j].y2, color.ed_outline);
				}
				// left
				if ((j == 0) || (!squares[i][j - 1].active)) {	
					sdl_draw_line2(squares[i][j].x1, squares[i][j].y1,
						squares[i][j].x1, squares[i][j].y2, color.ed_outline);
				}
			}
		}
	}
}


// =======================================
// FX
// =======================================


/* 
====================
sdl_draw_fade_fx

fading transition
====================
*/
static void sdl_draw_fade_fx()
{
	int fade_level = 255 - (fx_transition[FX_FADE].current_step * 
			(255 / fx_transition[FX_FADE].max_step));
	boxRGBA(screen, 0, 0, display_width, display_height, 0, 0, 0, fade_level);
}

/* 
====================
sdl_draw_player_change_fx

draw box under current player score
====================
*/
static void sdl_draw_player_change_fx()
{
	int x1, x2;

	if (fx_transition[FX_PLAYER_CHANGE].active) {
		if (fx_transition[FX_PLAYER_CHANGE].halfway) {
			// do only first half the transition cycle
			fx_transition[FX_PLAYER_CHANGE].active = FALSE;

			// draw static box (prevent flashing between state)
			goto no_animation; 
		
		}
		// animate the box
		if (player_turn - 1 == 1) {
			x1 = player[1].score_text_pos_x - TEXT_MARGIN +
					((fx_transition[FX_PLAYER_CHANGE].max_step -
					fx_transition[FX_PLAYER_CHANGE].current_step) * 
					((display_width - player[1].score_text_pos_x) / 
					fx_transition[FX_PLAYER_CHANGE].max_step));
			x2 = x1 + player[0].score_text_pos_x - player[1].score_text_pos_x;
			
			boxRGBA(screen, x1, 0, x2,button_topbar->h, 
					color.square_owner[PLAYER_1].r, 
					color.square_owner[PLAYER_1].g, 
					color.square_owner[PLAYER_1].b, 255);
			// second box color are fading
			boxRGBA(screen, x1, 0, x2,button_topbar->h, 
					color.square_owner[PLAYER_0].r, 
					color.square_owner[PLAYER_0].g, 
					color.square_owner[PLAYER_0].b, 
					255 - ((fx_transition[FX_PLAYER_CHANGE].max_step -
					fx_transition[FX_PLAYER_CHANGE].current_step) * 
					(255 / fx_transition[FX_PLAYER_CHANGE].max_step)));


		} else {
			x1 = player[0].score_text_pos_x - TEXT_MARGIN +
					(fx_transition[FX_PLAYER_CHANGE].current_step * 
					((player[1].score_text_pos_x - 
					player[0].score_text_pos_x) / 
					fx_transition[FX_PLAYER_CHANGE].max_step));
			x2 = x1 + player[1].score_text_pos_x - player[0].score_text_pos_x;

			boxRGBA(screen, x1, 0, x2, button_topbar->h, 
					color.square_owner[PLAYER_0].r, 
					color.square_owner[PLAYER_0].g, 
					color.square_owner[PLAYER_0].b, 255);
			// second box color are fading
			boxRGBA(screen, x1, 0, x2,button_topbar->h, 
					color.square_owner[PLAYER_1].r, 
					color.square_owner[PLAYER_1].g, 
					color.square_owner[PLAYER_1].b, 
					255 - ((fx_transition[FX_PLAYER_CHANGE].max_step -
					fx_transition[FX_PLAYER_CHANGE].current_step) * 
					(255 / fx_transition[FX_PLAYER_CHANGE].max_step)));
		}

	} else {
	// static box

no_animation: // prevent flashing between state

		if (player_turn - 1 == 1) {
			boxRGBA(screen, player[1].score_text_pos_x - TEXT_MARGIN, 0,
					display_width, button_topbar->h, 
					color.square_owner[PLAYER_1].r, 
					color.square_owner[PLAYER_1].g, 
					color.square_owner[PLAYER_1].b, 255);
		} else {
			boxRGBA(screen, player[0].score_text_pos_x - TEXT_MARGIN, 0, 
					player[1].score_text_pos_x - TEXT_MARGIN, button_topbar->h,
					color.square_owner[PLAYER_0].r, 
					color.square_owner[PLAYER_0].g, 
					color.square_owner[PLAYER_0].b, 255);
		}
	}
}

void sdl_draw_background_game_fx()
{	
	sdl_draw_player_change_fx();
}

void sdl_draw_game_fx()
{
	for (int i = 0; i < MAX_GLOW; i++) {
		if (seg_glow[i].square) {
				sdl_draw_glow_line(seg_glow[i].x1, seg_glow[i].y1, 
				seg_glow[i].x2, seg_glow[i].y2, 
				color.square_owner[seg_glow[i].player], 
				seg_glow[i].glow_level);
		}
	}
}

void sdl_draw_main_fx()
{
	// transition
	for (int i = 0; i < NUM_OF_TRANSITION; i++) {
		if (fx_transition[i].active) {
			switch (fx_transition[i].fx_type) {
				case FX_FADE:
					sdl_draw_fade_fx();
				break;
			}
		}
	}
}


// =======================================
// UI
// =======================================
void sdl_draw_button(Button *button)
{

	if (button == NULL) {
// 		DEBUG(printf("no button !\n"));
		return;
	}
	
	while (button != NULL) {
		if (button == ui_pressed_button) {
			sdl_draw_surface(button->surface, 0, 0, button->w, button->h,
					screen, button->x1, button->y1, 255);
			boxRGBA(screen, button->x1, button->y1, button->x2, button->y2, 
					color.button_highlight.r, 
					color.button_highlight.g, 
					color.button_highlight.b, 100);

		} else if (button == ui_highlight_button) {
			sdl_draw_surface(button->surface, 0, 0, button->w, button->h,
					screen, button->x1, button->y1, 255);
			boxRGBA(screen, button->x1, button->y1, button->x2, button->y2,
					color.button_highlight.r, 
					color.button_highlight.g, 
					color.button_highlight.b, 100);

		} else {
			sdl_draw_surface(button->surface, 0, 0, button->w, button->h, screen,
					button->x1, button->y1, 255);	
		}
		button = button->next;
	}
}


void sdl_draw_widget_scrollbar(scrollbar_t *scrollbar)
{
	int handle_center_x = scrollbar->handle.x1 + (scrollbar->handle.w / 2);
	int handle_center_y = scrollbar->handle.y1 + (scrollbar->handle.h / 2);
	
	if (scrollbar->orientation == HORIZONTAL) {
		sdl_draw_box2(scrollbar->x1 + SCROLLBAR_SIZE, scrollbar->y1, 
				scrollbar->x2 - SCROLLBAR_SIZE, scrollbar->y2, color.text);
	} else {
		sdl_draw_box2(scrollbar->x1, scrollbar->y1 + SCROLLBAR_SIZE, 
				scrollbar->x2, scrollbar->y2 - SCROLLBAR_SIZE, color.text);
	}

	sdl_draw_button(&scrollbar->arrow1);
	sdl_draw_button(&scrollbar->arrow2);

	sdl_draw_box2(scrollbar->handle.x1, scrollbar->handle.y1, 
			scrollbar->handle.x2, scrollbar->handle.y2, 
			color.topbar);
	
	// center lines
	lineRGBA(screen, handle_center_x - 2, handle_center_y, handle_center_x + 2,
			handle_center_y, 50, 50, 50, 255);
	lineRGBA(screen, handle_center_x - 2, handle_center_y - 3,
			handle_center_x + 2, handle_center_y - 3, 50, 50, 50, 255);
	lineRGBA(screen, handle_center_x - 2, handle_center_y + 3,
			handle_center_x + 2, handle_center_y + 3, 50, 50, 50, 255);
	
	// 3d effect
	lineRGBA(screen, scrollbar->handle.x2, scrollbar->handle.y1 + 1, 
			scrollbar->handle.x2, scrollbar->handle.y2, 50, 50, 50, 255);
	lineRGBA(screen, scrollbar->handle.x1, scrollbar->handle.y2, 
			scrollbar->handle.x2, scrollbar->handle.y2, 50, 50, 50, 255);
	lineRGBA(screen, scrollbar->handle.x1, scrollbar->handle.y1, 
			scrollbar->handle.x1, scrollbar->handle.y2 - 1, 130, 130, 130, 255);
	lineRGBA(screen, scrollbar->handle.x1, scrollbar->handle.y1, 
			scrollbar->handle.x2 - 1, scrollbar->handle.y1, 130, 130, 130, 255);

}


void sdl_draw_widget_list_box(widget_list_box_t *list_box)
{
	sdl_draw_box2(list_box->x1, list_box->y1, list_box->x2 - SCROLLBAR_SIZE, 
			list_box->y2, color.button_highlight);

	sdl_draw_widget_scrollbar(&list_box->scrollbar);

	
	
}



void sdl_create_button_3d_effect(SDL_Surface *surface)
{
	lineRGBA(surface, 0, 0, surface->w - 1, 0, 
			200, 200, 200, 200);
	lineRGBA(surface, 0, 0, 0, surface->h - 1, 
			200, 200, 200, 200);
	lineRGBA(surface, surface->w - 1, 0, surface->w - 1, surface->h, 
			0, 0, 0, 200);
	lineRGBA(surface, 0, surface->h - 1, surface->w - 1, surface->h - 1, 
			0, 0, 0, 200);

}


void sdl_create_gui_graphic(void)
{
	gui_surface.arrow_up = sdl_create_surface(SCROLLBAR_SIZE + 1,
			SCROLLBAR_SIZE + 1);
	boxRGBA(gui_surface.arrow_up, 0, 0, gui_surface.arrow_up->w, 
			gui_surface.arrow_up->w, color.topbar.r, color.topbar.g, 
			color.topbar.b, 255);
	sdl_draw_triangle(gui_surface.arrow_up, 
			gui_surface.arrow_up->w / 2, gui_surface.arrow_up->w * 0.3f, 
			gui_surface.arrow_up->w * 0.3f, gui_surface.arrow_up->w * 0.7f,
			gui_surface.arrow_up->w * 0.7f, gui_surface.arrow_up->w * 0.7f,
			color.button_highlight);
	sdl_create_button_3d_effect(gui_surface.arrow_up);

	gui_surface.arrow_down = sdl_create_surface(SCROLLBAR_SIZE + 1,
			SCROLLBAR_SIZE + 1);
	boxRGBA(gui_surface.arrow_down, 0, 0, gui_surface.arrow_up->w, 
			gui_surface.arrow_up->w, color.topbar.r, color.topbar.g, 
			color.topbar.b, 255);
	sdl_draw_triangle(gui_surface.arrow_down, 
			gui_surface.arrow_up->w * 0.3f, gui_surface.arrow_up->w * 0.3f,
			gui_surface.arrow_up->w * 0.7f, gui_surface.arrow_up->w * 0.3f,
			gui_surface.arrow_up->w / 2, gui_surface.arrow_up->w * 0.7f, 
			color.button_highlight);
	sdl_create_button_3d_effect(gui_surface.arrow_down);
}


// ================================
// UI
static void sdl_draw_topbar()
{
	int ui_bar_height = button_font.size + UI_BAR_PADDING;
	sdl_draw_box2(0, 0, display_width, ui_bar_height, color.topbar);
	sdl_draw_line2(0, ui_bar_height, display_width, ui_bar_height, color.button_highlight);
}


static void sdl_draw_dropmenu()
{
	sdl_draw_button(active_dropmenu);
}


static void sdl_draw_game_ui()
{
	sdl_draw_button(button_game);
	sdl_draw_game_info();
}


static void sdl_draw_editor_ui()
{
	sdl_draw_button(button_editor);
	
}

// ================================
void sdl_draw_menu()
{
	sdl_draw_button(button_topbar);
	if (active_dropmenu != NULL) {
		sdl_draw_dropmenu();
	}
}


void sdl_draw_game()
{
	SDL_FillRect( SDL_GetVideoSurface(), NULL, 0 );
	sdl_draw_topbar();
	sdl_draw_background_game_fx();
	sdl_draw_game_ui();
	sdl_draw_game_squares();
	sdl_draw_game_fx();
}


void sdl_draw_editor()
{
	SDL_FillRect( SDL_GetVideoSurface(), NULL, 0 );
	sdl_draw_topbar();
	sdl_draw_editor_ui();
	sdl_draw_grid();
	sdl_draw_ed_squares();

}
// ================================


void sdl_render()
{
	int delay;
	int currentTick;
	static int lastFrameTick;
		
	// Limit to max framerate
	if (LIMIT_FPS) {
		while (42) {
			currentTick = SDL_GetTicks();
			if (currentTick >= (lastFrameTick + MAX_FPS)) {
				if (SDL_MUSTLOCK(screen)) {
					SDL_UnlockSurface(screen);
				}
				SDL_UpdateRect(screen, 0, 0, 0, 0);
				lastFrameTick = currentTick;
				break;
			} else {
				delay = MAX_FPS - (currentTick - lastFrameTick);
				SDL_Delay(delay);
			}
		}
			
	// Unlimited framerate
	} else {
		if (SDL_MUSTLOCK(screen)) {
			SDL_UnlockSurface(screen);
		}
		SDL_UpdateRect(screen, 0, 0, 0, 0);
	}
}

