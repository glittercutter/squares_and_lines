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
// common.c -

#include "common.h"


void eprint(const char *fmt, ...) 
{
	va_list ap;
	char buf[512];
	va_start(ap, fmt);
	vsnprintf(buf, sizeof buf, fmt, ap);
	va_end(ap);
	fprintf(stderr, "%s", buf);
	if (fmt[0] && fmt[strlen(fmt) - 1] == ':')
		fprintf(stderr, " %s\n", strerror(errno));
	exit(EXIT_FAILURE);
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
	if (current_tick >= (last_tick + 1000)) {
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
	
	if (last_tick == current_tick) {
		++tick_instance;
		current_tick *= tick_instance; 
	} else {
		tick_instance = 0;
		last_tick = current_tick;
	}	
	srand(current_tick);	
	
	return rand() % max_number;
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


/* 
====================
add_string_node

Add element to a list
====================
*/
string_list_s* add_string_node(string_list_t *list_head)
{
	string_list_s **tmp_node = &list_head->list;
	string_list_s *new_node = calloc(1, sizeof(string_list_s));
	new_node->next = NULL;

	while (*tmp_node) { // find last node
		tmp_node = &(*tmp_node)->next;
	}
	*tmp_node = new_node;
	return new_node;
}

/* 
====================
rm_string_node

Remove element from a list
====================
*/
void rm_string_node(string_list_t *list_head, string_list_s *node)
{
	pthread_mutex_lock(&list_box_mutex);

	string_list_s **tmp_node = &list_head->list;

	if (list_head->list == node) {
		// first node
		if (node->next) {
			list_head->list = node->next;
		} else {
			list_head->list = NULL;
		}
		free(node);
	} else { 
		while (*tmp_node) {
			if ((*tmp_node)->next == node) {
				// replace with the next node
				(*tmp_node)->next = node->next;
				break;
			}
			tmp_node = &(*tmp_node)->next;
		}
		free(node);
	}
	pthread_mutex_unlock(&list_box_mutex);
}

void clear_strlist(string_list_t *list_head)
{
	string_list_s *tmp_node = list_head->list;
	string_list_s *next_node;

	while (tmp_node) {
		next_node = tmp_node->next;
		free(tmp_node);
		tmp_node = next_node;
	}
	list_head->list = NULL;
}


/* 
====================
strlist_len

Find list length
====================
*/
int strlist_len(string_list_s *strlist)
{
	int i = 0;

	while (strlist) {
		i++;
		strlist = strlist->next;
	}

	return i;
}


string_t* new_string(char* new_text)
{
	string_t *new_string = malloc(sizeof(string_t));
	new_string->size = strlen(new_text);
	new_string->text = malloc(sizeof new_string->size);	
	new_string->alloc = new_string->size;
	strcpy(new_string->text, new_text);
	return new_string;
}

void string_assign(string_t *str, char *new_text)
{
	int new_size = strlen(new_text);
	if (str->alloc < new_size) {
		free(str->text);
	   	str->text = malloc(sizeof new_size);
		str->alloc = new_size;
	}
	strcpy(str->text, new_text);
	str->size = new_size;
}

void string_cat(string_t *str, char *cat_text)
{
	int new_size = strlen(cat_text) + str->size;	
	if (str->alloc < new_size) {
		free(str->text);
	   	str->text = malloc(sizeof new_size);
		str->alloc = new_size;
	}
	strcat(str->text, cat_text);
	str->size = new_size;
}

void string_remove(string_t *str, int from, int to)
{
	char tmp_text[str->size];
	strcpy(tmp_text, str->text);
	strncpy(str->text, tmp_text, from);
	strncpy( (char*)&str->text[from], (char*)&tmp_text[to], str->size);
	str->size -= (to + 1) - from;
}


int string_grab(string_t *str)
{
	++str->refcount;
	return str->refcount;
}


int string_drop(string_t *str)
{
	if (!str->refcount) {
		free(str);
		return 0;
	}
	--str->refcount;
	return str->refcount;
}



