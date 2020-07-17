#include <stdlib.h>
#include <string.h>
#include "chaos/platform.h"
#include "chaos/porting.h"
#include "chaos/translate.h"

#define MAX_GAME_MENU 4
#include "chaos/options.h"
#include "chaos/touch.h"

#include "chaos/sound_data.h"
#include "chaos/text16.h"
#include "chaos/gamemenu.h"
#include "chaos/examine.h"
#include "chaos/players.h"
#include "chaos/casting.h"
#include "chaos/creature.h"
#include "chaos/wizards.h"
#include "chaos/chaos.h"
#include "chaos/gfx.h"
#include "chaos/arena.h"
#include "chaos/movement.h"

#include "chaos/cursor_defines.h"
#include "chaos/spellselect.h"

#define MENU_XPOS  4
#define MENU_YPOS  8
#define CHAOS_GAUGE_YPOS  4
static int s_hilite_item = 0;

static void deselect_game_item(uint8_t item)
{
	set_text16_colour(item, RGB16(0, 16, 16));
}

static void select_game_item(uint8_t item)
{
	set_text16_colour(item, RGB16(0, 31, 31));
}

int show_game_menu(void)
{

	int i;
	hilite_item = s_hilite_item;
	g_chaos_state.current_screen = SCR_GAME_MENU;
	clear_text();
	clear_arena();
	clear_palettes();
	clear_top_screen();

	set_text16_colour(12, RGB16(30, 31, 0));
	print_text16(players[g_chaos_state.current_player].name, 4, 2, 12);

	draw_decor_border(15, RGB16(31, 0, 0), RGB16(31, 31, 0));

	/* draw chaos/law val - colour 13 */
	if (g_chaos_state.world_chaos != 0) {
		uint8_t screen_x = MENU_XPOS;
		set_text16_colour(13, RGB16(31, 30, 0));

		if (g_chaos_state.world_chaos < 0) {
			/* chaos */
			print_text16(_("(CHAOS "), screen_x, CHAOS_GAUGE_YPOS,
					13);
			screen_x += 7;
			int A = g_chaos_state.world_chaos * -1;
			A >>= 2;

			if (A != 0) {
				for (i = 0; i < A; i++) {
					print_text16("*", screen_x++,
							CHAOS_GAUGE_YPOS, 13);
				}
			}

		} else {
			/* law */
			print_text16(_("(LAW "), screen_x, CHAOS_GAUGE_YPOS,
					13);
			screen_x += 5;
			int A = g_chaos_state.world_chaos;
			A >>= 2;

			if (A != 0) {
				for (i = 0; i < A; i++) {
					print_text16("+", screen_x++,
							CHAOS_GAUGE_YPOS, 13);
				}
			}

		}
		print_text16(")", screen_x, CHAOS_GAUGE_YPOS, 13);

	}

	for (i = 0; i <= 4; i++)
		deselect_game_item(i);

	select_game_item(hilite_item);
	int width, height;
	platform_screen_size(&width, &height);
	int incr = 3;
	if (height < 192)
		incr = 2;
	static const char *menu_entries[MAX_GAME_MENU + 1] = {
		T("EXAMINE SPELL"),
		T("SELECT SPELL"),
		T("EXAMINE BOARD"),
		T("CONTINUE WITH GAME"),
		T("QUIT GAME"),
	};
	for (i = 0; i <= MAX_GAME_MENU; i++) {
		int pos = MENU_YPOS + i * incr;
		print_text16(_(menu_entries[i]), MENU_XPOS + 2, pos, i);
	}
	return 0;
}

void game_menu_up(void)
{
	deselect_game_item(hilite_item);
	if (hilite_item > 0) {
		hilite_item--;
		play_soundfx(SND_MENU);
	}


	select_game_item(hilite_item);
}

void game_menu_down(void)
{
	deselect_game_item(hilite_item);
	if (hilite_item < MAX_GAME_MENU) {
		hilite_item++;
		play_soundfx(SND_MENU);
	}

	select_game_item(hilite_item);
}

static void intwrapper(void)
{
	show_game_menu();
}

static void do_quit_game(void)
{
	int i;
	for (i = 0; i <= 4; i++)
		deselect_game_item(i);
	int width, height;
	platform_screen_size(&width, &height);
	int pos = MENU_YPOS + 15;
	if (height < 192)
		pos = MENU_YPOS + 8;
	else if (height < 200)
		pos = MENU_YPOS + 12;
	get_yes_no(_("REALLY QUIT? "), 2, pos, 5,
		   RGB16(31, 31, 31),
		   RGB16(21, 21, 21),
		   quit_game, intwrapper);
}

void game_menu_a(void)
{
	/* get the hilited item... */
	play_soundfx(SND_CHOSEN);
	s_hilite_item = hilite_item;
	switch (hilite_item) {
		case 0:		/* examine spells */
			g_examining_only = 1;
			fade_down();
			show_spell_screen();
			fade_up();
			break;
		case 1:		/* view spells */
			g_examining_only = 0;
			fade_down();
			show_spell_screen();
			fade_up();
			break;
		case 2:		/* view arena */
			fade_down();
			examine_board();
			fade_up();
			break;
		case 3:		/* continue game */
			s_hilite_item = 0;
			continue_game();
			break;
		case 4:		/* continue game */
			s_hilite_item = 0;
			do_quit_game();
			break;
		default:
			break;
	}
}

void show_game_board(void)
{
	init_arena_tables();
	display_arena();
	g_chaos_state.current_player = 0;

	set_border_col(0);
	draw_game_border();
	draw_cursor(CURSOR_NORMAL_GFX);
	cursor_x = 0;
	cursor_y = 0;
	set_cursor_position(cursor_x, cursor_y, 0, 0);
}

/*
 * At the end of each cast/movement round, saves the game...
 */
void save_game(int screen)
{
	unsigned int idx;
	unsigned int cs = g_chaos_state.current_screen;
	size_t maxbuf = 400 * 8 + 6 * 2 * ARENA_SIZE;
	char *buffer = malloc(maxbuf);
	buffer[0] = 0;

	g_chaos_state.current_screen = screen;
	idx = 0;

	int version = CHAOS_SAVE_VERSION;
	if (screen == 0) {
		/* "delete" the save */
		version = 0;
	}

	set_save_version(&buffer[idx], version);
	idx += strlen(&buffer[idx]);
	players_to_str(&buffer[idx]);
	idx += strlen(&buffer[idx]);
	chaos_state_to_str(&buffer[idx]);
	g_chaos_state.current_screen = cs;
	idx += strlen(&buffer[idx]);
	arena_to_str(0, &buffer[idx]);
	idx += strlen(&buffer[idx]);
	arena_to_str(2, &buffer[idx]);
	idx += strlen(&buffer[idx]);
	arena_to_str(3, &buffer[idx]);
	idx += strlen(&buffer[idx]);
	arena_to_str(4, &buffer[idx]);
	idx += strlen(&buffer[idx]);
	arena_to_str(5, &buffer[idx]);
	idx += strlen(&buffer[idx]);
	platform_save_game(buffer, idx);
	if (idx >= maxbuf) {
		platform_dprint("Oh no!\n");
	}
	free(buffer);
}


void continue_game(void)
{
	fade_down();
	uint8_t next_player = get_next_human(g_chaos_state.current_player);
	if (next_player == 9) {
		/* continue game after spell selection... this is roughly at 95c7 */
		save_game(SCR_CASTING);
		show_game_board();
		fade_up();
		start_cast_round();
	} else {
		g_chaos_state.current_player = next_player;
		save_game(SCR_GAME_MENU);
		show_game_menu();
		fade_up();
	}
}

void game_menu_touch(int x, int y)
{
	int sqx, sqy;
	pixel_xy_to_square_xy(x, y, &sqx, &sqy);
	if (sqx < MENU_XPOS || sqx > (MENU_XPOS + 21))
		return;
	int row = (sqy - MENU_YPOS) / 3;
	if (row < 0 || row > MAX_GAME_MENU)
		return;
	deselect_game_item(hilite_item);
	hilite_item = row;
	select_game_item(hilite_item);
	game_menu_a();

}
