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
// parser_local.h - 

#ifndef PARSER_LOCAL_H
#define PARSER_LOCAL_H

#include "shared.h"
#include "parser_public.h"

#include "editor.h"
#include "game.h"
#include "ui.h"

// =========================================================================
// Config
// =========================================================================

// string position for varible name and default value in array
enum {
	VAR_NAME_POS = 0,
	DEFAULT_VALUE_POS = 1
};

//	{"name", "default value"}

static const char* display_width_str[] =
	{"display_width", "640"};
static const char* display_height_str[] =
	{"display_height", "480"};
static const char* display_fullscreen_str[]	=
	{"display_fullscreen", "0"};
static const char* display_allow_idle_str[]	=
	{"display_allow_idle", "1"};

static const char* button_font_name_str[] =
	{"button_font_name", "font/EnvyCodeR.ttf"};
static const char* button_font_size_str[] =
	{"button_font_size", "12"};
static const char* color_button_highlight_str[]	= 
	{"button_highlight_color", "50, 51, 50"};

static const char* ui_language_str[] =
	{"language", "lang/en"};

static const char* ed_square_size_str[] =
	{"editor_square_size", "20"};
static const char* g_line_thickness_str[] =
	{"game_line_thickness", "1"};

static const char* color_own_player0_str[] =
	{"color_game_player0", "66, 162, 206"};
static const char* color_own_player1_str[] =
	{"color_game_player1", "206, 49, 0"};
static const char* color_own_outline_str[] =
	{"color_game_outline", "145, 150, 145"};
static const char* color_own_none_str[]	=
	{"color_game_none", "50, 51, 50"};
static const char* color_square_background_str[]	=
	{"color_game_square_background", "99, 101, 99"};

static const char* color_ed_outline_str[] =
	{"color_editor_outline", "145, 155, 145"};
static const char* color_ed_grid_str[] =
	{"color_editor_grid", "99, 101, 99"};

static const char* color_text_str[]	=
	{"color_txt_normal", "200, 200, 200"};
static const char* color_txt_current_player_str[] = 
	{"color_txt_current_player", "0, 0, 0"};

static const char* color_topbar_str[] = 
	{"color_gui", "99, 101, 99"};
static const char* color_gui_3d_dark_str[] = 
	{"color_gui_3d_dark", "0, 0, 0"};
static const char* color_gui_3d_light_str[] = 
	{"color_gui_3d_light", "200, 200, 200"};


// type
enum {
	INT_T,
	FLOAT_T,
	DOUBLE_T,
	COLOR_T,
	STRING_T,
	DSTRING
};

/*
str: varname, 
ptr: pointer to the var, 
type: type, 
init: value assigned by the parser, load default otherwise
*/
struct var_info_s {
	const char** str;
	const void* ptr;
	const int type;
	bool init;
};

static struct var_info_s global_config_info[] = {
	
	//	NAME						VAR (ptr)							TYPE
	{ display_width_str,			&display_width,						INT_T		},
	{ display_height_str,			&display_height,					INT_T		},
	{ display_fullscreen_str,		&display_fullscreen,				INT_T		},
	{ display_allow_idle_str,		&display_allow_idle,				INT_T		},
	
	{ ed_square_size_str,			&ed_square_size,					INT_T		},
	{ g_line_thickness_str,			&g_line_thickness,					INT_T		},

	{ button_font_name_str,			&button_font.name,					STRING_T	},
	{ color_button_highlight_str,	&color.button_highlight,			COLOR_T		},

	{ ui_language_str,				&ui_language,						STRING_T	},

	{ color_own_player0_str,		&color.square_owner[PLAYER_1],		COLOR_T		},
	{ color_own_player1_str,		&color.square_owner[PLAYER_0],		COLOR_T		},
	{ color_own_outline_str,		&color.square_owner[OUTLINE],		COLOR_T		},
	{ color_own_none_str,			&color.square_owner[NONE],			COLOR_T		},
	{ color_square_background_str,	&color.square_background,			COLOR_T		},
	
	{ color_ed_outline_str,			&color.ed_outline,					COLOR_T		},
	{ color_ed_grid_str,			&color.ed_grid,						COLOR_T		},
	
	{ color_text_str,				&color.text,						COLOR_T		},
	{ color_txt_current_player_str,	&color.txt_current_player,			COLOR_T		},

	{ color_topbar_str,				&color.topbar,						COLOR_T		},
	{ color_gui_3d_dark_str,		&color.gui_3d_dark,					COLOR_T		},
	{ color_gui_3d_light_str,		&color.gui_3d_light,				COLOR_T		},

	{ button_font_size_str,			&button_font.size,					INT_T		},


};

// =========================================================================
// Language
// =========================================================================

static const char* lang_play_str[]				= {"play", "Play"};
static const char* lang_quit_str[]				= {"quit", "Quit"};
static const char* lang_main_menu_str[]			= {"main_menu", "Menu"};
static const char* lang_option_str[]			= {"option", "Option"};
static const char* lang_new_game_str[]			= {"new_game", "New game"};

static const char* lang_multiplayer_str[]		= {"multiplayer", "Multiplayer"};
static const char* lang_host_game_str[]			= {"host_game", "Host game"};
static const char* lang_join_game_str[]			= {"join_game", "Join game"};
static const char* lang_disconnect_str[]		= {"disconnect", "Disconnect"};

static const char* lang_random_str[]			= {"random", "Random"};
static const char* lang_score_str[]				= {"score", "Score"};
static const char* lang_player_str[]			= {"player", "Player"};
static const char* lang_win_str[]				= {"win", "win"};
static const char* lang_no_win_str[]			= {"no_win", "Draw!"};

static const char* lang_lbox_server_str[]		= {"lbox_server", "Server"};
static const char* lang_lbox_ping_str[]			= {"lbox_ping", "Ping"};
static const char* lang_lbox_player_str[]		= {"lbox_player", "Player"};

static const char* lang_txt_srv_name_is_str[]	= {"txt_srv_name_is", "Server name: "};
static const char* lang_txt_player_name_is_str[]= {"txt_player_name_is", "Player name: "};
static const char* lang_configure_str[]			= {"configure", "Configure"};
static const char* lang_join_str[]				= {"join", "Join"};
static const char* lang_update_str[]			= {"update", "Update"};
static const char* lang_cl_connected_str[]		= {"cl_connected", "Joined game"};
static const char* lang_srv_full_str[]			= {"srv_full", "Server is full"};

static const char* lang_warning_select_square_str[]		= {"warning_select_squares", "Select more square"};
static const char* lang_warning_option_str[]			= {"warning_option", "Open the file game.conf with a text editor"};
static const char* lang_warning_no_client_str[]			= {"warning_no_client", "No client connected!"};
static const char* lang_warning_no_server_selected_str[]= {"warning_no_server_selected", "No server selected!"};
static const char* lang_warning_only_server_str[]		= {"warning_only_server", "Only the server can do this"};


static struct var_info_s lang_info[] = {
	
	//	NAME						VAR (ptr)					TYPE
	{	lang_play_str,				&text.play,					STRING_T		},
	{	lang_quit_str,				&text.quit,					STRING_T		},
	{	lang_main_menu_str,			&text.main_menu,			STRING_T		},
	{	lang_option_str,			&text.option,				STRING_T		},
	{	lang_new_game_str,			&text.new_game,				STRING_T		},
	
	{	lang_multiplayer_str,		&text.multiplayer,			STRING_T		},
	{	lang_host_game_str,			&text.host_game,			STRING_T		},
	{	lang_join_game_str,			&text.join_game,			STRING_T		},
	{	lang_disconnect_str,		&text.disconnect,			STRING_T		},

	{	lang_random_str,			&text.random,				STRING_T		},
	{	lang_score_str,				&text.score,				STRING_T		},
	{	lang_player_str,			&text.player,				STRING_T		},
	{	lang_win_str,				&text.win,					STRING_T		},
	{	lang_no_win_str,			&text.no_win,				STRING_T		},

	{	lang_lbox_server_str,		&text.lbox_server,			STRING_T		},
	{	lang_lbox_ping_str,			&text.lbox_ping,			STRING_T		},
	{	lang_lbox_player_str,		&text.lbox_player,			STRING_T		},

	{	lang_txt_srv_name_is_str,	&text.txt_srv_name_is,		STRING_T		},
	{	lang_txt_player_name_is_str,&text.txt_player_name_is,	STRING_T		},
	{	lang_configure_str,			&text.configure,			STRING_T		},
	{	lang_join_str,				&text.join,					STRING_T		},
	{	lang_update_str,			&text.update,				STRING_T		},
	{	lang_cl_connected_str,		&text.cl_connected,			STRING_T		},
	{	lang_srv_full_str,			&text.srv_full,				STRING_T		},
	
	{	lang_warning_select_square_str,		&text.warning_select_square,	STRING_T},
	{	lang_warning_option_str,			&text.warning_option,			STRING_T},
	{	lang_warning_no_client_str,			&text.warning_no_client,		STRING_T},
	{	lang_warning_no_server_selected_str,&text.warning_no_server_selected,	STRING_T},
	{	lang_warning_only_server_str,		&text.warning_only_server,		STRING_T},

};


#endif // PARSER_LOCAL_H
