/* movement.c */
#include <string.h>
#include "chaos/platform.h"
#include "chaos/porting.h"
#include "chaos/translate.h"

#include "chaos/sound_data.h"
#include "chaos/text16.h"
#include "chaos/movement.h"
#include "chaos/chaos.h"
#include "chaos/wizards.h"
#include "chaos/computer.h"
#include "chaos/gfx.h"
#include "chaos/arena.h"
#include "chaos/spelldata.h"
#include "chaos/cursor_defines.h"
#include "chaos/creature.h"
#include "chaos/rand.h"
#include "chaos/players.h"
#include "chaos/gamemenu.h"
#include "chaos/input.h"
#include "chaos/casting.h"
#include "chaos/splash.h"
#include "chaos/options.h"
#include "chaos/touch.h"
#include "chaos/examine.h"
#include "chaos/spellselect.h"

int g_highlight_creations = 9;
unsigned char selected_creature;
unsigned char tmp_range_attack;
unsigned char tmp_movement_allowance;
unsigned char tmp_is_flying;
unsigned char tmp_engaged_flag;
static unsigned char tmp_creature_id;
static unsigned char tmp_range_attack_val;
static unsigned char tmp_wizard_movement_allowance;
static unsigned char attacker;

#ifdef _HEADLESS
extern int game_running;
#endif

/* based on be52 - get the owner of creature at x,y */
static void get_owner_at(uint8_t x, uint8_t y, uint8_t * surround_creature)
{
	if (y == 0 || y > 0xA)
		return;
	if (x == 0 || x > 0xF)
		return;

	/* based on code at bdd1 */
	uint8_t index = (x - 1) + ((y - 1) << 4);
	if (arena[0][index] == 0)
		return;
	if (is_dead(index))
		return;
	uint8_t owner = 0;
	if (arena[0][index] >= SPELL_GOOEY_BLOB) {

		/* a blob or more */
		if ((arena[0][index] - WIZARD_INDEX) < 0)	/*if it is a creature/spell from 22-28 */
			return;
		owner = arena[0][index] - WIZARD_INDEX + 1;
		if (owner == (g_chaos_state.current_player + 1))
			return;
	} else {

		/* an actual alive creature, get the owner */
		owner = get_owner(index) + 1;
		if (owner == (g_chaos_state.current_player + 1))
			return;
	}
	if (owner != 0) {
		*surround_creature = owner;
	}
}

static void set_engaged_to_enemy(void)
{
	tmp_engaged_flag = 1;
	tmp_movement_allowance = 0x1;
	tmp_is_flying = 0;	/* if engaged, then we can't fly about */
	/* print message */
	clear_message();
	print_text16(_("ENGAGED TO ENEMY"), MESSAGE_X, g_message_y, 12);
	set_text16_colour(12, RGB16(30, 30, 0));	/* yellow */
}

static int has_surround_creature(uint8_t index)
{
	/* first part based on code at be21 */
	uint8_t x, y;
	get_yx(index, &y, &x);
	uint8_t surround_creature = 0;
	y--;			/* x  y-1 */
	get_owner_at(x, y, &surround_creature);
	x--;			/* x-1  y-1 */
	get_owner_at(x, y, &surround_creature);
	y++;			/* x-1  y */
	get_owner_at(x, y, &surround_creature);
	y++;			/* x-1  y+1 */
	get_owner_at(x, y, &surround_creature);
	x++;			/* x  y+1 */
	get_owner_at(x, y, &surround_creature);
	x++;			/* x+1  y+1 */
	get_owner_at(x, y, &surround_creature);
	y--;			/* x+1  y */
	get_owner_at(x, y, &surround_creature);
	y--;			/* x+1  y-1 */
	get_owner_at(x, y, &surround_creature);
	return surround_creature != 0;
}

static void check_engaged(uint8_t index)
{
	/* check engaged to enemy for the creature at start_index */
	tmp_engaged_flag = 0;

	/* first part based on code at be21 */
	if (!has_surround_creature(index)) {
		return;
	}

	/* if here, then get the arena 4 value of "index" */
	/* In ZX Chaos, mounted wizards can never be engaged to enemy */
	/* change this so they can be, unless the wizard dismounts */
	if (Options[OPT_OLD_BUGS] && arena[4][index] != 0)	/* the old way, "bugged" */
		return;

	else if (arena[4][index] != 0 && selected_creature >= WIZARD_INDEX)	/* the new way, can be engaged */
		return;

	/* compare maneouvre ratings... */
	uint8_t our_man = CHAOS_SPELLS[arena[0][index]].manvr;
	uint8_t r = GetRand(10) + 2;

	/*  CALL be0a  (get parameter E=12 of creature D) */
	/*  LD   E,A    E = manoeuvre of creature in the square where the cursor is */
	/*  CALL be94        random number 0-9 stored in A */
	/*  INC  A           */
	/*  INC  A           */
	/*  CP   E      compare = rand(10) + 2  - manoeuvre */
	if ((r - our_man) >= 0) {
		/* engaged */
		set_engaged_to_enemy();
	}
}

static void dismount_wiz(void)
{
	/* dismount the wizard */
	tmp_movement_allowance = 3;
	tmp_wizard_movement_allowance = 1;
	tmp_is_flying = 0;

	/* make sure we move the wizard */
	selected_creature = arena[4][start_index];
}

/* taken from aced  */
static void select_creature(void)
{
	/* see where the cursor is in the arena.. */
	/* target_index contains the current cursor index. */
	start_index = target_index;
	selected_creature = arena[0][start_index];
	tmp_range_attack = 0;
	tmp_engaged_flag = 0;
	tmp_creature_id = 0;
	tmp_range_attack_val = 0;
	tmp_movement_allowance = 0;
	flying_target_found = 0;	/* CPU player flag */
	tmp_is_flying = 0;
	tmp_wizard_movement_allowance = 0;

	/* is there a creature here? */
	if (selected_creature == 0)
		return;

	/* check if we have moved already */
	if (HAS_MOVED(arena[3][start_index])) {
		selected_creature = 0;
		return;
	}
	if (selected_creature >= WIZARD_INDEX) {

		/* wizard - check it is ours */
		uint8_t sel_plyr = selected_creature - WIZARD_INDEX;
		if (sel_plyr != g_chaos_state.current_player) {
			selected_creature = 0;
			return;
		}
		if (sel_plyr > 7)
			return;
		tmp_movement_allowance = 0x2;	/* default */
		if (HAS_MAGICWINGS(players[sel_plyr].modifier_flag)) {
			tmp_movement_allowance = 0xD;
			tmp_is_flying = 1;
		} else if (HAS_SHADOWFORM(players[sel_plyr].modifier_flag)) {
			tmp_movement_allowance = 0x6;
		}
	} else {

		/* a creature is selected */
		/* set the movement value */
		tmp_movement_allowance = CHAOS_SPELLS[selected_creature].movement * 2;
		if (selected_creature >= SPELL_PEGASUS
				&& selected_creature < SPELL_SPECTRE) {

			/* between pegasus and vampire */
			tmp_is_flying = 1;
			tmp_movement_allowance++;
		}

		/* blindness reduces movement to 1 square. */
		if (is_blind(start_index))
			tmp_movement_allowance = 2;

		/* check if there is something inside */
		uint8_t inside_creature = arena[4][start_index];
		uint8_t yes_pressed = 0;
		if (inside_creature >= WIZARD_INDEX) {
			inside_creature -= WIZARD_INDEX;

			/* it is a wizard id  */
			if (inside_creature != g_chaos_state.current_player) {
				selected_creature = 0;
				return;
			}
			inside_creature += WIZARD_INDEX;

			/* check if the covering creature is a wood or up */
			if (selected_creature < SPELL_MAGIC_WOOD) {

				/* is not a wood, so carry on */
				/* display "DISMOUNT WIZARD? (Y OR N)" message */
				/* loop until Y or N pressed... */
				if (IS_CPU(g_chaos_state.current_player)) {
					/* "improved" */
					if (cpu_dismount()) {
						dismount_wiz();
						yes_pressed = 1;
					}
				} else {
					clear_message();
					get_yes_no(_("DISMOUNT WIZARD? "),
						   MESSAGE_X, g_message_y, 12,
						   RGB16(31, 30, 0),
						   RGB16(21, 20, 0),
						   dismount_wiz, 0);
					yes_pressed = selected_creature >= WIZARD_INDEX;
				}
			} else {

				/* bit of a hack this... */
				dismount_wiz();
				yes_pressed = 1;
				/* jump add2 */
			}
		}		/* else inside creature < WIZARD_INDEX */
		if (!yes_pressed) {

			/* adb3  */
			if (selected_creature != SPELL_SHADOW_WOOD) {

				/* not shaodw wood.. */
				if (selected_creature >=
						SPELL_GOOEY_BLOB) {

					/* blob etc, so do nothing */
					selected_creature = 0;
					return;
				}
			}

			/* get the owner - low 3 bits of arena 3. */
			uint8_t owner = get_owner(start_index);
			if (owner != g_chaos_state.current_player) {
				selected_creature = 0;
				return;
			}
			if (is_dead(start_index)) {
				/* dead  */
				selected_creature = 0;
				return;
			}
		}
	}

	/* add2 */
	/* check the engaged to enemy stuff */
	check_engaged(start_index);
	if (tmp_engaged_flag == 0) {

		/* ae32 */
		char str[3];
		clear_message();

		int2a(tmp_movement_allowance >> 1, str, 10);
		const char *rangetxt = _("MOVEMENT RANGE=");
		print_text16(rangetxt, MESSAGE_X, g_message_y,
				12);
		set_text16_colour(12, RGB16(0, 30, 0));	/* green */
		int pos = MESSAGE_X;
		pos += strlen(rangetxt);
		print_text16(str, pos, g_message_y, 13);
		pos += 2;
		set_text16_colour(13, RGB16(30, 30, 0));	/* yellow */
		if (tmp_is_flying) {
			print_text16(_("(FLYING)"), pos,
					g_message_y, 14);
			set_text16_colour(14, RGB16(0, 30, 31));	/* yellow */
		}
		wait_for_letgo();
	}
}

/* end turn for creature at start_index */
/* based on b06f */
static void end_movement(void)
{

	/* whatever happens, the wizard has definitely moved... */
	has_wizard_moved = 1;

	/* and we definitely need to remake the movement table */
	move_table_created = 0;
	attacker = 0;

	/* set has moved flag... */
	arena[3][start_index] |= 0x80;
	clear_message();
	if (arena[0][start_index] == SPELL_SHADOW_WOOD) {
		attacker = 0;
		tmp_range_attack = 0;
		selected_creature = 0;
		return;	/* if shadow wood */
	}

	/* check range attack... */
	if (tmp_range_attack) {
		tmp_range_attack = 0;
		selected_creature = 0;
		return;
	}
	if (arena[0][start_index] == 0) {
		selected_creature = 0;
		return;
	}
	uint8_t has_rc = 0;
	int creature = arena[0][start_index];
	if (creature >= WIZARD_INDEX) {
		int player = get_player_at(start_index);
		if (player >= 0)
			has_rc = (players[player].ranged_combat != 0);
	} else {
		has_rc = (CHAOS_SPELLS[creature].rangedCombat != 0);
	}
	if (is_blind(start_index))
		has_rc = 0;
	if (has_rc) {
		int rangedCombatRange;
		if (creature >= WIZARD_INDEX)
			rangedCombatRange = players[creature - WIZARD_INDEX].range;
		else
			rangedCombatRange = CHAOS_SPELLS[creature].rangedCombatRange;
		tmp_range_attack = rangedCombatRange * 2 + 1;

		/* code from b8dd - init range attacks */
		/* draw the border a funny colour... */

		/* print the message RANGED COMBAT,RANGE= range val */
		play_soundfx(SND_RANGE);
		char str[3];
		clear_message();

		/*    sprintf(str, _("%d"), (tmp_range_attack)>>1); */
		int2a((tmp_range_attack) >> 1, str, 10);
		const char *rangetxt = _("RANGED COMBAT,RANGE=");
		int pos = MESSAGE_X;
		print_text16(rangetxt, pos,
				g_message_y, 12);
		pos += strlen(rangetxt);
		set_text16_colour(12, RGB16(0, 30, 0));	/* green */
		print_text16(str, pos, g_message_y, 13);
		set_text16_colour(13, RGB16(30, 30, 0));	/* yellow */
		draw_cursor(CURSOR_FIRE_GFX);

		/* set the border colour too */
		platform_set_palette_entry(16 * 11 + 2, RGB16(31, 0, 31));
		platform_set_palette_entry(16 * 11 + 1, 0);

		/* seperate out the cpu stuff... */
		/* originally looped on keyboard here, moving cursor about then waited for S to be pressed */
		/* i moved that code to do_range_attack(), which is triggered when  */
		/* you press A again on the joypad */
		/* also, there was some CPU specific code here, which has gone into the cpu .c */
		/* but the initial cpu target table creation is easiest to leave in here... */
		if (IS_CPU(g_chaos_state.current_player)) {
			create_all_enemies_table();
			/*delay(20);*/
		}
	} else {
		tmp_range_attack = 0;
		selected_creature = 0;
	}
}

static void end_game_option(void)
{
	fade_down();
	/* clear arena screen... */
	remove_cursor();
	clear_arena();
	clear_text();
	clear_palettes();
	draw_decor_border(15, RGB16(0, 0, 31), RGB16(0, 31, 31));

	/*char str[30]; */
	set_text16_colour(9, RGB16(31, 31, 0));
	print_text16(_("CONTINUE THE BATTLE?"), 5, 4, 9);
	print_text16(_("YES"), 10, 8, 7);
	print_text16(_("NO"), 16, 8, 8);
	const char **key_names = platform_get_button_names();
	if (strcmp(key_names[CHAOS_KEY_START], "MENU") != 0) {
		char buffer[40];
		buffer[0] = 0;
		strcat(buffer, _("PRESS "));
		strcat(buffer, key_names[CHAOS_KEY_A]);
		print_text16(buffer, 11, 16, 9);
	}
	set_text16_colour(7, RGB16(31, 31, 0));
	set_text16_colour(8, RGB16(12, 12, 0));
	uint8_t decided = 0;
#ifdef HAS_STRESS_TEST
	int running_in_stress_mode(void);
	if (running_in_stress_mode())
		decided = 1;
#endif
	hilite_item = 0;
	fade_up();
	while (!decided) {
		platform_wait();
		platform_wait();
		platform_update_keys();
		if (platform_key_pressed_repeat(CHAOS_KEY_LEFT)) {
			hilite_item = 0;
		}
		if (platform_key_pressed_repeat(CHAOS_KEY_RIGHT)) {
			hilite_item = 1;
		}
		if (platform_key_pressed_repeat(CHAOS_KEY_A)) {
			decided = 1;
		}
		if (platform_key_pressed_repeat(CHAOS_KEY_B)) {
			decided = 1;
			hilite_item = 0;
		}
		if (platform_key_pressed(CHAOS_KEY_TOUCH)) {
			int sqx, sqy;
			int x = platform_touch_x();
			int y = platform_touch_y();
			pixel_xy_to_square_xy(x, y, &sqx, &sqy);
			if (sqy >= 7 && sqy <= 10) {
				if (sqx >= 9 && sqx <= 13) {
					decided = 1;
					hilite_item = 0;
				} else if (sqx >= 15 && sqx <= 18) {
					decided = 1;
					hilite_item = 1;
				}
			}
		}
		if (hilite_item == 0) {
			set_text16_colour(7, RGB16(31, 31, 0));
			set_text16_colour(8, RGB16(12, 12, 0));
		} else {
			set_text16_colour(8, RGB16(31, 31, 0));
			set_text16_colour(7, RGB16(12, 12, 0));
		}
	}
	g_chaos_state.current_player = 0;
	if (hilite_item) {
		g_chaos_state.current_player = 9;
	}
}

static void do_quit(void)
{
	clear_game_border();
	wait_for_letgo();
	reset_arena_tables();
	reset_players();
	g_chaos_state.world_chaos = 0;
	g_chaos_state.current_player = 0;
	g_chaos_state.dead_wizards = 0;
	g_chaos_state.playercount = 0;
	load_all_palettes();
}

void quit_game(void)
{
#ifdef _HEADLESS
	game_running = 0;
#else
	fade_down();
	do_quit();
	show_splash();
	move_screen_to(0);
	fade_up();
#endif
}

static void end_game(void)
{
	uint8_t keypressed;
	uint8_t i, j, k;
	keypressed = 0;
	i = 0;
	j = 4;
	k = 8;
	players[0].plyr_type = PLYR_HUMAN;
	g_chaos_state.current_player = 0;	/* otherwise cpu player might just immediately "press" a key! */
	wait_for_letgo();
#ifdef HAS_STRESS_TEST
	int running_in_stress_mode(void);
	if (running_in_stress_mode()) {
		keypressed = 1;
		platform_dprint("end_game called");
	}
#endif
	/* remove save game */
	save_game(0);

	while (!keypressed) {
		platform_wait();
		platform_wait();
		platform_wait();
		platform_wait();
		set_text16_colour(10, chaos_cols[i]);
		set_text16_colour(12, chaos_cols[j]);
		set_text16_colour(13, chaos_cols[k]);
		platform_set_palette_entry(16 * 15 + 1, chaos_cols[j]);
		platform_set_palette_entry(16 * 15 + 2, chaos_cols[i]);
		i++;
		j++;
		k++;
		if (i > 8)
			i = 0;
		if (j > 8)
			j = 0;
		if (k > 8)
			k = 0;
		platform_wait();
		platform_update_keys();
		if (platform_key_pressed(CHAOS_KEY_A) || platform_key_pressed(CHAOS_KEY_TOUCH))
			keypressed = 1;
	}
	quit_game();
}

static void show_draw(void)
{
	remove_cursor();
	clear_arena();
	clear_text();
	clear_palettes();
	draw_decor_border(15, RGB16(0, 0, 31), RGB16(0, 31, 31));
	set_text16_colour(10, RGB16(30, 31, 0));	/* yellow */
	print_text16(_("THE CONTEST IS DRAWN BETWEEN"), 1, 1, 10);
	uint8_t y = 3, i;
	for (i = 0; i < g_chaos_state.playercount; i++) {
		if (!IS_WIZARD_DEAD(players[i].modifier_flag)) {
			print_text16(players[i].name, 9, y, 13);
			y += 2;
		}
	}
}

static void drawn_contest(void)
{

	/* "THE CONTEST IS DRAWN BETWEEN" */
	/* then wipe all the flags and stuff and return to splash screen */
	/* clear arena screen... */
	fade_down();
	show_draw();
	fade_up();
	end_game();
}

void start_movement_round(void)
{
	/* start the movement round */
	/* code is very roughly based on speccy chaos... */
	/* code begins at ac36 */
	while (g_chaos_state.current_player < g_chaos_state.playercount) {
		selected_creature = 0;
		attacker = 0;
		tmp_range_attack = 0;
		tmp_engaged_flag = 0;
		tmp_creature_id = 0;
		tmp_range_attack_val = 0;
		tmp_movement_allowance = 0;
		tmp_is_flying = 0;
		tmp_wizard_movement_allowance = 0;
		save_game(SCR_MOVEMENT);
		if (IS_WIZARD_DEAD(players[g_chaos_state.current_player].modifier_flag)) {
			/* player is dead... */
			g_chaos_state.current_player++;
			continue;
		} else {

			/* player is alive...  */
			/* print "player'S TURN" string... */
			g_chaos_state.current_screen = SCR_MOVEMENT;
			hilite_item = 0;
			set_current_player_index();
			remove_cursor();
			platform_wait();
			uint8_t x, y;
			get_yx(wizard_index, &y, &x);
			move_cursor_to(x - 1, y - 1);
			set_border_col(g_chaos_state.current_player);
			remove_cursor();
			char str[30];
			clear_message();
			disable_interrupts();
			highlight_players_stuff(g_chaos_state.current_player);
			if (IS_CPU(g_chaos_state.current_player)) {
				play_soundfx(SND_CPUSTART);
			}
			strcpy(str, players[g_chaos_state.current_player].name);
			strcat(str, _("'S TURN"));

			/*      sprintf(str, "%s'S TURN",players[g_chaos_state.current_player].name); */
			print_text16(str, MESSAGE_X, g_message_y, 10);
			set_text16_colour(10, RGB16(30, 31, 0));	/* yellow */
			for (x = 0; x < 24; x++) {
				platform_wait();
			}
			enable_interrupts();
			if (players[g_chaos_state.current_player].last_spell == SPELL_MEDITATE) {
				players[g_chaos_state.current_player].last_spell = 0;
				strcpy(str, players[g_chaos_state.current_player].name);
				strcat(str, _(" MEDITATES"));
				print_text16(str, MESSAGE_X, g_message_y, 10);
				set_text16_colour(10, RGB16(30, 0, 31));	/* purple */
				for (x = 0; x < 24; x++) {
					platform_wait();
				}
			}

			/* highlight the current player's creatures */
			if (IS_CPU(g_chaos_state.current_player)) {

				/* cpu movement round */
				/* jump 96f3 */
				delay(10);
				do_ai_movement();
				g_chaos_state.current_player++;
				continue;
			} else {
				redraw_cursor();
				/* human movement selection... */
				return;
			}
		}
	}

	/* here, check that there are enough wizards left to carry on */
	/* and that we haven't been playing for players*2+15 rounds yet */
	if (g_chaos_state.dead_wizards == (g_chaos_state.playercount - 1)) {

		/* uh oh -  no wizards left, do winner screen */
		win_contest();
		return;
	}
	if (Options[OPT_ROUND_LIMIT] == DEFAULT_ROUNDS) {
		if (g_chaos_state.round_count >= g_chaos_state.playercount * 2 + 15) {

			/* we have been playing for ages... no one is gonna win */
			drawn_contest();
			return;
		}
	} else if (Options[OPT_ROUND_LIMIT] != 0) {
		if (g_chaos_state.round_count >= Options[OPT_ROUND_LIMIT]) {

			/* we have been playing for more than the optional round limit... draw the game */
			drawn_contest();
			return;
		}
	}		/* else if round limit is 0, then the game can go on for ever! */

	/* return to spell selection/cast round */
	g_chaos_state.current_player = 0;

	/* check if we need to show game menu... */
	if (IS_CPU(0) || IS_WIZARD_DEAD(players[0].modifier_flag))
		g_chaos_state.current_player = get_next_human(0);

	remove_cursor();
	save_game(g_chaos_state.current_player == 9 ? SCR_MOVEMENT : SCR_GAME_MENU);
	delay(64);
	if (g_chaos_state.current_player == 9) {

#if !defined(_REPLAY) && !defined(_HEADLESS)
		/* there is no human player! */
		end_game_option();
		if (g_chaos_state.current_player == 9) {
			drawn_contest();
			return;
		} else {
			continue_game();
		}
#else
		continue_game();
#endif
	} else {

		fade_down();
		show_game_menu();
		fade_up();
	}
}


static void dsm_move_screen(void)
{
	if (tmp_engaged_flag || selected_creature == SPELL_SHADOW_WOOD ||
	    !IS_CPU(g_chaos_state.current_player)) {
		return;
	}
	move_screen_to(target_index);
}

/* based on af50... */
static void do_successful_move(uint8_t distance_moved)
{
	if (tmp_engaged_flag)
		return;
	if (selected_creature == SPELL_SHADOW_WOOD) {
		/* shadow wood, don't move! */
		/* end creature turn? jump b06f */
		end_movement();
		return;
	}
	play_soundfx(SND_WALK);

	/* flag that we need to remake the CPU movement table */
	move_table_created = 0;

	/* tidy up the start square */

	/*uint8_t start_a0 = arena[0][start_index]; */
	uint8_t start_a3 = arena[3][start_index];
	uint8_t start_a4 = arena[4][start_index];
	if (selected_creature >= WIZARD_INDEX
			&& selected_creature != arena[0][start_index]) {

		/* the start creature is a wizard, but the visible thign is not */
		/* i.e. wizard moving out of something */
		start_a4 = 0;
	} else {
		/* af88 */
		if (arena[5][start_index] != 0) {

			/* af93   */
			arena[0][start_index] = arena[5][start_index];	/*creature in arena 5?? */
			arena[2][start_index] = 4;	/* dead */
			arena[5][start_index] = 0;	/*clear creature in arena 5 (not sure about this) */
		} else {
			arena[0][start_index] = 0;	/* 1 in the game... */
		}
	}

	/* afa3.. */
	arena[1][start_index] = 1;	/*update anim */
	arena[4][start_index] = 0;	/* nothing here */

	/* sort out the target square */

	/* continue at afc7.... */
	if (arena[0][target_index] == 0 || is_dead(target_index)) {

		/* afdf */
		uint8_t old_target = arena[0][target_index];
		arena[0][target_index] = selected_creature;
		arena[5][target_index] = old_target;
		arena[1][target_index] = 1;
		arena[2][target_index] = 0;
		arena[3][target_index] = start_a3;
		arena[4][target_index] = start_a4;
	} else {

		/* afd7 */
		/* something in target square and thing there is not dead */
		/* mustbe a wiz moving to a mount or castle */
		arena[4][target_index] = selected_creature;

		/* end the wizard's move */
		tmp_wizard_movement_allowance = 0;
		tmp_movement_allowance = 0;

		/* jump b007... */
	}

	/* b007 */
	/* do a sound effect... */
	/* check engaged flag if we haven't moved to a magic wood */
	start_index = target_index;
	if (attacker != 0) {
		end_movement();
		return;
	}
	if (arena[0][start_index] != SPELL_MAGIC_WOOD) {

		/* we have moved onto something that isn't a magic wood */
		check_engaged(start_index);
		if (tmp_engaged_flag)
			return;
		if (tmp_wizard_movement_allowance != 0) {
			/* jump b06f... */
			end_movement();
			return;
		}
	}

	/* b039 */
	wait_for_letgo();
	int8_t ma = tmp_movement_allowance - distance_moved;
	if (tmp_is_flying || ma <= 0) {
		tmp_movement_allowance = 0;
		/* If flying, can have allowance left - force engaged to enemy as it is cool */
		if (ma > 0) {
			tmp_engaged_flag = has_surround_creature(start_index) ? 1 : 0;
			if (tmp_engaged_flag)
				return;
		}
		/* jump b06f */
		end_movement();
		return;
	}
	tmp_movement_allowance = ma;

	/* show "movement points left" message */
	char str[3];
	clear_message();

	/*  sprintf(str, "%d", (tmp_movement_allowance+1)>>1); */
	int2a((tmp_movement_allowance + 1) >> 1, str, 10);
	print_text16(_("MOVEMENT POINTS LEFT="), MESSAGE_X, g_message_y, 12);
	set_text16_colour(12, RGB16(0, 30, 0));	/* green */
	print_text16(str, MESSAGE_X + 21, g_message_y, 13);
	set_text16_colour(13, RGB16(30, 30, 0));	/* yellow */
	/*////////   end message ///////////// */
}

/* b310 */
static void remove_creature(uint8_t creature)
{
	/* remove the creature in target_index and move there */
	tmp_engaged_flag = 0;
	tmp_movement_allowance = 0;
	tmp_wizard_movement_allowance = 0;
	if (creature == 0xFF) {

		/* leaves corpse */
		arena[1][target_index] = 1;
		arena[2][target_index] = 4;
		/* clear sleep / blind */
		arena[3][target_index] &= 7;
		delay(12);

		/* do sound fx */
		if (tmp_range_attack == 0) {
			dsm_move_screen();
			do_successful_move(3);
		}
	} else {

		/* creature doesn't leave a corpse */
		/* check arena 5 val */
		if (arena[5][target_index] == 0) {
			arena[0][target_index] = 0;
			arena[1][target_index] = 1;
		} else {
			arena[0][target_index] = arena[5][target_index];
			arena[1][target_index] = 1;
			arena[2][target_index] = 4;
			arena[5][target_index] = 0;
		}
		delay(6);
		if (tmp_range_attack == 0) {
			dsm_move_screen();
			do_successful_move(3);
		}
	}
}

static void do_attack_anim(uint8_t index)
{
	remove_cursor();
	/* do sound fx */
	uint8_t x, y;
	get_yx(index, &y, &x);
	int i, j;

	/* the bug here is that in the timer code, I do: */
	/*  samples--;
	    if (loop) {
	    play sample again
	    }

	    if (samples == 0)
	    really switch off all the sound effects

	    so looping sounds will have the sound reg pulled
	    from under them. Then when the register is turned
	    on again, the looping sample is raring to go and
	    plays straight away.

	    should do a samples++; in the "if (loop) " bit.

*/
	disable_interrupts();
	play_soundfx(SND_ATTACK);
	for (i = 0; i < 5; i++) {
		if (i == 3)
			play_soundfx(SND_ATTACK);
		for (j = 0; j < 4; j++) {
			platform_wait();
			platform_wait();
			draw_fight_frame(x - 1, y - 1, j);
		}
	}
	arena[1][index] = 1;
	enable_interrupts();
	if (!IS_CPU(g_chaos_state.current_player))
		redraw_cursor();
}

/* start_index attacks thing at target_index  */
/* code taken from b168 - either moving to a creature or firing on one */
static void make_attack(void)
{
	attacker = arena[0][start_index];
	uint8_t defender;
	int8_t attacking_val;
	int8_t defending_val;
	uint8_t attacker_undead;
	uint8_t defender_undead;
	uint8_t attacker_modifier;
	uint8_t defender_modifier;

	attacking_val = defending_val = attacker_undead =
		defender_undead = attacker_modifier = defender_modifier = 0;
	defender = arena[0][target_index];
	if ((attacker >= SPELL_VAMPIRE && attacker <= SPELL_ZOMBIE)
			|| IS_UNDEAD(arena[3][start_index])) {
		attacker_undead = attacker;
	}
	if (attacker >= WIZARD_INDEX) {
		/* wizard attacking */
		/* remove shadow form */
		int player = get_player_at(start_index);
		if (player < 0) {
			return;
		}
		players[player].modifier_flag &= ~0x8;
		attacker_modifier = players[player].modifier_flag & 0x7;
		attacker_undead = attacker_modifier;
		if (players[player].ranged_combat != 0)
			attacker_undead = players[player].ranged_combat;
		/* possible bug here - if has magic bow, can melee attack undead? */
	}

	/* continues at b1f8... */
	if ((defender >= SPELL_VAMPIRE && defender <= SPELL_ZOMBIE)
			|| IS_UNDEAD(arena[3][target_index])) {
		defender_undead = defender;
	}
	if (defender >= WIZARD_INDEX) {
		int player = get_player_at(target_index);
		if (player < 0) {
			return;
		}
		/* modify defence if defender has shield or armour */
		defender_modifier = players[player].modifier_flag & 0xC0;
		defender_modifier = defender_modifier >> 6;
		if (defender_modifier) {
			defender_modifier++;
		}

		/* if defender has shadow form, gets bonus 3 defence! */
		if (HAS_SHADOWFORM(players[player].modifier_flag)) {
			defender_modifier += 3;
		}
	}

	/* b23d */
	if (defender_undead != 0 && attacker_undead == 0) {

		/* UNDEAD-CANNOT BE ATTACKED and return  */
		clear_message();
		print_text16(_("UNDEAD-CANNOT BE ATTACKED"), MESSAGE_X,
				g_message_y, 12);
		set_text16_colour(12, RGB16(0, 30, 30));	/* light blue */
		/* do a sound fx */
		wait_for_letgo();
		wait_for_keypress();
		clear_message();
		return;
	}
	if (IS_CPU(g_chaos_state.current_player))
		move_screen_to(target_index);

	/* get attacking val... */
	if (tmp_range_attack) {
		if (attacker < WIZARD_INDEX)
			attacking_val = CHAOS_SPELLS[arena[0][start_index]].rangedCombat;
		else
			attacking_val = players[attacker - WIZARD_INDEX].ranged_combat;
	} else {
		if (attacker < WIZARD_INDEX)
			attacking_val = CHAOS_SPELLS[arena[0][start_index]].combat;
		else
			attacking_val = players[attacker - WIZARD_INDEX].combat;
	}
	/* penalty for blind attackers */
	if (is_blind(start_index))
		attacking_val = 1;

	/* bug here - if the wizard has magic knife, it improves his ranged combat! */
	attacking_val += attacker_modifier + GetRand(10);

	/* get defending val... */
	if (defender < WIZARD_INDEX)
		defending_val = CHAOS_SPELLS[defender].defence;
	else
		defending_val = players[defender - WIZARD_INDEX].defence;
	defending_val += defender_modifier + GetRand(10);
	wait_for_letgo();
	if (!tmp_range_attack) {

		/* do attack anim... */
		/* CALL b375 */
		do_attack_anim(target_index);
	}
	if (defending_val < attacking_val) {

		/* attack was a success... */
		if (arena[4][target_index] == 0) {

			/* nothing in arena 4... */
			if (defender >= WIZARD_INDEX) {

				/* was a wizard, do wizard death anim... */
				kill_wizard();
			}
			if (defender >= SPELL_VAMPIRE
					|| IS_UNDEAD(arena[3][target_index])
					|| IS_ILLUSION(arena[3][target_index])) {
				/* shouldn't leave anything underneath... */
			} else {
				defender = 0xFF;
			}

			/* jump b310 */
			remove_creature(defender);
		} else {

			/* there was something in the arena 4... b2e0 */
			uint8_t arena0 = arena[0][target_index];
			uint8_t arena3 = arena[3][target_index];
			uint8_t arena4 = arena[4][target_index];
			arena[4][target_index] = 0;
			if (defender == SPELL_GOOEY_BLOB) {
				liberate_from_blob(target_index);
			} else if (!Options[OPT_OLD_BUGS]) {
				/* the famous "undead wizard" bug is caused by not updating the arena[3] flag properly */
				if (arena4 >= WIZARD_INDEX) {
					arena[3][target_index] = arena4 - WIZARD_INDEX;
				}
			}

			/* Another mini-bug here - if we kill a ridden mount, the wizard appears in
			 * the square (that's correct), but the mount leaves no corpse. Needs a fix?
			 * arena[5] holds corpse info...
			 */
			arena[0][target_index] = arena4;
			if ((arena0 > SPELL_MANTICORE) || IS_UNDEAD(arena3) || IS_ILLUSION(arena3))
				arena[5][target_index] = 0;
			else
				arena[5][target_index] = arena0;

			/* jump b06f */
			end_movement();
		}
	} else {

		/* attack fails... */
		/* jump b06f */
		end_movement();
	}
}


/* after selecting a creature, pressing A again moves it to its target square */
static void move_creature(uint8_t distance_moved)
{
	/* code roughly based on that at aea7 onwards */
	/* this is new... */
	/* get the creature in the target square */
	if (arena[0][target_index] != 0) {
		/* is it dead? */
		uint8_t owner = get_owner(target_index);
		if (!is_dead(target_index)) {
			if (arena[0][target_index] == SPELL_MAGIC_WOOD) {
				/* if magic wood...aec0 */
				if ((selected_creature >= WIZARD_INDEX
						&& arena[4][target_index] != 0)
						|| selected_creature < WIZARD_INDEX) {
					/* we are a wizard and the tree is occupied */
					/* or we are a creature attacking a tree (can attack our own trees) */
					/* jump to attack b168 (via aeea) */
					make_attack();
					return;
				}
			} else {

				/* not a magic wood */
				/* get the target square creature */
				/*aede */
				if (arena[0][target_index] < SPELL_MAGIC_FIRE
					|| (arena[0][target_index] == SPELL_SHADOW_WOOD)) {
					/* a creature or shadow wood   */
					if (owner != g_chaos_state.current_player) {
						/* jump b168 attack */
						make_attack();
						return;
					} else {

						/* aeed */
						/* creature we are moving to belongs to current player */
						/* check if we have a wizard selected for movement... */
						if ((WIZARD_INDEX + g_chaos_state.current_player) == selected_creature) {
							/* we are a wizard... check the target creature */
							if (arena[0][target_index] < SPELL_HORSE
									|| arena[0][target_index] >= SPELL_BAT) {
								/* not a mount, return */
								return;
							}
							/* Cannot ride around on sleeping creatures. */
							if (is_asleep(target_index))
								return;
						} else {
							return;
						}
					}
				} else {

					/* not an animal-type creature or shadow wood (i.e. could be a wizard or a citadel) */
					/* jump af0d */
					if (arena[0][target_index] < SPELL_MAGIC_CASTLE
							|| arena[0][target_index] >= SPELL_WALL) {
						/* not a magic castle/citadel, must be 23 (fire) or wizard */
						/* jump af2b */
						if (arena[0][target_index] < WIZARD_INDEX)	/*must be fire - can't move here */
							return;
						if (selected_creature < WIZARD_INDEX) {
							/* not a wizard selected, so check we aren't attacking our owner */
							if (get_owner(start_index) == (arena[0][target_index] - WIZARD_INDEX))
								return;
						}

						/* got to here?, we are a wizard attacking another wizard */
						/* or a creature attacking an enemy wizard */
						/*attack - jump b168 */
						make_attack();
						return;
					} else {
						/* a magic castle/citadel */
						if ((WIZARD_INDEX + g_chaos_state.current_player) == selected_creature) {
							/* wizard selected for movement */
							if (owner != g_chaos_state.current_player) {
								/* not our castle so can't go in */
								return;
							}
						} else {
							/* not a wizard selected, so can't go inside or attack */
							return;
						}

						/* got to here? we are a wizard and the castle is ours, so move */
					}
				}
			}
		}
	}

	/*else nothing here, jump to af50 */
	/* af50... */
	dsm_move_screen();
	do_successful_move(distance_moved);
}

static void move_walking_creature(void)
{
	uint16_t movement_amount;
	get_distance(target_index, start_index, &movement_amount);
	if (tmp_is_flying == 0 && movement_amount > 3) {
		clear_message();
		print_text16(_("OUT OF RANGE"), MESSAGE_X, g_message_y, 12);
		set_text16_colour(12, RGB16(0, 30, 30));	/* light blue */
		return;
	} else if (target_index == start_index) {
		return;
	}
	move_creature(movement_amount);
}

static void move_flying_creature(void)
{
	/* based on b0a8 */
	if (tmp_engaged_flag) {
		tmp_movement_allowance = 0x3;
	}

	/*  if (IS_CPU(g_chaos_state.current_player)) { */
	/*    int16_t in_range = create_range_table(start_index, tmp_movement_allowance); */
	/*    // get best value in LUT */
	/*     */
	/*  }  */
	if (start_index == target_index)
		return;

	/* check in range... */
	uint16_t dist;
	get_distance(start_index, target_index, &dist);
	if (tmp_movement_allowance - dist >= 0) {

		/* JP b148 */
		move_creature(dist);
	} else {

		/* print out of range */
		clear_message();
		print_text16(_("OUT OF RANGE"), MESSAGE_X, g_message_y, 12);
		set_text16_colour(12, RGB16(0, 30, 30));	/* light blue */
	}
}

/* this code was originally at b94f... */
/* cpu specific stuff moved to computer.c */
static void do_range_attack(void)
{
	/* have a target... attack! */
	/*char str[30]; */
	uint8_t x, y;
	int i;
	if (target_index == start_index) {

		/* can't attack self. */
		return;
	}
	uint16_t distance;
	get_distance(start_index, target_index, &distance);
	if (tmp_range_attack < distance) {

		/* print out of range and return */
		clear_message();
		print_text16(_("OUT OF RANGE"), MESSAGE_X, g_message_y, 12);
		set_text16_colour(12, RGB16(0, 30, 30));	/* light blue */
		return;
	}

	/* in range, not attacking self... */
	if (los_blocked(target_index)) {
		clear_message();
		print_text16(_("NO LINE OF SIGHT"), MESSAGE_X, g_message_y,
				12);
		set_text16_colour(12, RGB16(31, 30, 0));	/* lblue */
		delay(4);
		return;
	}
	remove_cursor();
	delay(4);

	/* if got to here, do the range attack animation... */
	uint8_t ra_type = 1;	/* range attack type */
	if (selected_creature >= SPELL_GREEN_DRAGON
			&& selected_creature <= SPELL_GOLDEN_DRAGON) {
		ra_type = 4;
	}
	if (selected_creature == SPELL_MANTICORE) {
		ra_type = 2;
	}

	/* do the line animation, which is the same code as the los check */
	play_soundfx(SND_BEAM);
	draw_line(target_index, ra_type);

	/* redraw the creatures righ now */
	delay(4);

	/* do the attack animation  */
	void (*animFunc) (uint8_t x, uint8_t y, uint8_t frame);
	if (selected_creature >= SPELL_GREEN_DRAGON
			&& selected_creature <= SPELL_GOLDEN_DRAGON) {

		/* dragon breath attack */
		animFunc = draw_breath_frame;
		play_soundfx(SND_FIRE);
	} else {

		/* bloop attack thing */
		animFunc = draw_splat_frame;
		play_soundfx(SND_SPELLSUCCESS);
	}
	get_yx(target_index, &y, &x);
	disable_interrupts();
	for (i = 0; i < 8; i++) {
		platform_wait();
		platform_wait();
		platform_wait();
		platform_wait();
		animFunc(x - 1, y - 1, i);
	}
	/* ensure we redraw next tick */
	arena[1][target_index] = 1;
	if (arena[0][target_index] == 0)
		clear_square(x - 1, y - 1);
	enable_interrupts();
	delay(4);

	/* I was missing this bit before */
	if (!(arena[0][target_index] == 0
		 || is_dead(target_index))) {
		if (arena[0][target_index] < SPELL_MAGIC_FIRE
				|| arena[0][target_index] >= WIZARD_INDEX
				|| arena[0][target_index] == SPELL_MAGIC_WOOD
				|| arena[0][target_index] == SPELL_SHADOW_WOOD) {

			/* make the attack on the creature... */
			make_attack();
		}
	}
	set_border_col(g_chaos_state.current_player);
	selected_creature = 0;
	tmp_range_attack = 0;
	attacker = 0;
	if (!IS_CPU(g_chaos_state.current_player))
		redraw_cursor();
}

static void do_show_player_stuff(void)
{
	uint8_t playerid = 9;
	remove_cursor();
	g_highlight_creations = 9;
	if (arena[0][target_index] >= WIZARD_INDEX) {
		playerid = arena[0][target_index] - WIZARD_INDEX;
	}

	else if (!is_dead(target_index)) {
		playerid = get_owner(target_index);
	}
	if (playerid == 9)
		return;
	highlight_players_stuff(playerid);
	g_highlight_creations = playerid;
	char str[30];
	strcpy(str, players[playerid].name);
	strcat(str, _("'S CREATIONS"));
	clear_message();
	print_text16(str, MESSAGE_X, g_message_y, 10);
	set_text16_colour(10, RGB16(30, 31, 0));	/* yellow */
	disable_interrupts();
	wait_for_letgo();
	enable_interrupts();
	g_highlight_creations = 9;
	clear_message();
	redraw_cursor();
}

void movement_l(void)
{
	/* get the owner of the creature */
	/* make sure dead ones don't count... */
	if (arena[0][target_index] != 0 && !is_dead(target_index)) {
		do_show_player_stuff();
	}
}

/* When select is pressed, move the cursor to the next moveable creature. */
void movement_select(void)
{
	if (selected_creature == 0) {
		uint8_t i, j;
		uint8_t new_index = 0xFF;
		uint8_t lower_bounds, upper_bounds;
		for (j = 0; j < 2 && new_index == 0xFF; j++) {
			if (j == 0) {
				lower_bounds = target_index + 1;
				upper_bounds = 0x9F;
			} else {
				lower_bounds = 0;
				upper_bounds = target_index + 1;
			}

			/* search the rest of the arena for creatures... */
			for (i = lower_bounds; i < upper_bounds; i++) {

				/* if something here and it is not dead and it is... */
				/* (a creature, or a wizard, or a wizard inside a magic wood or castle) and */
				/* it hasn't moved yet AND it is our creature or wizard... */
				if (arena[0][i] && !is_dead(i)
						&& (arena[0][i] < SPELL_GOOEY_BLOB
							|| arena[0][i] >= WIZARD_INDEX
							|| arena[0][i] == SPELL_SHADOW_WOOD
							||(arena[0][i] >= SPELL_MAGIC_WOOD
								&& arena[4][i] >= WIZARD_INDEX))
						&& !HAS_MOVED(arena[3][i])) {
					/* get the owner - low 3 bits of arena 3 for creatures,  */
					/* (creatureid - WIZARD_INDEX) for wizards */
					uint8_t owner = 0xff;
					if (arena[0][i] >= WIZARD_INDEX) {
						owner = arena[0][i] - WIZARD_INDEX;
					} else {
						/* a creature - but is it a mount or a wood/castle? */
						if (arena[0][i] < SPELL_MAGIC_WOOD)	/* a mount... */
							owner = get_owner(i);
						else	/* a castle/wood - could be anyones, so base the owner on the wizard inside */
							owner = arena[4][i] - WIZARD_INDEX;
					}

					/* is it ours? */
					if (owner == g_chaos_state.current_player) {
						new_index = i;
						break;
					}
				}
			}
		}
		if (new_index != 0xFF) {
			uint8_t x, y;
			get_yx(new_index, &y, &x);
			move_cursor_to(x - 1, y - 1);
		} else {
			clear_message();
			print_text16(_("PRESS START TO END TURN"), MESSAGE_X,
					g_message_y, 12);
			set_text16_colour(12, RGB16(31, 30, 0));	/* yellow */
		}
	}
}

void movement_start(void)
{
	if (selected_creature == 0) {
		g_chaos_state.current_player++;
		start_movement_round();
	}
}

void movement_a(void)
{

	/* A pressed in movement round */

	/* check in the actual arena... */
	uint8_t x, y;
	get_yx(target_index, &y, &x);
	if (x >= 0x10) {
		return;
	}
	wait_for_letgo();
	do_movement_accept();
}

void do_movement_accept(void)
{
	if (selected_creature == 0) {
		/* select a creature */
		select_creature();
		if (selected_creature != 0) {
			if (IS_CPU(g_chaos_state.current_player))
				move_screen_to(target_index);
			play_soundfx(SND_CHOSEN);
			if (tmp_is_flying)
				draw_cursor(CURSOR_FLY_GFX);

			else if (tmp_engaged_flag == 1) {
				draw_cursor(CURSOR_ENGAGED_GFX);
			} else {
				draw_cursor(CURSOR_GROUND_GFX);
			}
		}
	} else if ((tmp_range_attack == 0) && selected_creature != 0) {

		/* try moving the selected creature */
		disable_interrupts();
		void (*func)(void);
		func = tmp_is_flying ? move_flying_creature : move_walking_creature;
		func();
		enable_interrupts();
		if (tmp_engaged_flag == 1) {
			draw_cursor(CURSOR_ENGAGED_GFX);
		}
	} else if (tmp_range_attack != 0) {
		disable_interrupts();
		do_range_attack();
		enable_interrupts();
	}
	if (selected_creature == 0) {
		draw_cursor(CURSOR_NORMAL_GFX);
	}
}

void movement_b(void)
{
	if (selected_creature != 0) {
		if (tmp_range_attack == 0) {
			end_movement();
		} else {
			set_border_col(g_chaos_state.current_player);
			tmp_range_attack = 0;
			selected_creature = 0;
		}
		if (selected_creature == 0) {
			draw_cursor(CURSOR_NORMAL_GFX);
		}
	}
}

static void show_win_screen(void)
{
	uint8_t winner = 0, i;
	for (i = 0; i < g_chaos_state.playercount; i++) {
		if (!IS_WIZARD_DEAD(players[i].modifier_flag)) {
			winner = i;
			break;
		}
	}

	/* clear arena screen... */
	remove_cursor();
	clear_arena();
	clear_text();
	clear_palettes();
	draw_decor_border(15, RGB16(0, 0, 31), RGB16(0, 31, 31));

	/*char str[30]; */
	set_text16_colour(10, chaos_cols[2]);
	set_text16_colour(12, chaos_cols[4]);
	set_text16_colour(13, chaos_cols[8]);
	print_text16(_("THE WINNER IS:"), 8, 2, 10);
	print_text16("++++++++++++++++", 7, 6, 12);
	print_text16("+", 7, 8, 12);
	print_text16("+", 22, 8, 12);
	print_text16("+", 7, 10, 12);
	print_text16("+", 22, 10, 12);
	print_text16(players[winner].name, 10, 10, 13);
	print_text16("+", 7, 12, 12);
	print_text16("+", 22, 12, 12);
	print_text16("++++++++++++++++", 7, 14, 12);
}

void win_contest(void)
{

	/* <player>" IS THE WINNER"  */
	/* then wipe all the flags and stuff and return to splash screen */
	fade_down();
	show_win_screen();
	fade_up();
	end_game();
}

int print_end_touch_message(int clear)
{
	int msgx, msgy;
	platform_screen_size(&msgx, &msgy /* do not care */);

	msgx /= 8;
	msgx -= 3;
	if (clear) {
		print_text16("   ", msgx, g_message_y, 14);
	} else {
		print_text16(_("END"), msgx, g_message_y, 14);
		set_text16_colour(14, RGB16(30, 31, 30));
	}
	return msgx;
}

void movement_touch(int x, int y)
{
	int sqx, sqy;
	pixel_xy_to_square_xy(x, y, &sqx, &sqy);
	sqx -= 1;
	sqy -= 1;
	int msgx = print_end_touch_message(0) - 1;

	if (sqx < 0 || sqy < 0 || (sqx >= 30 && sqy < 20)) {
		movement_l();
		return;
	}

	/* if touch bottom row, do meta things */
	if (sqy >= 20) {
		if (sqx < msgx) {
			/* on the left, examine */
			examine_square(target_index);
		} else {
			/* on the righ, cancel / end */
			if (selected_creature == 0)
				movement_start();
			else
				movement_b();
		}
		return;
	}

	sqx /= 2;
	sqy /= 2;

	if (cursor_x == sqx && cursor_y == sqy) {
		movement_a();
	} else {
		move_cursor_to(sqx, sqy);
		print_end_touch_message(0);
	}
}
