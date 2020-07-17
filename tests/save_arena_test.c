#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "chaos/arena.h"
#include "port/linux/utils.h"
#include "chaos/chaos.h"
#include "chaos/spellenums.h"
#include "chaos/creature.h"
#include "chaos/players.h"
#include "chaos/wizards.h"

#define ARENA_0_EXPECTED SRCDIR "/arena_test_expected_0.txt"
/* #define ARENA_1_EXPECTED SRCDIR "/arena_test_expected_1.txt" - skipped, since arena 1 is fluff */
#define ARENA_2_EXPECTED SRCDIR "/arena_test_expected_2.txt"
#define ARENA_3_EXPECTED SRCDIR "/arena_test_expected_3.txt"
#define ARENA_4_EXPECTED SRCDIR "/arena_test_expected_4.txt"
#define ARENA_5_EXPECTED SRCDIR "/arena_test_expected_5.txt"


extern unsigned char current_spell;
extern unsigned char temp_illusion_flag;

static void add_creature(int owner, int type, int illusion, int x, int y)
{
	int target = x + y * 16;
	int old_cs = current_spell;
	int old_cp = g_chaos_state.current_player;
	int old_if = temp_illusion_flag;

	current_spell = type;
	g_chaos_state.current_player = owner;
	temp_illusion_flag = illusion;

	/* hack - if casting blob, place the current creature under it */
	if (type == SPELL_GOOEY_BLOB) {
		arena[4][target] = arena[0][target];
	}
	creature_spell_succeeds(target);

	current_spell = old_cs;
	g_chaos_state.current_player = old_cp;
	temp_illusion_flag = old_if;
}

static void add_illusion(int owner, int type, int x, int y)
{
	add_creature(owner, type, 1, x, y);
}

static void add_real(int owner, int type, int x, int y)
{
	add_creature(owner, type, 0, x, y);
}

static void kill_creature(int x, int y)
{
	int ti = x + y * 16;
	arena[1][ti] = 1;
	arena[2][ti] = 4;
	/* clear sleep / blind */
	arena[3][ti] &= 7;
}

static void check_arena(int idx, const char *expected)
{
	char *result = malloc(ARENA_SIZE * 2);
	arena_to_str(idx, result);
	assert(result != 0);
	char *data = read_file_as_str(expected);
	assert(data != 0);
	int diff = strcmp(result, data);
	if (diff != 0) {
		printf("Failed check %d %s\n", idx, result);
	}
	assert(diff == 0);
	free(data);
	free(result);
}

static void check_str2arena(const char *filename, int layer, const int *expected)
{
	char *data = read_file_as_str(filename);
	assert(strncmp("ARENA:\n", data, 7) == 0);
	int res = str_to_arena(0, &data[7]);
	assert(res == 0);
	free(data);
	int idx = 0;
	int x, y;
	for (y = 0; y < 10; y++) {
		for (x = 0; x < 15; x++, idx++) {
			int t = x + y * 16;
			int a = arena[layer][t];
			if (a != expected[idx]) {
				printf("diff in layer %d @ %d: %d != %d\n", layer, idx, a, expected[idx]);
			}
			assert(a == expected[idx]);
		}
	}
}

static void check_str2arena_0(void)
{
	const int expected[] = {
		0x2B,0x00,0x00,0x00,0x00,0x00,0x00,0x2C,0x00,0x00,0x00,0x00,0x00,0x00,0x2D,
		0x13,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x2E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x2F,
		0x00,0x00,0x00,0x00,0x2A,0x00,0x00,0x09,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x24,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x30,0x00,0x00,0x00,0x00,0x00,0x00,0x31,0x00,0x00,0x00,0x00,0x00,0x28,0x32,
	};
	check_str2arena(ARENA_0_EXPECTED, 0, expected);
}

static void check_str2arena_2(void)
{
	const int expected[] = {
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	};
	check_str2arena(ARENA_2_EXPECTED, 2, expected);
}

static void check_str2arena_3(void)
{
	const int expected[] = {
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x02,
		0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x04,
		0x00,0x00,0x00,0x00,0x02,0x00,0x00,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x06,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x05,0x00,0x00,0x00,0x00,0x00,0x00,0x06,0x00,0x00,0x00,0x00,0x00,0x07,0x07,
	};
	check_str2arena(ARENA_3_EXPECTED, 3, expected);
}

static void check_str2arena_4(void)
{
	const int expected[] = {
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x18,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	};
	check_str2arena(ARENA_4_EXPECTED, 4, expected);
}

static void check_str2arena_5(void)
{
	const int expected[] = {
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x12,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	};
	check_str2arena(ARENA_5_EXPECTED, 5, expected);
}

void test_save_arena(void)
{
	/* set up players. */
	init_player_table();
	init_arena_table();
	setSeed(192);
	g_chaos_state.playercount = 8;
	create_default_wizs();
	init_players();

	/*
	 *  0123456789ABCDE
	 * 0W      W      W
	 * 1I
	 * 2
	 * 3  K
	 * 4W             W
	 * 5    w  L
	 * 6  +
	 * 7          *
	 * 8
	 * 9W      W     cW
	 *
	 * * = bat under a blob
	 * + = horse corpse under giant rat
	 */
	add_illusion(0, SPELL_UNICORN, 0, 1);
	add_real(0, SPELL_KING_COBRA, 2, 3);
	add_real(2, SPELL_WALL, 4, 5);
	add_real(7, SPELL_MAGIC_CASTLE, 0xd, 9);
	add_real(3, SPELL_LION, 7, 5);
	kill_creature(7, 5);
	add_real(5, SPELL_BAT, 0xa, 7);
	add_real(6, SPELL_GOOEY_BLOB, 0xa, 7);

	add_real(1, SPELL_HORSE, 2, 6);
	kill_creature(2, 6);
	add_real(4, SPELL_GIANT_RAT, 2, 6);

	check_arena(0, ARENA_0_EXPECTED);
	check_arena(2, ARENA_2_EXPECTED);
	check_arena(3, ARENA_3_EXPECTED);
	check_arena(4, ARENA_4_EXPECTED);
	check_arena(5, ARENA_5_EXPECTED);

	memset(arena, sizeof(arena), 0);

	check_str2arena_0();
	check_str2arena_2();
	check_str2arena_3();
	check_str2arena_4();
	check_str2arena_5();
}

int main(void)
{
	test_save_arena();
	return 0;
}