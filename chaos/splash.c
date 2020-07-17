/* splash.c */
#include <string.h>
#include <stdlib.h>
#include "chaos/platform.h"
#include "chaos/porting.h"
#include "chaos/translate.h"
#include "chaos/casting.h"

#include "chaos/splash.h"
#include "chaos/options.h"
#include "chaos/players.h"
#include "chaos/gfx.h"
#include "chaos/text16.h"
#include "chaos/chaos.h"
#include "chaos/creature.h"
#include "chaos/touch.h"
#include "chaos/arena.h"
#include "chaos/players.h"
#include "chaos/gamemenu.h"
#include "chaos/movement.h"

static uint8_t splash_menu_on;
static uint8_t anim_pal;
static uint8_t splash_options_x;

static void select_splash_item(uint8_t item)
{
	int i;
	anim_pal = 12 + item;
	set_text16_colour(anim_pal, RGB16(31, 31, 0));
	for (i = 12 ; i < 15; i++) {
		if (i != anim_pal)
			set_text16_colour(i, RGB16(20, 20, 0));
	}
}

static void print_text16_centered(const char *text,
		int w, int y, int palette)
{
	int x = (w - strlen(text)) / 2;
	print_text16(text, x, y, palette);
}

int show_splash(void)
{
	int width, height;
	clear_palettes();
	load_all_palettes();
	clear_text();
	clear_arena();
	anim_col = -31;
	anim_col_grad = -8;
	hilite_item = 0;
	splash_menu_on = 0;
	g_chaos_state.current_screen = SCR_SPLASH;
	platform_screen_size(&width, &height);
	width /= 8;
	set_text16_colour(1, RGB16(31, 0, 31));
	print_text16_centered(_("CHAOS -THE BATTLE OF WIZARDS"), width, 2, 1);
	set_text16_colour(2, RGB16(31, 0, 0));
	print_text16_centered(_("By Julian Gollop"), width, 4, 2);
	set_text16_colour(3, RGB16(0, 30, 30));

	const char *pn = platform_name();
	char buffer[40];
	buffer[0] = 0;
	strcat(buffer, pn);
	strcat(buffer, _(" version"));
	print_text16_centered(buffer, width, 10, 3);
	print_text16_centered(_("by Quirky"), width, 12, 3);

#ifdef DEBUG
	print_text16_centered(_("DEBUG BUILD"), width, 12, 4);
	set_text16_colour(4, RGB16(0, 28, 0));

#endif
	set_text16_colour(12, RGB16(31, 31, 0));
	const char **key_names = platform_get_button_names();
	buffer[0] = 0;
	if (strcmp(key_names[CHAOS_KEY_START], "MENU") == 0) {
		strcat(buffer, _("Tap to Start"));
	} else {
		strcat(buffer, _("Press "));
		strcat(buffer, key_names[CHAOS_KEY_START]);
	}
	print_text16_centered(buffer, width, 17, 12);
	anim_pal = 12;
	draw_decor_border(15, RGB16(31, 0, 0), RGB16(31, 0, 31));
	return 0;
}

static int really_has_save(void)
{
	int has_save = platform_has_saved_game();
	if (!has_save)
		return 0;
	char *gamedata = platform_load_game();
	int save_version = get_save_version(gamedata);
	free(gamedata);
	return save_version == CHAOS_SAVE_VERSION;
}

void load_game(char *gamedata)
{
	if (load_arenas(gamedata) != 0) {
		reset_arena_tables();
		return;
	}
	parse_players(gamedata);
	str_to_chaos_state(gamedata);
}

static void create_or_load(void)
{
	if (hilite_item == 2 && really_has_save()) {
		fade_down();
		g_chaos_state.current_screen = SCR_CREATE_PLAYERS;
		char *gamedata = platform_load_game();
		load_game(gamedata);
		free(gamedata);
		if (g_chaos_state.current_screen == SCR_CREATE_PLAYERS) {
			/* something not loaded right */
			show_create_players();
			fade_up();
		} else {
			int cp = g_chaos_state.current_player;
			g_chaos_state.current_screen == SCR_GAME_MENU ? show_game_menu() : show_game_board();
			g_chaos_state.current_player = cp;
			fade_up();
			if (g_chaos_state.current_screen == SCR_MOVEMENT)
				start_movement_round();
			else if (g_chaos_state.current_screen == SCR_CASTING)
				start_cast_round();
		}
	} else {
		select_splash_item(0);
		fade_down();
		show_create_players();
		fade_up();
	}
}

/* start key pressed, for now start the game */
void splash_start(void)
{
	if (splash_menu_on == 0) {
		splash_menu_on = 1;
		hilite_item = 0;
		int width, height;
		int has_save = really_has_save();
		int ypos = 17;
		platform_screen_size(&width, &height);
		width /= 8;
		if (has_save && height < 192) {
			ypos = 15;
			print_text16("                    ", 2, ypos + 2, 12);
		}
		const char *txt = _("START");

		int x = (width - (strlen(txt) + 4 + strlen(_("OPTIONS")))) / 2;
		print_text16("                    ", 2, ypos, 12);
		print_text16(txt, x, ypos, 12);
		splash_options_x = x + strlen(txt) + 4;
		print_text16(_("OPTIONS"), splash_options_x, ypos, 13);

		if (has_save) {
			txt = _("CONTINUE");
			print_text16_centered(txt, width, ypos + 2, 14);
		}
		select_splash_item(0);
	} else {
		create_or_load();
	}
}


void splash_right(void)
{
	if (splash_menu_on == 0)
		return;
	if (hilite_item == 0) {
		hilite_item = 1;
		select_splash_item(hilite_item);
	}
}

void splash_up(void)
{
	if (splash_menu_on == 0)
		return;
	if (really_has_save()) {
		hilite_item = 1;
		select_splash_item(hilite_item);
	}
}

void splash_down(void)
{
	if (splash_menu_on == 0)
		return;
	if (really_has_save()) {
		hilite_item = 2;
		select_splash_item(hilite_item);
	}
}

void splash_left(void)
{
	if (splash_menu_on == 0)
		return;
	if (hilite_item == 1) {
		hilite_item = 0;
		select_splash_item(hilite_item);
	}
}

void splash_b(void)
{
	platform_exit();
}

void splash_a(void)
{
	if (splash_menu_on == 0) {
		splash_start();
		return;
	}
	if (hilite_item == 1) {
		fade_down();
		show_options();
		fade_up();
	} else {
		create_or_load();
	}
}

void splash_touch(int x, int y)
{
	if (splash_menu_on == 0) {
		splash_start();
		return;
	}
	int sqx, sqy;
	pixel_xy_to_square_xy(x, y, &sqx, &sqy);
	if (sqy >= 16 && sqy < 19) {
		if (sqx > 2 && sqx < (splash_options_x - 2)) {
			hilite_item = 0;
			splash_a();
		} else if (sqx > splash_options_x && sqx < (splash_options_x + 8)) {
			hilite_item = 1;
			splash_a();
		}
	} else if (sqy >= 19 && sqx > 12 && sqx < 21 && really_has_save()) {
		hilite_item = 2;
		splash_a();
	}
}

void animate_splash_screen(void)
{
	anim_selection(anim_pal, 31, 31, 0);
}

