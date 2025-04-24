/* computer.c */
#include <stdio.h>
#include <stdlib.h>
#include "chaos/platform.h"

#include "chaos/computer.h"
#include "chaos/movement.h"
#include "chaos/arena.h"
#include "chaos/wizards.h"
#include "chaos/magic.h"
#include "chaos/casting.h"
#include "chaos/creature.h"
#include "chaos/rand.h"
#include "chaos/spelldata.h"
#include "chaos/chaos.h"
#include "chaos/gfx.h"
#include "chaos/pathfinder.h"

#ifdef _HEADLESS
#include "chaos/output.h"
#endif

unsigned char target_square_found;
unsigned char move_table_created;
unsigned char has_wizard_moved;
unsigned char flying_target_found;
unsigned short LUT_index;
unsigned short priority_offset;
static int last_move_index;
/* create the priority table - stored at d3f2 */
/* will contain {priority, index} for all living enemy creatures */
#define PRIORITY_TABLE_SIZE  330
static unsigned char *prio_table = 0;
static uint8_t target_count;
/* create a table of the strongest wizards, based on quantity/wuality of creatures */
static uint8_t prio_table_index;

/* attack preference array... */
/* taken from c6d4 */
/* except troll, which is new */
const char attack_pref[48] =
{
	0x08 /*cobra */ , 0x0b /*wolf */ , 0x07 /*goblin */ , 0x09 /*croc */ ,
	0x09 /*troll */ , 0x09 /*faun */ , 0x0f /*lion */ , 0x0e /*elf */ ,
	0x06 /*orc */ , 0x0e /*bear */ , 0x0b /*goril */ , 0x0a /*ogre */ ,
	0x0d /*hydra */ , 0x09 /*rat */ , 0x10 /*giant */ , 0x0b /*horse */ ,
	0x0f /*unicor */ , 0x0f /*cent */ , 0x0f /*peg */ , 0x10 /*gryph */ ,
	0x15 /*manti */ , 0x0f /*bat */ , 0x19 /*green */ , 0x19 /*red */ ,
	0x1b /*gold */ , 0x11 /*harpy */ , 0x12 /*eagle */ , 0x13 /*vamp */ ,
	0x0d /*ghost */ , 0x09 /*spectre */ , 0x0b /*wraith */ , 0x07 /*skeleton */ ,
	0x04 /*zombie */ , 0x04 /*blob */ , 0x04 /*fire */ , -0x01 /*wood */ ,
	0x01 /*shadow */ , 0x00 /*castle */ , 0x00 /*cit */ , 0x00 /*wall */ ,
	/*wizards */
	0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
};

/* based on code at cdaa */
static void reset_priority_table(void)
{
	if (prio_table == 0)
		prio_table = malloc(PRIORITY_TABLE_SIZE);
	int index = 0;
	int i;
	for (i = 0; i < 0x9E; i++) {
		prio_table[index] = 0x00;
		index++;
		prio_table[index] = 0xFF;
		index++;
	}
}

/* check the square in index to see if it contains a live enemy creature */
/* based on code at c67a */
static uint8_t contains_enemy(uint16_t index)
{
	uint8_t creature = arena[0][index];
	if (creature < SPELL_KING_COBRA) {
		return 0;	/* jr c6d2 - was if creature == 0 jump, but that is buggy */
	}
	if (is_dead(index)) {

		/* frame 4 showing, i.e. dead */
		return 0;	/* jr c6d2 */
	}
	uint8_t owner = get_owner(index);	/* mask lower 3 bits, the owner */
	if (owner == g_chaos_state.current_player) {
		return 0;	/* jr c6d2 */
	}

	/* got to here? contains a living enemy creature.... */
	/* get the "creature attack preference" value */
	uint16_t range;
	get_distance(index, start_index, &range);
	uint16_t pref = attack_pref[creature - SPELL_KING_COBRA] + 4;

	/* pref += var_cc55; // no idea what is stored here... 0x60 for lightning */
	pref += priority_offset;	/* usually 0, but can be other values when we care about all creatures */
	if ((pref - range) < 0) {
		return 0;
	}
	return (pref - range);
}

/* based on code at c7bc */
static void create_table_enemies(void)
{
	int i;
	start_index = wizard_index;
	uint16_t index = 0;		/* index to prio table */
	target_index = 0;	/* the current "target" square */
	target_count = 0;	/* the number of targets */
	reset_priority_table();
	uint8_t valid = 0;
	uint16_t range;
	for (i = 0; i < 0x9f; i++) {
		target_index = i;
		valid = contains_enemy(target_index);
		if (valid > 0) {
			target_count++;
			get_distance(target_index, start_index, &range);
			range = range >> 1;
			valid += 0x14;
			valid -= range;
			prio_table[index] = valid;
			index++;
			prio_table[index] = target_index;
			index++;
		}
	}
}

/* create an enemy table entry for this wizard */
/* code at c859 */
static void create_enemy_table_entry(uint8_t wizardid)
{
	if (wizardid == g_chaos_state.current_player)
		return;
	prio_table[prio_table_index + 1] = target_index;
	int i;
	for (i = 0; i < ARENA_SIZE; i++) {
		int here = arena[0][i];
		if (here >= SPELL_KING_COBRA && here < SPELL_MAGIC_CASTLE
				&& get_owner(i) == wizardid) {
			prio_table[prio_table_index] += (attack_pref[here] / 4);
		}
	}
	/* add priority offset for wizards regardless of creatures */
	prio_table[prio_table_index] += priority_offset;
	prio_table_index += 2;
}


static void create_table_wizards(void)
{
	/* code is from c825  */
	reset_priority_table();

	/*  char str[20]; */
	prio_table_index = 0;
	int i;
	target_index = 0;
	for (i = 0; i < 0x9f; i++) {
		if (arena[0][target_index] >= WIZARD_INDEX) {
			create_enemy_table_entry(arena[0][i] -
					WIZARD_INDEX);
		}
		if (arena[4][target_index] >= WIZARD_INDEX) {
			create_enemy_table_entry(arena[4][i] -
					WIZARD_INDEX);
		}
		target_index++;
	}
	order_table(8, prio_table);
}

/* based on code at c78d */
/* additionally, pass in the attakcer - or 0xff if no attacker ready  */
/* and check "current spell" value */
static uint8_t get_strongest_wizard(int attacker_index)
{
	if (players[g_chaos_state.current_player].timid < 0xA) {
		/* if we have less than 10 in this value,  */
		/* go for the wizard who's creature is closest to us */
		create_table_enemies();

		/* the best value will now be the index of the creature closest to us */
		/* or the wizard closest to us, as they are treated the same in the create_table_enemies routine */
		uint8_t enemy_creature = prio_table[0];
		if (enemy_creature == 0) {
			/* no dangerous creature found.. */
			/* create the priority table based on "strongest wizard" - one with most/best creatures */
			create_table_wizards();
			target_index = prio_table[1];

			/* in the actual game it does a pointless thing here... */
			return target_index;
		} else {
			/* first of all, get the most dangerous creature... regardless of whether we can kill it or not */
			/* if we can't kill it, later we will attack its owner to do away with it */
			order_table(target_count, prio_table);
			enemy_creature = prio_table[1];
			uint8_t enemy_wizard = get_owner(enemy_creature);
			int i;
			for (i = 0; i < 0x9f; i++) {
				if (arena[0][i] - WIZARD_INDEX == enemy_wizard) {
					enemy_wizard = i;
					break;
				}
				if (arena[4][i] - WIZARD_INDEX == enemy_wizard) {
					enemy_wizard = i;
					break;
				}
			}

			/* enemy wizard = index to the enemy wizard attacking us */
			/* enemy creature = index to the enemy creature attacking us */
			/* now see which is closer and attack them */
			uint16_t creature_range, wizard_range;
			get_distance(enemy_creature, start_index,
					&creature_range);
			get_distance(enemy_wizard, start_index,
					&wizard_range);

			/*
			   Magic Fire close to a wizard confuses the enemy creatures
			   Try this out, so that fire is not attractive
			   Also, undead creatures should not be attractive to living ones
			   */
			uint8_t attacker_undead = 0;
			if (attacker_index == 0xff) {
				/* a creature spell to cast... so use the spellid */
				attacker_undead = (current_spell >= SPELL_VAMPIRE
					 && current_spell <= SPELL_ZOMBIE);
			} else {
				/* is a valid square */
				/* see if the attacking square is undead */
				attacker_undead = IS_UNDEAD(arena[3][attacker_index])
					|| (arena[0][attacker_index] >= SPELL_VAMPIRE
						&& arena[0][attacker_index] <= SPELL_ZOMBIE);
			}
			uint8_t defender_undead = 0;
			int tmp_prio_index = 1;
			while (enemy_creature != 0xFF) {
				enemy_creature = prio_table[tmp_prio_index];
				if (arena[0][enemy_creature] != SPELL_MAGIC_FIRE) {
					/* not magic fire so check undeadness */
					/* remember that we are still scared of magic fire and undead creatures */
					/* just that we are not worried about attacking it */
					defender_undead = (arena[0][enemy_creature] >= SPELL_VAMPIRE
						 && arena[0][enemy_creature] <= SPELL_ZOMBIE)
						|| IS_UNDEAD(arena[3] [enemy_creature]);
					if (!defender_undead || attacker_undead) {
						break;
					}
				}
				tmp_prio_index += 2;
			}
			if (enemy_creature == 0xFF) {
				return enemy_wizard;
			}
			if (creature_range < wizard_range) {
				return enemy_creature;
			}
			return enemy_wizard;
		}
	} else {

		/* create the priority table based on "strongest wizard" - one with most/best creatures */
		create_table_wizards();
		target_index = prio_table[1];

		/* in the actual game it does a pointless thing here... */
		return target_index;
	}
}

/* get the best square for range attack */
static uint8_t get_best_rangeattack(void)
{
	/* cccb */
	uint8_t creature;
	while (prio_table[LUT_index] != 0xFF) {
		/* check the value at this index... */
		creature = arena[0][prio_table[LUT_index]];
		if (creature >= WIZARD_INDEX
				|| (creature == SPELL_MAGIC_WOOD
					&& arena[4][prio_table[LUT_index]] != 0)
				|| creature < SPELL_MAGIC_FIRE
				|| creature == SPELL_SHADOW_WOOD) {

			/* wizard, or wizard in wood, shadow wood or an actual creature */
			/* anything >= magic fire will be ignored */

			/* ccfd */
			/* check if we are trying to attack an undead creature... */
			/* original chaos didn't check the arena[3] value for raise deaded creatures */
			/* wizards can always attack undead with their magic bow range attacks */
			uint8_t attacker_undead =
				IS_UNDEAD(arena[3][start_index])
				|| (selected_creature >= WIZARD_INDEX);
			uint8_t defender_undead = (creature >= SPELL_VAMPIRE
					&& creature <=
					SPELL_ZOMBIE)
				|| IS_UNDEAD(arena[3][prio_table[LUT_index]]);
			if (!defender_undead || attacker_undead) {
				target_index = prio_table[LUT_index];
				LUT_index++;
				LUT_index++;
				return 0x53;
			}	/* else, trying to range attack undead with a living creature, so get next value...  */
		}
		LUT_index++;
		LUT_index++;
	}
	return 0x4b;
}

/* taken from cd92 - gets the "danger" of the square at i */
/* this is calculated based on enemy creature distance and how dangerous that creature is */
static uint8_t get_priority_val(uint8_t index)
{
	int i;
	uint8_t total = 0;
	start_index = index;
	for (i = 0; i < 0x9e; i++) {
		total += contains_enemy(i);	/* gets the priority val for this square from start_index */
	}
	return total;
}

/* defines for the special moves the wizard can make, with wings or on a mount */
#define SPECIAL_MOVE_MOUNT 1
#define SPECIAL_MOVE_WINGS 2

/* used for wizards on flying mounts or with wings */
static void wizard_flying_move(uint8_t type)
{

	/* based on code at cbb9/cbc7 */
	if (type == SPECIAL_MOVE_MOUNT) {
		tmp_movement_allowance = 1 + (CHAOS_SPELLS[arena[0][target_index]].movement * 2);
	} else {

		/* wings */
		tmp_movement_allowance = 0xd;
	}
	reset_priority_table();

	/*cbcf */
	target_count = 1;
	LUT_index = 0;
	uint8_t tmp_wiz = target_index;
	uint8_t i, x, y;
	uint16_t tmp_dist;
	uint8_t tmp_prio_val;

	/* loop over arena to get priority of all squares in range */
	for (i = 0; i < 0x9e; i++) {
		get_yx(i, &y, &x);
		if (x < 0x10) {
			/* in bounds */
			/* get the distance from wizard start square to this square (i) */
			get_distance(tmp_wiz, i, &tmp_dist);
			if (tmp_movement_allowance >= tmp_dist) {

				/* is in range of the wiz start square... */
				/* find the distance/danger of all enemies relative to this square  (call cd92) */
				/* store as priority // code to here is at cc0d */
				tmp_prio_val = get_priority_val(i);
				prio_table[LUT_index++] = tmp_prio_val;
				prio_table[LUT_index++] = i;
				target_count++;
			}
		}
	}

	/* cc31 */
	order_table(target_count, prio_table);
	start_index = tmp_wiz;
	LUT_index--;
}


/* ca92. */
static void setup_wizard_move(void)
{
	reset_priority_table();

	/* get the arena square with the wizard in it... */
	int i;
	uint8_t tmp_wiz = 0;
	for (i = 0; i < 0x9f; i++) {
		if (arena[0][i] >= WIZARD_INDEX
			&& (arena[0][i] - WIZARD_INDEX) == g_chaos_state.current_player) {
			tmp_wiz = i;
			break;
		}
		if (arena[4][i] >= WIZARD_INDEX
			&& (arena[4][i] - WIZARD_INDEX) == g_chaos_state.current_player) {
			tmp_wiz = i;
			break;
		}
	}
	target_index = tmp_wiz;

	/* check the value in arena 0 at the wizard's location */
	if (arena[0][target_index] >= SPELL_PEGASUS
		&& arena[0][target_index] <= SPELL_MANTICORE) {

		/* wizard on a flying mount... */
		/* cade... */
		/* call cbb9 */
		wizard_flying_move(SPECIAL_MOVE_MOUNT);
		has_wizard_moved = 1;
		return;
	} else {

		/* not on a flying mount (may be on a regular one though) */
		if (arena[0][target_index] >= WIZARD_INDEX
			&& HAS_MAGICWINGS(players[g_chaos_state.current_player].  modifier_flag)
				/* bug fix 26/12/04
				 * If wiz is flying and engaged to undead, causes problems
				 */
			&& tmp_is_flying) {
			/* has magic wings and is in the open */
			/* cafb.... */
			/* call cbc7 */
			wizard_flying_move(SPECIAL_MOVE_WINGS);
			has_wizard_moved = 1;
			return;
		} else {
			/* cb06... */
			uint8_t lookat_i;
			LUT_index = 0;
			for (i = 0; i < 8; i++) {
				lookat_i = apply_position_modifier(target_index, i);
				if (lookat_i == 0) {
					/* out of bounds */
					continue;
				}
				lookat_i--;
				if (arena[0][lookat_i] >= SPELL_MAGIC_CASTLE
					&& arena[0][lookat_i] <= SPELL_DARK_CITADEL) {
					prio_table[LUT_index++] = 0;
					prio_table[LUT_index++] = i;
					target_square_found = 1;
				} else if (arena[0][lookat_i] == SPELL_MAGIC_WOOD) {
					prio_table[LUT_index++] = 1;
					prio_table[LUT_index++] = i;
					target_square_found = 1;
				} else if (arena[0][lookat_i] >= SPELL_HORSE
					&& arena[0][lookat_i] <= SPELL_MANTICORE
					&& get_owner(lookat_i) == g_chaos_state.current_player
					&& arena[4][lookat_i] == 0) {
					/* is a mount and is ours and no one is on it  */
					prio_table[LUT_index++] = 2;
					prio_table[LUT_index++] = i;
					target_square_found = 1;
				} else {
					/* is none of the above */
					int prio = 3;
					/* increase last square to avoid ping-pong moves */
					if (last_move_index != -1 && lookat_i == last_move_index)
						prio += 2;
					prio_table[LUT_index++] = get_priority_val(lookat_i) + prio;
					prio_table[LUT_index++] = i;
				}
			}

			/* cb8b */
			order_table(8, prio_table);
			start_index = tmp_wiz;
			target_index = tmp_wiz;
			LUT_index = 0xF;
		}		/* end if has magic wings/out in open */
	}			/* end if wizard on mount */
	/* cba2 */
	move_table_created = 1;
}


/* taken from b0a8 */
static void do_flying_move(void)
{
	/*char str[30]; */
	uint8_t current_index = start_index;	/* save the current square, just in case */

	/* bug fix 26/12/04
	 * If wiz is flying and engaged to undead, causes problems
	 */
	if (tmp_engaged_flag) {
		tmp_movement_allowance = 3;
	}
	if (!flying_target_found) {
		uint16_t strongest_index;
		if (!has_wizard_moved) {

			/* ae67 - wizard has not moved yet... */
			if (!move_table_created) {

				/* need to create the wizard movement table */
				setup_wizard_move();
			}

			/* code here really follows after cc31 */
			/* gets the "worst" square for the wizard (i.e. least dangerous) */
			while (prio_table[LUT_index] == 0xFF) {
				if (LUT_index > 2) {
					LUT_index -= 2;
				} else {
					movement_b();
					return;
				}
			}
			strongest_index = prio_table[LUT_index];
		} else {

			/* wizard has moved already..  */
			/* need to create the creature movement table */
			reset_priority_table();
			strongest_index =
				get_strongest_wizard(current_index);
		}
		start_index = current_index;

		/* for creatures, si will be the one closest to the "best" enemy */
		/* for wizards, will be the safest square in range */
		int16_t in_range =
			create_range_table(strongest_index,
					tmp_movement_allowance);
		LUT_index = in_range * 2 + 1;
		flying_target_found = 1;
	}
	if (get_best_index() == 0x53) {
		start_index = current_index;

		/* found one! target_index contains the target square... */
		flying_target_found = 1;

		/* move_screen_to(target_index); */
		clear_message();
		do_movement_accept();
		delay(10);
	} else {
		/* ran out... */
		movement_b();
	}
}

/* code based on c8c7 */
/* get the priorities of squares surrounding "index" based on distance from "strongest" */
static void get_surrounding_square_prios(uint8_t index, uint8_t strongest)
{
	reset_priority_table();
	int i;
	uint16_t range;
	uint8_t lookat_i;
	LUT_index = 0;
	int astarbest = path_find(index, strongest);
	for (i = 0; i < 8; i++) {
		lookat_i = apply_position_modifier(index, i);
		if (lookat_i == 0) {
			/* out of bounds */
			continue;
		}
		lookat_i--;
		get_distance(strongest, lookat_i, &range);
		if (astarbest == lookat_i)
			range = 0;

		/* add a check in here to make sure we don't */
		/* tread on a magic wood. This table is ordered largest */
		/* first. But the potential squares are read starting  */
		/* at the end (LUT_index, then LUT_index-2, etc) */
		if (arena[0][lookat_i] == SPELL_MAGIC_WOOD)
			range += 1;	/* "increase" the distance so it isn't as appealing */
		/* careful not to increase too much, may go backwards! */
		if (last_move_index != -1 && last_move_index == lookat_i)
			range += 2;	/* increase last square to avoid ping-pong moves */
		prio_table[LUT_index++] = range;
		prio_table[LUT_index++] = i;
	}
	order_table(8, prio_table);
	LUT_index = 0xF;
}


/* code based on that at c8c7... */
static void setup_creature_move(void)
{
	reset_priority_table();
	uint8_t current_index = target_index;	/* save the current square, just in case */
	uint16_t strongest_index;
	strongest_index = get_strongest_wizard(current_index);
	get_surrounding_square_prios(current_index, strongest_index);
	start_index = current_index;
	target_index = current_index;
	move_table_created = 1;
}

/* GOTO label - for range attacks jump here */
static void range_attacking(void)
{
	int loops = 0;
	while (tmp_range_attack != 0) {
		/* range attack... this is identical for flying or not, wiz or not... */
		/* get the best value (call cccb) */
		if (get_best_rangeattack() == 0x4b) {
			/* the best square is crap */
			movement_b();
			return;
		} else {
			/* the target square is good */
			do_movement_accept();
		}
		/* infinite loop bug... */
		loops++;
		if (loops >= ARENA_SIZE)
			tmp_range_attack = 0;
	}
	delay(10);
}


static void do_this_movement(void)
{
	/* this is effectively the first half of the movement polling */
	/* the code in Chaos starts at ae50 and is only called if a  */
	/* creature is successfully selected... */
	while (selected_creature != 0) {
		if (tmp_is_flying) {
			/* jump b0a8 */
			do_flying_move();

			/* what if it becomes engaged? */
			/* in chaos code, jumps to ae7a */
			if (tmp_range_attack != 0) {
				return range_attacking();
			}
		} else {

			/* walking creature... */
			if (!has_wizard_moved) {
				/* ae67 - wizard has not moved yet... */
				if (!move_table_created) {
					/* need to create the wizard movement table */
					setup_wizard_move();
				}
			} else {
				/* wizard has moved already..  */
				/* ae70 */
				if (!move_table_created) {
					/* need to create the creature movement table */
					setup_creature_move();
				}
			}

			/* get the first best square to move to... */
			while (prio_table[LUT_index] == 0xFF) {
				if (LUT_index > 2) {
					LUT_index -= 2;
				} else {
					movement_b();
					/* if we have range attack after our movement, make sure it is handled properly */
					/* best solution is a goto here... otherwise I'd have a load of confusing ifs */
					if (tmp_range_attack != 0) {
						return range_attacking();
					}
					return;
				}
			}
			target_index = apply_position_modifier(start_index, prio_table[LUT_index]);
			if (target_index == 0) /* possible mess up */
				break;
			target_index--;
			clear_message();
			last_move_index = start_index;
			do_movement_accept();
			delay(4); /* wait a bit */
			if (tmp_range_attack != 0) {
				return range_attacking();
			}

			/* get the next square to move to */
			/* note that do_movement_accept() sets "move_table_created" to 0,  */
			/* so after a move, we need to remake the table. */
			if (LUT_index > 2) {
				LUT_index -= 2;
			} else {
				/* no good square to go to.. */
				movement_b();
				if (tmp_range_attack != 0) {
					/* again, could have used break instead of goto */
					/* but goto makes the intention clearer */
					return range_attacking();
				}
				break;
			}
		}
	}
}

static int priority_compar(const void *a, const void *b)
{
	uint8_t ap = ((const uint8_t*)a)[0];
	uint8_t bp = ((const uint8_t*)b)[0];
	if (ap < bp)
		return 1;
	if (ap > bp)
		return -1;
	return 0;
}

/* based on code at c64c */
void order_table(int count, uint8_t * table)
{
	qsort(table, count, 2, priority_compar);
}

/* based on code at c955  */
/* returns squares in range... */
/*int16_t create_range_table(uint8_t target, uint8_t range)  */
int create_range_table(int target, int range)
{

	/* fill the priority table with all squares in range  */
	/* distance is from target index */
	int i;
	int range_count = 1;
	int pi = 0;
	reset_priority_table();
	for (i = 0; i < 0x9f; i++) {
		uint16_t tmprange;
		get_distance(i, start_index, &tmprange);
		if (range >= tmprange) {
			/* square is in range */
			get_distance(i, target, &tmprange);
			prio_table[pi] = tmprange;
			pi++;
			prio_table[pi] = i;
			pi++;
			range_count++;
		}
	}
	order_table(range_count, prio_table);
	return range_count;
}

/* the AI spell casting is roughly based on code starting at 96f3 */
void do_ai_spell(void)
{
	/*
	   set spell 0 priorty = 0x0c + GetRand(10) (I imagine this is in case Disbelieve was top priority?)
	   order the spell list by priority
	   best spell = 0
	   while (best spell < spell list length) {
	   select the best spell
	   select the best square for this spell
	   if no square is good
	   best spell ++
	   else
	   break
	   }

	   if a spell has been chosen...
	   cast spell at chosen square
	   remove casted spell from list

	   move on to next player...
	   */
	if (IS_WIZARD_DEAD(players[g_chaos_state.current_player].modifier_flag)) {
		return;
	}
	temp_cast_amount = 0;
	/* reset meditation */
	players[g_chaos_state.current_player].last_spell = 0;
	players[g_chaos_state.current_player].spells[0] = 0x0c + GetRand(10);
	order_table(players[g_chaos_state.current_player].spell_count + 1,
			players[g_chaos_state.current_player].spells);
	uint8_t best_spell = 0;
	while (best_spell < players[g_chaos_state.current_player].spell_count) {
		current_spell = players[g_chaos_state.current_player].spells[best_spell * 2 + 1];

		/* "cast" the spell... each casting routine has the CPU AI code for that spell built into it. */
		/* if no good square was found, then go to the next spell */
		target_square_found = 0;
		if (current_spell >= SPELL_DISBELIEVE && current_spell <= SPELL_TURMOIL) {
			players[g_chaos_state.current_player].last_spell = current_spell;
			target_square_found = 1;
			set_current_spell_chance();
			CHAOS_SPELLS[current_spell].pFunc();
		}
		if (target_square_found) {
			/* spell was cast succesfully */
			if (current_spell >= SPELL_KING_COBRA) {
				/* spell used (if not disblv) */
				players[g_chaos_state.current_player].spells[best_spell * 2 + 1] = 0;
			}

			/*      break; */
			return;
		} else {
			best_spell++;
		}
	}
}


/* c9dc */
static void get_furthest_inrange(void)
{
	/* get the furthest square away still in range... */
	do {
		target_index = prio_table[LUT_index];
		/*delay(5); whhaaaa?? */

		/* check xpos < 10  - code at c63d */
		uint8_t x, y;
		get_yx(target_index, &y, &x);
		if (x < 0x10) {
			/* in range */
			if (!los_blocked(target_index)) {
				if (arena[0][target_index] == 0) {
					/* nothing here */
					target_square_found = 1;
				} else if (is_dead(target_index)) {
					/* is the thing here dead? */
					target_square_found = 1;
				}
			}
		}
		if (LUT_index - 2 < 0)
			LUT_index = 0;
		else
			LUT_index -= 2;
	} while (LUT_index > 0 && (target_square_found == 0));
}


/* the computer tries to cast a creature.. */
/* code based on 9984 */
void ai_cast_creature(void)
{
	/* get the square closest to the best target... */
	uint16_t strongest_index;
	strongest_index = get_strongest_wizard(0xFF);
	start_index = wizard_index;
	target_square_found = 0;
	uint8_t range = 3;
	if (current_spell >= SPELL_GOOEY_BLOB)
		range = 13;
	int16_t in_range = (int16_t) create_range_table(strongest_index, range);
	LUT_index = in_range * 2 + 1;
	get_furthest_inrange();
	if (target_square_found) {
		temp_illusion_flag = 0;
		if ((current_spell < SPELL_GOOEY_BLOB)) {
			/* randomly cast illusions, but more chance of trying if the spell is tricky to cast */
			if ((current_spell_chance < 4 && GetRand(10) > 5) || (GetRand(10) > 7))
				temp_illusion_flag = 1;
		}
		set_spell_success();
		
#ifdef _HEADLESS
		output_cast_creature(g_chaos_state.world_chaos, g_chaos_state.round_count, g_chaos_state.current_player, wizard_index, target_index, current_spell, temp_success_flag, temp_illusion_flag);
#endif

		print_name_spell();
		delay(20);
		spell_animation();
		do_creature_spell_if_ok(target_index&0xff);
		draw_all_creatures();
		delay(10);
	}			/* else no square found! */
	temp_cast_amount = 0;
}


void do_ai_movement(void)
{
	/*
	   if wiz has creatures to do his bidding...
		   get current position, set up relative
		   movement advantages for squares we can go to

	   if we are in a wood or castle, stay there
	   else ...
		   select the wizard
		   do the movement

	   end if

	   for all squares on the board...
		   if we have a creature here
		   then try selecting it
			   if we selected it successfully (it hadn't moved already)
			   then do the movement for this creature
	   end for

	   so if the wizard is creatureless, he himself acts like a normal creature
	   */

	/* the computer movement stuff... */
	if (players[g_chaos_state.current_player].timid == 0) {

		/*  ca92... */
		last_move_index = -1;
		setup_wizard_move();
		if (arena[0][start_index] == SPELL_MAGIC_WOOD
				|| arena[0][start_index] == SPELL_MAGIC_CASTLE
				|| arena[0][start_index] == SPELL_DARK_CITADEL) {

			/* set has moved... */
			has_wizard_moved = 1;
			arena[3][start_index] |= 0x80;
		} else if (HAS_MOVED(arena[3][start_index])) {
			/* e.g. meditate cast */
			has_wizard_moved = 1;
		} else {

			/* move wizard... */
			/* select the wizard... */
			has_wizard_moved = 0;

			/* and we have the move table in memory.. */
			move_table_created = 1;
			/* select the wizard... */
			do_movement_accept();
			delay(10);

			/* now try moving it to the target square... */
			do_this_movement();
			delay(5);
		}
	}

	/* here? move creatures */
	/* we have already dealt with the wizard... */
	has_wizard_moved = 1;
	int i;
	for (i = 0; i < 0x9f; i++) {
		target_index = i;
		start_index = i;
		last_move_index = -1;
		/* select the creature... */
		do_movement_accept();
		if (selected_creature != 0) {
			delay(10);
			do_this_movement();
		}
	}
	delay(5);
}


/* based on code around c9dc */
int get_best_index(void)
{
	/* get the best index of the prio table.. will be the one  */
	/* get the furthest square away still in range... */
	uint8_t y, x;
	do {
		if (LUT_index - 1 <= 0)
			return 0x4b;
		if (prio_table[LUT_index] == 0xFF) {
			LUT_index--;
			LUT_index--;
		} else {
			target_index = prio_table[LUT_index];

			/* check xpos < 10 */
			get_yx(target_index, &y, &x);
			if (x < 0x10) {
				/* in range */
				LUT_index--;
				LUT_index--;
				return 0x53;
			} else {
				LUT_index--;
				LUT_index--;
			}
		}
	} while (LUT_index > 0);
	return 0x4b;
}

/* called at the start of lightning casting amongst other stuff... */
/* creates the priority table based on all enemy creatures and wizards */
/* wizards are rated higher than creatures and table is ordered and  */
/* the first value in the table is pointed to by value in cd86 */
void create_all_enemies_table(void)
{
	/* based on cc56 */
	int i;
	priority_offset = 0x3c;

	/* create the priority table based on "strongest wizard" */
	create_table_wizards();
	priority_offset = 0x20;
	LUT_index = 14;
	uint8_t table_size = 9;
	uint8_t tmp;

	/* now create the rest of the table, for the enemy creatures */
	for (i = 0; i < 0x9e; i++) {
		/* call c67a */
		tmp = contains_enemy(i);
		if (tmp != 0) {
			table_size++;
			prio_table[LUT_index++] = tmp;
			prio_table[LUT_index++] = i;
		}
	}
	order_table(table_size, prio_table);
	LUT_index = 1;
	priority_offset = 0;
}

static int should_cast_sleep_blind(int creature)
{
	if (creature == SPELL_SHADOW_WOOD || creature == SPELL_MAGIC_WOOD)
		/* avoid trees for sleep/blind */
		return 0;
	if (current_spell == SPELL_BLIND && creature == SPELL_GOOEY_BLOB)
		/* Avoid blinding a Blob.
		 * "My blob's got no eyes. Or nose."
		 * "How does it smell?"
		 * "Terrible!" */
		return 0;
	if (is_asleep(target_index) || is_blind(target_index))
		/* already cast on this creature. */
		return 0;
	int include_blob = (current_spell == SPELL_MAGIC_SLEEP);
	if (creature >= WIZARD_INDEX
			&& creature_count(creature - WIZARD_INDEX, include_blob) == 0)
		/* do not cast on creature-less wizard (no effect) */
		return 0;
	if (creature >= SPELL_HORSE
			&& creature <= SPELL_MANTICORE
			&& arena[4][target_index] != 0)
		/* mounted creature - will have no effect */
		return 0;
	return 1;
}

/* used for lightning and magic bolt */
/* code from 9d8a */
void ai_cast_magic_missile(void)
{
	create_all_enemies_table();
	target_index = prio_table[LUT_index];
	int is_sleep = current_spell == SPELL_MAGIC_SLEEP || current_spell == SPELL_BLIND;
	while (target_index != 0xFF) {
		int creature;
		target_index = prio_table[LUT_index];
		/* sanity check */
		if (target_index >= ARENA_SIZE)
			break;
		creature = arena[0][target_index];
		/* a bit hacky - pretend the creature is a WALL to skip the check. */
		if (is_sleep && !should_cast_sleep_blind(creature))
			creature = SPELL_WALL;
		if (creature == SPELL_SHADOW_WOOD
			|| creature >= WIZARD_INDEX
			|| creature < SPELL_MAGIC_FIRE) {

			/* see if in range... */
			if (is_spell_in_range(wizard_index, target_index,
					 CHAOS_SPELLS[current_spell].castRange)) {
				if (!los_blocked(target_index)) {
					/* spell is good! */
					target_square_found = 1;
					temp_cast_amount = 0;
					
#ifdef _HEADLESS
					output_cast_disbelieve(g_chaos_state.world_chaos, g_chaos_state.round_count, g_chaos_state.current_player, wizard_index, target_index, current_spell, 1);
#endif

					/* cas tthe actual spell... */
					print_name_spell();
					delay(20);
					do_magic_missile();
					return;
				}
			}
		}

		/* got to here? then the spell is not valid... look at the next index */
		LUT_index += 2;
	}
	target_square_found = 0;
	temp_cast_amount = 0;
}


/* code based on 853b */
void ai_cast_subversion(void)
{

	/* ths usual, store start pos, create table of all enemies, retrieve start pos and order table */
	uint8_t current_index = start_index;
	create_table_enemies();
	start_index = current_index;
	order_table(target_count, prio_table);

	/* get the best target square */
	LUT_index = 1;
	uint8_t creature;
	uint16_t tmp_dist;
	uint8_t tmp_cast_range =
		CHAOS_SPELLS[current_spell].castRange;
	temp_cast_amount = 1;
	while (prio_table[LUT_index] != 0xFF) {
		creature = arena[0][prio_table[LUT_index]];
		if (arena[4][prio_table[LUT_index]] == 0
				&& creature < SPELL_GOOEY_BLOB && creature != 0) {

			/* a valid creature for subversion... */
			/* compare the start and end locations to see if in spell range */
			get_distance(start_index,
					prio_table[LUT_index], &tmp_dist);
			if (tmp_cast_range >= tmp_dist
					&& !los_blocked(prio_table[LUT_index])
					&& (arena[3][prio_table[LUT_index]] & 0x20)
					== 0) {

				/* in rangem, have los and is not an illusion (because some one has cast disbeilve on it) */
				target_index = prio_table[LUT_index];
				target_square_found = 1;
				return;
			}
		}
		LUT_index += 2;
	}
	if (prio_table[LUT_index] == 0xFF) {
		/* no decent square */
		target_square_found = 0;
		temp_cast_amount = 0;
		return;
	}
	target_square_found = 0;
	temp_cast_amount = 0;
}

static int get_a_wall(void)
{
	while (1) {
		target_square_found = 0;
		get_furthest_inrange();
		/* if a suitable square was found, target_sq_found will be 1 */
		if (target_square_found == 0) {
			/* we have run out of chackable squares! */
			return 0;
		}
		uint8_t y, x;
		/* check that this is a valid square for walls... */
		get_yx(target_index, &y, &x);
		if (x >= 0x10) {
			continue;
		}
		if (arena[0][target_index] != 0) {
			continue;
		}
		if (is_wall_adjacent(target_index)) {
			continue;
		}
		/* found */
		return 1;
	}
	return 0;
}

static void one_wall_cast(void)
{
	int tmp_square_found;
	tmp_square_found = 0;
	/* the global target_square_found is used as a temporary flag here */
	/* tmp_square_found indicates if we find any castable-to squares for this spell at all */
	while (temp_cast_amount) {
		if (!get_a_wall()) {
			break;
		}
		/* we have found a suitable square... */
#ifdef _HEADLESS
		output_cast_creature(g_chaos_state.world_chaos, g_chaos_state.round_count, g_chaos_state.current_player, wizard_index, target_index, current_spell, temp_success_flag, 0);
#endif
		do_wall_cast();
		if (tmp_square_found == 0) {
			print_success_status();
			if (temp_success_flag == 0) {
				delay(30);
			}
		}
		tmp_square_found = 1;
	}
}

/* works a treat this .... */
void ai_cast_wall(void)
{

	/* 9b85 */
	/* CALL c7bc */
	temp_cast_amount = 4;
	uint8_t current_index = start_index;
	create_table_enemies();
	start_index = current_index;
	order_table(target_count, prio_table);
	LUT_index = 1;
	while (prio_table[LUT_index] != 0xFF) {
		if (arena[0][prio_table[LUT_index]] >= SPELL_SPECTRE)
			break;
		if (arena[0][prio_table[LUT_index]] < SPELL_PEGASUS)
			break;
		LUT_index += 2;
	}
	if (prio_table[LUT_index] == 0xFF) {
		/* no decent square */
		target_square_found = 0;
		temp_cast_amount = 0;
		return;
	}

	/* got to here? OK, a "good" square was found for wall. */
	/* calculate spell success and update world chaos ... */
	set_spell_success();

#ifdef _HEADLESS
	if (temp_success_flag == 0) {
		output_cast_fail(g_chaos_state.world_chaos, g_chaos_state.round_count, g_chaos_state.current_player, wizard_index, current_spell);
	}
#endif
	print_name_spell();
	delay(20);
	int16_t in_range = create_range_table(prio_table[LUT_index], 0x9);
	clear_message();
	LUT_index = 2 * in_range + 1;
	target_square_found = 0;

	one_wall_cast();
	/* see if a wall is worth casting, don't do it yet.
	 * If it is, mark target_square_found */
	int tmp = LUT_index;
	int foundone = get_a_wall();
	LUT_index = tmp;
	target_square_found = foundone;
}


/* from 9ef9 */
void ai_cast_justice(void)
{

	/* print name and spell */
	target_square_found = 1;
	print_name_spell();
	delay(20);
	set_spell_success();
	if (!temp_success_flag) {
		temp_cast_amount = 0;
		print_success_status();

#ifdef _HEADLESS
		output_cast_fail(g_chaos_state.world_chaos, g_chaos_state.round_count, g_chaos_state.current_player, wizard_index, current_spell);
#endif
		return;
	}

#ifdef _HEADLESS
	output_cast_success(g_chaos_state.world_chaos, g_chaos_state.round_count, g_chaos_state.current_player, wizard_index, current_spell);
#endif

	uint8_t tmp_start = wizard_index;
	uint8_t creature;
	while (temp_cast_amount != 0) {
		start_index = tmp_start;
		target_index = start_index;
		create_all_enemies_table();
		LUT_index = 1;

		/* get the best index for each cast... */
		while (prio_table[LUT_index] != 0xFF) {

			/* check the creature at this index... */
			creature = arena[0][prio_table[LUT_index]];
			if (creature >= WIZARD_INDEX
					|| (creature < SPELL_GOOEY_BLOB
						&& creature != 0)) {
				/* wizard or proper creature */
				target_index = prio_table[LUT_index];

				LUT_index++;
				LUT_index++;
				break;
			}
			LUT_index++;
			LUT_index++;
		}
		if (prio_table[LUT_index] == 0xFF) {
			return;
		}
		delay(20);
		do_justice_cast();
		temp_cast_amount--;
	}
}


/* taken from 8639 */
void ai_cast_raisedead(void)
{
	int i;
	uint8_t id;
	target_square_found = 0;

	/* make the byte pair table... */
	reset_priority_table();
	target_count = 0;
	LUT_index = 0;
	for (i = 0; i < 0x9f; i++) {
		id = arena[0][i];
		if (is_dead(i)) {

			/* is dead, store the priority for this creature */
			prio_table[LUT_index++] = attack_pref[id];
			prio_table[LUT_index++] = i;
			target_count++;
		}
	}
	order_table(target_count, prio_table);

	/* get the best value and try and cast it... */
	/* get the best index for each cast... */
	LUT_index = 1;
	uint8_t creature;
	while (temp_cast_amount != 0) {
		target_index = 0xFF;
		while (prio_table[LUT_index] != 0xFF) {

			/* check the creature at this index... */
			creature = arena[0][prio_table[LUT_index]];
			if (creature >= WIZARD_INDEX
					|| (creature < SPELL_GOOEY_BLOB
						&& creature != 0)) {

				/* wizard or proper creature */
				target_index = prio_table[LUT_index];
				LUT_index++;
				LUT_index++;
				break;
			}
			LUT_index++;
			LUT_index++;
		}
		if (target_index == 0xFF) {
			return;
		}

		/* ok, got a square to cast to... is it in range, los, etc? */
		if (arena[0][target_index] == 0 || !is_dead(target_index))
			continue;

		if (!is_spell_in_range(wizard_index, target_index,
				 CHAOS_SPELLS[current_spell].castRange))
			continue;

		if (los_blocked(target_index))
			continue;

		/* got to here? good, print name and spell and do cast */
		print_name_spell();
		delay(20);
		do_raisedead_cast();
		print_success_status();
		temp_cast_amount = 0;
		
#ifdef _HEADLESS
		output_cast_disbelieve(g_chaos_state.world_chaos, g_chaos_state.round_count, g_chaos_state.current_player, wizard_index, target_index, current_spell, temp_success_flag);
#endif
	}
	target_square_found = 1;
}

/* based on code at 9a95 */
/* should set target square flag and the target index */
/* when returns, the disbelieve spell is cast based on these */
void ai_cast_disbelieve(void)
{
	uint8_t current_index = start_index;
	create_all_enemies_table();
	start_index = current_index;
	while (prio_table[LUT_index] != 0xFF) {
		target_index = prio_table[LUT_index];
		if (arena[0][target_index] < SPELL_GOOEY_BLOB
				&& !(arena[3][target_index] & 0x20)) {

			/* is a creature and has not had disbeleive cast on it yet... */
			target_square_found = 1;
			print_name_spell();
			delay(20);
			return;
		}
		LUT_index++;
		LUT_index++;
	}

	/* got to here? must have run out of decent targets */
	target_index = current_index;
	target_square_found = 0;
}

/* Get the amount of "good" spells the player has left. Doesn't count permanent
 * spells or law/chaos ones. */
static int get_spells(void)
{
	int i;
	player_data *p = &players[g_chaos_state.current_player];
	int end = p->spell_count;
	int count = 0;
	for (i = 0; i < end; i++) {
		int idx = i * 2 + 1;
		int spell = p->spells[idx];
		if (spell >= SPELL_MEDITATE
			&& (spell <= SPELL_LAW_1
			|| spell >= SPELL_CHAOS_2))
			count++;
	}
	return count;
}

/* Check if the CPU should cast meditate or not. Sets target_square_found to 1
 * if meditate is a good choice. */
void ai_cast_meditate(void)
{
	target_square_found = 0;

	/* only cast when low on spells and not near anything */
	int spells = get_spells();
	if (spells > 2)
		return;

	int c = arena[0][wizard_index];

	/* inside a castle or wood is safe */
	target_square_found = (c == SPELL_MAGIC_CASTLE
			|| c == SPELL_DARK_CITADEL
			|| c == SPELL_MAGIC_WOOD);

	if (target_square_found)
		return;

	/* do not cast when on a mount */
	if (c < WIZARD_INDEX)
		return;

	create_table_enemies();
	int enemy_creature = prio_table[0];

	/* if an enemy is found, don't risk it */
	target_square_found = enemy_creature == 0;
}

/* code at 86f9 */
void ai_cast_turmoil(void)
{

	/* CALL c7bc */
	uint8_t current_index = start_index;
	create_table_enemies();
	start_index = current_index;
	order_table(target_count, prio_table);

	/* this first part depends not on the index values, like most other spells. */
	/* it instead depends on the priority value... the thinking behind this is:  */
	/* if there aren't enough enemy creatures really close, it isn't worth casting turmoil */
	LUT_index = 0;
	uint8_t total_danger = 0;
	while (prio_table[LUT_index] != 0) {
		total_danger += prio_table[LUT_index];
		LUT_index += 2;
		if (total_danger >= 0x1E) {
			break;
		}
	}
	if (prio_table[LUT_index] == 0) {
		/* no decent square */
		target_square_found = 0;
		temp_cast_amount = 0;
		return;
	}

	/* the spell is worth casting */
	target_square_found = 1;
	temp_cast_amount = 1;
	do_turmoil_cast();
}

/* check if the cpu could dismount (traditionally, the cpu never dismounts) */
/* return true to dismount */
/* return false to stay mounted */
int cpu_dismount(void)
{

	/* type records type of thing that attracts us */
	int type = 0xff;
	int i;

	int creature = arena[0][target_index];
	/* which type of mount are we on? */
	if (IS_FLYING_MOUNT(creature)) {
		/* too complex to work out... */
		return 0;
	}
	for (i = 0; i < 8; i++) {
		uint8_t lookat_i = apply_position_modifier(target_index, i);
		if (lookat_i == 0) {
			/* out of bounds */
			continue;
		}
		lookat_i--;

		creature = arena[0][lookat_i];
		/* rules for leaving our mount are consistent with the */
		/* movement patterns when not mounted, i.e. prefer */
		/* to move to a castle or tree before getting on a horse. */
		if (creature >= SPELL_MAGIC_CASTLE && creature <= SPELL_DARK_CITADEL) {
			/*      && (players[g_chaos_state.current_player].timid == 0)  // is this right? */
			type = 0;
		} else if (creature == SPELL_MAGIC_WOOD) {
			if (type > 1) {
				type = 1;
			}
		} else if (IS_FLYING_MOUNT(creature)
				&& get_owner(lookat_i) == g_chaos_state.current_player
				&& arena[4][lookat_i] == 0) {

					/*
					 * next to us stands a winged mount, we are on a walking mount so swap!
					 * Really, we should change to any better mount but that is quite a
					 * complex operation - the cpu move table would need resetting and extra
					 * checks put in all over the place. Not really worth it.
					 */
					if (type > 2) {
						type = 2;
					}
				}
	}
	return type != 0xff;
}
