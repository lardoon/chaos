#include <stdlib.h>
#include "chaos/platform.h"
#include "chaos/porting.h"

#include "chaos/sound_data.h"
#include "chaos/wizards.h"
#include "chaos/wizard_data.h"
#include "chaos/arena.h"
#include "chaos/gfx.h"
#include "chaos/chaos.h"
#include "chaos/spelldata.h"
#include "chaos/creature.h"

#ifdef _HEADLESS
#include "chaos/output.h"
#endif

/* player data */
player_data *players = 0;

/*unsigned char g_chaos_state.current_player; */
/* SPELL_MAGIC_SHIELD */
/* SPELL_MAGIC_ARMOUR */
/* SPELL_MAGIC_SWORD  */
/* SPELL_MAGIC_KNIFE  */
/* SPELL_MAGIC_BOW */
/* SPELL_MAGIC_WINGS  */
const struct WIZARD_DATA WizardGFX[] = {
	{ wiz1Tiles, wiz1Map, },
	{ wiz2Tiles, wiz2Map, },
	{ wiz3Tiles, wiz3Map, },
	{ wiz4Tiles, wiz4Map, },
	{ wiz5Tiles, wiz5Map, },
	{ wiz6Tiles, wiz6Map, },
	{ wiz7Tiles, wiz7Map, },
	{ wiz8Tiles, wiz8Map, },
	{ shieldTiles, shieldMap,},
	{ armourTiles, armourMap,},
	{ swordTiles,  swordMap, },
	{ knifeTiles,  knifeMap, },
	{ bowTiles,    bowMap,   },
	{ wingsTiles,  wingsMap, },
};

void init_player_table(void)
{
	players = calloc(8, sizeof(player_data));
}

int creature_count(int player_id, int include_blob)
{
	int creatures = 0;
	int i;
	int limit = SPELL_GOOEY_BLOB;
	if (include_blob)
		limit = SPELL_MAGIC_FIRE;
	for (i = 0; i < ARENA_SIZE; i++) {
		int creature = arena[0][i];
		/* gah, need to include gooey blob for sleep... */

		if (creature >= SPELL_KING_COBRA && creature < limit) {
			/* is a creature */
			int underneath = arena[4][i];
			if (underneath != WIZARD_INDEX + player_id) {
				/* is not a ridden creature */
				int owner = get_owner(i);
				/* check it is ours and is not blind/asleep */
				if (owner == player_id && !is_asleep(i) && !is_blind(i))
					creatures++;
			}
		}
	}
	return creatures;
}

/* update the player's creature count table - 7d04 */
int update_creaturecount(void)
{
	if (g_chaos_state.round_count < 6) {
		players[g_chaos_state.current_player].timid = 0;
		return 0;
	}
	int creatures = creature_count(g_chaos_state.current_player, 0);
	if (creatures > 0)
		players[g_chaos_state.current_player].timid = 0;
	else
		/* if we are past round 6 and have no creatures */
		/* then "be brave"... */
		players[g_chaos_state.current_player].timid = 0x14;
	return 0;
}

/* kill the wizard at the target_index */
void kill_wizard(void)
{
	static int i, j;
	static uint8_t pal;
	static int8_t y2_1, y2_2, x2_1, x2_2;
	static const unsigned short *pGFX = 0, *pMap = 0;
	/* convert the x, y target location to y, x tile locations */
	static uint8_t x, y;
	pal = 9;
	/* code from b3c9 */
	remove_cursor();
	delay(4);
	g_chaos_state.dead_wizards++;

	/* do a sound fx.. */
	uint8_t deadid = arena[0][target_index] - WIZARD_INDEX;
	if (deadid > 7) {
		arena[0][target_index] = 0;
		return;
	}
#ifdef _HEADLESS
	output_wizard_killed(deadid, target_index);
#endif

	pGFX = WizardGFX[players[deadid].image].pGFX;
	pMap = WizardGFX[players[deadid].image].pMap;
	move_screen_to(target_index);

	disable_interrupts();
	/* load the wizard pal into 10 too... */
	load_bg_palette(10, 9);
	get_yx(target_index, &y, &x);
	clear_square(x - 1, y - 1);

	for (j = 0; j < 0x8; j++) {
		/* FIXME: Looped sounds are not too good... */
		play_soundfx(SND_SCREAM); /* loop 8 */
		get_yx2(target_index, &y, &x);
		y2_1 = y;
		y2_2 = y;
		x2_1 = x;
		x2_2 = x;

		/* loop over 29 frames and draw the wizard "breaking" */

		/* set the player colour... */
		if (j == 7) {
			/* clear palette 10 to make the end of run gfx effect */
			/* palette 10 is the final one */
			clear_palette(10);
		} else {
			platform_set_palette_entry(16 * pal + WIZARD_COLOUR, chaos_cols[j]);
		}
		for (i = 0; i < 0x1D; i++) {
			if (y2_1 - 1 != 0) {
				/* draw the wiz line upwards... */
				y2_1--;
				set_palette8(x - 1, y2_1 - 1, pal);
				draw_gfx8(pGFX, pMap,
					x - 1, y2_1 - 1, 0);
			}
			if (y2_2 + 1 != 0x14) {

				/* draw the wiz line downwards... */
				y2_2++;
				set_palette8(x - 1, y2_2 - 1, pal);
				draw_gfx8(pGFX, pMap,
					x - 1, y2_2 - 1, 0);
			}
			if (x2_1 - 1 != 0) {

				/* draw the wiz line left */
				x2_1--;
				set_palette8(x2_1 - 1, y - 1, pal);
				draw_gfx8(pGFX, pMap,
					x2_1 - 1, y - 1, 0);
			}
			if (x2_2 + 1 != 0x1E) {

				/* draw the wiz line right */
				x2_2++;
				set_palette8(x2_2 - 1, y - 1, pal);
				draw_gfx8(pGFX, pMap,
					x2_2 - 1, y - 1, 0);
			}
			if (x2_1 != 1 && y2_1 != 1) {
				set_palette8(x2_1 - 1, y2_1 - 1, pal);
				draw_gfx8(pGFX, pMap,
						x2_1 - 1, y2_1 - 1, 0);
			}
			if (x2_2 != 0x1D && y2_1 != 1) {
				set_palette8(x2_2 - 1, y2_1 - 1, pal);
				draw_gfx8(pGFX, pMap,
						x2_2 - 1, y2_1 - 1, 0);
			}
			if (x2_1 != 1 && y2_2 != 0x13) {
				set_palette8(x2_1 - 1, y2_2 - 1, pal);
				draw_gfx8(pGFX, pMap,
						x2_1 - 1, y2_2 - 1, 0);
			}
			if (x2_2 != 0x1D && y2_2 != 0x13) {
				set_palette8(x2_2 - 1, y2_2 - 1, pal);
				draw_gfx8(pGFX, pMap,
						x2_2 - 1, y2_2 - 1, 0);
			}
			if (i & 1)
				platform_wait();
		}
		if (pal == 9)
			pal = 10;
		else
			pal = 9;
	}
	play_soundfx(SND_URGH);
	clear_palettes();
	load_all_palettes();
	set_border_col(g_chaos_state.current_player);
	players[arena[0][target_index] - WIZARD_INDEX].modifier_flag |= 0x10;
	arena[0][target_index] = arena[5][target_index];
	arena[5][target_index] = 0;
	delay(10);
	destroy_all_creatures(deadid);
#ifdef _HEADLESS
	output_wizard_all_creatures_destroyed(deadid, target_index);
#endif
	invalidate_cache();
	if (!IS_CPU(g_chaos_state.current_player))
		redraw_cursor();
}

void reset_players(void)
{
	uint8_t i, s;
	for (i = 0; i < 8; i++) {
		/*    players[i].name[0] = 0; */
		players[i].combat = 0;
		players[i].ranged_combat = 0;
		players[i].range = 0;
		players[i].defence = 0;
		players[i].movement_allowance = 0;
		players[i].manoeuvre_rating = 0;
		players[i].magic_resistance = 0;
		players[i].spell_count = 0;
		for (s = 0; s < 40; s++) {
			players[i].spells[s] = 0;
		}
		players[i].ability = 0;
		players[i].image = 0;
		players[i].colour = 0;
		players[i].plyr_type = PLYR_HUMAN;
		players[i].modifier_flag = 0;
		players[i].illusion_cast = 0;
		players[i].selected_spell = 0;
		players[i].timid = 0;
	}
}
