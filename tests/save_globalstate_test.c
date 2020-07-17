#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "chaos/chaos.h"
#include "chaos/creature.h"

void test_save_state(void)
{
	/* set up players. */
	g_chaos_state.dead_wizards = 1;
	g_chaos_state.playercount = 4;
	g_chaos_state.world_chaos = -2;
	g_chaos_state.round_count = 1;
	g_chaos_state.current_screen = SCR_CREATE_PLAYERS;
	g_chaos_state.current_player = 1;
	setSeed(0x8f98abc0);

	char *buffer = malloc(200);
	chaos_state_to_str(buffer);

	const char *expected = ("STATE:\n"
			"\tCHAOS -2\n"
			"\tROUNDS 1\n"
			"\tKILLED 1\n"
			"\tPLAYERS 4\n"
			"\tSCREEN 30\n"
			"\tCURRENT 1\n"
			"\tSEED 0x8F98ABC0\n"
			);
	int diff = strcmp(buffer, expected);
	if (diff != 0) {
		fprintf(stderr, "BUFFER=\n%s", buffer);
		fprintf(stderr, "EXPECTED=\n%s", expected);
	}
	assert(diff == 0);

	memset(&g_chaos_state, 0, sizeof g_chaos_state);

	char *statedata = strdup(expected);
	str_to_chaos_state(statedata);
	free(statedata);

	assert(g_chaos_state.dead_wizards == 1);
	assert(g_chaos_state.playercount == 4);
	assert(g_chaos_state.world_chaos == -2);
	assert(g_chaos_state.round_count == 1);
	assert(g_chaos_state.current_screen == SCR_CREATE_PLAYERS);
	assert(g_chaos_state.current_player == 1);
	assert(GetSeed() == 0x8f98abc0);
}

static void test_save_version(void)
{
	int v;
	v = get_save_version("JUNK");
	assert(v == 0);
	v = get_save_version("VERSION 2\n");
	assert(v == 2);

	v = get_save_version("VERSION 40");
	assert(v == 40);

	v = get_save_version("VERSION 40000000000");
	/* gets truncated, but should not crash */
	assert(v == 400000000);
}

int main(void)
{
	test_save_state();
	test_save_version();
	return 0;
}
