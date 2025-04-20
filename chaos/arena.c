/*/ arena.c */
#include <stdlib.h>
#include <string.h>
#include "chaos/porting.h"
#include "chaos/platform.h"
#include "chaos/translate.h"
#include "chaos/players.h"
#include "chaos/arena.h"
#include "chaos/gfx.h"
#include "chaos/chaos.h"
#include "chaos/casting.h"
#include "chaos/creature.h"
#include "chaos/rand.h"
#include "chaos/spelldata.h"
#include "chaos/sound_data.h"
#include "chaos/wizards.h"
#include "chaos/options.h"
#include "chaos/text16.h"
#include "chaos/examine.h"
unsigned char **arena = 0;	/* the 6 arena tables... 960 bytes */
unsigned char wizard_index;	/* index into arena of current player */
unsigned char start_index;	/* index into arena for start square of current spell */
unsigned char target_index;	/* index into arena for target square of current spell */
int g_message_y;

static void debug_cursor_pos(void)
{
#ifdef DEBUG
	char str[30];
	sprintf(str, "%2d, %2d", cursor_x, cursor_y);
	print_text16(str, 0, 0, 0);
#endif
}

void init_arena_table(void)
{
	int i;
	arena = malloc(6 * sizeof(unsigned char*));
	for (i = 0; i < 6; i++)
		arena[i] = malloc(ARENA_SIZE + 1);
}

static void get_new_scroll_pos(uint32_t x, uint32_t y, int32_t * bgx, int32_t * bgy)
{
	if (x < 7) {

		/* x in the actual middle means scrolling off the edge... */
		*bgx = 0;
	} else {
		if (x > 8) {

			/* scrolls off the other edge */
			*bgx = 32;
		} else {
			*bgx = (x - 7) * 16;
		}
	}

	/* there are 8 visible squares vertically */
	if (y < 4) {

		/* y in the actual middle means scrolling off the edge... */
		*bgy = 0;
	} else {
		if (y > 6) {

			/* scrolls off the other edge */
			*bgy = 48;
		} else
			*bgy = (y - 4) * 16;
	}
}

static void set_current_index(uint8_t index)
{
	wizard_index = index;
	start_index = index;
	target_index = index;
}

/*//////////////////////////////////////////  */
/*
   Cursor movement stuff.
   When the curosr moves to a position, the screen should automagically
   offset itself to be in sync with that location

   At x = 0, the scroll = 0
   at x = 14, the scroll should be 31

*/
void move_cursor_left(void)
{
	if (cursor_x > 0) {
		uint16_t tmp_curs = cursor_x;
		tmp_curs--;

		/* work out where the scroll should be... */
		int32_t bgx = platform_get_x_scroll();
		uint8_t top_bloc = bgx / 16;
		if (top_bloc != 0 && (cursor_x - top_bloc <= 1)) {

			/* need to scroll... */
			bgx -= 16;
		}
		set_cursor_position(tmp_curs, cursor_y, bgx,
				platform_get_y_scroll());
		cursor_x = tmp_curs;
	}
	debug_cursor_pos();
}

void move_cursor_right(void)
{
	if (cursor_x < 14) {
		uint16_t tmp_curs = cursor_x;
		tmp_curs++;

		int32_t bgx = platform_get_x_scroll();
		uint8_t top_bloc = bgx / 16;
		if ((top_bloc + 12) != 14 && (cursor_x - top_bloc >= 11)) {

			/* need to scroll */
			bgx += 16;
		}
		set_cursor_position(tmp_curs, cursor_y, bgx,
				platform_get_y_scroll());
		cursor_x = tmp_curs;
	}
	debug_cursor_pos();
}

void move_cursor_up(void)
{
	if (cursor_y > 0) {

		/* work out where the scroll should be... */
		uint16_t tmp_curs = cursor_y;
		tmp_curs--;

		int32_t bgy = platform_get_y_scroll();
		uint8_t top_bloc = bgy / 16;
		if (top_bloc != 0 && (cursor_y - top_bloc <= 1)) {

			/* need to scroll... */
			bgy -= 16;
		}
		set_cursor_position(cursor_x, tmp_curs,
				platform_get_x_scroll(), bgy);
		cursor_y = tmp_curs;
	}
	debug_cursor_pos();
}

void move_cursor_down(void)
{
	if (cursor_y < 9) {

		/* check if the screen scroll is needed... */
		uint16_t tmp_curs = cursor_y;
		tmp_curs++;

		int32_t bgy = platform_get_y_scroll();
		uint8_t top_bloc = bgy / 16;
		if ((top_bloc + 6) != 9 && (cursor_y - top_bloc >= 5)) {

			/* need to scroll */
			bgy += 16;
		}
		set_cursor_position(cursor_x, tmp_curs,
				platform_get_x_scroll(), bgy);
		cursor_y = tmp_curs;
	}
	debug_cursor_pos();
}

/* move CURSOR to an arbitrary location */
/* handles screen scroll for you */
void move_cursor_to(uint32_t x, uint32_t y)
{

	/* move so that the screen is centered on the given location */
	/* check if the screen scroll is needed... */

	/* now see where we would like the screen to be... */
	/* this is going to be so that the x, y position is in the middle of the screen */
	/* there are 14 visible squares at once horizontally */
	int32_t bgx, bgy;
	get_new_scroll_pos(x, y, &bgx, &bgy);
	set_cursor_position(x, y, bgx, bgy);
	cursor_x = x;
	cursor_y = y;
	set_cursor_position(x, y, bgx, bgy);
}

/* moves screen to higlight a square */
/* doesn't move the cursor though */
void move_screen_to(int index)
{
	if (!platform_has_scroll())
		return;
	int x_scroll, y_scroll;
	uint8_t y, x;
	get_yx(index, &y, &x);

	/* x and y in "chaos coords" need to -1 to get standard zero-indexed values */
	y--;
	x--;
	remove_cursor();
	int32_t bgx, bgy;
	get_new_scroll_pos(x, y, &bgx, &bgy);
	int8_t dx, dy;
	dx = dy = 0;
	x_scroll = platform_get_x_scroll();
	if (bgx < x_scroll)
		dx = -4;
	else if (bgx > x_scroll)
		dx = 4;
	y_scroll = platform_get_y_scroll();
	if (bgy < y_scroll)
		dy = -4;
	else if (bgy > y_scroll)
		dy = 4;

	while (dx != 0 || dy != 0) {
		x_scroll = platform_get_x_scroll();
		if (x_scroll != bgx)
			platform_set_x_scroll(x_scroll + dx);
		else
			dx = 0;
		y_scroll = platform_get_y_scroll();
		if (y_scroll != bgy)
			platform_set_y_scroll(y_scroll + dy);
		else
			dy = 0;
		platform_wait();
		platform_update_scroll();
	}
	int16_t y_offset = 16 - platform_get_y_scroll();
	int16_t x_offset = 16 - platform_get_x_scroll();
	platform_wait();
	int id = 0;
	platform_move_sprite(id,
			x_offset + cursor_x * 16,
			y_offset + cursor_y * 16);
}

/* x is the arena x coord */
/* y is the arena y coord */
void set_cursor_position(uint32_t x, uint32_t y, int32_t bgx, int32_t bgy)
{
	/* the cursor position depends on the screen scroll as well as x/y position */
	/* smooth scroll to the given bg x,y */
	if (platform_has_scroll()) {
		int8_t dx = 0;
		int8_t dy = 0;
		int x_scroll = platform_get_x_scroll();
		if (bgx < x_scroll)
			dx = -4;
		else if (bgx > x_scroll)
			dx = 4;
		int y_scroll = platform_get_y_scroll();
		if (bgy < y_scroll)
			dy = -4;
		else if (bgy > y_scroll)
			dy = 4;
		if (dx == 0 && dy == 0) {
			/* halve the difference between cursor_x and x / cursor_y and y  */
			/* and scroll a small amount */
			int32_t y_offset, x_offset;
			x_offset = 16 - x_scroll;
			y_offset = 16 - y_scroll;
			int16_t x_diff = x - cursor_x;
			int16_t y_diff = y - cursor_y;
			platform_move_sprite(0,
					x_offset + cursor_x * 16 + x_diff * 8,
					y_offset + cursor_y * 16 + y_diff * 8);
			platform_move_sprite(0,
					x_offset + x * 16,
					y_offset + y * 16);
		} else {
			while (dx != 0 || dy != 0) {
				x_scroll = platform_get_x_scroll();
				if (x_scroll != bgx)
					platform_set_x_scroll(x_scroll + dx);
				else
					dx =  0;
				y_scroll = platform_get_y_scroll();
				if (y_scroll != bgy)
					platform_set_y_scroll(y_scroll + dy);
				else
					dy =  0;
				platform_update_scroll();
				platform_wait();
			}
			/* This sucks - the sprite is in the wrong place at the end now */
		}
	} else {
		platform_move_sprite(0, 8 + x * 16, 8 + y * 16);
	}

	/* display the message for this square */
	target_index = x + y * 16;
	display_cursor_contents(target_index);
}

/* clear arena is in the creatures file... */
void display_arena(void)
{

	/* redraw the arena... */
	int i;
	int x, y;
	clear_text();
	clear_arena();
	clear_palettes();
	load_all_palettes();

	if (platform_has_scroll()) {
		platform_set_x_scroll(0);
		platform_set_y_scroll(0);
		platform_update_scroll();
	}
	clear_bg(0);

	i = 1;
	int hs = platform_has_scroll() ? 1 : 0;
	for (y = 1; y < 21; y++)
		for (x = 1; x < 31; x++)
			platform_set_map_entry(x + hs, y + hs, i++);

	draw_game_border();

	/* draw all the creatures in the arena array */
	draw_all_creatures();
	for (i = 0; i < g_chaos_state.playercount; i++)
		set_player_col(i, players[i].colour);
}

void clear_message(void)
{
	/*            12345678901234567890123456789012 */
	char buf[40];
	int width, i;
	platform_screen_size(&width, &i);
	width /= 8;
	memset(buf, ' ', width);
	buf[width] = 0;
	print_text16(buf, MESSAGE_X, g_message_y, 12);
}

static const uint8_t anim_speed_data[] = {
	/*KING COBRA   */ 0x1e,
	/*DIRE WOLF    */ 0x0c,
	/*GOBLIN       */ 0x0c,
	/*CROCODILE    */ 0x22,
	/*FAUN         */ 0x14,
	/*LION         */ 0x26,
	/*ELF          */ 0x1a,
	/*ORC          */ 0x15,
	/*BEAR         */ 0x17,
	/*GORILLA      */ 0x12,
	/*OGRE         */ 0x17,
	/*HYDRA        */ 0x24,
	/*GIANT RAT    */ 0x0d,
	/*GIANT        */ 0x17,
	/*HORSE        */ 0x15,
	/*UNICORN      */ 0x10,
	/*CENTAUR      */ 0x17,
	/*PEGASUS      */ 0x10,
	/*GRYPHON      */ 0x0a,
	/*MANTICORE    */ 0x0d,
	/*TROLL        */ 0x16,
	/*BAT          */ 0x08,
	/*GREEN DRAGON */ 0x20,
	/*RED DRAGON   */ 0x22,
	/*GOLDEN DRAGON */ 0x1b,
	/*HARPY        */ 0x0d,
	/*EAGLE        */ 0x0e,
	/*VAMPIRE      */ 0x28,
	/*GHOST        */ 0x0f,
	/*SPECTRE      */ 0x0f,
	/*WRAITH       */ 0x0a,
	/*SKELETON     */ 0x11,
	/*ZOMBIE       */ 0x19,
	/*BLOB         */ 0x28,
	/*FIRE         */ 0x0c,
	/*MAGIC WOOD   */ 0xfa,
	/*SHADOW WOOD  */ 0x1e,
	/*MAGIC CASTLE */ 0x32,
	/*DARK CITADEL */ 0x32,
	/*WALL         */ 0x1e,
	/*JULIANLL     */ 0x1e,
	/*GANDALFRT    */ 0x1e,
	/*GREATFOGEY   */ 0x1e,
	/*DYERARTI     */ 0x1e,
	/*GOWIN        */ 0x1e,
	/*MERLIN       */ 0x1e,
	/*ILIAN RANE   */ 0x1e,
	/*ASIMONO ZARK */ 0x1e,
};

/* counts down the arena[1] values and updates the arena 2 val if needed */
void countdown_anim(void)
{
	int i;
	for (i = 0; i < 0x9f; i++) {
		if (arena[0][i] >= SPELL_KING_COBRA) {
			if ((--arena[1][i]) == 0) {
				arena[2][i]++;
				if (arena[2][i] == 4) {
					arena[2][i] = 0;
				}
				if (arena[2][i] == 5) {
					arena[2][i]--;
				}

				/* each spell has a "speed" count */
				arena[1][i] = anim_speed_data[arena[0][i] - SPELL_KING_COBRA];
			}
		}
	}
}

void animate_arena(void)
{
	draw_all_creatures();
}

void get_cursor_pos(unsigned char *x, unsigned char *y)
{
	*x = cursor_x;
	*y = cursor_y;
}

/* calculate the distance between 2 squares in the arena in "chaos range" terms (roughly 2*distance) */
/* the squares should be indices into the arena (i.e. 0,0 is 0, 1,1 is 1+1*16, etc) */
void get_distance(uint16_t square1, uint16_t square2, uint16_t * distance)
{

	/* based on code at 9786  */
	uint8_t x1, y1, x2, y2;
	uint16_t diff;
	get_yx(square1, &y1, &x1);
	get_yx(square2, &y2, &x2);

	/* calculate 2*(larger of xposdiff and yposdiff) + (the smaller of the 2) */
	get_chaosdistance(x1, y1, x2, y2, &diff);
	*distance = diff;

	/* further check in here, compare to cast range */
	/*  return spell_list[current_spell].cast_range - distance; */
}

/* calculate 2*(larger of xposdiff and yposdiff) + (the smaller of the 2) */
/* based on code at 0xbeef */
void get_chaosdistance(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint16_t * diff)
{
	uint16_t xdiff, ydiff;
	uint16_t distance;
	xdiff = abs(x2 - x1);
	ydiff = abs(y2 - y1);
	if (xdiff < ydiff) {
		distance = ydiff * 2;
		distance += xdiff;
	} else {
		distance = xdiff * 2;
		distance += ydiff;
	}
	*diff = distance;
}

void get_yx(uint16_t arena_index, uint8_t * H, uint8_t * L)
{
	get_yx2(arena_index, H, L);
	*H = 1 + (*H) / 2;
	*L = 1 + (*L) / 2;
}

void get_xy(uint16_t arena_index, uint8_t* L, uint8_t* H)
{
	get_yx2(arena_index, H, L);
	*H = 1 + (*H) / 2;
	*L = 1 + (*L) / 2;
}

void get_yx2(uint16_t arena_index, uint8_t * H, uint8_t * L)
{
	int val1 = arena_index & 0xF0;
	int val2 = arena_index;	/* (& 0xFF); */
	val1 = val1 / 8;
	val2 = val2 * 2;
	val2 = val2 & 0x1e;
	val1++;
	val2++;
	*H = val1;
	*L = val2;
}

int get_player_at(int idx)
{
	int creature = arena[0][idx];
	int player = creature - WIZARD_INDEX;
	if (player > 7) {
		arena[0][idx] = 0;
		return -1;
	}
	return player;
}

void init_arena_tables(void)
{

	/* initialises the arena tables - code based on that at c0dd */
	int i;
	disable_interrupts();
	for (i = 0; i < 0xA0; i++) {
		if ((i + 1) & 0xF) {
			if (arena[0][i] == 0) {
				arena[0][i] = 0;	/* set to one in the actual game... */
				arena[2][i] = 0;
			}
		}
		arena[1][i] = 1;
	}
	enable_interrupts();
}

/* code from 97d1 - sets the index of the current player for spell casting */
void set_current_player_index(void)
{
	int i;
	for (i = 0; i < 0x9f; i++) {
		if (arena[0][i] - WIZARD_INDEX == g_chaos_state.current_player) {
			set_current_index(i);
			return;
		}
		if (arena[4][i] - WIZARD_INDEX == g_chaos_state.current_player) {
			set_current_index(i);
			return;
		}
	}
}

void display_cursor_contents(uint8_t index)
{

	/* dispay the creature name, etc in the right colours... */
	/* taken from bd18 onwards */
	/* clear the message box */
	uint8_t creature, x;
	char str[30];
	clear_message();
	creature = arena[0][index];
	if (creature == 0) {
		clear_top_screen();
		return;
	}
	set_text16_colour(12, RGB16(0, 28, 31));	/* light blue */
	x = MESSAGE_X;
	if (creature >= WIZARD_INDEX) {
		int player = get_player_at(index);
		if (player < 0) {
			return;
		}
		/* wizard... write wiz name */
		print_text16(players[player].name, x,
				g_message_y, 12);
		x += strlen(players[player].name);
	} else {

		/* creature... print name and owner */
		print_text16(_(CHAOS_SPELLS[creature].spellName), x, g_message_y, 12);
		x += strlen(_(CHAOS_SPELLS[creature].spellName));
	}

	/* check underneath */
	int underneath = 0;
	if (arena[4][index] != 0) {
		/* underneath colour 47 (white?)  */
		set_text16_colour(14, RGB16(30, 30, 30));	/* white   */
		underneath = 1;
	} else if (arena[5][index] != 0) {
		set_text16_colour(14, RGB16(27, 4, 28));	/* purple? */
		underneath = 1;
	} else {
		set_text16_colour(14, RGB16(0, 0, 0));	/* purple? */
	}

	/* the character # is actually a \ in my character array as I don't have many entries. */
	if (underneath) {
		str[0] = '\\';
		str[1] = 0;
		print_text16(str, x, g_message_y, 14);
	}
	x++;
	if (creature < WIZARD_INDEX) {
		/* print the creature owner or status... */
		if (is_dead(index)) {
			/* dead */
			set_text16_colour(13, RGB16(0, 30, 0));	/* green */
			strcpy(str, _("(DEAD)"));
			print_text16(str, x, g_message_y, 13);
		} else {
			uint8_t own = get_owner(index);
			int c = RGB16(30, 30, 0);	/* yellow */
			str[0] = '(';
			str[1] = 0;
			if (is_asleep(index)) {
				c = RGB16(0, 30, 0);
				strcat(str, _("ASLEEP"));
			} else if (is_blind(index)) {
				c = RGB16(0, 30, 0);
				strcat(str, _("BLIND"));
			} else {
				strcat(str, players[own].name);
			}
			strcat(str, ")");
			set_text16_colour(13, c);
			print_text16(str, x, g_message_y, 13);
		}
	}

	int two_screens = platform_swap_screen() == 0;
	platform_swap_screen();
	if (!IS_CPU(g_chaos_state.current_player) && two_screens)
		examine_square(index);
}

/* called at the end of the moves round */
/* resets the movement flags so that the creatures can move next time */
void unset_moved_flags(void)
{
	unsigned char i;
	for (i = 0; i < 0x9f; i++) {
		arena[3][i] &= 0x7F;	/* unset bit 7 */
	}
}

uint8_t apply_position_modifier(uint8_t square, uint8_t i)
{
	if (i >= 8)
		return 0;

	/* use the look up table to convert the square to a new one  */
	unsigned char y, x, tmp;
	get_yx(square, &y, &x);
	y = y + surround_table[i][0];
	if (y == 0 || y == 0xB) {
		return 0;
	}
	x = x + surround_table[i][1];
	if (x == 0 || x == 0x10) {
		return 0;
	}
	x--;
	y--;
	tmp = ((y << 4) + x) + 1;

	/* Add the "3 trees in top corner" bug... */
	if (Options[OPT_OLD_BUGS] && tmp == 2
			&& current_spell == SPELL_MAGIC_WOOD)
		tmp = 0;
	return tmp;
}

void get_yx_upper(unsigned char arena_index, unsigned char *H,
		unsigned char *L)
{
	uint8_t x, y;
	get_yx(arena_index, &y, &x);
	*L = x << 4;
	*H = y << 4;
}

/* taken from b4fa   */
/* destroys this wizard's creations */
void destroy_all_creatures(unsigned char playerid)
{
	/* static coroutine */
	int frame, i;
	int sample_played = 0;

	for (frame = 0; frame < 7; frame++) {
		platform_wait();
		platform_wait();
		platform_wait();
		platform_wait();
		platform_wait();
		for (i = 0; i < 0x9f; i++) {
			if (arena[0][i] == 0)
				continue;
			if (arena[0][i] >= WIZARD_INDEX)
				continue;
			if (is_dead(i))
				/* is dead.. */
				continue;

			/* check the owner */
			if (get_owner(i) != playerid) {

				/* chek if it is a blob */
				if (arena[0][i] != SPELL_GOOEY_BLOB)
					continue;

				/* blob... is anything under it? */
				if (arena[4][i] == 0)
					continue;

				/* is the trapped creature the effected player? */
				if ((arena[5][i] & 7) != playerid)
					continue;
			}

			/* draw the pop frame at this creatures location... */
			uint8_t y, x;
			get_yx(i, &y, &x);
			draw_splat_frame(x - 1, y - 1, frame);

			/* do a sound effect, once only */
			if (!sample_played) {
				play_soundfx(SND_SPELLSUCCESS);
				sample_played = 1;
			}

			/* check if we have finished the anim... */
			if (frame == 6) {

				/* get rid of the creature */
				if (get_owner(i) != playerid) {
					arena[4][i] = 0;
					arena[5][i] = 0;
				} else {

					/* same owner as effected wiz */
					if (arena[0][i] == SPELL_GOOEY_BLOB
							&& arena[4][i] != 0) {

						/* b5a9 */
						liberate_from_blob(i);
						continue;
					}

					/* b5be */
					uint8_t arena4 = arena[4][i];
					arena[4][i] = 0;
					if (arena4 != 0) {
						arena[0][i] = arena4;
						continue;
					}

					/* b5d1 */
					/*uint8_t arena5 = arena[5][i]; */
					arena[0][i] = arena[5][i];
					if (arena[5][i] != 0) {
						arena[2][i] = 4;
						arena[5][i] = 0;
					}
				}
			}
		}
	}
}

/* randomly destroy castles at the end of each casting round */
/* code originally followed on from gooey blob, but it's here */
/* to make it clearer what's going on  */
/* a120 */
void destroy_castles(void)
{
	int i;
	for (i = 0; i < 0x9f; i++) {
		if (arena[0][i] >= SPELL_MAGIC_CASTLE
				&& arena[0][i] <= SPELL_DARK_CITADEL) {

			/* is a castle... */
			if (GetRand(10) < 8)
				continue;

			/* got here? then the castle is to be destroyed */
			move_screen_to(i);
			arena[0][i] = arena[4][i];
			arena[4][i] = 0;
			target_index = i;
			pop_animation();
			delay(4); /* one frame */
		}
	}
	delay(20); /* 5 frames */
}

void update_sleepers(void)
{
	int i;
	for (i = 0; i < ARENA_SIZE; i++) {
		target_index = i;
		if (is_asleep(i)) {
			if (GetRand(10) < 8) {
				/* setHasMoved(i); */
				arena[3][i] |= 0x80;
				continue;
			}
			/* else wake up */
			pop_animation();
			arena[1][i] = 1;
			arena[2][i] = 1;
			arena[3][i] &= ~8;
			platform_wait();
		} else if (is_blind(i)) {
			if (GetRand(10) >= 8) {
				arena[3][i] &= ~8;
				pop_animation();
			}
		}
		/* random wake up */
	}
}

static int post_new_spell(int i)
{
	int lucky_player = arena[4][i] - WIZARD_INDEX;
	arena[0][i] = arena[4][i];
	arena[4][i] = 0;
	arena[3][i] = lucky_player;	/* bug here, what if the wizard was undead? ;) */
	return 0;
}

/* give a player a spell from the magic wood */
void random_new_spell(void)
{
	int i;
	for (i = 0; i < 0x9f; i++) {
		if (arena[0][i] == SPELL_MAGIC_WOOD && arena[4][i] != 0) {

			/* is a wood with someone inside */
			if (GetRand(10) <= 6)
				continue;

			/* got here? then the wood has given us a spell! */
			int lucky_player = arena[4][i] - WIZARD_INDEX;
			new_random_spell(lucky_player);
			post_new_spell(i);
			delay(20);
		}
	}
}

/* called at the end of the game... resets the tables to the initial state */
void reset_arena_tables(void)
{
	uint8_t i, j;
	for (j = 0; j < 6; j++) {
		for (i = 0; i < ARENA_SIZE; i++) {
			arena[j][i] = 0;
		}
	}
}

void highlight_players_stuff(unsigned char playerid)
{
	/* highlight the given players stuff */
	int i;
	for (i = 0; i < 0x9f; i++) {
		int arena0 = arena[0][i];
		if (arena0 == 0)
			continue;
		if (arena0 >= WIZARD_INDEX) {
			/* wizard */
			if ((arena0 - WIZARD_INDEX) == playerid) {
				/* highlight the player */
				draw_silhouette_gfx(i,
						WizardGFX[players[playerid].image].pGFX,
						WizardGFX[players[playerid].image].pMap,
						players[playerid].colour, 11, 1);
			}
		} else if (!is_dead(i)) {
			/* creature, not dead */
			if (get_owner(i) == playerid) {
				int col = get_main_color(arena0, arena[2][i]);
				const uint16_t *gfx = CHAOS_SPELLS[arena0].pGFX;
				const uint16_t *map = CHAOS_SPELLS[arena0].pGFXMap;
				if (map != 0 && gfx != 0)
					draw_silhouette_gfx(i, gfx, map,
							    col, -1, 1);
			}
		}
	}
	invalidate_cache();
}

static int blind_bit_set(int square)
{
	  return (arena[3][square] & 8);
}

int is_blind(int square)
{
	return blind_bit_set(square) && (arena[2][square] != 4)
		&& arena[0][square] < WIZARD_INDEX
		&& arena[0][square] >= SPELL_KING_COBRA;
}

int is_asleep(int square)
{
  return blind_bit_set(square) && (arena[2][square] == 4);
}

int get_spell(int square) {
	return arena[0][square];
}

int get_owner(int square)
{
	return arena[3][square] & 7;
}

int is_dead(int square)
{
	if (!blind_bit_set(square))
		return arena[2][square] == 4;
	/* if blind bit is set, cannot be dead */
	return 0;
}

void sleep_blind(void)
{
	/*
	 * blind sets bit flag
	 * sleep sets bit flag and sets animation frame to 4
	 * casting on wizard affects all creatures for the wizard
	 */
	int target = arena[0][target_index];
	int victim = 0;
	int end = 0;
	int start = 0;
	int i;
	if (target >= WIZARD_INDEX) {
		end = ARENA_SIZE;
		victim = target - WIZARD_INDEX;
	} else {
		victim = get_owner(target_index);
		start  = target_index;
		end = start + 1;
	}
	for (i = start; i < end; i++) {
		int spellId = arena[0][i];
		int frame = arena[4][i];
		if (spellId >= SPELL_KING_COBRA
				&& spellId < SPELL_MAGIC_FIRE
				&& frame != 4
				&& get_owner(i) == victim) {
			if (current_spell == SPELL_MAGIC_SLEEP) {
				arena[1][i] = 1;
				arena[2][i] = 4;
				/* delay is to get the screen to update */
				delay(4);
			}
			arena[3][i] |= 0x8;
		}
	}
}

void freeze_meditating_wizards(void)
{
	player_data *p;
	int i, c, u;
	/* meditating wizards cannot move */
	for (i = 0; i < ARENA_SIZE; i++) {
		c = arena[0][i];
		u = arena[4][i];
		if (c < WIZARD_INDEX && u < WIZARD_INDEX)
			continue;
		int player;
		if (c >= WIZARD_INDEX) {
			player = c - WIZARD_INDEX;
		} else {
			player = u - WIZARD_INDEX;
		}
		if (player > 7)
			continue;
		p = &players[player];
		if (p->last_spell == SPELL_MEDITATE) {
			arena[3][i] |= 0x80;
		}
	}
}

static const char *HEX_STR = "0123456789ABCDEF";
static char hexify(int a)
{
	return HEX_STR[a & 0xf];
}

static int unhex(char c)
{
	if (c >= '0' && c <= '9') {
		return c - '0';
	} else if (c >= 'A' && c <= 'F') {
		return 10 + c - 'A';
	}
	return 0;
}

static int checksum(const unsigned char *block, int save_size)
{
	int ch, cy, i;
	save_size--;
	ch = save_size;
	cy = 0;
	for (i = save_size; i != 1; i--) {
		ch = ch + cy + block[i - 1];
		cy = (ch > 255) & 1;
		ch = ch % 256;
		ch ^= block[i];
	}
	return ch;
}

/** convert arena[n] to a string.
 * mem is malloc'd has to be freed by caller. */
void arena_to_str(int layer, char *arenastr)
{
	int x, y;
	arenastr[0] = 0;
	/* format is simply 10 rows of 15 x 2 hex nibbles */
	strcat(arenastr, "ARENA:\n");
	/* reset the edges */
	for (y = 0 ; y < 10; y++) {
		arena[layer][15 + y * 16] = 0;
	}

	int ch = checksum(arena[layer], ARENA_SIZE);
	int ch_xor = (ch ^ 0xa9) & 0xff;
	int idx = 7;

	arenastr[idx++] = hexify(ch >> 4);
	arenastr[idx++] = hexify(ch);
	arenastr[idx++] = hexify(ch_xor >> 4);
	arenastr[idx++] = hexify(ch_xor);
	arenastr[idx++] = '\n';

	for (y = 0; y < 10; y++) {
		for (x = 0; x < 15; x++) {
			int val = arena[layer][x + y * 16];
			arenastr[idx++] = hexify(val >> 4);
			arenastr[idx++] = hexify(val);
		}
		arenastr[idx++] = '\n';
	}
	arenastr[idx++] = 0;
}

/* returns 0 on success, anything else on error */
int str_to_arena(int layer, const char *data)
{
	int x, y, i;
	int idx = 0;
	int ch, ch_xor;
	/* check the checksum */
	for (i = 0; i < 4; i++) {
		if (data[idx + i] == 0)
			return -1;
	}
	ch = unhex(data[idx++]) << 4;
	ch |= unhex(data[idx++]);
	ch_xor = unhex(data[idx++]) << 4;
	ch_xor |= unhex(data[idx++]);
	if (data[idx++] != '\n')
		return -1;

	for (y = 0 ; y < 10; y++) {
		arena[layer][15 + y * 16] = 0;
	}

	/* format is simply 10 rows of 15 x 2 hex nibbles */
	for (y = 0; y < 10 && data[idx]; y++) {
		for (x = 0; x < 15; x++) {
			if (data[idx] == 0)
				return -1;
			int val = unhex(data[idx++]) << 4;
			if (data[idx] == 0)
				return -1;
			val |= unhex(data[idx++]);
			arena[layer][x + y * 16] = val;
		}
		if (data[idx] == 0)
			break;
		/* skip nl */
		idx++;
	}
	int ch2 = checksum(arena[layer], ARENA_SIZE);
	int ch2_xor = (ch2 ^ 0xa9) & 0xff;

#if DEBUG
	if (ch2 != ch || ch2_xor != ch_xor) {
		char str[50];
		str[0] = 0;
		platform_dprint("checksum failed");
		strcat(str, "got ");
		int2a((ch << 8) | ch_xor, &str[4], 16);
		platform_dprint(str);
		str[0] = 0;
		strcat(str, "expected ");
		int2a((ch2 << 8) | ch2_xor, &str[9], 16);
		platform_dprint(str);
	}
#endif

	if (ch2 != ch)
		return -1;
	if (ch2_xor != ch_xor)
		return -1;
	return 0;
}

int load_arenas(const char *arenastr)
{
	/* search for ARENA: tags */
	/* then load 0, 2, 3, 4, 5 */
	const char *tmp = arenastr;
	int idx = 0;
	while (*tmp != 0) {
		const char *linebegin = tmp;
		while (*tmp != '\n' && *tmp != 0) {
			tmp++;
		}
		if (*tmp == 0)
			break;
		tmp++;
		if (strncmp(linebegin, "ARENA:\n", 7) == 0) {
			if (str_to_arena(idx, &linebegin[7]) != 0) {
				return -1;
			}
			idx++;
			/* skip arena 1, it is just the timer data that can be recreated */
			if (idx == 1) idx = 2;
		}
	}
	for (idx = 0; idx < ARENA_SIZE; idx++) {
		arena[1][idx] = 1;
	}
	return 0;
}
