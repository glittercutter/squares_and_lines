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
	char buf[512];;
	va_start(ap, fmt);
	vsnprintf(buf, sizeof buf, fmt, ap);
	va_end(ap);
	fprintf(stderr, "%s", buf);
	if(fmt[0] && fmt[strlen(fmt) - 1] == ':')
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


/* 
====================
com_add_string_node

Add element to a list
====================
*/
void com_add_string_node(string_list_t *list_head, char *str1, ...)
{
	size_t alloc_char = 0; // lenght of all string
	char *tmp_char; // keep position for next string in allocated memory
	char *tmp_str = str1;
	string_list_s **tmp_node = &list_head->list;

	va_list ap;
	va_start(ap, str1);
	
	while (tmp_str) {
		alloc_char += strlen(tmp_str) + 1;
		tmp_str = va_arg(ap, char*);
	}
	tmp_char = calloc(alloc_char, sizeof(char));
	printf("alloc_char: %d\n", alloc_char);

	string_list_s *new_node = calloc(1, sizeof(string_list_s));
	new_node->next = NULL;

	va_end(ap);
	va_start(ap, str1);

	tmp_str = str1;
	for (int i = 0; tmp_str != NULL && i < LS_MAX_STRING; i++) {
		printf("i: %d\n", i);
		printf("string: %s\n", tmp_str);
		printf("string: %s\n", tmp_str);
		strcpy(tmp_char, tmp_str);
		new_node->string[i] = tmp_char;
		tmp_char += strlen(tmp_str) + 1;
		tmp_str = va_arg(ap, char*);
	}

	if (!*tmp_node) { // we are the first node
		printf("first node\n");
		*tmp_node = new_node;
		va_end(ap);
		return;
	}
	int i = 0;
	while (*tmp_node) { // find last node
		i++;
		tmp_node = &(*tmp_node)->next;
	}
	*tmp_node = new_node;
	printf("node: %d\n", i);
	va_end(ap);
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
	printf("len: %d\n", i);
	return i;
}


