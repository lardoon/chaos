#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "chaos/wizards.h"
#include "chaos/arena.h"
#include "chaos/chaos.h"
#include "chaos/computer.h"
#include "chaos/spellenums.h"

int get_selected_spell_index(int row, int col);

static void test_select_spell_touch(void)
{
	int result;
	result = get_selected_spell_index(0, 0);
	assert(result == 0);
	result = get_selected_spell_index(0, 1);
	assert(result == 1);
	result = get_selected_spell_index(1, 0);
	assert(result == 2);
	result = get_selected_spell_index(1, 1);
	assert(result == 3);
}

static void set_spells(int *spells, int count)
{
	int i;
	player_data *p = &players[g_chaos_state.current_player];
	memset(p->spells, 0, sizeof(p->spells));
	for (i = 0; i < count ; i++)
		p->spells[i * 2 + 1] = spells[i];
	p->spell_count = count;
}

struct arena_map {
	char key;
	int value;
};

static void set_arena(const char *map, struct arena_map *key)
{
	int i;
	for (i = 0; i < 160; i++) {
		char c = map[i];
		if (key != NULL) {
			struct arena_map *tmp = key;
			while (tmp->key && tmp->key != c) tmp++;
			arena[0][i] = tmp->value;
		} else {
			arena[3][i] = c - '0';
		}
	}
}

static void test_ai_meditate(void)
{
	g_chaos_state.current_player = 1;
	int spells[4] = {
		SPELL_DISBELIEVE,
		SPELL_MEDITATE,
		SPELL_LAW_1,
		SPELL_LAW_2,
	};

	set_spells(spells, 4);
	struct arena_map key[] = {
		{'.', 0},
		{'0', WIZARD_INDEX + 0},
		{'1', WIZARD_INDEX + 1},
		{'2', WIZARD_INDEX + 2},
		{'K', SPELL_KING_COBRA},
		{'C', SPELL_MAGIC_CASTLE},
		{'H', SPELL_HORSE},
		{0, 0},
	};
	/* coast clear, ok to cast */
	set_arena(
		"................"
		".1............0."
		".............K.."
		"................"
		"................"
		"................"
		"................"
		"................"
		"............2..."
		"................",
		key);
	/* set owner map */
	set_arena(
		"0000000000000000"
		"0100000000000000"
		"0020000000000200"
		"0000000000000000"
		"0000000000000000"
		"0000000000000000"
		"0000000000000000"
		"0000000000000000"
		"0000000000002000"
		"0000000000000000",
		NULL);

	g_chaos_state.playercount = 3;
	wizard_index = 17;
	ai_cast_meditate();
	assert(target_square_found == 1);

	/* enemy nearby, do not cast */
	set_arena(
		"................"
		".1............0."
		"..K..........K.."
		"................"
		"................"
		"................"
		"................"
		"................"
		"............2..."
		"................",
		key);
	ai_cast_meditate();
	assert(target_square_found == 0);

	/* in a magic castle, doesn't matter */
	set_arena(
		"................"
		".C............0."
		"..K..........K.."
		"................"
		"................"
		"................"
		"................"
		"................"
		"............2..."
		"................",
		key);
	ai_cast_meditate();
	assert(target_square_found == 1);

	/* same as before, but more spells: should not cast although still
	 * inside the castle */
	int many_spells[] = {
		SPELL_DISBELIEVE,
		SPELL_MEDITATE,
		SPELL_LAW_1,
		SPELL_LAW_2,
		SPELL_MAGIC_CASTLE,
		SPELL_GREEN_DRAGON,
	};

	set_spells(many_spells, 6);
	ai_cast_meditate();
	assert(target_square_found == 0);

	/* in the open, spells left, do not cast */
	set_arena(
		"................"
		".1............0."
		".............K.."
		"................"
		"................"
		"................"
		"................"
		"................"
		"............2..."
		"................",
		key);
	ai_cast_meditate();
	assert(target_square_found == 0);

	/* on a mount, do not cast */
	arena[0][wizard_index] = SPELL_HORSE;
	set_spells(spells, 4);
	ai_cast_meditate();
	assert(target_square_found == 0);
}

int main(void)
{
	init_player_table();
	init_arena_table();
	test_select_spell_touch();
	test_ai_meditate();
	return 0;
}
