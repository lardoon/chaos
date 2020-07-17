/* casting.c */
#include "chaos/platform.h"
#include "chaos/porting.h"
#include "chaos/translate.h"

/* called after the arena is drawn... */
#include <stdlib.h>
#include <string.h>
#include "chaos/sound_data.h"
#include "chaos/casting.h"
#include "chaos/cursor_defines.h"
#include "chaos/arena.h"
#include "chaos/spelldata.h"
#include "chaos/chaos.h"
#include "chaos/gfx.h"
#include "chaos/wizards.h"
#include "chaos/players.h"
#include "chaos/creature.h"
#include "chaos/rand.h"
#include "chaos/computer.h"
#include "chaos/movement.h"
#include "chaos/text16.h"
#include "chaos/input.h"
#include "chaos/touch.h"
#include "chaos/gamemenu.h"
#include "chaos/options.h"
#include "chaos/spellenums.h"

unsigned char current_spell;
signed char current_spell_chance;
unsigned char temp_illusion_flag;
unsigned char temp_cast_amount;
unsigned char temp_success_flag;
static uint8_t line_table[0x68];		/* the line table stores the current positions... */
static int line_type;
static int line_table_offset;
static int fudge_factor;
static int segment_count;
static int line_length;
static int tmp_gfx;
static int line_end;
static int colour;

/* fudge factor for refreshing the screen
 * when pixels_drawn reaches a limit, wait for a redraw */
static int pixels_drawn = 0;

/* called at the start of the human player spell casting */
static void setup_human_player_cast(void)
{
	set_current_spell_chance();

	/* print player name and spell... */
	remove_cursor();
	print_name_spell();
	wait_for_keypress();
	redraw_cursor();

	/* clear message display */
	clear_message();
	temp_illusion_flag = players[g_chaos_state.current_player].illusion_cast;

	/* NB: Spell Success is calculated twice for some spells! */
	/* not sure if that is a bug or not */
	/* e.g. for creatures, this code is called once -> here */
	/* Magic Wood, it is called here and again just before the cast */
	set_spell_success();
}

static void remove_pixel(unsigned short x, unsigned short y)
{
	/* remove the pixel at x, y */
	/* completely hardware dependent this... */
	uint16_t tile = 1 + (x >> 3) + ((y >> 3) * 30);

	/* then from here it's easy (ish) just work out the remainder from /8 */
	uint8_t xrem = x & 0x7;
	uint8_t yrem = y & 0x7;
	uint16_t address = tile * 16 /* 16-bit vals per tile */
		+ (xrem / 4) + (yrem * 2);
	uint16_t pixel = platform_get_tile_entry(address);
	if ((x & 3) == 0) {
		pixel &= 0xFFF0;
	} else if ((x & 3) == 1) {
		pixel &= 0xFF0F;
	} else if ((x & 3) == 2) {
		pixel &= 0xF0FF;
	} else if ((x & 3) == 3) {
		pixel &= 0x0FFF;
	}
	platform_set_tile_entry(address, pixel);
}

/* sets or unsets pixel depending on line segment and other things */
/* b8bd... */
static void set_pixel(unsigned short x, unsigned short y)
{
	pixels_drawn++;
	if ((line_type & 0x80) == 0x80) {
		/* remove pixel here */
		remove_pixel(x, y);
		return;
	}
	if ((tmp_gfx & 0x4) != 0) {
		if ((segment_count & 0x7) != 0)
			return;
	}

	draw_pixel_4bpp(x, y);
	/* get current palette... copy here.. */
	int pal = (platform_get_map_entry(1 + (x / 8), 1 +(y / 8)) >> 12) & 0xf;
	platform_set_palette_entry((pal * 16) + 15, colour);
}

static void set_line_segment(unsigned short x, unsigned short y)
{
	uint8_t A = line_type & 0x7f;
	if (A == 3) {

		/* b85d... */
		set_pixel(x, y);
		set_pixel(x, y + 1);
		set_pixel(x, y - 1);
		set_pixel(x + 1, y);
		set_pixel(x - 1, y);
		if ((tmp_gfx & 0x4) == 0x4) {
			set_pixel(x - 2, y + 2);
			set_pixel(x, y + 3);
			set_pixel(x + 2, y + 2);
			set_pixel(x + 3, y);
			set_pixel(x + 2, y - 2);
			set_pixel(x, y - 3);
			set_pixel(x - 2, y - 2);
			set_pixel(x - 3, y);
		}
	} else {
		/* draw pixel at x,y and return */
		set_pixel(x, y);
	}
	segment_count++;
	/* bolt and arrow need more oomph */
	if (line_table_offset <= 0xe)
		pixels_drawn *= 2;
	if (pixels_drawn >= fudge_factor) {
		pixels_drawn = 0;
		platform_wait();
	}
}

/* based on code at b7d3 and b84d */
static void draw_line_segment(unsigned short x, unsigned short y)
{
	int i;
	line_table[0] = (uint8_t) x;
	line_table[1] = (uint8_t) y;

	set_line_segment(x, y);

	/* if reach the end of line, jump... */
	if ((x + (y * 16)) != line_end) {
		for (i = 0x64; i >= 0; i--) {
			line_table[i + 2] = line_table[i];
		}
		if (line_length < line_table_offset) {
			line_length += 2;
		}
		uint8_t newx, newy;
		newx = line_table[line_length];
		newy = line_table[line_length + 1];
		line_type |= 0x80;
		set_line_segment(newx, newy);
		line_type = line_type & 0x7f;
	} else {
		/* end of line reached */
		int A = (line_length / 2) + 2;
		int index = line_length;
		for (i = 0; i < A && index >= 0; i++) {
			uint8_t newx, newy;
			newx = line_table[index];
			newy = line_table[index + 1];
			line_type |= 0x80;
			set_line_segment(newx, newy);
			index -= 2;
		}
	}
}

/* set up the next player ready for casting a spell */
static void next_player_cast(void)
{
	/* need checks here for if CPU, if less than g_chaos_state.playercount, etc, */
	if (g_chaos_state.current_player < g_chaos_state.playercount) {
		update_creaturecount();
		g_chaos_state.current_player++;
		draw_cursor(CURSOR_NORMAL_GFX);
		start_cast_round();
	}
}

static void end_of_cast(void)
{
	if (temp_cast_amount == 0) {
		/* next players turn */
		next_player_cast();
	}
}

/*//////////////////////////////////////////////// */
/* approximately at 95ee */
void start_cast_round(void)
{
	while (g_chaos_state.current_player < g_chaos_state.playercount) {
		save_game(SCR_CASTING);
		temp_cast_amount = 0;
		if (IS_WIZARD_DEAD(players[g_chaos_state.current_player].modifier_flag)) {
			/* player is dead... */
			update_creaturecount();
			g_chaos_state.current_player++;
			continue;
		} else {
			g_chaos_state.current_screen = SCR_CASTING;
			hilite_item = 0;

			/* this moves to the player, even if they don't have a spell to cast */
			/* need to rethink this.... */
			set_current_player_index();
			if (IS_CPU(g_chaos_state.current_player)) {

				/* cpu spell casting... */
				/* jump 96f3 */
				remove_cursor();
				do_ai_spell();
				delay(10);
				update_creaturecount();
				g_chaos_state.current_player++;
				/* need to call here again after running the pending spell events */
				continue;
			} else {

				player_data *cp = &players[g_chaos_state.current_player];
				current_spell = cp->spells[cp->selected_spell];
				if (current_spell == 0) {
					update_creaturecount();
					g_chaos_state.current_player++;
					continue;
				} else {
					cp->last_spell = cp->spells[cp->selected_spell];
					if (current_spell >= SPELL_KING_COBRA) {
						/* set the current spell to 0 if it isn't disblv/meditate. */
						cp->spells[cp->selected_spell] = 0;
					}
					cp->selected_spell = 0;	/* set to 0 for "no spell selected" */

					/* sets the success flag, etc */
					setup_human_player_cast();

					/* auto cast certain spells... */
					if (current_spell == SPELL_MAGIC_WOOD
						|| CHAOS_SPELLS[current_spell].castRange == 0) {
						remove_cursor();
						CHAOS_SPELLS[current_spell].pFunc();
						next_player_cast();
						return;
					} else if (current_spell >= SPELL_VENGEANCE
							&& current_spell <= SPELL_JUSTICE) {
						/* set up the casting chance first... if fails go to the next player */
						set_spell_success();

						if (temp_success_flag == 0) {
							/* print spell success/fail message... */
							print_success_status();
							delay(20);
							temp_cast_amount = 0;
							next_player_cast();
						}
						return;
					} else {
						draw_cursor(CURSOR_SPELL_GFX);
						redraw_cursor();
						return;
					}
				}
			}
		}
	}
	/* start movement round.. */
	g_chaos_state.round_count++;
	g_chaos_state.current_player = 0;
	unset_moved_flags();
	clear_message();
	spread_fire_blob();
	destroy_castles();
	update_sleepers();
	random_new_spell();
	freeze_meditating_wizards();
	/* here, check that there are enough wizards left to carry on */
	if (g_chaos_state.dead_wizards == (g_chaos_state.playercount - 1)) {
		/* uh oh -  no wizards left, do winner screen */
		win_contest();
		return;
	} else {
		save_game(SCR_MOVEMENT);
		start_movement_round();
	}
}

void casting_r(void)
{
	/* move to the next player... */
	if (hilite_item < g_chaos_state.playercount - 1)
		hilite_item++;
	else
		hilite_item = 0;

	/* move cursor to the hilited player */
	/*  move_cursor_to(all_creatures[hilite_item+1].xpos, all_creatures[hilite_item+1].ypos); */
}

void casting_a(void)
{
	/* cast the spell  */
	uint8_t x, y;
	get_cursor_pos(&x, &y);
	uint16_t spellid = current_spell;

	remove_cursor();
	CHAOS_SPELLS[spellid].pFunc();
	redraw_cursor();
	end_of_cast();
}

void casting_b(void)
{
	/* cancel the spell */
	temp_cast_amount = 0;
	next_player_cast();
}

/* 92f9 */
void set_current_spell_chance(void)
{
	/* sets the current spell chance based on default value, world chaos and wizard ability */
	/* just set to default for now */
	int8_t current_spell_chaos = CHAOS_SPELLS[current_spell].chaosRating;
	current_spell_chance = CHAOS_SPELLS[current_spell].castChance;
	if (g_chaos_state.world_chaos > 0 && current_spell_chaos > 0) {
		current_spell_chance += (g_chaos_state.world_chaos) >> 2;
	} else if (g_chaos_state.world_chaos < 0 && current_spell_chaos < 0) {
		current_spell_chance += (g_chaos_state.world_chaos * -1) >> 2;
	}

	/* 9353 - add ability */
	current_spell_chance += players[g_chaos_state.current_player].ability;
	if (current_spell_chance >= 0x0a) {
		current_spell_chance = 9;
	}
	if (current_spell_chance < 0) {
		current_spell_chance = 0;
	}
}


/* based on 9786 */
unsigned char is_spell_in_range(unsigned char index1, unsigned char index2,
		unsigned char range)
{
	uint16_t distance;
	get_distance(index1, index2, &distance);
	if (range >= distance) {
		return 1;
	}
	return 0;
}


/* code based on that at 9856 - after S is pressed it validates the players spell cast choice */
unsigned char player_cast_ok(void)
{
	/* check spell is in range .... call 9786 */
	if (!is_spell_in_range(wizard_index, target_index,
			 CHAOS_SPELLS[current_spell].castRange)) {
		clear_message();
		print_text16(_("OUT OF RANGE"), MESSAGE_X, g_message_y, 12);
		set_text16_colour(12, RGB16(30, 31, 0));	/* yel */
		return 0;
	}

	/* in range, so do some more checks  */
	/* 9877... */
	if (arena[0][target_index] != 0) {
		if (current_spell >= SPELL_MAGIC_WOOD)
			return 0;
		if (!is_dead(target_index))
			/* creature is not dead, can't cast here. */
			return 0;
	}

	/* 9892...  */
	/* do tree check */
	if (is_tree_adjacent(target_index)) {
		return 0;
	}

	/* do wall check */
	if (is_wall_adjacent(target_index)) {
		return 0;
	}

	/* do LOS check... */
	if (los_blocked(target_index)) {
		clear_message();
		print_text16(_("NO LINE OF SIGHT"), MESSAGE_X, g_message_y,
				12);
		set_text16_colour(12, RGB16(31, 30, 0));	/* lblue */
		return 0;
	}

	/* call spell anim, etc... */
	return 1;
}


/* do the tree check.. code from 98f1 */
int is_tree_adjacent(int index)
{
	if (current_spell < SPELL_MAGIC_WOOD
			|| current_spell >= SPELL_MAGIC_CASTLE)
		return 0;
	uint8_t surround_index = 0;
	int i = 0;
	uint8_t lookat_i;
	uint8_t found = 0;
	for (i = 0; i < 8; i++) {
		lookat_i = apply_position_modifier(index, surround_index);
		surround_index++;
		if (lookat_i == 0) {

			/* out of bounds */
			continue;
		}
		lookat_i--;
		if (arena[0][lookat_i] < SPELL_MAGIC_WOOD
				|| arena[0][lookat_i] >= SPELL_MAGIC_CASTLE) {
			/* not a wood */
			continue;
		} else {
			found = 1;
		}
	}
	return found;
}


/* 9c0f code - check that the casting wizard isn't next to the wall target square */
int is_wall_adjacent(int index)
{
	if (current_spell != SPELL_WALL)
		return 0;
	uint8_t surround_index = 0;
	int i = 0;
	uint8_t lookat_i;
	uint8_t found = 0;
	for (i = 0; i < 8; i++) {
		lookat_i = apply_position_modifier(index, surround_index);
		surround_index++;
		if (lookat_i == 0) {
			/* out of bounds */
			continue;
		}
		lookat_i--;
		if (arena[0][lookat_i] == (WIZARD_INDEX + g_chaos_state.current_player)) {
			/* casting wizard is adjacent */
			found = 1;
		}
#if 0
		else if (arena[0][lookat_i] < WIZARD_INDEX && arena[4][lookat_i] != 0) {
			/* casting wizard is adjacent and in a wood/mount etc */
			if (arena[4][lookat_i] == (WIZARD_INDEX + g_chaos_state.current_player))
				found = 1;
		}
#endif
	}
	return found;
}

struct line_segment_data
{
	int numpixels;
	int index;
	int d;
	int dinc1;
	int dinc2;
	int currentx, currenty;
	int xinc1, xinc2;
	int yinc1, yinc2;
};

static void line_segment(struct line_segment_data *data)
{
	for (;data->index < data->numpixels; data->index++) {
		draw_line_segment(data->currentx, data->currenty);

		if (data->d < 0) {
			data->d = data->d + data->dinc1;
			data->currentx = data->currentx + data->xinc1;
			data->currenty = data->currenty + data->yinc1;
		} else {
			data->d = data->d + data->dinc2;
			data->currentx = data->currentx + data->xinc2;
			data->currenty = data->currenty + data->yinc2;
		}
		if (pixels_drawn >= fudge_factor) {
			pixels_drawn = 0;
			platform_wait();
		}
	}
	data->index++;
}

/* check line of sight between start_index and the given index */
static unsigned char draw_line_check_los(unsigned char index, unsigned char gfx)
{
	/* remove fire, ghost wraiths, dead creatures, target squ, casting wizard */
	/* but only if we are doing the LOS check.. otherwise scroll the screen to the target square */
	if (gfx != 0)
		move_screen_to(index);
	fudge_factor = platform_line_fudge_factor();
	switch (gfx) {
		case 0:
			colour = 0;
			line_table_offset = 0;
			line_type = 0;
			break;
		case 1:
			colour = RGB16(31, 31, 31);
			line_table_offset = 0x2A;
			line_type = 1;
			break;		/* arrow */
		case 2:
			colour = RGB16(31, 31, 0);
			line_table_offset = 0x0E;
			line_type = 2;
			break;		/* short arrow attack */
		case 3:
			colour = RGB16(0, 31, 31);
			line_table_offset = 0x3C;
			line_type = 3;
			break;		/* spell */
		case 4:
			colour = RGB16(31, 0, 0);
			line_table_offset = 0x64;
			line_type = 3;
			break;		/* dragon */
		case 5:
			colour = RGB16(0, 31, 10);
			line_table_offset = 0x08;
			line_type = 3;
			fudge_factor *= 10;
			break;		/* bolt */
		case 6:
			colour = RGB16(31, 25, 10);
			line_table_offset = 0x64;
			line_type = 3;
			break;		/* lightning */
		default:
			line_type = 0x0;
			break;		/* arrow */
	}

	/* init segment counter */
	segment_count = 0;
	pixels_drawn = 0;
	line_length = 0;
	tmp_gfx = gfx;

	/* check from start_index to index if LOS */
	uint16_t deltax, deltay, numpixels;
	int16_t d, dinc1, dinc2;
	int16_t xinc1, xinc2;
	int16_t yinc1, yinc2;
	uint8_t currentx, currenty;
	uint8_t startx, starty, targetx, targety;
	get_yx_upper(start_index, &starty, &startx);
	startx -= 8;
	starty -= 8;
	get_yx_upper(index, &targety, &targetx);
	targetx -= 8;
	targety -= 8;
	deltax = abs(startx - targetx);
	deltay = abs(starty - targety);
	if (deltax >= deltay) {

		/*If x is independent variable */
		numpixels = deltax + 1;
		d = (2 * deltay) - deltax;
		dinc1 = deltay * 2;
		dinc2 = (deltay - deltax) * 2;
		xinc1 = 1;
		xinc2 = 1;
		yinc1 = 0;
		yinc2 = 1;
	} else {
		/*If y is independant variable */
		numpixels = deltay + 1;
		d = (2 * deltax) - deltay;
		dinc1 = deltax * 2;
		dinc2 = (deltax - deltay) * 2;
		xinc1 = 0;
		xinc2 = 1;
		yinc1 = 1;
		yinc2 = 1;
	}

	/*Move the right direction */
	if (startx > targetx) {
		xinc1 = -xinc1;
		xinc2 = -xinc2;
	}
	if (starty > targety) {
		yinc1 = -yinc1;
		yinc2 = -yinc2;
	}
	currentx = startx;
	currenty = starty;
	line_end = targetx + targety * 16;

	if (gfx == 0) {
		int i;
		for (i = 0; i < numpixels; i++) {
			if (is_pixelset (currentx, currenty, index,
					 start_index) != 0) {
				return 1;
			}
			if (d < 0) {
				d = d + dinc1;
				currentx = currentx + xinc1;
				currenty = currenty + yinc1;
			} else {
				d = d + dinc2;
				currentx = currentx + xinc2;
				currenty = currenty + yinc2;
			}
		}
	} else {
		struct line_segment_data data;
		data.numpixels = numpixels;
		data.index = 0;
		data.d = d;
		data.dinc1 = dinc1;
		data.dinc2 = dinc2;
		data.currentx = currentx;
		data.currenty = currenty;
		data.xinc1 = xinc1;
		data.xinc2 = xinc2;
		data.yinc1 = yinc1;
		data.yinc2 = yinc2;
		line_segment(&data);
	}
	load_all_palettes();
	invalidate_cache();
	return 0;
}

int los_blocked(unsigned char index)
{
	/* synchronous call to check LOS */
	return draw_line_check_los(index, 0);
}

int draw_line(int index, int gfx)
{
	/* ugh */
	return draw_line_check_los(index, gfx);
}


/* do a spell anim from start_index to target_index */
/* delay (4) first to make sure we are drawn.. */
/* based on code at a18a */
uint8_t spellframetable[0x12] = {
	0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 4, 5, 6, 7, 8, 8,
};

static void draw_spellcast_frame_wrapper(int x, int y)
{
	int i;
	for (i = 0; i < 18; i++) {
		draw_spellcast_frame(x, y, spellframetable[i]);
		platform_wait();
	}
}

/* do the spell animation between start_index and target_index */
void spell_animation(void)
{
	delay(1);

	/* don't do spell line anim for wizard modifier spells... */
	if (CHAOS_SPELLS[current_spell].castRange != 0
		|| current_spell == SPELL_TURMOIL) {
		play_soundfx(SND_BEAM);
		draw_line(target_index, 3);
		delay(4);
	}

	play_soundfx(SND_SPELLSUCCESS);
	/* draw the spell animation gfx... */
	uint8_t x, y;
	get_yx(target_index, &y, &x);
	x--;
	y--;

	disable_interrupts();
	draw_spellcast_frame_wrapper(x, y);
	enable_interrupts();
	platform_wait();
}


/* based on code at 9760 */
/* only used for cpu player - and for chaos/law spells */
void set_spell_success(void)
{
	temp_success_flag = 0;
	if (temp_illusion_flag) {
		temp_success_flag = 1;
	} else {
		uint8_t r = GetRand(10);
		if (r < (current_spell_chance + 1)) {
			temp_success_flag = 1;
			g_chaos_state.world_chaos += CHAOS_SPELLS[current_spell].chaosRating;
		}
	}
	if (Options[OPT_CHEAT] > 0) {
		/* if 1, success = 0, if > 2, success = 1 */
		temp_success_flag = !!(Options[OPT_CHEAT] - 1);
	}
}


void print_success_status(void)
{

	/* based on 97a3 */
	clear_message();
	if (temp_success_flag == 0) {

		/* print spell fails in purple */
		print_text16(_("SPELL FAILS"), MESSAGE_X, g_message_y, 12);
		set_text16_colour(12, RGB16(31, 0, 30));	/* purple */
	} else {
		print_text16(_("SPELL SUCCEEDS"), MESSAGE_X, g_message_y, 12);
		set_text16_colour(12, RGB16(31, 31, 31));	/* white */
	}
}

static void print_name_spell_2(intptr_t arg)
{
	player_data *plyr = (player_data*)arg;
	size_t x = strlen(plyr->name) + 1;
	x += strlen(_(CHAOS_SPELLS[current_spell].spellName)) + 1;
	/* range in white */
	uint8_t rng = CHAOS_SPELLS[current_spell].castRange / 2;
	if (rng >= 10) {
		rng = 20;
	}
	char str[3];
	int2a(rng, str, 10);
	print_text16(str, x, g_message_y, 15);
	set_text16_colour(15, RGB16(31, 30, 31));

	/* do a sound effect... */
	play_soundfx(SND_SPELLSTEP);
}

static void print_name_spell_1(intptr_t arg)
{
	player_data *plyr = (player_data*)arg;
	size_t x = strlen(plyr->name) + 1;

	/* spell in green */
	print_text16(_(CHAOS_SPELLS[current_spell].spellName), x, g_message_y, 13);
	set_text16_colour(13, RGB16(0, 30, 0));

	/* do a sound effect... */
	play_soundfx(SND_SPELLSTEP);
}


static int print_name_spell_0(intptr_t arg)
{
	player_data *plyr = (player_data*)arg;
	int x = 0;

	/* player name in yellow */
	print_text16(plyr->name, x, g_message_y, 12);
	set_text16_colour(12, RGB16(31, 30, 0));

	/* do a sound effect... */
	play_soundfx(SND_SPELLSTEP);
	return 0;
}

void print_name_spell(void)
{

	/* print player name, the spell and the range - code from 967a */
	uint8_t x, y;
	get_yx(wizard_index, &y, &x);
	if (IS_CPU(g_chaos_state.current_player))
		move_screen_to(wizard_index);
	else
		move_cursor_to(x - 1, y - 1);
	set_border_col(g_chaos_state.current_player);
	clear_message();

	player_data *current = &players[g_chaos_state.current_player];
	print_name_spell_0((intptr_t)current);
	delay(6);

	print_name_spell_1((intptr_t)current);
	delay(6);

	print_name_spell_2((intptr_t)current);
	delay(6);
}

struct draw_frame_data
{
	int x, y;
	int i;
	int max;
	void (*func)(uint8_t, uint8_t, uint8_t);
};

static void draw_frame_wrapper(struct draw_frame_data *data)
{
	for (; data->i < data->max; data->i++) {
		platform_wait();
		platform_wait();
		data->func(data->x, data->y, data->i);
	}
	clear_square(data->x, data->y);
}

static void draw_frames(void (*func)(uint8_t, uint8_t, uint8_t), int frames)
{
	uint8_t x, y;
	get_yx(target_index, &y, &x);
	x--;
	y--;
	struct draw_frame_data data;
	data.i = 0;
	data.max = frames;
	data.x = x;
	data.y = y;
	data.func = func;

	disable_interrupts();
	draw_frame_wrapper(&data);
	enable_interrupts();
	delay(2);
}

void splat_animation(void)
{
	draw_frames(draw_splat_frame, 8);
}

void pop_animation(void)
{
	play_soundfx(SND_SPELLSUCCESS);
	draw_frames(draw_pop_frame, 7);
}

void casting_touch(int x, int y)
{
	int sqx, sqy;
	int msgx = print_end_touch_message(0) - 1;
	pixel_xy_to_square_xy(x, y, &sqx, &sqy);
	sqx -= 1;
	sqy -= 1;
	if (sqx < 0 || sqx >= 30 || sqy < 0) {
		return;
	}
	if (sqx >= msgx && sqy >= 20) {
		casting_b();
		return;
	}
	if (sqy >= 20)
		return;

	sqx /= 2;
	sqy /= 2;

	if (cursor_x == sqx && cursor_y == sqy) {
		print_end_touch_message(1);
		casting_a();
	} else {
		move_cursor_to(sqx, sqy);
		print_end_touch_message(0);
	}
}
