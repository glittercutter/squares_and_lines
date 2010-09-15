#ifndef __COMMON_H__
#define __COMMON_H__

#include "shared.h"


void eprint(const char *fmt, ...); 
int get_random_number(int);
int ipow(int, int);
int longest_string(char*, ...);
string_list_s* add_string_node(string_list_t*);
void rm_string_node(string_list_t *tmp_node, string_list_s *node);
void clear_strlist(string_list_t*);
int strlist_len(string_list_s*);

#endif
