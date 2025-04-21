#include <stdlib.h>
#include "chaos/platform.h"
#include "chaos/porting.h"
#include "chaos/translate.h"
#include "chaos/sound_data.h"
#include "chaos/magic.h"
#include "chaos/arena.h"
#include "chaos/chaos.h"
#include "chaos/wizards.h"
#include "chaos/casting.h"
#include "chaos/creature.h"
#include "chaos/rand.h"
#include "chaos/computer.h"
#include "chaos/players.h"
#include "chaos/gfx.h"
#include "chaos/spelldata.h"
#include "chaos/options.h"
#include "chaos/text16.h"
#include "chaos/input.h"

#ifdef _HEADLESS
#include "chaos/output.h"
#endif

void do_creature_spell_if_ok(intptr_t arg)
{
	int ti = (int)arg;
	if (temp_success_flag) {
		creature_spell_succeeds(ti);
		if (temp_cast_amount > 0)
			temp_cast_amount--;
	} else {
		temp_cast_amount = 0;
	}
	if (temp_cast_amount == 0) {
		print_success_status();
		platform_wait();
	} else {
		delay(20);
		redraw_cursor();
	}
}

void cast_creature(void)
{
	/* if human just do the check on target_index, if CPU find the optimum square to cast to */
	/* if CPU and no square, set a target_square_found = 0 and return withough doing anything. */

	temp_cast_amount = 1;
	if (IS_CPU(g_chaos_state.current_player)) {
		/* do the cpu creature cast ai routine  */
		ai_cast_creature();
	} else {

		/* call code at 9856 - validates player's spell cast */
		if (!player_cast_ok()) {
			redraw_cursor();
			return;
		}
		spell_animation();
		do_creature_spell_if_ok(target_index&0xff);
		delay(30);
		draw_all_creatures();
		delay(20);
	}

}

void cast_fire_goo(void)
{
	/* these are treated the same as normal creatures */
	cast_creature();
}

struct auto_tree
{
	int index;
	int i;
};

static void end_auto_tree(void)
{
	target_square_found = 1;
	temp_cast_amount = 0;
	print_success_status();
	delay(60);
}

static void do_one_auto_tree(struct auto_tree *data)
{
	uint8_t x, y;
	for (; data->index < temp_cast_amount; data->index++) {
		for (;;) {
			if (get_best_index() == 0x4b) {
				/* no more squares to cast to */
				/*          redraw_los_targets(); */
				data->index = temp_cast_amount;
				break;
			}

			get_yx(target_index, &y, &x);
			if (x == 0x10) {
				continue;
			}
			if (arena[0][target_index] != 0) {
				continue;
			}


			if (is_tree_adjacent(target_index) == 1) {
				continue;
			}

			if (is_wall_adjacent(target_index) == 1) {
				continue;
			}

			if (los_blocked(target_index) == 1) {
				continue;
			}
			/* do spell cast anim, etc */
			spell_animation();
			/* check success... */
			if (temp_success_flag) {
				arena[0][target_index] = current_spell;
				arena[3][target_index] = g_chaos_state.current_player;
				delay(10);
			} else {
				data->index = temp_cast_amount;
			}
			break;
		}
	}
	/* assume for cpu that the spell was worth casting  */
	/* so cpu doesn't semi check for "line of sight" stuff when seeing if spell is worth casting */
	end_auto_tree();
}

static void auto_cast_trees_castles(void)
{
	uint8_t current_location = wizard_index;
	uint8_t tmp_cast_range = 0xD;

	set_spell_success();
	LUT_index = create_range_table(current_location, tmp_cast_range);
	LUT_index = LUT_index * 2 + 1;
	struct auto_tree data;
	/* loop over all spells... */
	data.index = 0;
	data.i = 0;
	do_one_auto_tree(&data);
}


/* trees castles begins at 9add */
void cast_trees_castles(void)
{
	if (temp_cast_amount == 0) {
		if (current_spell >= SPELL_MAGIC_CASTLE) {
			temp_cast_amount = 1;
		} else {
			temp_cast_amount = 8;
		}

	}
	if (IS_CPU(g_chaos_state.current_player)) {
		/* jump 9b00 */
		if (current_spell < SPELL_MAGIC_CASTLE) {
			/* shadow wood or magic wood - uses the same routine */
			/* jump 9b1c (print player name and call magic wood casting...) */
			temp_cast_amount = 8;
			print_name_spell();
			delay(2);
			auto_cast_trees_castles();

		} else {
			/* check if spell is worth casting... */
			if (arena[0][start_index] < SPELL_MAGIC_CASTLE
					|| arena[0][start_index] >
					SPELL_DARK_CITADEL) {
				temp_cast_amount = 1;
				print_name_spell();
				delay(20);
				auto_cast_trees_castles();
				target_square_found = 1;
			} else {
				target_square_found = 0;
			}
		}

	} else {
		if (current_spell == SPELL_MAGIC_WOOD) {
			/* do magic wood spell cast... */
			auto_cast_trees_castles();
		} else {
			/* do same checks as for creatures */
			if (!player_cast_ok()) {
				redraw_cursor();
				return;
			}
			spell_animation();
			do_creature_spell_if_ok(target_index&0xff);
			draw_all_creatures();
			delay(10);
		}
	}


}



/* */
void cast_wall(void)
{
	if (temp_cast_amount == 0)
		temp_cast_amount = 4;

	if (IS_CPU(g_chaos_state.current_player)) {
		/* 9b85 = wall ai casting */
		ai_cast_wall();
		if (target_square_found != 0) {
			delay(20);
		}
	} else {
		if (!player_cast_ok()) {
			redraw_cursor();
			return;
		}

		do_wall_cast();
	}
}

static void end_magic_missile(void)
{
	/* get the defence of the creature here */
	uint8_t attack,defence;
	uint8_t magic_resistance; /* for sleep/blind */
	int creature = arena[0][target_index];
	if (creature >= WIZARD_INDEX) {
		/* wiz */
		uint8_t plyr = creature - WIZARD_INDEX;
		if (plyr > 7) {
			/* something has gone wrong.. */
			arena[0][target_index] = 0;
			return;
		}

		defence = players[plyr].defence;
		magic_resistance = players[plyr].magic_resistance; /* for sleep/blind */
		if ((players[plyr].modifier_flag & 0xc0) == 0x40) {
			/* has magic shield */
			defence += 2;
		} else if ((players[plyr].modifier_flag & 0xc0) == 0x80) {
			/* has magic armour */
			defence += 4;
		}

	} else {
		/* creature */
		defence = CHAOS_SPELLS[creature].defence;
		magic_resistance = CHAOS_SPELLS[creature].magicRes;
	}

	defence += GetRand(10);
	magic_resistance += GetRand(10);
	attack = 3 + GetRand(10);

	if (current_spell == SPELL_LIGHTNING) {
		attack += 3;

	}

	if (current_spell == SPELL_MAGIC_SLEEP || current_spell == SPELL_BLIND) {
		/* for blind/sleep */
		temp_success_flag = 0;
		if (attack > magic_resistance) {
			temp_success_flag = 1;
			sleep_blind();
		}
		if (current_spell == SPELL_MAGIC_SLEEP) {
			delay(4);
		}
		print_success_status();
		return;
	}

	if (attack < defence)
		return;

	delay(4);

	/* do the pop animation... */
	draw_pop_frame_2(target_index);

	/* new code... */
	if (arena[4][target_index] == 0) {
		/* nothing in arena 4... */
		if (arena[0][target_index] >= WIZARD_INDEX) {
			/* was a wizard, do wizard death anim... */
			kill_wizard();
		} else {
			/* remove the creature */
#ifdef _HEADLESS
			output_creature_killed(g_chaos_state.current_player, wizard_index, target_index, IS_ILLUSION(arena[3][target_index]));
#endif
			arena[0][target_index] = 0;
			if (!Options[OPT_OLD_BUGS]
					&& arena[5][target_index] != 0) {
				/* what about dead bodies? */
				/* but only if arena 4 was empty */
				/* bug fix v0.7a (disbelieve failed with old bugs turned off, fixed here too) */
				arena[0][target_index] = arena[5][target_index];	/*creature in arena 5 */
				arena[2][target_index] = 4;	/* dead */
				arena[5][target_index] = 0;	/*clear creature in arena 5  */
			}
		}

	} else {
		/* arena 4 had something in it */
		uint8_t arena4 = arena[4][target_index];
		arena[4][target_index] = 0;
		if (!Options[OPT_OLD_BUGS]) {
			/* an old bug was destroying gooey blob results in creature under blob  */
			/* taking the same owner as the blob...  */
			/* e.g. wizard 1 blob covers wizard 2 creature */
			/* someone kills the blob, wizard 2's creature now belongs to wizard 1! */
			if (arena[0][target_index] == SPELL_GOOEY_BLOB) {
				liberate_from_blob(target_index);
			} else {
				/* the famous "undead wizard" bug is caused by not updating the arena[3] flag properly */
				if (arena4 >= WIZARD_INDEX) {
					arena[3][target_index] = arena4 - WIZARD_INDEX;
				}
			}
		} else {
			/* if old bugs are on, make sure the owner changes */
			int blobOwner = get_owner(target_index);
			if (arena[0][target_index] == SPELL_GOOEY_BLOB) {
				liberate_from_blob(target_index);
				/* set the owner to be that of the blob */
				arena[3][target_index] &= ~7;
				arena[3][target_index] |= blobOwner;
			}
		}
		arena[0][target_index] = arena4;
		arena[5][target_index] = 0;
	}
}

/* the implementation of a magic missile */
/* 9ca9  */
void do_magic_missile(void)
{
	/* spell anim */
	int anim_type = 5;
	if (current_spell == SPELL_LIGHTNING)
		anim_type = 6;
	else if (current_spell == SPELL_MAGIC_SLEEP || current_spell == SPELL_BLIND)
		anim_type = 3;
	/* wait for redraw */

	play_soundfx(SND_BEAM);
	draw_line(target_index, anim_type);

	if (current_spell == SPELL_LIGHTNING) {
		play_soundfx(SND_ELECTRO);
	} else {
		/* a splat sound? */
		play_soundfx(SND_SPELLSUCCESS);
	}

	/* do "splat" animation */
	draw_splat_frame_2(target_index);

	if (arena[0][target_index] == SPELL_MAGIC_FIRE ||
			(arena[0][target_index] >= SPELL_MAGIC_CASTLE
			 && arena[0][target_index] <= SPELL_WALL)) {
		/* no effect */
		return;
	}
	end_magic_missile();
}

/* for lightning too... */
void cast_magic_missile(void)
{
	temp_cast_amount = 1;
	if (IS_CPU(g_chaos_state.current_player)) {
		ai_cast_magic_missile();
	} else {
		/* check this is a vlid square for human casting */
		if (arena[0][target_index] == 0
				|| target_index == wizard_index) {
			/* if no creature, or attacking self, return */
			return;
		}
		/* in range? */
		if (!is_spell_in_range
				(wizard_index, target_index,
				 CHAOS_SPELLS[current_spell].castRange)) {
			clear_message();
			print_text16(_("OUT OF RANGE"), MESSAGE_X, g_message_y,
					12);
			set_text16_colour(12, RGB16(30, 31, 0));	/* yellow */
			return;
		}
		/* do LOS check... */
		if (los_blocked(target_index)) {
			clear_message();
			print_text16(_("NO LINE OF SIGHT"), MESSAGE_X,
					g_message_y, 12);
			set_text16_colour(12, RGB16(31, 30, 0));	/* yellow */
			return;
		}
		/* got to here? well cast the spell! */
		do_magic_missile();
		delay(20);
		temp_cast_amount = 0;
	}

}

static void mutate_group(int dest, int spellId, const char *vals, int size)
{
	int i;
	int r;
	for (i = 0; i < size; i++) {
		if (spellId == vals[i]) {
			/* randomly mutate other things */
			r = 10;
			while (r >= size || vals[r] == spellId)
				r = GetRand(r);
			arena[0][dest] = vals[r];
			draw_pop_frame_2(dest);
			delay(4);
			temp_success_flag = 1;
			break;
		}
	}
}

static void do_mutate(void)
{
	/* d53c
	 * cheat: make subversion and mutation always work:
	 * set 0x85c7 0xc3    ; changes conditional jump to always jump
	 * set 0x791c 70      ; change spell 2 to mutation
	 * set 0x791e 16      ; change spell 3 to horse
	 */
	int target = 0;
	int targetOwner = 0;
	int loopCount = 0;
	int startIndex = 0;
	int i;
	temp_success_flag = 0;
	target = arena[0][target_index];
	if (target >= WIZARD_INDEX) {
		/* for wizards, loop over all their creatures */
		loopCount = ARENA_SIZE;
		targetOwner = target - WIZARD_INDEX;
	} else {
		/* for creature, just mutate that one */
		targetOwner = get_owner(target_index);
		startIndex  = target_index;
		loopCount = startIndex + 1;
	}
	int r, spellId;

	for (i = startIndex; i < loopCount; ++i) {
		if (get_owner(i) != targetOwner)
			continue;
		spellId = arena[0][i];
		/* if it is a creature, is not dead and is the owner's, then mutate. */
		if (spellId >= SPELL_KING_COBRA
			&& spellId <= SPELL_ZOMBIE
			&& !is_dead(i)) {
			/* mutate to a random creature. */
			r = GetRand(SPELL_ZOMBIE + 1);
			while (r < SPELL_KING_COBRA || r == spellId) {
				r = GetRand(SPELL_ZOMBIE + 1);
			}
			arena[0][i] = r;
			draw_pop_frame_2(i);
			delay(4);
			temp_success_flag = 1;
		} else {
			static const char woods[] = { SPELL_MAGIC_WOOD, SPELL_SHADOW_WOOD, SPELL_WALL };
			/* randomly mutate walls and trees to other things */
			mutate_group(i, arena[0][i], woods, sizeof(woods));
		}
	}
}

static void do_subversion(void)
{
	/* do the actual casting of subvert - target_index is valid 859c */
	if (IS_CPU(g_chaos_state.current_player)) {
		print_name_spell();
		delay(20);
	}

	spell_animation();

	temp_success_flag = 0;
	if (!(arena[3][target_index] & 0x10)) {
		/* if BIT 4 is set, the spell would fail  */
		/* i.e. we are in (if !illusion)... */

		uint8_t magic_res = CHAOS_SPELLS[arena[0][target_index]].magicRes;
		magic_res++;

		uint8_t r = GetRand(10);
		if (r > magic_res) {
			/* A possible bug in the original: Subversion succeeded if the random value */
			/* was less than magic resistance! Fixed here. See code at 85c7. */
			if (current_spell == SPELL_MUTATION) {
				/* do this before the end-of-spell */
				do_mutate();
			} else {
				uint8_t creature_val = arena[3][target_index];
				creature_val &= 0xF8;	/* mask lower 3 bits */
				creature_val |= g_chaos_state.current_player;
				arena[3][target_index] = creature_val;
				temp_success_flag = 1;
			}
		}
	}
#ifdef _HEADLESS
	output_cast_disbelieve(g_chaos_state.current_player, wizard_index, target_index, current_spell, temp_success_flag);
#endif
	end_auto_tree();

}

void cast_subversion(void)
{
	temp_cast_amount = 1;
	if (IS_CPU(g_chaos_state.current_player)) {
		ai_cast_subversion();
		if (target_square_found) {
			do_subversion();
			temp_cast_amount = 0;
		}
	} else {
		/* human... */
		if (current_spell == SPELL_SUBVERSION && (arena[0][target_index] >= SPELL_GOOEY_BLOB
				|| arena[4][target_index] != 0
				|| arena[0][target_index] == 0)) {
			/* can't cast here, as the creature is ridden by a wizard,  */
			/* is not a proper creature or there's nothing here */
			redraw_cursor();
			return;
		}

		if (!is_spell_in_range(wizard_index, target_index,
				 CHAOS_SPELLS[current_spell].castRange)) {
			clear_message();
			print_text16(_("OUT OF RANGE"), MESSAGE_X, g_message_y,
					12);
			set_text16_colour(12, RGB16(30, 31, 0));	/* lblue */
			redraw_cursor();
			return;
		}
		/* do LOS check... */
		if (los_blocked(target_index)) {
			clear_message();
			print_text16(_("NO LINE OF SIGHT"), MESSAGE_X,
					g_message_y, 12);
			set_text16_colour(12, RGB16(31, 30, 0));	/* lblue */
			redraw_cursor();
			return;
		}
		/* got to here then spell is valid */
		do_subversion();
	}

}

void cast_chaos_law(void)
{
	/* no need for AI on this one... the spell is really scraping the barrel! */

	/* if cpu, print player name and spell */
	if (IS_CPU(g_chaos_state.current_player)) {
		print_name_spell();
		delay(20);
	}
	/* do cast chance and update world chaos */
	set_spell_success();

	end_auto_tree();
}

static int decr_cast_amount(void)
{
	temp_cast_amount--;
	return 0;
}

/* casts justice, dark power etc */
void cast_justice(void)
{
	if (temp_cast_amount == 0) {
		/* setup cast amount */
		temp_cast_amount = 1;
		if (current_spell >= SPELL_DARK_POWER)
			temp_cast_amount = 3;
	}

	if (IS_CPU(g_chaos_state.current_player)) {
		/* this will always be chosen, but might not actually be any good */
		ai_cast_justice();

	} else {
		/* code from 9df4 - spell must have passed initial cast rate to get this far */
		clear_message();
		if (temp_success_flag == 0) {
			/* not strictly needed ? */
			temp_cast_amount = 0;
			return;
		}
		/* ok, got here then check spell is OK to cast here... */
		if (arena[0][target_index] == 0)
			return;

		/* 9e27 - only allow on proper creatures and wizards */
		if (arena[0][target_index] < WIZARD_INDEX
				&& arena[0][target_index] >= SPELL_GOOEY_BLOB)
			return;

		do_justice_cast();
		decr_cast_amount();
	}

}

void cast_raise_dead(void)
{
	if (temp_cast_amount == 0) {
		/* setup cast amount */
		temp_cast_amount = 1;
	}

	if (IS_CPU(g_chaos_state.current_player)) {

		/* checks if it is worht casting, and casts if so */
		/* otherwise sets relevent flags to ignore this spell */
		ai_cast_raisedead();

	} else {
		/* check that can be cast on this square */
		/* code from 861a */
		if (arena[0][target_index] == 0)
			return;
		if (!is_dead(target_index))
			return;

		if (!is_spell_in_range
				(wizard_index, target_index,
				 CHAOS_SPELLS[current_spell].castRange)) {
			clear_message();
			print_text16(_("OUT OF RANGE"), MESSAGE_X, g_message_y,
					12);
			set_text16_colour(12, RGB16(30, 31, 0));	/* lblue */
			return;
		}
		/* do LOS check... */
		if (los_blocked(target_index)) {
			clear_message();
			print_text16(_("NO LINE OF SIGHT"), MESSAGE_X,
					g_message_y, 12);
			set_text16_colour(12, RGB16(31, 30, 0));	/* lblue */
			return;
		}
		do_raisedead_cast();
	}

}

/*/////////////  wizard effect spells /////////////////////// */

/* set up the spell, do the cast the effect */
static void setup_wizard_spell(void)
{
	/* code from 8357 */
	if (IS_CPU(g_chaos_state.current_player)) {
		print_name_spell();
		delay(20);
	}
	set_spell_success();
	set_current_player_index();
	remove_cursor();
	spell_animation();
}

static void end_shield(void)
{
	if (temp_success_flag) {
		players[g_chaos_state.current_player].image = GFX_MAGIC_SHIELD;
		players[g_chaos_state.current_player].modifier_flag &= 0x7f;
		players[g_chaos_state.current_player].modifier_flag |= 0x40;
		delay(4);
	}
	temp_cast_amount = 0;
}

static void do_shield_cast(void)
{
	setup_wizard_spell();
	end_shield();
	print_success_status();
	wait_for_keypress();
	wait_for_letgo();
}

void cast_magic_shield(void)
{
	if (IS_CPU(g_chaos_state.current_player)) {
		if ((players[g_chaos_state.current_player].modifier_flag & 0x40)) {
			/* already have something similar... */
			target_square_found = 0;
			return;
		}
		target_square_found = 1;
		do_shield_cast();
	} else {
		do_shield_cast();
	}

}

static void end_armour(void)
{
	if (temp_success_flag) {
		players[g_chaos_state.current_player].image = GFX_MAGIC_ARMOUR;
		players[g_chaos_state.current_player].modifier_flag |= 0xC0;
		delay(4);
	}
	print_success_status();
	temp_cast_amount = 0;
}

static void do_armour_cast(void)
{
	setup_wizard_spell();
	end_armour();
	wait_for_keypress();
	wait_for_letgo();
}

void cast_magic_armour(void)
{
	if (IS_CPU(g_chaos_state.current_player)) {
		if ((players[g_chaos_state.current_player].modifier_flag & 0x80)) {
			/* already have something similar... */
			target_square_found = 0;
			return;
		}
		target_square_found = 1;
		do_armour_cast();
	} else {
		do_armour_cast();
	}
}

static void end_sword(void)
{

	if (temp_success_flag) {
		players[g_chaos_state.current_player].image = GFX_MAGIC_SWORD;
		players[g_chaos_state.current_player].modifier_flag &= 0xfc;	/* 11111100  - low 2 bits set to 0 */
		players[g_chaos_state.current_player].modifier_flag |= 0x04;
		delay(4);
	}
	print_success_status();
}

static void do_sword_cast(void)
{
	setup_wizard_spell();
	end_sword();
	wait_for_keypress();
	wait_for_letgo();
}

void cast_magic_sword(void)
{
	if (IS_CPU(g_chaos_state.current_player)) {
		if ((players[g_chaos_state.current_player].modifier_flag & 0x04)) {
			/* already have something similar... */
			target_square_found = 0;
			return;
		}
		target_square_found = 1;
		do_sword_cast();
	} else {
		do_sword_cast();
	}
}

static void end_knife_cast(void)
{
	if (temp_success_flag) {
		players[g_chaos_state.current_player].image = GFX_MAGIC_KNIFE;
		players[g_chaos_state.current_player].modifier_flag &= 0xf8;	/* 11111000  - low 3 bits set to 0 */
		players[g_chaos_state.current_player].modifier_flag |= 0x02;
		delay(4);
	}
	print_success_status();
	temp_cast_amount = 0;
}

static void do_knife_cast(void)
{
	setup_wizard_spell();
	end_knife_cast();
	wait_for_keypress();
	wait_for_letgo();
}

void cast_magic_knife(void)
{
	if (IS_CPU(g_chaos_state.current_player)) {
		if ((players[g_chaos_state.current_player].modifier_flag & 0x07)) {
			/* already have something similar... */
			target_square_found = 0;
			return;
		}
		target_square_found = 1;
		do_knife_cast();
	} else {
		do_knife_cast();
	}
}

/* 8486 */
static void end_bow_cast(void)
{
	if (temp_success_flag) {
		players[g_chaos_state.current_player].image = GFX_MAGIC_BOW;
		players[g_chaos_state.current_player].ranged_combat = 3;
		players[g_chaos_state.current_player].range = 6;
		delay(4);
	}
	print_success_status();
	temp_cast_amount = 0;
}

static void do_bow_cast(void)
{
	setup_wizard_spell();
	end_bow_cast();
	wait_for_keypress();
	wait_for_letgo();
}

void cast_magic_bow(void)
{
	if (IS_CPU(g_chaos_state.current_player)) {
		if (players[g_chaos_state.current_player].ranged_combat) {
			/* already fitted with the bow */
			target_square_found = 0;
			return;
		}
		target_square_found = 1;
		do_bow_cast();
	} else {
		do_bow_cast();
	}
}

/*  */
static void end_wings_cast(void)
{
	if (temp_success_flag) {
		players[g_chaos_state.current_player].image = GFX_MAGIC_WINGS;
		players[g_chaos_state.current_player].modifier_flag |= 0x20;
		delay(4);
	}
	print_success_status();
	temp_cast_amount = 0;
}

static void do_wings_cast(void)
{
	setup_wizard_spell();
	end_wings_cast();
	wait_for_keypress();
	wait_for_letgo();
}

void cast_magic_wings(void)
{
	if (IS_CPU(g_chaos_state.current_player)) {
		if (players[g_chaos_state.current_player].modifier_flag & 0x20) {
			/* already fitted with wings */
			target_square_found = 0;
			return;
		}
		target_square_found = 1;
		do_wings_cast();
	} else {
		do_wings_cast();
	}
}

static void end_shadowform_cast(void)
{
	if (temp_success_flag) {
		players[g_chaos_state.current_player].modifier_flag |= 0x8;
		delay(4);
	}
	print_success_status();
	temp_cast_amount = 0;
}

static void do_shadowform_cast(void)
{
	setup_wizard_spell();
	end_shadowform_cast();
	wait_for_keypress();
	wait_for_letgo();
}

void cast_shadow_form(void)
{
	if (IS_CPU(g_chaos_state.current_player)) {
		if ((players[g_chaos_state.current_player].modifier_flag & 0x8) == 0x8) {
			/* already have */
			target_square_found = 0;
			return;
		}
		target_square_found = 1;
		do_shadowform_cast();
	} else {
		do_shadowform_cast();
	}
}

/*/////////////  end wizard effect spells /////////////////////// */

static void do_meditate_cast(void)
{
	setup_wizard_spell();
	if (temp_success_flag) {
		/* new spell */
		new_random_spell(g_chaos_state.current_player);
		delay(4);
	}
	print_success_status();
}

void cast_meditate(void)
{
	/* If a Wizard does not cast a spell or move for one turn, there could be a
	 * 10% chance of the Wizard gaining another spell. */
	if (IS_CPU(g_chaos_state.current_player)) {
		ai_cast_meditate();
		if (target_square_found != 0)
			do_meditate_cast();
	} else {
		temp_cast_amount = 1;
		do_meditate_cast();
	}
}


void cast_turmoil(void)
{
	if (IS_CPU(g_chaos_state.current_player)) {

		ai_cast_turmoil();

		/* // computer should never cast turmoil! */
		/* target_square_found = 0; */
		/* temp_cast_amount =0; */
	} else {
		/* humans can cast turmoil at their own risk! */
		temp_cast_amount = 1;
		do_turmoil_cast();
	}
}

void do_wall_cast(void)
{
	spell_animation();
	do_creature_spell_if_ok(target_index&0xff);
	draw_all_creatures();
	load_all_palettes();
}

void do_justice_cast(void)
{
	/* code from 9e38 onwards */
	/* do sound fx, flashing graphics */
	int i, j, k;
	move_screen_to(target_index);
	play_soundfx(SND_JUSTICE);

	for (i = 0; i < 3; i++) {
		for (k = 0; k < 3; k++) {
			for (j = 0; j < 8; j++) {
				/* do the flashing gfx... */
				platform_wait();
				platform_wait();
				if (arena[0][target_index] >= WIZARD_INDEX) {
					int player = arena[0][target_index] - WIZARD_INDEX;
					if (player < 8) {
						draw_silhouette_gfx(target_index,
								    WizardGFX[players[player].image].pGFX,
								    WizardGFX[players[player].image].pMap,
								    chaos_cols[j], 11, 0);	/*  final 0 is to draw the gfx "positive" */
					}
				} else {
					draw_silhouette_gfx(target_index,
							CHAOS_SPELLS[arena[0][target_index]].pGFX,
							CHAOS_SPELLS[arena[0][target_index]].pGFXMap,
							chaos_cols[j], 11, 0);	/*  final 0 is to draw the gfx "positive" */
				}

			}

		}
	}
	enable_interrupts();


	/* 9e96 */
	/* refresh arena */
	/* set the has cast spell flag */

	/* get magic resistance of creature... */
	uint8_t magres;
	if (arena[0][target_index] >= WIZARD_INDEX) {
		/* wizard res */
		magres =
			players[arena[0][target_index] -
			WIZARD_INDEX].magic_resistance;
	} else {
		/* creature res */
		magres = CHAOS_SPELLS[arena[0][target_index]].magicRes;
	}
	magres++;

	uint8_t success = (GetRand(10) >= magres);
#ifdef _HEADLESS
	output_magic_attack(g_chaos_state.current_player, wizard_index, target_index, current_spell, success);
#endif

	if (success) {
		/* spell succeeded... */
		/* do pop anim */
		draw_pop_frame_2(target_index);
		if (arena[0][target_index] >= WIZARD_INDEX) {
			/* wizard - destroy all his creations! */
			delay(4);
			uint8_t wizard = arena[0][target_index] - WIZARD_INDEX;
			destroy_all_creatures(wizard);
#ifdef _HEADLESS
			output_wizard_all_creatures_destroyed(wizard, target_index);
#endif
		} else {
			/* single creature only... */
			/* there's the famous "rise from the dead" bug here... */

#ifdef _HEADLESS
			output_creature_killed(g_chaos_state.current_player, wizard_index, target_index, IS_ILLUSION(arena[3][target_index]));
#endif
			arena[0][target_index] = 0;
			if (arena[4][target_index] != 0) {
				arena[0][target_index] = arena[4][target_index];
				arena[4][target_index] = 0;
			} else if (arena[5][target_index] != 0) {
				arena[0][target_index] = arena[5][target_index];
				arena[5][target_index] = 0;
				if (!Options[OPT_OLD_BUGS]) {
					/* make sure this creature is dead, as arena 5 creatures are dead bodies */
					arena[2][target_index] = 4;
				}
			}
		}
		delay(4);
	}
	if (!IS_CPU(g_chaos_state.current_player))
		redraw_cursor();
}

static int end_raise_dead(intptr_t args)
{
	int idx = (int)args;
	/* set target frame to 0 */
	arena[2][idx] = 0;
	uint8_t flag = 0x60;		/* bits 5 & 6 - is real, is known to the ai to be real and is undead */
	flag |= g_chaos_state.current_player;
	/* update this creature's flag val so it is undead and this player's */
	arena[3][idx] = flag;
	arena[5][idx] = 0;	/* just in case */
	temp_cast_amount = 0;
	print_success_status();
	return 0;
}

/* code from 86c3 */
void do_raisedead_cast(void)
{
	/* implementation of the raise dead spell */
	/* used by human and cpu players */

	/* spell animation to target square */
	spell_animation();
	set_spell_success();

	if (!temp_success_flag) {
		temp_cast_amount = 0;
		print_success_status();
		delay(40);
		return;
	}
	int idx = target_index;
	end_raise_dead(idx);
	delay(40);
}

static void end_disbelieve(void)
{
	/* check arena 3 value, bit 4 */
	if (IS_ILLUSION(arena[3][target_index])) {
		/* place the arena 4 riding wizard, or 0, in arena 0 */
		arena[0][target_index] = arena[4][target_index];
		arena[4][target_index] = 0;
		if (!Options[OPT_OLD_BUGS] && arena[5][target_index] != 0
				&& arena[0][target_index] == 0) {
			/* what about dead bodies? */
			/* but only if arena 4 was empty */
			/* bug fix v0.7a (disbelieve failed with old bugs turned off) */
			arena[0][target_index] = arena[5][target_index];	/*creature in arena 5 */
			arena[2][target_index] = 4;	/* dead */
			arena[5][target_index] = 0;	/*clear creature in arena 5  */
		}
		temp_success_flag = 1;
		draw_pop_frame_2(target_index);
	}
	print_success_status();
	/* set bit 5 for this creature (won't matter if disbelieved already...) */
	/* bit 5 is "has been disbelieved" and is used for the AI to know which creatures */
	/* are known to be real and which are not known...  */
	arena[3][target_index] |= 0x20;
}

/* implements the casting of disb...  */
/* code from 9a21 and used by AI and humans */
static void do_disbelieve_cast(void)
{
	spell_animation();
	temp_success_flag = 0;
	end_disbelieve();

}

void cast_disbelieve(void)
{
	if (temp_cast_amount == 0)
		temp_cast_amount = 1;

	if (IS_CPU(g_chaos_state.current_player)) {
		ai_cast_disbelieve();
		if (target_square_found) {
			do_disbelieve_cast();
#ifdef _HEADLESS
			output_cast_disbelieve(g_chaos_state.current_player, wizard_index, target_index, current_spell, temp_success_flag);
#endif
		}

	} else {
		if (arena[0][target_index] == 0
				|| arena[0][target_index] >= SPELL_GOOEY_BLOB)
			return;

		do_disbelieve_cast();
		delay(40);
		temp_cast_amount = 0;
	}
}

void do_turmoil_cast(void)
{
	struct turmoil_context {
		uint8_t start_index;
		uint8_t arena0, arena2, arena3, arena4, arena5;
		int i;
	} tc;

	tc.start_index = start_index;
	set_spell_success();
	temp_success_flag = 1;
#ifdef _HEADLESS
	output_cast(g_chaos_state.current_player, wizard_index, current_spell, temp_success_flag);
#endif
	if (temp_success_flag) {
		/* unset bit 7 of all things */
		unset_moved_flags();
		for (tc.i = 0; tc.i < 0x9f; tc.i++) {
			/* loop over the whole arena */
			if (arena[0][tc.i] == 0)
				continue;
			if (HAS_MOVED(arena[3][tc.i]))	/* moved and turmoil effect the same BIT */
				continue;
			/* store the start square data */
			tc.arena0 = arena[0][tc.i];
			tc.arena2 = arena[2][tc.i];
			tc.arena3 = arena[3][tc.i];
			tc.arena4 = arena[4][tc.i];
			tc.arena5 = arena[5][tc.i];
			/* get a new square to send this baby to */
			uint8_t x, y;
			uint8_t r;
			r = GetRand(10) + GetRand(255);
			if (r < 0x9f) {
				get_yx(r, &y, &x);
			} else {
				/* make sure we get another val */
				x = 0x10;
			}

			/*
			   All these squares are full:

			   1  2  3
			   4  5  6
			   7  8  9

			   Start at 0
			   get random, eg "3"
			   Aha! can't go there, get new random square...
			   Whatever square we get is full
			   and so on for ever...
			   */

			while (r >= 0x9f || x >= 0x10 || arena[0][r] != 0) {
				r = GetRand(10) + GetRand(255);
				if (r < 0x9f) {
					get_yx(r, &y, &x);
				} else {
					x = 0x10;
				}

			}

			/* got a new square to cast to */
			target_index = r;
			start_index = tc.i;
			spell_animation();

			arena[0][target_index] = tc.arena0;
			arena[2][target_index] = tc.arena2;
			arena[3][target_index] = tc.arena3 | 0x80;	/* moved/turmoiled */
			arena[4][target_index] = tc.arena4;
			arena[5][target_index] = tc.arena5;

			arena[0][tc.i] = 0;
			arena[1][tc.i] = 1;
			arena[4][tc.i] = 0;
			arena[5][tc.i] = 0;

			enable_interrupts();
			delay(4);
			disable_interrupts();
		}
	}
	start_index = tc.start_index;
	print_success_status();
	temp_cast_amount = 0;
}
