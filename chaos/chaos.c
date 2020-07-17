#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "chaos/platform.h"
#include "chaos/porting.h"
#include "chaos/chaos.h"
#include "chaos/options.h"
#include "chaos/text16.h"
#include "chaos/rand.h"

const unsigned short int chaos_cols[] = {
	RGB16(31, 31, 31),	/* white */
	RGB16(31, 31, 0),	/* yellow */
	RGB16(0, 31, 31),	/*lightblue */
	RGB16(0, 31, 0),	/*green */
	RGB16(31, 0, 31),	/*purple */
	RGB16(31, 0, 0),	/*red */
	RGB16(0, 0, 31),	/*blue */
	RGB16(20, 20, 20),	/*grey */
	RGB16(20, 20, 0),	/* mustard */
};

struct chaos_state g_chaos_state;

/* on the player select screen, the colours are:
   red purple green lightblue mustard yellow grey white
*/
const signed char surround_table[8][2] = {
	{-1, -1},
	{-1, 0}, {-1, 1}, {0, 1}, {1, 1}, {1, 0}, {1, -1}, {0, -1}
};

const unsigned char key_table[8] = {
	'Q', 'W', 'E', 'D', 'C', 'X', 'Z', 'A'
};
int hilite_item;
signed char anim_col;
signed char anim_col_grad;

void play_soundfx(int soundfx)
{
	if (Options[OPT_SOUND_ENABLED])
		platform_play_soundfx(soundfx);
}

void append_stat(char *buffer, const char *name, int value)
{
	char tmp[10];
	strcat(buffer, "\t");
	strcat(buffer, name);
	strcat(buffer, " ");
	int2a(value, tmp, 10);
	strcat(buffer, tmp);
	strcat(buffer, "\n");
}

static void append_seed(char *buffer)
{
	/* SEED 0xFFFFAAAA */
	static const char *hex = "0123456789ABCDEF";
	strcat(buffer, "\tSEED 0x");
	unsigned int seed = GetSeed();
	/* append as hex... */
	int i;
	size_t pos = strlen(buffer);
	for (i = 28 ; i >= 0; i -= 4) {
		buffer[pos++] = hex[(seed >> i) & 0xf];
	}
	buffer[pos++] = '\n';
	buffer[pos++] = 0;
}

void chaos_state_to_str(char *buffer)
{
	strcat(buffer, "STATE:\n");
	append_stat(buffer, "CHAOS", g_chaos_state.world_chaos);
	append_stat(buffer, "ROUNDS", g_chaos_state.round_count);
	append_stat(buffer, "KILLED", g_chaos_state.dead_wizards);
	append_stat(buffer, "PLAYERS", g_chaos_state.playercount);
	append_stat(buffer, "SCREEN", g_chaos_state.current_screen);
	append_stat(buffer, "CURRENT", g_chaos_state.current_player);
	append_seed(buffer);
}

static void parse_line(const char *line)
{
	const char *tmp = line;
	while (isspace((unsigned char)*tmp)) {
		tmp++;
	}
	const char *namestart = tmp;
	while (!isspace((unsigned char)*tmp))
		tmp++;
	char *nameend = (char *)tmp;
	while (isspace((unsigned char)*tmp)) {
		tmp++;
	}
	const char *valstart = tmp;
	while (*tmp != '\n')
		tmp++;
	char *valend = (char *)tmp;
	char tmpvalend = *valend;
	char tmpnameend = *nameend;
	*valend = 0;
	*nameend = 0;
	if (strcmp(namestart, "KILLED") == 0) {
		g_chaos_state.dead_wizards = strtol(valstart, 0, 0);
	} else if (strcmp(namestart, "CHAOS") == 0) {
		g_chaos_state.world_chaos = strtol(valstart, 0, 0);
	} else if (strcmp(namestart, "ROUNDS") == 0) {
		g_chaos_state.round_count = strtol(valstart, 0, 0);
	} else if (strcmp(namestart, "PLAYERS") == 0) {
		g_chaos_state.playercount = strtol(valstart, 0, 0);
	} else if (strcmp(namestart, "SCREEN") == 0) {
		g_chaos_state.current_screen = strtol(valstart, 0, 0);
	} else if (strcmp(namestart, "CURRENT") == 0) {
		g_chaos_state.current_player = strtol(valstart, 0, 0);
	} else if (strcmp(namestart, "SEED") == 0) {
		unsigned int seed = strtoul(valstart, 0, 0);
		setSeed(seed);
	}
	*valend = tmpvalend;
	*nameend = tmpnameend;
}

int get_save_version(const char *data)
{
	if (strncmp(data, "VERSION", 7) != 0) {
		return 0;
	}
	const char *tmp = &data[7];
	if (*tmp == 0) {
		return 0;
	}
	while (isspace((unsigned char)*tmp)) {
		tmp++;
	}
	char num[10];
	int i;
	for (i = 0; i < 9 && isdigit((unsigned char)*tmp); i++, tmp++) {
		num[i] = *tmp;
	}
	num[i] = 0;
	return strtol(num, 0, 0);
}

void set_save_version(char *buffer, int version)
{
	char tmp[10];
	strcat(buffer, "VERSION ");
	int2a(version, tmp, 10);
	strcat(buffer, tmp);
	strcat(buffer, "\n");
}

void str_to_chaos_state(char *data)
{
	if (data == NULL)
		return;
	const char *tmp = data;
	int in_state = 0;
	while (*tmp != 0) {
		const char *linebegin = tmp;
		while (*tmp != '\n' && *tmp != 0) {
			tmp++;
		}
		tmp++;
		if (linebegin[0] != '\t') {
			if (strncmp(linebegin, "STATE:\n", 7) == 0)
				in_state = 1;
			else if (in_state != 0)
				break;
		} else {
			parse_line(linebegin);
		}
	}
}
