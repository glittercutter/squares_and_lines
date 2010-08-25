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
// menu.c -

#include "parse_public.h"
#include "parse_local.h"

#define VAR_NAME_POS 0
#define DEFAULT_VALUE_POS 1


/* 
====================
parse_get_default

Set default value to uninitialized variable.
====================
*/
void parse_get_default(struct Var_info* var_info, int tab_length)
{
	int** tmp_int;
	float** tmp_float;
	double** tmp_double;
	char** tmp_char;
	ColorRGB** tmp_color;
	int r, g, b;
	
	for (int i = 0; i < tab_length; i++) {
		if (var_info[i].init != TRUE) {
			printf("parse: getting default value for: %s -> ", 
					var_info[i].str[ VAR_NAME_POS ]);

			if (var_info[i].str[ DEFAULT_VALUE_POS ] == NULL) {
				printf("NULL\n");
				continue;
			}

			switch (var_info[i].type) {
			case COLOR_T:
				tmp_color = (ColorRGB **)&var_info[i].ptr;
				sscanf(var_info[i].str[ DEFAULT_VALUE_POS ], 
					"%d, %d, %d", &r, &g, &b);
				(**tmp_color).r = r; 
				(**tmp_color).g = g; 
				(**tmp_color).b = b; 

				DEBUG(printf("%d, %d, %d\n", (**tmp_color).r, 
					(**tmp_color).g, (**tmp_color).b));
				break;

			case INT_T:
				tmp_int = (int **)&var_info[i].ptr;
				**tmp_int = (int)strtod(var_info[i].str[ DEFAULT_VALUE_POS ], 
						NULL);
				
				DEBUG(printf("%d\n", **tmp_int));
				break;
			
			case FLOAT_T:
				tmp_float = (float **)&var_info[i].ptr;
				**tmp_float = (float)strtof(var_info[i].str[ DEFAULT_VALUE_POS ],
						NULL);
				
				DEBUG(printf("%f\n", **tmp_float));
				break;

			case DOUBLE_T:
				tmp_double = (double **)&var_info[i].ptr;
				**tmp_double = (double)strtod(var_info[i].str[ DEFAULT_VALUE_POS ],
						NULL);

				DEBUG(printf("%lf\n", **tmp_double));
				break;

			case STRING_T:
				tmp_char = (char**)&var_info[i].ptr;
				strcpy((char*)*tmp_char, (char*)var_info[i].str[DEFAULT_VALUE_POS]);
				
				DEBUG(printf("%s\n", (char *)*tmp_char));
				break;
			}
		}
	}
}


void parse_read(struct Var_info* var_info, int tab_length, char* filename)
{	
	int** tmp_int;
	float** tmp_float;
	double** tmp_double;
	char** tmp_char;
	ColorRGB** tmp_color;

	int r, g, b;
	double num_value;
	char string_value[ STRING_LENGTH ];
	char readed_line[ STRING_LENGTH * 2 ], readed_var_name[ STRING_LENGTH ];

	FILE* readed_file;
	readed_file = fopen(filename, "r");
	if (readed_file == NULL) {
		printf("parse: cannot read file; %s\n", filename);
		return;
	}

	while (fgets(readed_line, sizeof readed_line, readed_file) != NULL) {
		/*
		color
		Check for "string = int, int, int" formatting
		*/
		if (sscanf(readed_line, "%s = %d, %d, %d", readed_var_name, 
				&r, &g, &b) == 4) {
			for (int i = 0; i < tab_length; i++) {
				if (strncmp(readed_var_name, var_info[i].str[ VAR_NAME_POS ],
						strlen(readed_var_name)) == 0) {

					if (var_info[i].type == COLOR_T) {
						tmp_color = (ColorRGB **)&var_info[i].ptr;
						(**tmp_color).r = r;
						(**tmp_color).g = g;
						(**tmp_color).b = b;

						var_info[i].init = TRUE;
					}
					break;
				}
			}
		/*
		num
		Check for "string = numeric value" formatting
		*/
		} else if (sscanf(readed_line, "%s = %lf", readed_var_name, &num_value) == 2) {
			for (int i = 0; i < tab_length; i++) {
				// search matching variable
				if (strncmp(readed_var_name, var_info[i].str[ VAR_NAME_POS ],
						strlen(*var_info[i].str)) == 0) {
					switch (var_info[i].type) {
					case INT_T:
						tmp_int = (int **)&var_info[i].ptr;
						**tmp_int = (int)num_value;

						var_info[i].init = TRUE;
						break;
					
					case FLOAT_T:
						tmp_float = (float **)&var_info[i].ptr;
						**tmp_float = (float)num_value;

						var_info[i].init = TRUE;
						break;

					case DOUBLE_T:
						tmp_double = (double **)&var_info[i].ptr;
						**tmp_double = num_value;

						var_info[i].init = TRUE;
						break;
					}
					break;
				}
			}
		/*
		string
		Check for "string = string" formatting (spaces are allowed)
		*/
		} else if (sscanf(readed_line, "%s = %[^\n]s", readed_var_name, string_value) == 2) {
			for (int i = 0; i < tab_length; i++) {
				if (strncmp(readed_var_name, var_info[i].str[ VAR_NAME_POS ],
						strlen(readed_var_name)) == 0) {

					if (var_info[i].type == STRING_T) {
						tmp_char = (char **)&var_info[i].ptr;
						strncpy(*tmp_char, string_value, STRING_LENGTH -1);
						var_info[i].init = TRUE;
					}
					break;
				}
			}	
		}
	}

	fclose(readed_file);	
	return;	
}


int parse_write(struct Var_info *var_info, int tab_length, char* filename)
{
// TODO don't save if nothing changed ?
	int** tmp_int;
	float** tmp_float;
	double** tmp_double;
	char** tmp_char;
	ColorRGB** tmp_color;

	FILE* writed_file;
	writed_file = fopen(filename, "w");
	if (writed_file == NULL) {
		return 1;
	}
	
	for(int i = 0; i < tab_length; i++) {
		switch (var_info[i].type) {
		case INT_T:
			tmp_int = (int **)&var_info[i].ptr;
			fprintf(writed_file, "%s = %d\n", *var_info[i].str, **tmp_int);
			break;

		case FLOAT_T:
			tmp_float = (float **)&var_info[i].ptr;
			fprintf(writed_file, "%s = %f\n", *var_info[i].str, **tmp_float);
			break;

		case DOUBLE_T:
			tmp_double = (double **)&var_info[i].ptr;
			fprintf(writed_file, "%s = %lf\n", *var_info[i].str, **tmp_double);
			break;

		case STRING_T:	
			tmp_char = (char **)&var_info[i].ptr;
			fprintf(writed_file, "%s = %s\n", *var_info[i].str, *tmp_char);
			break;

		case COLOR_T:
			tmp_color = (ColorRGB **)&var_info[i].ptr;
			fprintf(writed_file, "%s = %d, %d, %d\n", *var_info[i].str, 
				(**tmp_color).r, (**tmp_color).g, (**tmp_color).b);
			break;
		}
	}

	fclose(writed_file);
	return 0;	
}

	
void load_config()
{
	parse_read(global_config_info, TAB_LENGTH(global_config_info), CONFIG_FILENAME);
	parse_get_default(global_config_info, TAB_LENGTH(global_config_info));
}

void save_config()
{
	if (parse_write(global_config_info, TAB_LENGTH(global_config_info),
			CONFIG_FILENAME)) {
		printf("parse: cannot save to file\n");
	}
}

void load_lang()
{
	parse_read(lang_info, TAB_LENGTH(lang_info), ui_language);
	parse_get_default(lang_info, TAB_LENGTH(lang_info));
}


