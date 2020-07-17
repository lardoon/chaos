#include <string.h>
#include <stdlib.h>
#include "chaos/platform.h"
#include "chaos/porting.h"
#include "chaos/translate.h"

#include "chaos/sound_data.h"
#include "chaos/spellselect.h"
#include "chaos/chaos.h"
#include "chaos/gfx.h"
#include "chaos/arena.h"
#include "chaos/examine.h"
#include "chaos/spelldata.h"
#include "chaos/casting.h"
#include "chaos/wizards.h"
#include "chaos/creature.h"
#include "chaos/text16.h"
#include "chaos/gamemenu.h"
#include "chaos/input.h"
#include "chaos/touch.h"
/* golden dragon */
#define MAX_SPELL_NAME_LEN 14
char cast_chance_needed;
static int s_top_offset;

struct spell_select_entry {
	int spell;
	int spellindex;
	int x;
	int y;
};

/* sizeof(spells) / 2. more or less. */
static struct spell_select_entry s_spell_select_entries[20];
static int s_spell_entries;
int g_examining_only = 0;

static const uint16_t s_casting_chance_palette[6] = {
	RGB16(31, 0, 0),	/*Red = 0-10%       p */
	RGB16(31, 0, 31),	/*purple = 20-30%    */
	RGB16(0, 31, 0),	/*Green = 40-50%     */
	RGB16(0, 31, 31),	/*LightBlue = 60-70% */
	RGB16(31, 31, 0),	/*Yellow = 80-90%    */
	RGB16(31, 31, 31),	/*White = 100%       */
};


static void set_spell_palette(void)
{
	uint8_t loop;
	for (loop = 0; loop < 6; loop++) {
		set_text16_colour(loop, s_casting_chance_palette[loop]);
	}
}

static int get_last_row(void)
{
	int w, h;
	platform_screen_size(&w, &h);
	return (h / 8) - 2;
}

static void generate_spell_string(char *str, uint8_t spellid)
{
	char chaoslvl = '-';
	if (CHAOS_SPELLS[spellid].chaosRating > 0)
		chaoslvl = '+';
	else if (CHAOS_SPELLS[spellid].chaosRating < 0)
		chaoslvl = '*';
	str[0] = chaoslvl;
	str[1] = 0;
	strcat(str, _(CHAOS_SPELLS[spellid].spellName));
}


static void print_spell_name(int spellid, int x, int y)
{
	/* parse the spell id into a proper spell name  */
	/* print it to the screen at the given location with the right colour */
	char str[30];
	current_spell = spellid;
	set_current_spell_chance();
	generate_spell_string(str, spellid);
	current_spell_chance++;
	print_text16("              ", x, y, current_spell_chance / 2);
	print_text16(str, x, y, current_spell_chance / 2);
}

static void print_spells(void)
{
	int i;
	int last_row = get_last_row();
	for (i = 2; i < last_row; i++) {
		int x;
		for (x = 0; x < 32; x++)
			platform_set_text_map_entry(x, i, 0);
	}
	/* for scroll, sub 2 */
	last_row -= 2;
	for (i = 0; i < s_spell_entries; i++) {
		int y;
		struct spell_select_entry *entry;
		entry = &s_spell_select_entries[i];
		y = entry->y + (s_top_offset * 2);
		if (y < 2 || y > last_row)
			continue;
		print_spell_name(entry->spell, entry->x, y);
	}
}

static void select_spell(uint8_t item)
{
	/* select the spell at index "item" */
	char str[30];
	struct spell_select_entry *entry;
	entry = &s_spell_select_entries[item];
	generate_spell_string(str, entry->spell);
	int y = entry->y + (2 * s_top_offset);
	print_text16(str, entry->x, y, 10);
	current_spell = entry->spell;
	set_current_spell_chance();
	current_spell_chance++;

	uint8_t r = GetRed(s_casting_chance_palette[current_spell_chance / 2]);
	uint8_t g = GetGreen(s_casting_chance_palette[current_spell_chance / 2]);
	uint8_t b = GetBlue(s_casting_chance_palette[current_spell_chance / 2]);

	r = r + 31;
	g = g + 31;
	b = b + 31;
	r = r > 31 ? 31 : r;
	g = g > 31 ? 31 : g;
	b = b > 31 ? 31 : b;
	set_text16_colour(10, RGB16(r, g, b));
	anim_col = -15;
	anim_col_grad = -8;

	int two_screens = (platform_swap_screen() == 0);
	platform_swap_screen();
	if (two_screens)
		spell_select_r();
}

static void deselect_spell(uint8_t item)
{
	int y;
	char str[30];
	struct spell_select_entry *entry;
	entry = &s_spell_select_entries[item];
	y = entry->y + (2 * s_top_offset);
	generate_spell_string(str, entry->spell);
	current_spell = entry->spell;
	set_current_spell_chance();
	current_spell_chance++;
	print_text16(str, entry->x, y, current_spell_chance / 2);
}


void spell_select_up(void)
{
	if (hilite_item < 2)
		return;

	deselect_spell(hilite_item);
	if ((hilite_item + (s_top_offset * 2)) < 2) {
		s_top_offset++;
		print_spells();
	}
	hilite_item -= 2;
	select_spell(hilite_item);
}

void spell_select_down(void)
{
	if ((hilite_item + 2) >= s_spell_entries)
		return;
	int last_row = get_last_row() - 4;
	deselect_spell(hilite_item);
	if ((hilite_item + (s_top_offset * 2)) >= last_row) {
		s_top_offset--;
		print_spells();
	}
	/* no need to scroll */
	hilite_item += 2;
	select_spell(hilite_item);
}

void spell_select_left(void)
{
	if ((hilite_item & 1) == 0)
		return;
	/* go to top of list */
	deselect_spell(hilite_item);
	hilite_item--;
	select_spell(hilite_item);
}

void spell_select_right(void)
{
	if (hilite_item & 1)
		return;
	/* go to top of list */
	deselect_spell(hilite_item);
	if ((hilite_item + 1) == s_spell_entries)
		hilite_item--;
	else
		hilite_item++;
	select_spell(hilite_item);
}

static int yes_no_touch_check(
		int msg_y,
		int yes_start, int yes_end,
		int no_start, int no_end)
{
	if (!platform_key_pressed(CHAOS_KEY_TOUCH)) {
		return 0;
	}
	int x = platform_touch_x();
	int y = platform_touch_y();
	int sqx, sqy;
	pixel_xy_to_square_xy(x, y, &sqx, &sqy);
	if (sqy >= msg_y && sqy <= (msg_y + 2)) {
		if (sqx >= yes_start && sqx < yes_end)
			return 2;
		if (sqx >= no_start && sqx < no_end)
			return 1;
	}
	return 0;
}

struct get_a_b_data {
	int y;
	int base_pal;
	int main_col;
	int not_selected_col;
	int selection;
	int yes_start;
	int yes_end;
	int no_start;
	int no_end;
	void (*yes_callback)(void);
	void (*no_callback)(void);
};

static void update_color(int yes_type, struct get_a_b_data *data)
{
	if (yes_type == 2) {
		set_text16_colour(data->base_pal + 1, data->main_col);
		set_text16_colour(data->base_pal + 2, data->not_selected_col);
	} else {
		set_text16_colour(data->base_pal + 1, data->not_selected_col);
		set_text16_colour(data->base_pal + 2, data->main_col);
	}
}

void get_a_b(struct get_a_b_data *data)
{
	int yes_pressed = 0;
	/* by default, NO is selected */
	while (!yes_pressed) {
		platform_wait();
		platform_update_keys();
		if (platform_key_pressed(CHAOS_KEY_LEFT)) {
			data->selection = 0;
			update_color(2, data);
		}
		int select_no = 0;
		if (platform_key_pressed(CHAOS_KEY_B)) {
			if (data->selection == 1) {
				yes_pressed = 1;
				break;
			}
			select_no = 1;
		}
		if (platform_key_pressed(CHAOS_KEY_RIGHT) || select_no) {
			data->selection = 1;
			update_color(1, data);
		}
		if (platform_key_pressed(CHAOS_KEY_A)) {
			yes_pressed = data->selection == 0 ? 2 : 1;
		}
		if (!yes_pressed) {
			yes_pressed = yes_no_touch_check(data->y,
					data->yes_start,
					data->yes_end,
					data->no_start,
					data->no_end);
			if (yes_pressed)
				update_color(yes_pressed, data);
		}
	}
	yes_pressed == 2 ? (data->yes_callback ? data->yes_callback() : 0) : (data->no_callback ? data->no_callback() : 0);
	free(data);
}

/**
 * Print a question and get a yes/no response. Blocks until a response is
 * given.
 *@param question text of the question
 *@param x x position of the question
 *@param y y position of the question
 *@param pal palette
 *@param yes_cb the func called when yes is chosen
 *@param no_cb the func called when no is chosen
 *
 */

static struct get_a_b_data* print_yes_no(const char *question, int x, int y, int pal,
				  int main_col, int not_selected_col,
				  void (*yes_cb)(void),
				  void (*no_cb)(void))
{
	int no_start, no_end, yes_start, yes_end;
	print_text16(question, x, y, pal);
	yes_start = strlen(question) + x;

	const char *yes = _("YES");
	int yessize = strlen(yes);
	print_text16(yes, yes_start, y, pal + 1);
	yes_end = yes_start + yessize;
	/* padding*/
	print_text16(" ", yes_start + yessize, y, pal);

	no_start = yes_end + 1;

	const char *no = _("NO");
	int nosize = strlen(no);
	no_end = no_start + nosize;
	print_text16(no, no_start, y, pal + 2);

	struct get_a_b_data *data = malloc(sizeof(struct get_a_b_data));
	data->y = y;
	data->base_pal = pal;
	data->selection = 1;
	data->main_col = main_col;
	data->not_selected_col = not_selected_col;
	data->yes_start = yes_start;
	data->yes_end = yes_end;
	data->no_start = no_start;
	data->no_end = no_end;
	data->yes_callback = yes_cb;
	data->no_callback = no_cb;

	return data;
}

void get_yes_no(const char *question, int x, int y, int pal,
		int main_col, int not_selected_col,
		void (*yes_cb)(void),
		void (*no_cb)(void))
{
	set_text16_colour(pal, main_col);	/* question */
	set_text16_colour(pal+1, not_selected_col);	/* YES */
	set_text16_colour(pal+2, main_col);	/* NO */
	struct get_a_b_data *data = print_yes_no(question, x, y, pal,
						main_col, not_selected_col,
				  yes_cb, no_cb);
	wait_for_letgo();
	get_a_b(data);
	wait_for_letgo();
}

static void choose_illusion(void)
{
	play_soundfx(SND_CHOSEN);
	players[g_chaos_state.current_player].illusion_cast = 1;
}

static void choose_real(void)
{
	play_soundfx(SND_CHOSEN);
	players[g_chaos_state.current_player].illusion_cast = 0;
}

static void show_illusion_screen(struct spell_select_entry *entry)
{
	clear_text();
	int w, h;
	platform_screen_size(&w, &h);

	draw_decor_border(15, RGB16(0, 31, 0), RGB16(0, 0, 0));
	/* Show ILLUSION? in the middle, more or less */
	int illusion_y = h / (3 * 8);
	print_spell_name(entry->spell, 5, illusion_y - 4);
	/* ask if they want an illusion and wait for response */
	print_text16(_("ILLUSION? "), 5, illusion_y, 12);
	print_description(_("Spells cast as illusions never fail but can be disbelieved"), 2, illusion_y + 6);
	get_yes_no("", 8, illusion_y + 2, 12,
		   RGB16(31, 31, 31),
		   RGB16(21, 21, 21),
		   choose_illusion,
		   choose_real);
	fade_down();
	show_game_menu();
	fade_up();
}

void spell_select_a(void)
{
	if (g_examining_only) {
		spell_select_r();
		return;
	}

	/* store the spell... */
	struct spell_select_entry *entry;
	entry = &s_spell_select_entries[hilite_item];
	players[g_chaos_state.current_player].illusion_cast = 0;
	players[g_chaos_state.current_player].selected_spell = entry->spellindex;

	/* check for illusion... */
	if (entry->spell < SPELL_KING_COBRA
		|| entry->spell >= SPELL_GOOEY_BLOB
		|| current_spell_chance == 10) {
		players[g_chaos_state.current_player].illusion_cast = 0;
		play_soundfx(SND_CHOSEN);
		fade_down();
		show_game_menu();
		fade_up();
		return;
	}
	show_illusion_screen(entry);
}

void spell_select_b(void)
{
	fade_down();
	show_game_menu();
	fade_up();
}

static int list_spells(void)
{
	int i, w;
	int last_row = get_last_row();
	platform_screen_size(&w, /* unused */&i);
	s_spell_entries = 0;

	int y = 2;
	/* |  XXXXX  XXXXXX  |
	 *  ^^     ^^      ^^
	 *       padding
	 */
	int padding = ((w / 8) - (2 * MAX_SPELL_NAME_LEN)) / 3;
	int x = padding;
	for (i = 0 ; i < 20; i++) {
		int index = 1 + i * 2;
		if (players[g_chaos_state.current_player].spells[index] != 0) {
			struct spell_select_entry *entry;
			entry = &s_spell_select_entries[s_spell_entries++];
			entry->spellindex = index;
			entry->spell = players[g_chaos_state.current_player].spells[index];
			entry->x = x;
			entry->y = y;
			x += 2 * padding + MAX_SPELL_NAME_LEN;
			if (x >= 20) {
				x = padding;
				y += 2;
			}
		}
	}
	s_top_offset = 0;
	print_spells();
	const char **key_names = platform_get_button_names();
	char buffer[40];
	buffer[0] = 0;
	if (strlen(key_names[CHAOS_KEY_TOUCH])) {
		const char *tmp = _(key_names[CHAOS_KEY_TOUCH]);
		strcat(buffer, tmp);
		strcat(buffer, _(" HERE OR "));
		tmp = _(key_names[CHAOS_KEY_B]);
		strcat(buffer, tmp);
		strcat(buffer, _(" TO GO BACK"));
	} else {
		strcat(buffer, _("PRESS "));
		strcat(buffer, key_names[CHAOS_KEY_B]);
		strcat(buffer, _(" TO GO BACK"));
	}
	print_text16(buffer, 1, last_row, 6);
	return 0;
}

static int restore_cast_chance(void)
{
	cast_chance_needed = 0;
	return 0;
}

static int restore_hilite_item(intptr_t arg)
{
	hilite_item = (int)arg;
	return 0;
}

static int restore_top_offset(intptr_t arg)
{
	s_top_offset = (int)arg;
	return 0;
}

static int restore_select_spell(intptr_t arg)
{
	select_spell((int)arg);
	return 0;
}

static int reset_arena15(void)
{
	arena[0][15] = 0;
	return 0;
}

void spell_select_r(void)
{
	cast_chance_needed = 1;
	struct spell_select_entry *entry;
	entry = &s_spell_select_entries[hilite_item];
	arena[0][15] = entry->spell;
	int to = s_top_offset;
	int hi = hilite_item;
	examine_spell(15);
	reset_arena15();
	if (platform_swap_screen() == 0) {
		platform_swap_screen();
	} else {
		fade_down();
		show_spell_screen();
		restore_cast_chance();
		restore_hilite_item(hi);
		restore_top_offset(to);
		list_spells();
		restore_select_spell(hi);
		fade_up();
	}
}

void anim_spell_select(void)
{
	current_spell = s_spell_select_entries[hilite_item].spell;
	set_current_spell_chance();
	current_spell_chance++;
	uint8_t r = GetRed(s_casting_chance_palette[current_spell_chance / 2]);
	uint8_t g = GetGreen(s_casting_chance_palette[current_spell_chance / 2]);
	uint8_t b = GetBlue(s_casting_chance_palette[current_spell_chance / 2]);
	anim_selection(10, r, g, b);
}

/* new code, this isn't from speccy chaos but */
/* helps when dealing with the list of spells */
void remove_null_spells(void)
{
	int i;
	uint8_t new_count = 0;
	uint8_t new_spells[20];
	for (i = 0; i < 20; i++) {
		if (players[g_chaos_state.current_player].spells[i * 2 + 1] != 0) {
			new_spells[new_count++] =
				players[g_chaos_state.current_player].spells[i * 2 + 1];
		}
	}

	/* copy the valid spells back again */
	for (i = 0; i < 20; i++) {
		if (i < new_count) {
			players[g_chaos_state.current_player].spells[i * 2] = 0x12;	/* default priority... */
			players[g_chaos_state.current_player].spells[i * 2 + 1] =
				new_spells[i];
		} else {
			players[g_chaos_state.current_player].spells[i * 2 + 1] = 0;
		}
	}
	players[g_chaos_state.current_player].spells[0] = 0;
	players[g_chaos_state.current_player].spell_count = new_count - 1;
}

int get_selected_spell_index(int row, int col)
{
	return row * 2 + col;
}

static void make_selection(int idx)
{
	deselect_spell(hilite_item);
	hilite_item = idx;
	select_spell(hilite_item);
}

void spell_select_touch(int x, int y)
{
	int sqx, sqy, last_row;
	pixel_xy_to_square_xy(x, y, &sqx, &sqy);
	last_row = get_last_row();
	int two_screens = (platform_swap_screen() == 0);
	platform_swap_screen();
	if (sqy >= 2 && sqy < last_row) {
		int row = (sqy - 2) / 2;
		int padding = s_spell_select_entries[0].x;
		int col = -1;
		if (sqx < (padding + MAX_SPELL_NAME_LEN))
			col = 0;
		else if (sqx > (2 * padding + MAX_SPELL_NAME_LEN))
			col = 1;
		if (col < 0)
			return;

		int idx = get_selected_spell_index(row, col);
		if (idx >= s_spell_entries)
			return;
		int already_selected = hilite_item == idx;
		if (!already_selected)
			make_selection(idx);
		if (!two_screens || (two_screens && !g_examining_only && already_selected)) {
			/* 1-screen versions, touch is immediate choice.
			 * 2-screen version, click twice to select. When
			 * examining never select, or would show info on both
			 * screens. */
			spell_select_a();
		}
	} else if (sqy >= last_row) {
		spell_select_b();
	}
}

/*////////////////////////////////////// */
int show_spell_screen(void)
{
	char str[30];
	hilite_item = 0;

	/* need to sort out the players spells here... */
	/* should remove all 0 spells and update spell count accordingly */
	if (!IS_CPU(g_chaos_state.current_player))
		remove_null_spells();
	g_chaos_state.current_screen = SCR_SELECT_SPELL;
	clear_text();
	clear_arena();
	clear_palettes();

	/* set up the palette so that 0-5 are the spell colours */
	set_spell_palette();

	/* 6 can be the default colour, 10 is the selected colour */
	strcpy(str, players[g_chaos_state.current_player].name);
	strcat(str, _("'S SPELLS"));

	print_text16(str, 0, 0, 6);
	set_text16_colour(6, RGB16(30, 30, 0));

	/* write all the spells */
	list_spells();
	select_spell(0);
	return 0;
}
