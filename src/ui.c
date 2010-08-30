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
// ui.c -

#include "ui.h"

#include "draw.h"
#include "input.h"

char draw_txt_buf[200];

int ui_scrollbar_check(scrollbar_t*);


/* 
====================
ui_new_button

Allocate memory, create surface, set position.
Button is added at the tail of the familly node.
Return a pointer to the created button.
====================
*/
Button* ui_new_button(int x, int y, int w, int h, int min_w, int max_w, 
		int align, char *text, void func(), int three_d, int gradient,
		Button **node)
{
	Button **node2 = node;
	int alfa = 255;
	int text_x = 0;
	int nor_w;
	
	if (w > max_w) {
		nor_w = max_w;
	} else if (w < min_w) {
		nor_w = min_w;
	} else nor_w = w;
	nor_w += (UI_BAR_PADDING * 2);

	Button *button;
	button = malloc(sizeof(Button));
	button->next = NULL;

	button->x1 = x;
	button->y1 = y;
	button->x2 = x + nor_w;
	button->y2 = y + h;
	button->w = nor_w;
	button->h = h;

	button->func = func;
	
	// surface
	button->surface = sdl_create_surface(nor_w, h);
	boxRGBA(button->surface, 0, 0, nor_w, h, color.topbar.r, 
			color.topbar.g, color.topbar.b, alfa);

	// draw gradient
	if (gradient) 
		sdl_create_button_gradient_effect(button->surface);

	// draw 3d effect
	if (three_d) {
		rectangleRGBA(button->surface, 0, 0, button->w - 1, button->h - 1, 
				color.button_highlight.r, color.button_highlight.g, 
				color.button_highlight.b, 150);
		lineRGBA(button->surface, 1, 0, button->w - 2, 0, color.ed_outline.r, 
				color.ed_outline.g, color.ed_outline.b, 200);
		lineRGBA(button->surface, 0, 0, 0, button->h, color.ed_outline.r, 
				color.ed_outline.g, color.ed_outline.b, 200);
	}

	// draw text
	switch (align) {
	case ALIGN_LEFT:
		text_x = (UI_BAR_PADDING * 2);
		break;

	case ALIGN_CENTER:
		text_x = ((nor_w - w) / 2 + (UI_BAR_PADDING / 2));
		break;

	case ALIGN_RIGHT:
		text_x = (nor_w - w) - (UI_BAR_PADDING * 2);
		break;
	}
	sdl_draw_text_blended2(button->surface, text_x, -1, text, 
			button_font.data, color.text);

	// Add to button list
	while (*node2) {
		node2 = &(*node2)->next;
	}
	*node2 = button;

	return *node2;
}


/* 
====================
ui_button_check_click

Return true and store a pointer in "ui_pressed_button"
if a button is under the mouse.
====================
*/
int ui_button_check_click(Button **button_type)
{
	Button *button = *button_type;

	while (button) {
		if ((input.mouse_y < button->y2) && 
				(input.mouse_x < button->x2) && 
				(input.mouse_y > button->y1) && 
				(input.mouse_x > button->x1)) {
			ui_pressed_button = button;
			input.mouse_button_left = FALSE;
			return 1;
		} else {
			button = button->next;
		}
	}
	return 0;
}

/* 
====================
ui_button_check_click

Return true if "button" is under the mouse
====================
*/
int ui_singlebutton_check_click(Button *button)
{
	if ((input.mouse_y < button->y2) && 
			(input.mouse_x < button->x2) && 
			(input.mouse_y > button->y1) && 
			(input.mouse_x > button->x1)) {
		ui_pressed_button = button;
		return 1;
	}
	return 0;
}

/* 
====================
ui_button_check_click

Return a pointer to the button under the mouse.
====================
*/
Button* ui_button_check_pos(Button **button_type)
{
	Button *button = *button_type;
	static const int offset = 1; // remove hole between buttons

	while (button) {
		if ((input.mouse_y < button->y2 + offset) && 
				(input.mouse_x < button->x2 +offset) && 
				(input.mouse_y > button->y1 - offset) && 
				(input.mouse_x > button->x1 + offset)) {
			return button;
		} else {
			button = button->next;
		}
	}
	return NULL;
}


/* 
====================
ui_button_function

Called by get_input() to execute the function of the button.
====================
*/
void ui_button_function()
{
	if (ui_pressed_button->func != NULL) {
		// check if the mouse is still on the button
		if ((input.mouse_y < ui_pressed_button->y2) && 
				(input.mouse_x < ui_pressed_button->x2) && 
				(input.mouse_y > ui_pressed_button->y1) && 
				(input.mouse_x > ui_pressed_button->x1)) {
			ui_pressed_button->func();
		}
	}
	ui_pressed_button = NULL;
}


void ui_button_close_window()
{
	active_window = active_window->last_window;
}


void ui_button_drag_window()
{
	;
}


#define MESSAGE_TIME 50
void ui_new_message(char* text)
{
	int w;
	ui_message.active = TRUE;
	ui_message.time = MESSAGE_TIME;
	strncpy(ui_message.text, text, LONG_STRING_LENGTH - 1);
	
	w = strlen(ui_message.text) * button_font.w + (UI_BAR_PADDING * 2);

	ui_message.x1 = (display_width - w) / 2;
	ui_message.y1 = (display_height - button_topbar->y2) / 2 + button_topbar->y2;
	ui_message.x2 = ui_message.x1 + w;
	ui_message.y2 = ui_message.y1 + button_font.h;
}


void ui_display_message()
{
	if (!ui_message.active) 
		return;

	sdl_draw_box2(ui_message.x1, ui_message.y1, ui_message.x2, 
			ui_message.y2, color.topbar);
	sdl_draw_rect2(screen, ui_message.x1, ui_message.y1, ui_message.x2, 
			ui_message.y2, color.button_highlight);
	sdl_draw_text_solid2(ui_message.x1 + UI_BAR_PADDING, ui_message.y1, 
			ui_message.text, button_font.data, color.text);

	--ui_message.time;
	if (!ui_message.time) 
		ui_message.active = FALSE;
}


int ui_list_box_check(widget_list_box_t *list_box)
{
	// collum name button
	ui_button_check_click(&list_box->list->col_name);
	
	// check if the mouse is over the box
	if (input.mouse_x > list_box->x2 - SCROLLBAR_SIZE || 
			input.mouse_x < list_box->x1)
		return 0;
	if (input.mouse_y > list_box->y2 || input.mouse_y < list_box->y1)
		return 0;

	list_box->selected_row = ((input.mouse_y - list_box->y1) / button_font.h) + 
			list_box->scrollbar.offset;

	return 1;
}


void ui_display_window()
{
	if (!active_window) return;

	widget_t *widget_node = active_window->widget;

	// draw background
	sdl_draw_box2(active_window->x1, active_window->y1, active_window->x2,
			active_window->y2, color.topbar);

	if (active_window->widget) {
		while (widget_node != NULL) {
			// all widget for the window are in a single linked list
			switch (widget_node->type) {
			case LIST_BOX:
				sdl_draw_widget_list_box(widget_node->widget.list_box);
				
				if (input.mouse_button_left || widget_node->
						widget.list_box->scrollbar.dragging_handle) {
					if (ui_scrollbar_check(&widget_node->widget.list_box->
							scrollbar)) {
					} else (ui_list_box_check(widget_node->widget.list_box));
				}
				break;

			case PLAIN_TEXT:
				snprintf(draw_txt_buf, sizeof(draw_txt_buf), "%s%s%s", 
						widget_node->widget.plain_text->text1,
						widget_node->widget.plain_text->text2, 
						widget_node->widget.plain_text->text3),

				sdl_draw_text_solid2(widget_node->widget.plain_text->x1, 
						widget_node->widget.plain_text->y1,
						draw_txt_buf,
						button_font.data,
						widget_node->widget.plain_text->color);
				break;

			}
			widget_node = widget_node->next;
		}
	}

	// window's buttons
	if (active_window->button) {
		if (input.mouse_button_left) {
			if (!ui_button_check_click(&active_window->close_button))
				ui_button_check_click(&active_window->button);
		}
		sdl_draw_button(active_window->button);
		sdl_draw_button(active_window->close_button);
	}
}

void ui_scrollbar_update_size(int no_of_element, int max_element, 
		scrollbar_t *scrollbar)
{	
	if(scrollbar->orientation == HORIZONTAL) {
		if((no_of_element == 0) || (no_of_element <= max_element)) {
			scrollbar->handle.w = scrollbar->handle_max_w;
			return;
		}
		*scrollbar->element = no_of_element;
		scrollbar->ratio = max_element / (float)no_of_element;
		scrollbar->handle.w = scrollbar->handle_max_w * scrollbar->ratio;
		scrollbar->handle.x2 = scrollbar->handle.x1 + scrollbar->handle.w;
	} else { // scrollbar is vertical
		if((no_of_element == 0) || (no_of_element <= max_element)) {
			scrollbar->handle.h = scrollbar->handle_max_h;
			return;
		}
		*scrollbar->element = no_of_element;
		scrollbar->ratio = max_element / (float)no_of_element;
		scrollbar->handle.h = scrollbar->handle_max_h * scrollbar->ratio;
		scrollbar->handle.y2 = scrollbar->handle.y1 + scrollbar->handle.h;
	}
}


int ui_scrollbar_check(scrollbar_t *scrollbar)
{
	static int click_pos = 0;
	int mouse_pos;
	
	if (!input.mouse_button_left) {
		scrollbar->dragging_handle = FALSE;
		ui_dragged_scrollbar = NULL;
		return 0;
	}

	if(scrollbar->dragging_handle) {
		if(scrollbar->orientation == HORIZONTAL) {
			mouse_pos = (input.mouse_x - scrollbar->x1 - SCROLLBAR_SIZE -
				  	click_pos);
			if(mouse_pos == 0 || scrollbar->handle_max_w - 
					scrollbar->handle.w <= 0) {
				scrollbar->handle_pos = 0;
			} else {
				scrollbar->handle_pos = (mouse_pos / 
						(float)(scrollbar->handle_max_w - 
						scrollbar->handle.w)) * 100.0f;
			}

		} else {
			// scrollbar is vertical
			mouse_pos = input.mouse_y - scrollbar->y1 - SCROLLBAR_SIZE -
			   		click_pos;	
			if(mouse_pos == 0 || scrollbar->handle_max_h - 
						scrollbar->handle.h <= 0) {
				scrollbar->handle_pos = 0;
			} else {
				scrollbar->handle_pos = (mouse_pos / 
						(float)(scrollbar->handle_max_h - 
						scrollbar->handle.h)) * 100.0f;
			}
			scrollbar->offset = ((scrollbar->handle.y1 - (scrollbar->y1 +
					SCROLLBAR_SIZE)) / (float)(scrollbar->handle_max_h -
					scrollbar->handle.h)) * (float)(*scrollbar->element -
					*scrollbar->viewable_element);
		}

	} else if(ui_singlebutton_check_click(&scrollbar->handle)) {
		// scrollbar now dragged
		scrollbar->dragging_handle = TRUE;
		ui_dragged_scrollbar = scrollbar;
		// keep position of the mouse when clicked for drag
		if(scrollbar->orientation == HORIZONTAL) {
			click_pos = input.mouse_x - scrollbar->handle.x1;
		} else {
			click_pos = input.mouse_y - scrollbar->handle.y1;
		}

	// check arrows button
	} else if(ui_singlebutton_check_click(&scrollbar->arrow1)) {
		scrollbar->handle_pos -= 1; // TODO increment one element height
	} else if(ui_singlebutton_check_click(&scrollbar->arrow2)) {	
		scrollbar->handle_pos += 1; // TODO increment one element height

	} else return 0;
	
	// Limit handle travel
	if(scrollbar->handle_pos < 0) scrollbar->handle_pos = 0;
	if(scrollbar->handle_pos > 100) scrollbar->handle_pos = 100;

	if(scrollbar->orientation == HORIZONTAL) {
		scrollbar->handle.x1 = ((scrollbar->handle_pos / 100.0f) *
				(float)(scrollbar->handle_max_w - scrollbar->handle.w)) +
				scrollbar->x1 + SCROLLBAR_SIZE;
	} else { // scrollbar is vertical
		scrollbar->handle.y1 = ((scrollbar->handle_pos / 100.0f) * 
				(float)(scrollbar->handle_max_h - scrollbar->handle.h)) +
				scrollbar->y1 + SCROLLBAR_SIZE;
	}
	
	// set handle dimension for drawing and position check
	scrollbar->handle.x2 = scrollbar->handle.x1 + scrollbar->handle.w;
	scrollbar->handle.y2 = scrollbar->handle.y1 + scrollbar->handle.h;

	return 1;
}


void ui_new_widget_list_box(int x1, int y1, int x2, int y2, 
		string_list_t *strlist, widget_t **widget_head_node)
{
	widget_t **tmp_node = widget_head_node;

	widget_t *new_widget = malloc(sizeof(widget_t));
	widget_list_box_t *list_box = calloc(1, sizeof(widget_list_box_t));

	list_box->x1 = x1;
	list_box->y1 = y1;
	list_box->x2 = x2;
	list_box->y2 = y2;
	list_box->w = x2 - x1;
	list_box->h = y2 - y1;

	list_box->scrollbar.x1 = x2 - SCROLLBAR_SIZE;
	list_box->scrollbar.y1 = y1 - button_font.h;
	list_box->scrollbar.x2 = x2;
	list_box->scrollbar.y2 = y2;

	list_box->scrollbar.orientation = VERTICAL;
	list_box->scrollbar.handle_max_w = SCROLLBAR_SIZE - 1;
	list_box->scrollbar.handle_max_h = (list_box->scrollbar.y2 - SCROLLBAR_SIZE) - 
			(list_box->scrollbar.y1 + SCROLLBAR_SIZE);
	
	list_box->scrollbar.dragging_handle = FALSE;

	// arrows button
	list_box->scrollbar.arrow1.surface = gui_surface.arrow_up;
	list_box->scrollbar.arrow1.next = NULL;
	list_box->scrollbar.arrow1.func = NULL;
	
	list_box->scrollbar.arrow2.surface = gui_surface.arrow_down;
	list_box->scrollbar.arrow2.next = NULL;
	list_box->scrollbar.arrow2.func = NULL;
	
	list_box->scrollbar.arrow1.x1 = list_box->scrollbar.x1;
	list_box->scrollbar.arrow1.y1 = list_box->scrollbar.y1;
	list_box->scrollbar.arrow1.x2 = list_box->scrollbar.x2;
	list_box->scrollbar.arrow1.y2 = list_box->scrollbar.y1 + SCROLLBAR_SIZE;
	list_box->scrollbar.arrow1.w = list_box->scrollbar.x2 - list_box->scrollbar.x1;
	list_box->scrollbar.arrow1.h = list_box->scrollbar.y2 - list_box->scrollbar.y1;

	list_box->scrollbar.arrow2.x1 = list_box->scrollbar.x1;
	list_box->scrollbar.arrow2.y1 = list_box->scrollbar.y2 - SCROLLBAR_SIZE;
	list_box->scrollbar.arrow2.x2 = list_box->scrollbar.x2;
	list_box->scrollbar.arrow2.y2 = list_box->scrollbar.y2;
	list_box->scrollbar.arrow2.w = list_box->scrollbar.x2 - list_box->scrollbar.x1;
	list_box->scrollbar.arrow2.h = list_box->scrollbar.y2 - list_box->scrollbar.y1;

	list_box->list = strlist;
	strlist->list_box = list_box;

	list_box->viewable_element = list_box->h / button_font.h;
	(list_box->list)->max_element = list_box->h / button_font.h;
	list_box->scrollbar.viewable_element = &list_box->viewable_element;
	list_box->scrollbar.element = &list_box->list->element;
	
	ui_scrollbar_update_size(0, list_box->viewable_element, 
			&list_box->scrollbar);

	if(list_box->scrollbar.orientation == HORIZONTAL) {
		list_box->scrollbar.handle.h = SCROLLBAR_SIZE;
		
		list_box->scrollbar.handle.x1 = ((list_box->scrollbar.handle_pos / 
				100.0f) * (float)(list_box->scrollbar.handle_max_w - 
				list_box->scrollbar.handle.w)) + list_box->scrollbar.x1 +
				SCROLLBAR_SIZE + 1;
		list_box->scrollbar.handle.x2 = list_box->scrollbar.handle.x1 + 
				list_box->scrollbar.handle.w;
		
		list_box->scrollbar.handle.y1 = list_box->scrollbar.y2;
		list_box->scrollbar.handle.y2 = list_box->scrollbar.handle.y1 + 
				list_box->scrollbar.handle.h;
		list_box->scrollbar.handle.w = list_box->scrollbar.handle.x2 - 
				list_box->scrollbar.handle.x1;
	
	} else { // scrollbar is vertical
		list_box->scrollbar.handle.w = SCROLLBAR_SIZE;
		
		list_box->scrollbar.handle.y1 = ((list_box->scrollbar.handle_pos / 
				100.0f) * (float)(list_box->scrollbar.handle_max_h - 
				list_box->scrollbar.handle.h)) + list_box->scrollbar.y1 +
				SCROLLBAR_SIZE + 1;
		list_box->scrollbar.handle.y2 = list_box->scrollbar.handle.y1 + 
				list_box->scrollbar.handle.h;
		
		list_box->scrollbar.handle.x1 = list_box->scrollbar.x1;
		list_box->scrollbar.handle.x2 = list_box->scrollbar.handle.x1 + 
				list_box->scrollbar.handle.w;
		list_box->scrollbar.handle.h = list_box->scrollbar.handle.y2 - 
				list_box->scrollbar.handle.y1;

	}

	list_box->scrollbar.handle.func = NULL;
	list_box->selected_row = -1;
	
	new_widget->type = LIST_BOX;
	new_widget->widget.list_box = list_box;
	new_widget->next = NULL;
	
	printf("created list_box\n");	

	// add to window widget list
	if (!*tmp_node) {
		// first node
		*tmp_node = new_widget;
		return;
	}

	while (*tmp_node) {
		tmp_node = &(*tmp_node)->next;
	}
	*tmp_node = new_widget;

}

void ui_new_widget_plain_text(char *text1, char *text2, char *text3, 
		int x1, int y1, ColorRGB color, widget_t **widget_head_node)
{
	widget_t **tmp_node = widget_head_node;

	widget_t *new_widget;
	new_widget = malloc(sizeof(widget_t));

	widget_plain_text_t *plain_text = malloc(sizeof(widget_plain_text_t));

	new_widget->type = PLAIN_TEXT;
	new_widget->widget.plain_text = plain_text;
	new_widget->next = NULL;
	
	plain_text->text1 = text1;
	plain_text->text2 = text2;
	plain_text->text3 = text3;
	plain_text->color = color;
	plain_text->x1 = x1;
	plain_text->y1 = y1;

	// add to window widget list
	while (*tmp_node) {
		tmp_node = &(*tmp_node)->next;
	}
	*tmp_node = new_widget;
}


