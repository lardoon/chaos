#include <gba.h>
#include <string.h>
#include <stdlib.h>
#include "chaos/porting.h"
#define MAX_OPT_SAVE_SIZE 120
#define MAX_GAME_SAVE_SIZE 5000
#define SAVE_OPTS_OFFSET 0
#define SAVE_GAME_OFFSET 200
/*
 * Splits SRAM like this:
 *
 * 0000: 4 byte int with size of options
 * 0004: [ options ]
 * ...
 * 00c8: 4 byte int with size of game-save
 * 00cc: [ game save ]
 *
 */

static void write_size(char *sram, int size)
{
	*sram++ = size       & 0xff;
	*sram++ = (size>>8)  & 0xff;
	*sram++ = (size>>16) & 0xff;
	*sram++ = (size>>24) & 0xff;
}

static int read_size(char *sram)
{
	int b[4];
	b[0] = (*sram++) & 0xff;
	b[1] = (*sram++) & 0xff;
	b[2] = (*sram++) & 0xff;
	b[3] = (*sram++) & 0xff;

	return b[0] | (b[1] << 8) |
		(b[2] << 16) |
		(b[3] << 24);
}

static void write_data(int offset, const char *saves, unsigned int size)
{
	unsigned int i;
	char *sram = (char*)(SRAM + offset);
	write_size(sram, size);
	sram += 4;
	for (i = 0; i < size; i++) {
		sram[i] = saves[i];
	}
}

void platform_save_options(const char *saves, unsigned int size)
{
	write_data(SAVE_OPTS_OFFSET, saves, size);
}

static char *read_from_sram(int offset, int maxsize)
{
	int i;
	char *sram = (char*)(SRAM + offset);
	char *tmp;
	int size = read_size(sram);
	if (!size)
		return NULL;
	sram += 4;
	if (size > maxsize)
		size = maxsize;
	tmp = malloc(size + 1);
	for (i = 0; i < size; i++) {
		tmp[i] = sram[i];
		if (tmp[i] == 0)
			break;
	}
	tmp[i] = 0;
	return tmp;
}

char *platform_load_options(void)
{
	return read_from_sram(SAVE_OPTS_OFFSET, MAX_OPT_SAVE_SIZE);
}

char *platform_load_game(void)
{
	return read_from_sram(SAVE_GAME_OFFSET, MAX_GAME_SAVE_SIZE);
}

int platform_has_saved_game(void)
{
	char *sram = (char*)(SRAM + SAVE_GAME_OFFSET);
	int size = read_size(sram);
	return size != 0;
}

void platform_save_game(const char *game, unsigned int size)
{
	write_data(SAVE_GAME_OFFSET, game, size);
}
