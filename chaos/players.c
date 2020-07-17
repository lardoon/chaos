/* players.c */
/* create the players */
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "chaos/platform.h"
#include "chaos/porting.h"
#include "chaos/translate.h"
#define PLAYER_WIZ_Y 1

#include "chaos/sound_data.h"
#include "chaos/players.h"
#include "chaos/gfx.h"
#include "chaos/chaos.h"
#include "chaos/gamemenu.h"
#include "chaos/wizards.h"
#include "chaos/creature.h"
#include "chaos/arena.h"
#include "chaos/spelldata.h"
#include "chaos/cursor_defines.h"
#include "chaos/computer.h"
#include "chaos/editname.h"
#include "chaos/options.h"
#include "chaos/touch.h"

#include "chaos/text16.h"
#include "chaos/input.h"
#include "chaos/splash.h"
#include "chaos/rand.h"

static int hilite_wizard_item;

static const char *const s_default_names[22] = {
	"Rich Q  ", "Fat Matt", "Colin   ", "Rich B  ", "Leek    ",
	"Dan     ",
	"Marce   ", "Smithy  ", "Kiff    ", "Ben     ", "Nick    ",
	"Tim     ",
	"Gandalf ", "H.Potter", "Houdini ", "Merlin  ", "Grotbags",
	"Venger  ",
	"Presto  ", "Saruman ", "Sooty   ", "Paul D  ",
};

static const uint8_t position_table[56] = {

	0x41, 0x4d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x17, 0x81, 0x8d, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x11, 0x1d, 0x81, 0x8d, 0x00, 0x00, 0x00, 0x00,
	0x07, 0x30, 0x3e, 0x93, 0x9b, 0x00, 0x00, 0x00,
	0x07, 0x10, 0x1e, 0x80, 0x97, 0x8e, 0x00, 0x00,
	0x07, 0x11, 0x1d, 0x60, 0x6e, 0x94, 0x9a, 0x00,
	0x00, 0x07, 0x0e, 0x40, 0x4e, 0x90, 0x97, 0x9e
};


/*/////////////////////////////////////////////////////// */
static void select_player_item(uint8_t item)
{
	anim_col = -31;
	anim_col_grad = -8;

	if (item == 0)
		set_text16_colour(10, RGB16(31, 30, 30));
	else
		set_text16_colour(item - 1, RGB16(31, 30, 30));
}

static void deselect_player_item(uint8_t item)
{
	if (item == 0)
		set_text16_colour(10, RGB16(0, 30, 30));
	else
		set_text16_colour(item - 1, RGB16(0, 30, 30));
}

static int array_contains(const unsigned char *array, unsigned char val)
{
	/* check the 8 long array to see if it contains val */
	/* return 1 if it does contain the val, return 0 if not */
	int i;
	for (i = 0; i < 8; i++) {
		if (array[i] == val)
			return 1;
	}
	return 0;
}


void create_default_wizs(void)
{
	/* creates 8 default wizards with sensible colours */
	/* names, type and colour are random and not repeated */
	int i;
	uint8_t names[8];
	uint8_t colours[8];
	uint8_t images[8];
	uint8_t tmp;
	for (i = 0; i < 8; i++) {
		names[i] = 0;
		colours[i] = 0;
		images[i] = 0;
	}


	for (i = 0; i < 8; i++) {
		tmp = 1 + GetRand(22);
		while (array_contains(names, tmp)) {
			tmp = 1 + GetRand(22);
		}
		names[i] = tmp;
		strncpy(players[i].name, s_default_names[names[i] - 1], sizeof(players[i].name));
		trim_whitespace(players[i].name);

		tmp = 1 + GetRand(8);
		while (array_contains(images, tmp)) {
			tmp = 1 + GetRand(8);
		}
		images[i] = tmp;
		players[i].image = images[i] - 1;

		tmp = 1 + GetRand(9);
		while (array_contains(colours, tmp)) {
			tmp = 1 + GetRand(9);
		}
		colours[i] = tmp;
		players[i].colour = colours[i] - 1;	/*chaos_cols[colours[i]-1]; */


		players[i].modifier_flag = 0;
		players[i].plyr_type = PLYR_CPU | (4 << 4);
	}

	players[0].plyr_type = PLYR_HUMAN;
}

static void update_players(void)
{
	/* use the global variables to update the amount of players shown... */
	int i, offset;
	char str[30];
	clear_arena();
	offset = platform_has_scroll() ? 0 : 1;
	/* clear the arena and redraaw it... */
	for (i = 0; i < 8; i++) {
		/* draw a wizard at this screen position */
		if (i < g_chaos_state.playercount) {
			set_player_col(i, chaos_cols[players[i].colour]);
			draw_wizard8(offset, (i * 2) + PLAYER_WIZ_Y + offset,
					players[i].image, 0, i);

			if (hilite_item - 1 == i)
				set_text16_colour(i, RGB16(31, 30, 30));
			else
				set_text16_colour(i, RGB16(0, 30, 30));
			print_text16(players[i].name, 6, 3 + i * 2, i);

			if (IS_CPU(i)) {
				strcpy(str, _("Computer "));
				char istr[3];
				int level = players[i].plyr_type >> 4;
				int2a(level, istr, 10);
				strcat(str, istr);
				print_text16(str, 16, 3 + i * 2, i);
			} else {
				print_text16(_("Human     "), 16, 3 + i * 2,
						i);
			}


		} else {
			set_text16_colour(i, RGB16(0, 0, 0));
		}

	}

	set_text16_colour(10, RGB16(0, 30, 30));
	int2a(g_chaos_state.playercount, str, 10);
	print_text16("<", 22, 1, 10);
	print_text16(str, 23, 1, 10);
	print_text16(">", 24, 1, 10);

}

static int generate_random_spell(int include_turmoil)
{
	int spellid = 0;
	int is_new = 0;
	int limit = SPELL_TURMOIL;
	if (include_turmoil)
		limit++;
	do {
		/* turmoil is not included. */
		spellid = GetRand(limit);
		is_new = (CHAOS_SPELLS[spellid].flags & NEW_FEATURE);
	}
	while (spellid < SPELL_KING_COBRA || spellid >= limit
			|| (!Options[OPT_NEW_FEATURES] && is_new));
	return spellid;
}

struct spell_sack {
	int size;
	uint8_t *contents;
};
static struct spell_sack s_spell_bag = {0, 0};

void init_spell_sack(void)
{
	/* initializes the sack to hold the spells,
	 * this sack contains "frequency" number of each spell.
	 * e.g. 5 cobras, 3 lightnings, etc.
	 */
	int i;
	int total = 0;
	if (s_spell_bag.contents)
		return;
	for (i = 0; i <= SPELL_TURMOIL; i++) {
		total += CHAOS_SPELLS[i].frequency;
	}
	s_spell_bag.size = total;
	s_spell_bag.contents = malloc(2 * total);
}

#if 0
void dumpsack(int count)
{
	printf("There are %d spells to show\n", count);
	int i;
	for (i = 0; i < count; i++) {
		int s = s_spell_bag.contents[1 + i * 2];
		int c = s_spell_bag.contents[i * 2];
		printf("%03d. %03d  %s\n", i, c, CHAOS_SPELLS[s].spellName);
	}

}
#endif

uint8_t *new_spell_bag(int minspell, int maxspell, int count)
{
	if (s_spell_bag.size == 0) {
		init_spell_sack();
	}
	int i;
	int idx = 0;
	for (i = minspell; i <= maxspell; i++) {
		int is_new = (CHAOS_SPELLS[i].flags & NEW_FEATURE);
		if ((!Options[OPT_NEW_FEATURES] && is_new))
			continue;
		int j;
		for (j = 0; j < CHAOS_SPELLS[i].frequency; j++, idx++) {
			s_spell_bag.contents[idx * 2] = GetRand(256);
			s_spell_bag.contents[1 + idx * 2] = i;
		}
	}
	order_table(idx, s_spell_bag.contents);
	uint8_t *bag = malloc(count);
	for (i = 0; i < count; i++) {
		bag[i] = s_spell_bag.contents[1 + 2 * i];
	}
	return bag;
}

void init_players(void)
{
	uint8_t loop;
	uint8_t k = 0;
	uint8_t cpulvl = 0;
	uint8_t offset = (g_chaos_state.playercount - 2) * 8;
	uint8_t square;
	/* in speccy chaos, the spell data for players starts at 7f47.  */
	/* For player 1, it goes 7f47 - 7f6e (inclusives) = 0x28 = 40 (20 spells) */
	for (loop = 0; loop < g_chaos_state.playercount; loop++) {

		cpulvl = players[loop].plyr_type >> 4;	/*cpu level, or 0 for humans */

		/* generate the player stats */
		players[loop].combat =
			(GetRand(10) >> 1) + 1 + (cpulvl >> 1);
		players[loop].ranged_combat = 0;
		players[loop].range = 0;
		players[loop].defence =
			(cpulvl >> 1) + (GetRand(10) >> 1) + 1;
		players[loop].movement_allowance = 1;
		players[loop].manoeuvre_rating =
			(cpulvl >> 2) + (GetRand(10) >> 1) + 3;
		players[loop].magic_resistance = (GetRand(10) >> 1) + 6;
		players[loop].spell_count =
			cpulvl + (GetRand(10) >> 1) + 0xB;
		if (players[loop].spell_count > 20)
			players[loop].spell_count = 20;



		players[loop].ability = 0;
		if ((5 - (cpulvl >> 1)) <= GetRand(10))
			players[loop].ability = GetRand(10) >> 2;

		/* change from index representation to the actual 15 bit colour value */
		players[loop].colour = chaos_cols[players[loop].colour];

		/* generate the spells... */
		uint8_t spellindex = 0;
		if (IS_CPU(loop)) {
			players[loop].spells[spellindex] = GetRand(10) + 0x0c;	/* disblv, random priority */
		} else {
			players[loop].spells[spellindex] = 0;	/* set to 0 for "no spell selected" */
			players[loop].selected_spell = 0;	/* set to 0 for "no spell selected" */
		}

		int spellid = SPELL_DISBELIEVE;
		spellindex++;
		players[loop].spells[spellindex] = spellid;
		spellindex++;
		int startIndex = 1;
		if (Options[OPT_NEW_FEATURES]) {
			spellid = SPELL_MEDITATE;
			players[loop].spells[spellindex] = CHAOS_SPELLS[spellid].castPriority;
			players[loop].spells[spellindex+1] = spellid;
			spellindex += 2;
			startIndex = 2;
		}

		uint8_t *bag = new_spell_bag(SPELL_KING_COBRA, SPELL_RAISE_DEAD, players[loop].spell_count);
		for (k = startIndex; k < players[loop].spell_count; k++) {
			spellid = bag[k - startIndex];
			players[loop].spells[spellindex] = CHAOS_SPELLS[spellid].castPriority;	/* priority */
			spellindex++;
			players[loop].spells[spellindex] = spellid;
			spellindex++;
		}
		free(bag);
#if 0
		spellindex++;
		spellid = SPELL_MAGIC_WOOD;
		players[loop].spells[spellindex] = spellid;
		players[loop].spell_count++;
		spellindex++;

		spellindex++;
		spellid = SPELL_WALL;
		players[loop].spells[spellindex] = spellid;
		players[loop].spell_count++;
		spellindex++;
#endif
		players[loop].spells[40] = 0;
		players[loop].spells[41] = 0;
		if (IS_CPU(loop)) {
			/* order the spells by priority   */
			order_table(20, players[loop].spells);
		}
		/* set the player positions.. */
		square = position_table[offset + loop];
		arena[0][square] = loop + WIZARD_INDEX;
		arena[3][square] = loop;

	}

	g_chaos_state.dead_wizards = 0;

}


int show_create_players(void)
{
	g_chaos_state.current_screen = SCR_CREATE_PLAYERS;
	clear_text();
	clear_bg(0);
	clear_palettes();
	load_bg_palette(9, 9);
	g_chaos_state.playercount = 2;
	hilite_item = 0;
	anim_col = 30;
	hilite_wizard_item = 0;

	print_text16(_("How many players?"), 4, 1, 10);
	set_text16_colour(10, RGB16(31, 30, 30));

	/* create the default start wizards */
	create_default_wizs();

	int x, y, i, offset;
	i = 1;
	offset = platform_has_scroll() ? 1 : 0;
	for (y = 1; y < 21; y++)
		for (x = 1; x < 31; x++)
			platform_set_map_entry(x + offset, y + offset, i++);
	for (y = 21; y < 24; y++)
		for (x = 1; x < 31; x++)
			platform_set_map_entry(x + offset, y + offset, 1);

	draw_decor_border(15, RGB16(0, 0, 31), RGB16(0, 31, 31));

	update_players();

	select_player_item(0);
	return 0;
}

/* used to just redraw, not reinit players */
int redraw_create_players(void)
{
	g_chaos_state.current_screen = SCR_CREATE_PLAYERS;
	clear_text();
	clear_palettes();
	load_bg_palette(9, 9);
	hilite_item = 0;
	anim_col = 30;
	hilite_wizard_item = 0;

	draw_decor_border(15, RGB16(0, 0, 31), RGB16(0, 31, 31));

	print_text16(_("How many players?"), 2, 1, 10);
	set_text16_colour(10, RGB16(0, 30, 30));
	update_players();

	hilite_item = g_chaos_state.current_player + 1;
	select_player_item(hilite_item);
	g_chaos_state.current_player = 0;
	return 0;
}

void create_players_up(void)
{
	deselect_player_item(hilite_item);
	anim_col = 30;
	if (hilite_item > 0)
		hilite_item--;

	select_player_item(hilite_item);
}

void create_players_down(void)
{
	deselect_player_item(hilite_item);
	anim_col = 30;
	if (hilite_item < g_chaos_state.playercount)
		hilite_item++;
	select_player_item(hilite_item);
}

void create_players_left(void)
{
	if (hilite_item == 0) {
		if (g_chaos_state.playercount > 2) {
			g_chaos_state.playercount--;
			play_soundfx(SND_MENU);
		}
	} else {
		if (IS_CPU(hilite_item - 1)) {
			/* get the level - stored in upper 4 bits */
			int level = players[hilite_item - 1].plyr_type >> 4;
			players[hilite_item - 1].plyr_type &= 0xf;
			level--;
			players[hilite_item - 1].plyr_type |= (level << 4);
			if (!level)
				players[hilite_item - 1].plyr_type =
					PLYR_HUMAN;
		}
		play_soundfx(SND_MENU);
	}
	update_players();

}

void create_players_right(void)
{
	if (hilite_item == 0) {
		if (g_chaos_state.playercount < 8) {
			g_chaos_state.playercount++;
			play_soundfx(SND_MENU);
		}
	} else {
		int level = players[hilite_item - 1].plyr_type >> 4;
		if (level < 8) {
			level++;
			players[hilite_item - 1].plyr_type =
				PLYR_CPU | (level << 4);
		}
		play_soundfx(SND_MENU);
	}
	update_players();
}

void create_players_l(void)
{
	/* change colour */
	if (hilite_item == 0)
		return;
	if (players[hilite_item - 1].colour < 8) {
		players[hilite_item - 1].colour++;
	} else {
		players[hilite_item - 1].colour = 0;
	}
	update_players();
}

void create_players_r(void)
{
	/* change image */
	if (hilite_item == 0)
		return;
	if (players[hilite_item - 1].image < 7) {
		players[hilite_item - 1].image++;
	} else {
		players[hilite_item - 1].image = 0;
	}
	update_players();
}

void create_players_accept(void)
{
	if (hilite_item == 0) {
		/* A prssed on the "How many players?" bit */
		/* reset the players!   */
		if (platform_key_pressed(CHAOS_KEY_L) && platform_key_pressed(CHAOS_KEY_R)) {
			create_default_wizs();
			clear_text();
			print_text16(_("How many players?"), 4, 1, 10);
			set_text16_colour(10, RGB16(0, 30, 30));
			update_players();
		}
	} else {
		/* go to the edit name screen */
		g_chaos_state.current_player = hilite_item - 1;
		fade_down();
		show_editname();
		fade_up();
	}
}

void create_players_start(void)
{
	play_soundfx(SND_CHOSEN);
	init_players();
	g_chaos_state.current_player = 0;
	g_chaos_state.round_count = 0;
	/* check if we need to show game menu... */
	if (IS_CPU(0))
		g_chaos_state.current_player = get_next_human(0);
	else
		g_chaos_state.current_player = 0;

	if (g_chaos_state.current_player == 9) {
		/* there is no human player! */

		continue_game();
	} else {
		fade_down();
		show_game_menu();
		fade_up();
	}

}

void create_players_back(void)
{
	fade_down();
	show_splash();
	fade_up();
}

unsigned char get_next_human(unsigned char id)
{
	int i;
	for (i = id + 1; i < g_chaos_state.playercount; i++) {
		if (!IS_CPU(i)
				&& !IS_WIZARD_DEAD(players[i].modifier_flag))
			return i;
	}
	return 9;
}


/* need to select/ unselect items */
/* hilite_item contains current selected item */
/* hilite_item: */
/* PLayercount (0) */
/* CPU level (1) */
/* wizard 1 (2) */
/* wizard 2 (3) */
/* ... */
/* wizard 8 (9) */

/* currently selected item should flash  */
void animate_player_screen(void)
{
	if (hilite_item == 0)
		anim_selection(10, 0, 31, 31);
	else
		anim_selection(hilite_item - 1, 0, 31, 31);
}

void new_random_spell(intptr_t arg)
{
	int lucky_player = (int)arg;
	if (lucky_player > 7)
		return;
	char str[30];
	player_data *lp = &players[lucky_player];
	strcpy(str, _("NEW SPELL FOR "));
	strncat(str, lp->name, sizeof(lp->name));
	print_text16(str, MESSAGE_X, g_message_y, 13);
	set_text16_colour(13, RGB16(30, 30, 0));	/* yellow */
	delay(100);

	/* generate new spell... */
	clear_message();
	uint8_t randspell = generate_random_spell(1);

	/* find an empty slot for the spell */
	int j, k = 3;
	for (j = 0; j < 0x13; j++) {
		if (lp->spells[k] == 0) {
			lp->spells[k] = randspell;
			/* CPU does not decrease spell count */
			if (!IS_CPU(lucky_player))
				lp->spell_count++;
			break;
		}
		k += 2;
	}
}

void create_players_touch(int x, int y)
{
	int sqx, sqy, okx, oky;
	platform_screen_size(&okx, &oky);
	okx /= 8;
	oky = (oky / 8) - 3;
	set_text16_colour(11, RGB16(31, 30, 30));
	const char *done = _("DONE");
	okx -= strlen(done) + 1;
	print_text16(done, okx, oky, 11);
	pixel_xy_to_square_xy(x, y, &sqx, &sqy);
	if (sqx >= okx && sqy >= oky) {
		create_players_start();
	}

	if (sqy < 3) {
		deselect_player_item(hilite_item);
		hilite_item = 0;
		select_player_item(hilite_item);
		/* How many players? N
		 * 4          15     22       */
		if (sqx < 23 && sqx > 15)
			create_players_left();
		if (sqx > 23 && sqx < 27)
			create_players_right();
	} else {
		int player = (sqy - 3) / 2;
		if (player >= g_chaos_state.playercount)
			return;
		deselect_player_item(hilite_item);
		hilite_item = player + 1;
		select_player_item(hilite_item);
		if (sqx <= 2)
			create_players_l();
		else if (sqx >= 3 && sqx <= 5)
			create_players_r();
		else if (sqx >= 7 && sqx <= 13)
			create_players_accept();
		else {
			int level = players[hilite_item - 1].plyr_type >> 4;
			if (level == 8) {
				players[hilite_item - 1].plyr_type = PLYR_HUMAN;
				play_soundfx(SND_MENU);
				update_players();
			} else {
				create_players_right();
			}
		}
	}
}

static void append_player(const player_data *p, char *buffer)
{
	int i;
	strcat(buffer, "PLAYER:\n");
	strcat(buffer, "\t""NAME \"");
	strcat(buffer, p->name);
	strcat(buffer, "\"\n");
	append_stat(buffer, "COMBAT", p->combat);
	append_stat(buffer, "RCOMBAT", p->ranged_combat);
	append_stat(buffer, "RANGE", p->range);
	append_stat(buffer, "DEFENCE", p->defence);
	append_stat(buffer, "MOVEMENT", p->movement_allowance);
	append_stat(buffer, "MANOEUVRE", p->manoeuvre_rating);
	append_stat(buffer, "MAGICRES", p->magic_resistance);
	append_stat(buffer, "SPELLC", p->spell_count);

	strcat(buffer, "\t""SPELLS ");
	for (i = 0; i < p->spell_count; i++) {
		char tmp[10];
		int idx = i * 2;
		int2a(p->spells[idx], tmp, 10);
		strcat(buffer, tmp);
		strcat(buffer, ",");
		int2a(p->spells[idx + 1], tmp, 10);
		strcat(buffer, tmp);
		strcat(buffer, ",");
	}
	strcat(buffer, "\n");

	append_stat(buffer, "ABILITY", p->ability);
	append_stat(buffer, "IMAGE", p->image);
	append_stat(buffer, "COLOUR", p->colour);
	append_stat(buffer, "TYPE", p->plyr_type);
	append_stat(buffer, "FLAG", p->modifier_flag);
	append_stat(buffer, "ILLUSION", p->illusion_cast);
	append_stat(buffer, "SELECTED_SPELL", p->selected_spell);
	append_stat(buffer, "LAST_SPELL", p->last_spell);
	append_stat(buffer, "TIMID", p->timid);
}

void players_to_str(char *buffer)
{
	int i;
	/* 500 bytes should be enough per wiz..*/
	buffer[0] = 0;
	for (i = 0; i < g_chaos_state.playercount; i++) {
		append_player(&players[i], buffer);
	}
}

static void parse_line(char *line, int player_idx)
{
	char *tmp = line;
	while (isspace((unsigned char)*tmp)) {
		tmp++;
	}
	const char *namestart = tmp;
	while (!isspace((unsigned char)*tmp))
		tmp++;
	char *nameend = tmp;
	while (isspace((unsigned char)*tmp)) {
		tmp++;
	}
	char *valstart = tmp;
	while (*tmp != '\n')
		tmp++;
	char *valend = tmp;
	char tmpvalend = *valend;
	char tmpnameend = *nameend;
	*valend = 0;
	*nameend = 0;
	player_data *p = &players[player_idx];
	if (strcmp(namestart, "NAME") == 0) {
		unsigned int i;
		const char *nametemp = valstart + 1;
		for (i = 0; i < sizeof(p->name)
		     && *nametemp != 0; i++) {
			if (nametemp[i] == '"') {
				p->name[i] = 0;
				break;
			}
			p->name[i] = nametemp[i];
		}
	} else if (strcmp(namestart, "COMBAT") == 0) {
		p->combat = strtol(valstart, 0, 0);
	} else if (strcmp(namestart, "RCOMBAT") == 0) {
		p->ranged_combat = strtol(valstart, 0, 0);
	} else if (strcmp(namestart, "RANGE") == 0) {
		p->range = strtol(valstart, 0, 0);
	} else if (strcmp(namestart, "DEFENCE") == 0) {
		p->defence = strtol(valstart, 0, 0);
	} else if (strcmp(namestart, "MOVEMENT") == 0) {
		p->movement_allowance = strtol(valstart, 0, 0);
	} else if (strcmp(namestart, "MANOEUVRE") == 0) {
		p->manoeuvre_rating = strtol(valstart, 0, 0);
	} else if (strcmp(namestart, "MAGICRES") == 0) {
		p->magic_resistance = strtol(valstart, 0, 0);
	} else if (strcmp(namestart, "SPELLC") == 0) {
		p->spell_count = strtol(valstart, 0, 0);
	} else if (strcmp(namestart, "SPELLS") == 0) {
		/* need to split on , :-( */
		char spellnumbuf[5];
		const char *spelltemp = valstart;
		int spellidx = 0;
		while (*spelltemp != 0) {
			int idx = 0;
			while (isdigit((unsigned char)*spelltemp)) {
				spellnumbuf[idx++] = *spelltemp;
				spelltemp++;
			}
			/* skip over commas */
			while (*spelltemp && !isdigit((unsigned char)*spelltemp)) {
				spelltemp++;
			}
			spellnumbuf[idx] = 0;
			p->spells[spellidx++] = strtol(spellnumbuf, 0, 0);
		}
	} else if (strcmp(namestart, "ABILITY") == 0) {
		p->ability = strtol(valstart, 0, 0);
	} else if (strcmp(namestart, "IMAGE") == 0) {
		p->image = strtol(valstart, 0, 0);
	} else if (strcmp(namestart, "COLOUR") == 0) {
		p->colour = strtol(valstart, 0, 0);
	} else if (strcmp(namestart, "TYPE") == 0) {
		p->plyr_type = strtol(valstart, 0, 0);
	} else if (strcmp(namestart, "FLAG") == 0) {
		p->modifier_flag = strtol(valstart, 0, 0);
	} else if (strcmp(namestart, "ILLUSION") == 0) {
		p->illusion_cast = strtol(valstart, 0, 0);
	} else if (strcmp(namestart, "SELECTED_SPELL") == 0) {
		p->selected_spell = strtol(valstart, 0, 0);
	} else if (strcmp(namestart, "LAST_SPELL") == 0) {
		p->last_spell = strtol(valstart, 0, 0);
	} else if (strcmp(namestart, "TIMID") == 0) {
		p->timid = strtol(valstart, 0, 0);
	}
	*valend = tmpvalend;
	*nameend = tmpnameend;
}

void parse_players(char *plystr)
{
	if (plystr == NULL)
		return;
	/* read until PLAYER: */
	/* then have NAME, etc... */
	int player_idx = -1;
	char *tmp = plystr;
	while (*tmp != 0) {
		char *linebegin = tmp;
		while (*tmp != '\n' && *tmp != 0) {
			tmp++;
		}
		if (*tmp == 0)
			break;
		tmp++;
		/* skip VERSION header first time through */
		if (player_idx == -1 && strncmp(linebegin, "VERSION", 7) == 0)
			continue;
		/*printf("linebegin: @%s@\n", linebegin);*/
		if (linebegin[0] != '\t') {
			if (strncmp(linebegin, "PLAYER:", 7) == 0)
				player_idx++;
			else
				break;
		} else {
			parse_line(linebegin, player_idx);
		}
	}
	g_chaos_state.playercount = player_idx + 1;
}
