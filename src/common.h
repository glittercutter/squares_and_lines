#ifndef __COMMON_H__
#define __COMMON_H__

#include "shared.h"


// function
void eprint(const char *fmt, ...); 
int get_random_number(int);
int ipow(int, int);
int longest_string(char*, ...);
void com_add_string_node(string_list_t*, char*, ...);
int strlist_len(string_list_s*);

#endif
