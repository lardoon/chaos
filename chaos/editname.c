#include <string.h>
#include "chaos/platform.h"
#include "chaos/porting.h"
#include "chaos/translate.h"
#include "chaos/sound_data.h"
#include "chaos/editname.h"
#include "chaos/chaos.h"
#include "chaos/gfx.h"
#include "chaos/wizards.h"
#include "chaos/text16.h"
#include "chaos/players.h"
#include "chaos/touch.h"
static char name_string[12];

static void index_to_position(uint8_t letter, uint8_t * x, uint8_t * y, char *c)
{

	/* 0 - 26 = A - Z */
	/* 27 - 52 = a - z */
	/* position = A = (2,6) B = (4,6)... M = 26,6 */
	/* i.e. A-M = (letter - startx * 2), 6 */
	if (letter <= 12) {
		*x = (letter) * 2;
		*y = 6;
		*c = 'A' + letter;
	}
	if (letter >= 13 && letter <= 25) {
		*x = (letter - 13) * 2;
		*y = 8;
		*c = 'N' + letter - 13;
	}
	if (letter >= 26 && letter <= 38) {
		*x = (letter - 26) * 2;
		*y = 10;
		*c = '_';
	}
	if (letter >= 39 && letter <= 51) {
		*x = (letter - 39) * 2;
		*y = 12;
		*c = 'a' + letter - 39;
	}
	if (letter >= 52 && letter <= 64) {
		*x = (letter - 52) * 2;
		*y = 14;
		*c = 'n' + letter - 52;
	}
	*x += 2;
}

static void select_letter(uint8_t letter)
{
	if (letter > 64) {

		/* OK hilited */
		set_text16_colour(13, RGB16(0, 30, 0));	/* white */
	} else {
		uint8_t x = 2;
		uint8_t y = 6;
		char c;
		index_to_position(letter, &x, &y, &c);
		char str[3];
		str[0] = c;
		str[1] = 0;

		/*    sprintf(str, "%c", c); */
		print_text16(str, x, y, 11);
	}
}


/* convert a hilite index to the lette and x, y position */
static void update_name(void)
{
	print_text16("             ", 4, 2, 12);
	print_text16(name_string, 4, 2, 12);
	uint8_t len = strlen(name_string);
	if (len < 11) {
		print_text16("_", 4 + len, 2, 12);
	}
}

static void deselect_letter(uint8_t letter)
{
	if (letter > 64) {

		/* OK hilited */
		set_text16_colour(13, RGB16(31, 30, 30));	/* white */
	} else {
		uint8_t x = 2;
		uint8_t y = 6;
		char c;
		index_to_position(letter, &x, &y, &c);
		char str[3];
		if (c == '_')
			c = ' ';

		/*    sprintf(str, "%c", c); */
		str[0] = c;
		str[1] = 0;
		print_text16(str, x, y, 10);
	}
}

int show_editname(void)
{

	/* the edit name screen...  */
	g_chaos_state.current_screen = SCR_EDIT_NAME;
	clear_text();
	clear_palettes();
	strcpy(name_string, players[g_chaos_state.current_player].name);
	hilite_item = 71;

	/* draw the alphabet... */
	update_name();
	print_text16("A B C D E F G H I J K L M", 2, 6, 10);
	print_text16("N O P Q R S T U V W X Y Z", 2, 8, 10);
	print_text16("                         ", 2, 10, 10);
	print_text16("a b c d e f g h i j k l m", 2, 12, 10);
	print_text16("n o p q r s t u v w x y z", 2, 14, 10);
	print_text16(_("OK"), 13, 16, 13);
	set_text16_colour(10, RGB16(0, 30, 30));	/* l blue */
	set_text16_colour(11, RGB16(31, 30, 30));	/* white */
	set_text16_colour(12, RGB16(31, 30, 0));	/* yellow */
	set_text16_colour(13, RGB16(31, 30, 30));	/* white */
	select_letter(hilite_item);
	draw_decor_border(15, RGB16(0, 0, 31), RGB16(0, 31, 31));
	return 0;
}


void edit_name_up(void)
{

	/* move up a row... */
	deselect_letter(hilite_item);
	if (hilite_item > 12) {
		hilite_item -= 13;
	} else {
		hilite_item = 71;
	}
	select_letter(hilite_item);
}

void edit_name_down(void)
{

	/* move down a row... */
	deselect_letter(hilite_item);
	if (hilite_item < 52) {
		hilite_item += 13;
	} else {

		/*hilite_item -= 13*4; */
		/* select OK */
		hilite_item = 71;
	}
	select_letter(hilite_item);
}

void edit_name_left(void)
{
	deselect_letter(hilite_item);

	/* get the multiple of 13.. */
	uint8_t this_row_index = hilite_item;
	while (this_row_index - 13 >= 0) {
		this_row_index -= 13;
	}
	if (this_row_index > 0) {

		/* not on the zeroth column */
		hilite_item--;
	} else {

		/* on the zeroth col, so move to the end of this row */
		hilite_item += 12;
	}
	select_letter(hilite_item);
}

void edit_name_right(void)
{
	deselect_letter(hilite_item);

	/* get the multiple of 12.. */
	uint8_t this_row_index = hilite_item;
	while (this_row_index - 13 >= 0) {
		this_row_index -= 13;
	}
	if (this_row_index < 12) {
		hilite_item++;
	} else {
		hilite_item -= 12;
	}
	select_letter(hilite_item);
}

void edit_name_a(void)
{

	/* add the currently selected character to the name */
	if (hilite_item > 64) {

		/* OK hilited... */
		edit_name_start();
	} else {
		uint8_t x;
		uint8_t y;
		char c;
		index_to_position(hilite_item, &x, &y, &c);
		if (c == '_')
			c = ' ';
		uint8_t len = strlen(name_string);
		if (len < 11) {
			name_string[len] = c;
			name_string[len + 1] = 0;

			/*      sprintf(name_string, "%s%c", name_string, c); */
			play_soundfx(SND_MENU);
		}
		update_name();
	}
}

void edit_name_start(void)
{
	trim_whitespace(name_string);
	uint8_t len = strlen(name_string);
	if (len > 0) {

		/* name is valid */
		play_soundfx(SND_SPELLSUCCESS);
		strcpy(players[g_chaos_state.current_player].name, name_string);
		fade_down();
		redraw_create_players();
		fade_up();
	}
}

void edit_name_b(void)
{

	/* delete a letter... */
	uint8_t len = strlen(name_string);
	if (len > 0) {
		name_string[len - 1] = 0;
		play_soundfx(SND_CHOSEN);
	}
	update_name();
}

void edit_name_touch(int x, int y)
{
	int sqx, sqy;
	pixel_xy_to_square_xy(x, y, &sqx, &sqy);
	if (sqy == 2 || sqy == 3) {
		/* erase 1 when clicking the name */
		edit_name_b();
	} else {
		/* are we on OK? */
		if ((sqy >= 16 && sqy <= 18) && (sqx > 15 && sqx < 18)) {
			edit_name_start();
		} else {
			/* if we are on an odd square, return, nothing hit */
			if ((sqx & 1) && (sqy < 10 || sqy > 11))
				return;
			int letterx = (sqx - 2) / 2;
			if (letterx < 0)
				return;
			int lettery = ((sqy - 6) / 2);
			if (lettery < 0)
				return;
			int letter = letterx + lettery * 13;
			deselect_letter(hilite_item);
			hilite_item = letter;
			select_letter(hilite_item);
			edit_name_a();
		}
	}
}
