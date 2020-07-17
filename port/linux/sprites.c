#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "chaos/porting.h"
#include "chaos/options.h"

uint16_t g_spritePaletteMem[256];
/* Not much sprite data needed... */
uint16_t g_spriteTileData[8 * 64];
int g_cursor_on = 0;
int g_cursor_x = 0;
int g_cursor_y = 0;
extern int g_debug_mode;

void platform_load_sprite_palette(int offset, const uint16_t *data, int size)
{
	int i;
	for (i = 0; i < size; i++) {
		g_spritePaletteMem[offset + i] = data[i];
	}
	if (g_debug_mode) {
		Options[OPT_CHEAT] = g_debug_mode;
	}
	if (Options[OPT_CHEAT])
		printf("Cheat mode active. Spells always %s\n",
		       Options[OPT_CHEAT] == 1 ? "FAIL":"WORK");
}

void platform_load_sprite_data(int offset, const uint16_t *data)
{
	if (offset > (int)(sizeof(g_spriteTileData) / sizeof(g_spriteTileData[0]))) {
		printf("DANGER!! Sprite data overflow: %d\n", offset);
		return;
	}
	memcpy(&g_spriteTileData[offset], data, 64);
}

void platform_move_sprite(int spriteid, int x, int y)
{
	if (spriteid != 0)
		return;
	g_cursor_x = x / 8;
	g_cursor_y = y / 8;
}

void platform_set_sprite_enabled(int enabled)
{
	g_cursor_on = enabled;
}
