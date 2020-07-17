/*

   examine.c

   arena viewer - used when "Examine Board" is clicked,
   also has the spell/creature data displaying routines

*/
#include <string.h>
#include "chaos/platform.h"
#include "chaos/porting.h"
#include "chaos/translate.h"

#include "chaos/arena.h"
#include "chaos/spelldata.h"
#include "chaos/spellselect.h"
#include "chaos/casting.h"
#include "chaos/creature.h"
#include "chaos/examine.h"
#include "chaos/chaos.h"
#include "chaos/arena.h"
#include "chaos/wizards.h"
#include "chaos/gfx.h"
#include "chaos/cursor_defines.h"
#include "chaos/gamemenu.h"
#include "chaos/input.h"
#include "chaos/text16.h"
#include "chaos/touch.h"

static const char *const stats_strings[] = {
	T("COMBAT="),
	T("RANGED COMBAT="),
	T("RANGE="),
	T("DEFENCE="),
	T("MOVEMENT ALLOWANCE="),
	T("MANOEUVRE RATING="),
	T("MAGIC RESISTANCE="),
	T("CASTING CHANCE="),	/* 7 */
	T("SPELLS="),		/* 8 */
	T("ABILITY="),		/* 9 */
};

/* define the positions of the stats */
typedef struct statpos {
	uint8_t startx;
	uint8_t starty;
} statpos;

static const statpos stats_postions[] = {
	{2, 5},			/* combat */
	{2, 7},			/* ranged combat */
	{2, 9},			/* range */
	{12, 5},			/* defence */
	{2, 11},			/* movement allowance */
	{2, 13},			/* manoeuvre rating */
	{2, 15},			/* magic resistance */
	{2, 17},			/* casting chance */
	{2, 17},			/* spells */
	{14, 17},			/* ability */
};

static void draw_stat(int idx, int namepal, int valuepal, int value)
{
	char statval[10];
	const char *statname = _(stats_strings[idx]);
	print_text16(statname,
		     stats_postions[idx].startx,
		     stats_postions[idx].starty, namepal);

	/* write the stat in white */
	int2a(value, statval, 10);
	if (idx == 7) {
		/* for casting chance, add a '%' symbol */
		strcat(statval, "/");
	}
	int statx = stats_postions[idx].startx + strlen(statname);
	print_text16(statval, statx,
		     stats_postions[idx].starty, valuepal);
}

static void draw_stats(const uint8_t * stat_pointer)
{
	int i;
	/* uint8_t stat; */
	/* loop over the 7 stats... this is a nice way to do it as it works for creatures and wizards,  */
	/* but it relies on structures being unpadded (devkitadvance doesn't pad to 4 byte boundaries) */
	for (i = 0; i < 7; i++) {
		/* write the description in lblue */
		int namepal = 2;
		/* write the stat in white */
		int valuepal = 3;
		draw_stat(i, namepal, valuepal, *stat_pointer);

		/* point to next stat */
		stat_pointer++;
	}
}

/* c419 */
/* to get everything, POKE ac16,FE or POKE 44054,254 */
static void display_wizard_data(uint8_t playerid)
{
	if (playerid > 7)
		return;

	/* print name in yellow */
	print_text16(players[playerid].name, 2, 1, 1);
	uint8_t need_comma = 0;
	uint8_t x = 2;
	if (players[playerid].modifier_flag & 0x2) {

		/* bit 1 set, "KNIFE" */
		const char *str = _("KNIFE");
		if (need_comma) {
			/* draw a comma... */
			print_text16(",", x, 3, 1);
			x++;
		}

		/* write value */
		print_text16(str, x, 3, 1);
		x += strlen(str);
		need_comma = 1;
	}
	if (players[playerid].modifier_flag & 0x4) {
		/* bit 2 set, */
		const char *str = _("SWORD");
		if (need_comma) {

			/* draw a comma... */
			print_text16(",", x, 3, 1);
			x++;
		}

		/* write value */
		print_text16(str, x, 3, 1);
		x += strlen(str);
		need_comma = 1;
	}
	if ((players[playerid].modifier_flag & 0xC0) == 0xC0) {

		/* "ARMOUR" */
		const char *str = _("ARMOUR");
		if (need_comma) {

			/* draw a comma... */
			print_text16(",", x, 3, 1);
			x++;
		}

		/* write value */
		print_text16(str, x, 3, 1);
		x += strlen(str);
		need_comma = 1;
	}
	if ((players[playerid].modifier_flag & 0xC0) == 0x40) {

		/* "SHIELD" */
		const char *str = _("SHIELD");
		if (need_comma) {

			/* draw a comma... */
			print_text16(",", x, 3, 1);
			x++;
		}

		/* write value */
		print_text16(str, x, 3, 1);
		x += strlen(str);
		need_comma = 1;
	}
	if (HAS_MAGICWINGS(players[playerid].modifier_flag)) {

		/* bit 5 */
		/* FLYING */
		const char *str = _("FLYING");
		if (need_comma) {

			/* draw a comma... */
			print_text16(",", x, 3, 1);
			x++;
		}

		/* write value */
		print_text16(str, x, 3, 1);
		x += strlen(str);
		need_comma = 1;
	}
	if (HAS_SHADOWFORM(players[playerid].modifier_flag)) {

		/* bit 3 */
		/* SHADOW */
		const char *str = _("SHADOW");
		if (need_comma) {

			/* draw a comma... */
			print_text16(",", x, 3, 1);
			x++;
		}

		/* write value */
		print_text16(str, x, 3, 1);
	}

	/* now draw the actual stats... */
	draw_stats(&players[playerid].combat);

	/* spells */
	uint8_t stat = 8;
	draw_stat(stat, 1, 3, players[playerid].spell_count);

	/* and ability... */
	stat = 9;
	draw_stat(stat, 1, 3, players[playerid].ability);
}


static void display_creature_data(uint8_t id, uint8_t arena4, uint8_t arena3)
{
	if (id != 0) {

		clear_text();
		draw_decor_border(15, RGB16(0, 31, 0), 0);
		if (id < WIZARD_INDEX) {
			/* creature */
			/* c479 */
			/* print the creature's name */
			const char *spellName = _(CHAOS_SPELLS[id].spellName);
			print_text16(spellName, 2, 1, 1);

			/* its chaos/law type */
			if (CHAOS_SPELLS[id].chaosRating != 0) {
				uint8_t screen_x = 3 + strlen(spellName);
				uint8_t col = 1;
				char str[30];
				if (CHAOS_SPELLS[id].chaosRating < 0) {

					/* chaos value, drawn in purple */
					col = 4;	/* purp */
					print_text16(_("(CHAOS "), screen_x,
							1, col);
					screen_x += 7;
					int2a((CHAOS_SPELLS[id].chaosRating * -1), str,
							10);
					print_text16(str, screen_x++, 1,
							col);
				} else {

					/* law, drawn in light blue */
					col = 2;	/* l blue */
					print_text16(_("(LAW "), screen_x, 1,
							col);
					screen_x += 5;
					int2a(CHAOS_SPELLS[id].chaosRating, str, 10);
					print_text16(str, screen_x++, 1,
							col);
				}
				print_text16(")", screen_x, 1, col);
			}	/* end chaos / law type display */
			uint8_t need_comma = 0;
			uint8_t x = 2;

			/* creature's special skills */
			if (id >= SPELL_HORSE && id <= SPELL_MANTICORE) {

				/* mount... */
				const char *str = _("MOUNT");
				if (need_comma) {

					/* draw a comma... */
					print_text16(",", x, 3, 5);
					x++;
				}

				/* write value */
				print_text16(str, x, 3, 5);
				x += strlen(str);
				need_comma = 1;
			}

			/* draw any "inside" wizards - for mounts or trees/castles */
			if (arena4 >= WIZARD_INDEX) {
				int player = arena4 - WIZARD_INDEX;
				if (player < 8) {
					print_text16("(", x, 3, 5);
					x++;
					print_text16(players[player].name, x, 3, 5);
					size_t namelength = strlen(players[player].name);
					if ((id >= SPELL_PEGASUS && id <= SPELL_GHOST) &&
					    (arena3 & 0x40) && namelength > 6) {
						/* undead flying mount - need to save space... */
						x += 6;
					} else {
						x += namelength;
					}
					print_text16(")", x, 3, 5);
					x++;
				}
			}
			if (id >= SPELL_PEGASUS && id <= SPELL_GHOST) {

				/* flying */
				const char *str = "FLYING";
				if (need_comma) {

					/* draw a comma... */
					print_text16(",", x, 3, 5);
					x++;
				}

				/* write value */
				print_text16(str, x, 3, 5);
				x += strlen(str);
				need_comma = 1;
			}
			if ((arena3 & 0x40) || (id >= SPELL_VAMPIRE
					&& id <= SPELL_ZOMBIE)) {

				/* undead... */
				const char *str = _("UNDEAD");
				if (need_comma) {
					/* draw a comma... */
					print_text16(",", x, 3, 5);
					x++;
				}

				/* write value */
				print_text16(str, x, 3, 5);
			}
			draw_stats(&CHAOS_SPELLS[id].combat);

			/* draw casting chance too if needed... */
			if (cast_chance_needed) {
				uint8_t stat = 7;
				draw_stat(stat, 2, 2, current_spell_chance * 10);
			}
		} else {

			/* wizard */
			/* c419 */
			display_wizard_data(id - WIZARD_INDEX);
		}
	}
}

void print_description(const char *description, int x, int y)
{
	if (description == 0)
		return;
	const char *tmp = description;
	int origx = x;
	while (*tmp) {
		const char *wrd = tmp;
		int pos = x;
		while (*wrd && *wrd != ' ') {
			wrd++;
			pos++;
		}

		if (pos > 29) {
			x = origx;
			y += 2;
		}
		wrd = tmp;
		while (*wrd && *wrd != ' ') {
			print_char16(*wrd, x, y, 1);
			wrd++;
			tmp++;
			x++;
		}
		if (*wrd) {
			print_char16(' ', x, y, 1);
			x++;
			tmp++;
		}
	}
}

static void display_spell_data(uint8_t id)
{

	/* this is used for displaying a spell sheet... */
	/* set up the colours */
	set_text16_colour(1, RGB16(30, 31, 0));	/* yel */
	set_text16_colour(2, RGB16(0, 31, 31));	/* light blue */
	set_text16_colour(3, RGB16(30, 31, 31));	/* white */
	set_text16_colour(4, RGB16(30, 0, 31));	/* purple */
	set_text16_colour(5, RGB16(0, 31, 0));	/* green */
	draw_decor_border(15, RGB16(0, 0, 31), RGB16(0, 31, 31));

	/* code from 94e2 */
	/* write spell name */
	uint8_t start_x = 4;
	uint8_t start_y = 4;
	int width, height;
	platform_screen_size(&width, &height);
	if (height < 192) {
		start_y = 2;
		start_x = 2;
	}
	print_text16(_(CHAOS_SPELLS[id].spellName), start_x,
			start_y, 1);
	start_y += 2;

	/* its chaos/law type */
	if (CHAOS_SPELLS[id].chaosRating != 0) {
		uint8_t screen_x = start_x;
		uint8_t col = 1;
		char str[30];
		if (CHAOS_SPELLS[id].chaosRating < 0) {

			/* chaos value, drawn in purple */
			col = 4;	/* purp */
			print_text16(_("(CHAOS "), screen_x, start_y, col);
			screen_x += 7;
			int2a((CHAOS_SPELLS[id].chaosRating * -1), str, 10);
			print_text16(str, screen_x++, start_y, col);
		} else {

			/* law, drawn in light blue */
			col = 2;	/* l blue */
			print_text16(_("(LAW "), screen_x, start_y, col);
			screen_x += 5;
			int2a(CHAOS_SPELLS[id].chaosRating, str, 10);
			print_text16(str, screen_x++, start_y, col);
		}
		print_text16(")", screen_x, start_y, col);
	}			/* end chaos / law type display */
	start_y += 3;

	/* casting chance... */
	const char *statname = _(stats_strings[7]);
	print_text16(statname, start_x, start_y, 5);
	char statval[10];
	int2a(current_spell_chance * 10, statval, 10);
	strcat(statval, "/");
	print_text16(statval, start_x + strlen(statname), start_y, 1);
	start_y += 3;

	/* casting range */
	statname = _(stats_strings[2]);
	print_text16(statname, start_x, start_y, 5);
	uint8_t A = CHAOS_SPELLS[id].castRange >> 1;
	if (A > 10)
		A = 20;
	int2a(A, statval, 10);
	print_text16(statval, start_x + strlen(statname), start_y, 1);

	start_y += 3;
	print_description(_(CHAOS_SPELLS[id].description), start_x, start_y);
}

static int event_display_spell_data(intptr_t arg)
{
	int id = (int)arg;
	display_spell_data(id);
	return 0;
}

/* examine creature on top screen */
static void examine_creature_top(unsigned char index)
{
	uint8_t arena0 = arena[0][index];
	uint8_t arena4 = arena[4][index];
	static int last_arena0 = -1;
	static int last_arena4 = -1;
	static int last_index = -1;

	if (!arena0)
		return;

	clear_arena();
	clear_text();
	clear_palettes();

	/* set up the text colours used */
	set_text16_colour(1, RGB16(30, 31, 0));	/* yel */
	set_text16_colour(2, RGB16(0, 31, 31));	/* light blue */
	set_text16_colour(3, RGB16(30, 31, 31));	/* white */
	set_text16_colour(4, RGB16(30, 0, 31));	/* purple */
	set_text16_colour(5, RGB16(0, 31, 0));	/* green */
	if (arena4 && last_index == index && last_arena0 == arena0 && last_arena4 == arena4) {
		display_creature_data(arena4, 0, arena[3][index]);
		last_index = -1;
	} else {
		display_creature_data(arena0, arena4, arena[3][index]);
		last_arena0 = arena0;
		last_arena4 = arena4;
		last_index = index;
	}
}

static int show_creature_arena0(uint8_t arena0,
				uint8_t arena3,
				uint8_t arena4)
{
	remove_cursor();
	clear_arena();
	clear_text();
	clear_palettes();

	/* set up the text colours used */
	set_text16_colour(1, RGB16(30, 31, 0));	/* yel */
	set_text16_colour(2, RGB16(0, 31, 31));	/* light blue */
	set_text16_colour(3, RGB16(30, 31, 31));	/* white */
	set_text16_colour(4, RGB16(30, 0, 31));	/* purple */
	set_text16_colour(5, RGB16(0, 31, 0));	/* green */
	display_creature_data(arena0, arena4, arena3);
	return 0;
}

static void examine_creature(intptr_t data)
{
	int index = (int)data;
	/* examine the given square of the arena... */
	/* c3b3 */
	uint8_t arena0 = arena[0][index];
	uint8_t arena3 = arena[3][index];
	uint8_t arena4 = arena[4][index];
	keypress_waiting(WAIT_FOR_LETGO);
	if (arena0) {

		/* clear arena screen... */
		fade_down();
		show_creature_arena0(arena0, arena3, arena4);
		fade_up();
		keypress_waiting(WAIT_FOR_KEYPRESS);
		keypress_waiting(WAIT_FOR_LETGO);
		if (arena4) {
			fade_down();
			display_creature_data(arena4, 0, arena3);
			fade_up();
			keypress_waiting(WAIT_FOR_KEYPRESS);
			keypress_waiting(WAIT_FOR_LETGO);
		}
	}
}

int examine_board(void)
{

	/* "examine board" selected on the menu... */
	disable_interrupts();
	display_arena();
	set_border_col(0);
	cursor_x = 7;
	cursor_y = 4;
	draw_cursor(CURSOR_NORMAL_GFX);
	set_cursor_position(cursor_x, cursor_y, 0, 0);
	redraw_cursor();
	enable_interrupts();
	g_chaos_state.current_screen = SCR_EXAMINE_BOARD;
	return 0;
}

void examine_back(void)
{
	fade_down();
	remove_cursor();
	show_game_menu();
	fade_up();
}

void clear_top_screen(void)
{
	if (platform_swap_screen() == 0) {
		clear_text();
		clear_palettes();
		platform_swap_screen();
	}
}

void examine_square(int index)
{
	int bgx, bgy;
	/* return instantly if we examine an empty square */
	if (arena[0][index] == 0) {
		return;
	}

	bgx = platform_get_x_scroll();
	bgy = platform_get_y_scroll();
	if (platform_swap_screen() == 0) {
		examine_creature_top(index);
		platform_swap_screen();
	} else {
		disable_interrupts();
		examine_creature(index);
		fade_down();
		enable_interrupts();
		display_arena();
		platform_set_x_scroll(bgx);
		platform_set_y_scroll(bgy);
		platform_update_scroll();
		clear_message();
		set_border_col(g_chaos_state.current_player);
		redraw_cursor();
		display_cursor_contents(index);
		fade_up();
	}
}

static int clear_all(void)
{
	remove_cursor();
	clear_arena();
	clear_text();
	clear_palettes();
	return 0;
}

void examine_spell(unsigned char index)
{
	arena[4][index] = 0;
	if (arena[0][index] > SPELL_MEDITATE
			&& arena[0][index] < SPELL_GOOEY_BLOB) {
		if (platform_swap_screen() == 0) {
			examine_creature_top(index);
			platform_swap_screen();
		} else {
			examine_creature(index);
		}
	} else {
		int id = arena[0][index];
		if (platform_swap_screen() != 0) {
			wait_for_letgo();
			fade_down();
			clear_all();
			event_display_spell_data(id);
			fade_up();
			wait_for_keypress();
			wait_for_letgo();
		} else {
			wait_for_letgo();
			clear_text();
			clear_palettes();
			event_display_spell_data(id);
			/* draw to main screen again */
			platform_swap_screen();
		}
	}
}


void examine_board_touch(int x, int y)
{
	int sqx, sqy;
	pixel_xy_to_square_xy(x, y, &sqx, &sqy);
	sqx -= 1;
	sqy -= 1;
	if (sqx < 0 || sqy < 0 || sqx >= 30 || sqy >= 20) {
		examine_back();
		return;
	}

	sqx /= 2;
	sqy /= 2;

	int two_screens = platform_swap_screen() == 0;
	platform_swap_screen();

	if (cursor_x == sqx && cursor_y == sqy) {
		examine_square(target_index);
	} else {
		move_cursor_to(sqx, sqy);
		if (two_screens) {
			if (arena[0][target_index] != 0) {
				examine_square(target_index);
			} else {
				clear_top_screen();
			}
		}
	}
}
