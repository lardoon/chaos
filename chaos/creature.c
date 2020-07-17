/* creature.c */
#include <string.h>
#include "chaos/platform.h"
#include "chaos/porting.h"
#include "chaos/sound_data.h"
#include "chaos/spelldata.h"
#include "chaos/creature.h"
#include "chaos/arena.h"
#include "chaos/chaos.h"
#include "chaos/wizards.h"
#include "chaos/casting.h"
#include "chaos/gfx.h"
#include "chaos/options.h"
#include "chaos/rand.h"

static void uncover_square(unsigned char start, unsigned char target)
{
	unsigned char r = GetRand(10);
	if (r > 2) {
		return;
	}
	unsigned char target_creature = arena[0][target];

	if (target_creature != SPELL_MAGIC_FIRE) {
		r = GetRand(10);
		if (r > 3) {
			return;
		}
	}
	/* remove the blob and show the creature underneath... */
	if (arena[4][start] == 0) {
		arena[0][start] = 0;
		arena[5][start] = 0;
	} else {
		liberate_from_blob(start);
	}
	delay(24);
}

/* code from a09b */
static void do_spread(unsigned char start, unsigned char target)
{
	unsigned char start_creature = arena[0][start];

	arena[0][target] = start_creature;
	arena[2][target] = 0;	/* frame */
	arena[3][target] = arena[3][start];	/* set owner */

	arena[3][target] |= 0x80;	/* set spread flag */
	arena[3][start] |= 0x80;	/* set spread flag */

	move_screen_to(target);
	if (start_creature == SPELL_GOOEY_BLOB) {
		play_soundfx(SND_GOOEY);
	} else if (start_creature == SPELL_MAGIC_FIRE) {
		play_soundfx(SND_FIRE);
	}

	delay(24);

	/* at the end of everything, randomly uncover the "spawn" square */
	uncover_square(start, start);
}

void creature_spell_succeeds(unsigned char target)
{
	if (!(current_spell == SPELL_GOOEY_BLOB
	      || current_spell == SPELL_MAGIC_FIRE)) {
		arena[5][target] = arena[0][target];	/* whatever is in this square, place in arena 5... */
	}
	arena[0][target] = current_spell;
	arena[1][target] = 1;
	arena[2][target] = 0;
	arena[3][target] = g_chaos_state.current_player;
	if (temp_illusion_flag) {
		arena[3][target] |= 0x10;	/* set bit 4 */
	}
}

static struct arena_cache {
	uint8_t creature;
	uint8_t frame;
} s_cache[ARENA_SIZE];

void invalidate_cache(void)
{
	memset(s_cache, 0, sizeof(s_cache));
}

void clear_arena(void)
{
	int x, y;
	for (x = 0; x < 15; x++) {
		for (y = 0; y < 10; y++) {
			clear_square(x, y);
		}
	}
	invalidate_cache();
}

/* this should be called every now and again... */
int draw_all_creatures(void)
{
	int i;
	uint8_t x, y;
	for (i = 0; i < ARENA_SIZE; i++) {
		get_yx(i, &y, &x);
		if (x == 17)
			continue;
		if (arena[0][i] >= SPELL_KING_COBRA) {
			if (s_cache[i].creature == arena[0][i] &&
					s_cache[i].frame == arena[2][i])
				continue;
			if (arena[0][i] < WIZARD_INDEX) {
				draw_creature(x - 1, y - 1,
						arena[0][i], arena[2][i]);
			} else {
				int wizard = arena[0][i] - WIZARD_INDEX;
				if (wizard < 8) {
					draw_wizard(x - 1, y - 1,
						    players[wizard].image,
						    arena[2][i],
						    wizard);
				}
			}
			s_cache[i].creature = arena[0][i];
			s_cache[i].frame = arena[2][i];
		} else {
			if (s_cache[i].creature == 0xff)
				continue;
			if (x > 0 && x < 0x10 && y > 0 && y < 0xB)
				clear_square(x - 1, y - 1);
			s_cache[i].creature = 0xff;
		}

	}
	return 0;
}

/* called at the end of the casting round */
/* based on code at 9f50..  */
void spread_fire_blob(void)
{
	int i = 0, current_index = 0;
	unsigned char this_owner, target_owner, r, creature, target_creature;
	unsigned char attack, defence;
	current_index = 0;
	target_index = 0;
	unset_moved_flags();
	for (i = 0; i < 0x9f; i++) {
		target_index = current_index;

		if (HAS_MOVED(arena[3][current_index])) {
			current_index++;
			continue;
		}
		creature = arena[0][target_index];
		if (creature < SPELL_GOOEY_BLOB) {
			current_index++;
			continue;
		}
		if (creature >= SPELL_MAGIC_WOOD) {
			current_index++;
			continue;
		}
		/* sleeping blob does not spread */
		if (is_asleep(i)) {
			arena[3][i] |= 0x80;
			continue;
		}
		this_owner = get_owner(target_index);
		r = GetRand(10);

		if (r < 0x9) {
			if (creature != SPELL_MAGIC_FIRE) {
				r = GetRand(10);
				if (r > 0x8) {
					/* jump to a0c6... */
					/* do uncover creature routine */
					uncover_square(current_index,
							target_index);
					current_index++;
					continue;
				}
			}
			r = GetRand(10);
			while (r >= 0x8) {
				r = GetRand(10);
			}
			target_index =
				apply_position_modifier(target_index, r);
			if (target_index == 0) {
				current_index++;
				continue;
			} else {
				target_index--;
			}

			/* do in range check...  */
			uint16_t dist;
			get_distance(current_index, target_index, &dist);
			if (dist > 4) {
				current_index++;
				continue;
			}

			target_creature = arena[0][target_index];
			if (target_creature != 0) {
				if (target_creature < WIZARD_INDEX) {
					/* is NOT a wizard... */
					if (target_creature >= SPELL_MAGIC_CASTLE
						|| target_creature == SPELL_MAGIC_WOOD) {
						/* a magic wood or castle, etc */
						/* jump to a0c6... */
						/* do uncover creature routine */
						uncover_square(current_index, target_index);
						current_index++;
						continue;
					}

					if (!is_dead(target_index)) {
						/* if not dead */
						target_owner = get_owner(target_index);
						if (target_owner == this_owner) {
							current_index++;
							continue;
						}
						if (target_creature <= SPELL_MANTICORE
								&& target_creature >= SPELL_HORSE
								&& arena[4][target_index] >= WIZARD_INDEX) {
							/* if the creature is a mount and has a wizard on it */
							/* a011... */
							if ((arena[4][target_index] - WIZARD_INDEX) == this_owner) {
								/* do nothing - own blob can't kill */
								current_index++;
								continue;
							}
							/* clear arena 4 (wizard) and place wizard in arena 0... */
							arena[0][target_index] = arena[4][target_index];
							arena[4][target_index] = 0x00;

							/* a033... kill wizard */
							kill_wizard();
							remove_cursor();
							/* jump to a09b  */
							do_spread(current_index, target_index);
							current_index++;
							continue;

						}
						/* a03d */
						if (creature == SPELL_MAGIC_FIRE) {
							/* jump a075 - for fire */
							/* get defence of target creature */
							defence = CHAOS_SPELLS[target_creature].defence + GetRand(10);
							attack = GetRand(10) + 5;
							if (attack > defence) {
								/* jump a0c6 */
								uncover_square(current_index, target_index);
								current_index++;
								continue;
							}
							/* jump a09b... */
						} else {	/*if (creature == SPELL_GOOEY_BLOB)*/
							/* gooey blob */
							if (target_creature >= SPELL_MAGIC_FIRE) {
								/* jump a0c6 - uncover creature routine */
								uncover_square(current_index, target_index);
								current_index++;
								continue;
							}
							if (!is_dead(target_index)) {
								/* creature alive.. set arena 4 with the current creature and cover it */
								arena[4][target_index] = arena[0][target_index];
								if (Options[OPT_OLD_BUGS]) {
									/* old bugs on... */
									/* a066 */
									/* I think the plan here was to set the creature's owner, the first 3 bits */
									arena[5][target_index] = get_owner(target_index);	/* bug here */
								} else {
									/* old bug removed here... */
									/* what about undead and illusionary flags? */
									arena[5][target_index] = arena[3][target_index];
								}
								/* spread blindness and sleep */
								if (is_asleep(target_index)) {
									arena[5][target_index] |= 8;
									arena[5][target_index] |= 0x80;
								}
								if (is_blind(target_index))
									arena[5][target_index] |= 8;
							}	/* else jump to a09b - target dead */
						}

					}	/* else target is dead */
						/* jump to a09b */
				} else {
					/* "jump a011" - target creature is a wizard */
					if ((target_creature - WIZARD_INDEX) == this_owner) {
						/* do nothing - own blob can't kill */
						current_index++;
						continue;
					}
					/* a033 - kill wizard */
					kill_wizard();
					remove_cursor();
					/* jmp a09b */
				}
			}	/* else  target creature == 0 */
			/* jump a09b */
			do_spread(current_index, target_index);
		} else {
			/* jump to a0c6... */
			/* do uncover creature routine */
			uncover_square(current_index, target_index);
		}
		current_index++;
	}
}

void liberate_from_blob(int start)
{
	arena[0][start] = arena[4][start];
	arena[4][start] = 0;
	arena[3][start] = arena[5][start];
	arena[5][start] = 0;
	int bits = arena[3][start];
	/* blind bit set, moved set */
	if ((bits & 0x88) == 0x88) {
		/* take advantage of move flag to indicate sleep */
		arena[2][start] = 4;
	}
}

