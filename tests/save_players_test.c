#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "chaos/wizards.h"
#include "chaos/chaos.h"
#include "port/linux/utils.h"
#include "chaos/creature.h"
#include "chaos/players.h"

#define EXPECTED_TXT SRCDIR "/save_players_test_expected.txt"
void parse_players(char *plystr);
void create_default_wizs(void);

static void check_equals(player_data *expected, int id)
{
	player_data *p = &players[id];
	int i;
	int namediff = strcmp(p->name, expected->name);
	if (namediff)
		printf("%s != %s\n", p->name, expected->name);
	assert(namediff == 0);
	assert(expected->combat == p->combat);
	assert(expected->ranged_combat == p->ranged_combat);
	assert(expected->range == p->range);
	assert(expected->defence == p->defence);
	assert(expected->movement_allowance == p->movement_allowance);
	assert(expected->manoeuvre_rating == p->manoeuvre_rating);
	assert(expected->magic_resistance == p->magic_resistance);
	assert(expected->spell_count == p->spell_count);
	for (i = 0; i < p->spell_count; i++) {
		int idx = i * 2;
		assert(expected->spells[idx] == p->spells[idx]);
		assert(expected->spells[idx + 1] == p->spells[idx + 1]);
	}
	assert(expected->ability == p->ability);
	assert(expected->image == p->image);
	assert(expected->colour == p->colour);
	assert(expected->plyr_type == p->plyr_type);
	assert(expected->modifier_flag == p->modifier_flag);
	assert(expected->illusion_cast == p->illusion_cast);
	assert(expected->selected_spell == p->selected_spell);
	assert(expected->last_spell == p->last_spell);
	assert(expected->timid == p->timid);
}

void test_save_players(void)
{
	init_player_table();
	init_arena_table();
	setSeed(192);
	g_chaos_state.playercount = 8;
	create_default_wizs();
	init_players();

	char *result = malloc(500 * 8);
	players_to_str(result);
	char *data = read_file_as_str(EXPECTED_TXT);
	assert(data != 0);
	int diff = strcmp(data, result);
	if (diff) {
		fprintf(stderr, "%s\n", result);
	}
	assert(diff == 0);

	/* scramble players */
	setSeed(111);
	create_default_wizs();
	init_players();
	/* make sure load works */
	parse_players(result);

	/* make sure parse_players didn't screw up the data */
	diff = strcmp(data, result);
	assert(diff == 0);
	{
		player_data tmp = {
			"Rich Q",
			1, 0, 0, 1, 1, 4, 10, 13,
			{0,1,23,41,12,42,18,26,18,30,18,3,11,
				50,18,23,18,45,18,17,18,23,18,37,18,13},
			0, 5, 31775, 0, 0, 0, 0, 0, 0,
		};
		check_equals(&tmp, 0);
	}
	{
		player_data tmp = {
			"Ben",
			7, 0, 0, 4, 1, 7, 6, 15,
			{21,1,18,20,18,5,18,12,18,4,18,9,18,23,18,43,18,
				29,18,17,18,26,14,56,6,58,6,60,5,59,},
			0, 4, 32736, 65, 0, 0, 0, 0, 0,
		};
		check_equals(&tmp, 1);
	}
	{
		player_data tmp = {
			"Saruman",
			6, 0, 0, 6, 1, 7, 9, 15,
			{18,10,18,35,18,64,18,17,18,26,18,20,18,11,18,
				6,18,9,18,18,18,61,13,1,11,49,6,58,5,59},
			0, 3, 21140, 65, 0, 0, 0, 0, 0,
		};
		check_equals(&tmp, 2);
	}
	{
		player_data tmp = {
			"Kiff",
			7, 0, 0, 7, 1, 5, 10, 15,
			{18,17,18,29,18,6,18,45,18,4,18,22,18,6,18,16,18,21,18,33,18,26,18,36,17,55,14,1,6,58,},
			1, 6, 660, 65, 0, 0, 0, 0, 0,
		};
		check_equals(&tmp, 3);
	}
	{
		player_data tmp = {
			"Grotbags",
			5, 0, 0, 4, 1, 6, 9, 15,
			{23,41,18,39,18,14,18,35,18,17,18,33,18,15,18,33,18,6,18,9,18,37,18,3,18,5,18,11,15,1,},
			0, 2, 992, 65, 0, 0, 0, 0, 0,
		};
		check_equals(&tmp, 4);
	}
	{
		player_data tmp = {
			"Tim",
			7, 0, 0, 4, 1, 6, 6, 18,
			{ 18,1,18,28,18,15,18,31,18,3,18,35,18,36,18,6,18,30,18,33,18,19,18,33,18,36,18,23,12,48,12,42,12,42,11,50,},
			0, 0, 31744, 65, 0, 0, 0, 0, 0,
		};
		check_equals(&tmp, 5);
	}
	{
		player_data tmp = {
			"Sooty",
			4, 0, 0, 5, 1, 5, 6, 16,
			{18,22,18,20,18,14,18,17,18,35,18,5,18,32,18,39,18,32,18,31,18,61,18,15,18,32,18,10,18,45,14,1,},
			0, 1, 31, 65, 0, 0, 0, 0, 0,
		};
		check_equals(&tmp, 6);
	}
	{
		player_data tmp = {
			"H.Potter",
			6,
			0,
			0,
			3,
			1,
			6,
			6,
			17,
			{23,40,23,52,21,1,18,62,18,17,18,29,18,34,18,5,18,32,18,11,18,25,18,29,18,37,18,25,18,11,18,24,12,47,},
			2,
			7,
			32767,
			65,
			0,
			0,
			0,
			0,
			0,
		};
		check_equals(&tmp, 7);
	}
}

int main(void)
{
	test_save_players();
	return 0;
}
