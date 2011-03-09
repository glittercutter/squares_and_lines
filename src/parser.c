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
// parser.c -

#include "parser_public.h"
#include "parser_local.h"


/* 
====================
parser_get_default

Set default value for uninitialized variables.
====================
*/
void parser_get_default(struct var_info_s* var_table, int tab_length)
{
	for (int i = 0; i < tab_length; i++) {
		if (!var_table[i].init) {
			if (var_table[i].str[ DEFAULT_VALUE_POS ] == NULL) {
				continue;
			}

			switch (var_table[i].type) {
			case COLOR_T:
				sscanf(var_table[i].str[ DEFAULT_VALUE_POS ], 
					"%d, %d, %d", 
					&(((colorRGB_t*)(var_table[i].ptr))->r), 
					&(((colorRGB_t*)(var_table[i].ptr))->g),
					&(((colorRGB_t*)(var_table[i].ptr))->b));
				break;

			case INT_T:
				*(int*)(var_table[i].ptr) =
					(int)strtod(var_table[i].str[ DEFAULT_VALUE_POS ], NULL);
				break;
			
			case FLOAT_T:
				*(float*)(var_table[i].ptr) =
					(float)strtof(var_table[i].str[ DEFAULT_VALUE_POS ], NULL);
				break;

			case DOUBLE_T:
				*(double*)(var_table[i].ptr) =
					strtod(var_table[i].str[ DEFAULT_VALUE_POS ], NULL);
				break;

			case STRING_T:
				strcpy((char*)(var_table[i].ptr),
					(char*)var_table[i].str[DEFAULT_VALUE_POS]);
				break;
			}
		}
	}
}


void parser_read(struct var_info_s* var_table, int tab_length, char* filename)
{	
	int r, g, b;
	double num_value;
	char str_value[ STRING_LENGTH ];
	char line_buffer[ STRING_LENGTH * 2 ], name_buffer[ STRING_LENGTH ];

	FILE* file;
	file = fopen(filename, "r");
	if (file == NULL) {
		printf("parser: cannot read file; %s\n", filename);
		return;
	}

	while (fgets(line_buffer, sizeof line_buffer, file) != NULL) {
		
		// color
		if (sscanf(line_buffer, "%s = %d, %d, %d", name_buffer, 
				&r, &g, &b) == 4) {
			for (int i = 0; i < tab_length; i++) {
				if (strncmp(name_buffer, var_table[i].str[ VAR_NAME_POS ],
						strlen(name_buffer)) == 0) {
					if (var_table[i].type == COLOR_T) {
						((colorRGB_t*)(var_table[i].ptr))->r = r;
						((colorRGB_t*)(var_table[i].ptr))->g = g;
						((colorRGB_t*)(var_table[i].ptr))->b = b;
						var_table[i].init = true;
					}
					break;
				}
			}
		
		// numbers
		} else if (sscanf(line_buffer, "%s = %lf", name_buffer, &num_value) == 2) {
			for (int i = 0; i < tab_length; i++) {
				// search matching variable
				if (strncmp(name_buffer, var_table[i].str[ VAR_NAME_POS ],
						strlen(*var_table[i].str)) == 0) {
					var_table[i].init = true;
					switch (var_table[i].type) {
					case INT_T:
						*(int*)(var_table[i].ptr) = (int)num_value;
						break;
					
					case FLOAT_T:
						*(float*)(var_table[i].ptr) = (float)num_value;
						break;

					case DOUBLE_T:
						*(double*)(var_table[i].ptr) = num_value;
						break;

					default:
						var_table[i].init = false;
					}
					break;
				}
			}
		
		// string
		} else if (sscanf(line_buffer, "%s = %[^\n]s", name_buffer, str_value) == 2) {
			for (int i = 0; i < tab_length; i++) {
				if (strncmp(name_buffer, var_table[i].str[ VAR_NAME_POS ],
						strlen(name_buffer)) == 0) {
					if (var_table[i].type == STRING_T) {
						strncpy((char*)(var_table[i].ptr), str_value, STRING_LENGTH -1);
						var_table[i].init = true;
					}
					break;
				}
			}	
		}
	}

	fclose(file);	
	return;	
}


int parser_write(struct var_info_s *var_table, int tab_length, char* filename)
{
	FILE* file;
	file = fopen(filename, "w");
	if (file == NULL) {
		return 1;
	}
	
	for (int i = 0; i < tab_length; i++) {
		switch (var_table[i].type) {
		case INT_T:
			fprintf(file, "%s = %d\n", *var_table[i].str, *(int*)(var_table[i].ptr));
			break;

		case FLOAT_T:
			fprintf(file, "%s = %f\n", *var_table[i].str, *(float*)(var_table[i].ptr));
			break;

		case DOUBLE_T:
			fprintf(file, "%s = %lf\n", *var_table[i].str, *(double*)(var_table[i].ptr));
			break;

		case STRING_T:	
			fprintf(file, "%s = %s\n", *var_table[i].str, (char*)(var_table[i].ptr));
			break;

		case COLOR_T:
			fprintf(file, "%s = %d, %d, %d\n", *var_table[i].str, 
				((colorRGB_t*)(var_table[i].ptr))->r,
				((colorRGB_t*)(var_table[i].ptr))->g,
				((colorRGB_t*)(var_table[i].ptr))->b);
			break;
		}
	}

	fclose(file);
	return 0;	
}

	
void load_config()
{
	parser_read(global_config_info, TAB_LENGTH(global_config_info), CONFIG_FILENAME);
	parser_get_default(global_config_info, TAB_LENGTH(global_config_info));
}

void save_config()
{
	if (parser_write(global_config_info, TAB_LENGTH(global_config_info),
			CONFIG_FILENAME)) {
		printf("parser: cannot save to file\n");
	}
}

void load_lang()
{
	parser_read(lang_info, TAB_LENGTH(lang_info), ui_language);
	parser_get_default(lang_info, TAB_LENGTH(lang_info));
}
