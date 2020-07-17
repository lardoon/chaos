#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "chaos/porting.h"

static uint16_t s_palette[256];
static uint16_t s_tileData[32 * 32 * 8 * 8];
static uint16_t s_spritePalette[256];
static uint16_t s_mapData[32 * 32];
static uint16_t s_textMapData[32 * 32];
static uint16_t s_textTileData[32 * 32 * 8 * 8];

const char *s_buttons[] = {
	"A",
	"B",
	"SELECT",
	"START",
	"RIGHT",
	"LEFT",
	"UP",
	"DOWN",
	"R",
	"L",
	"TOUCH",
};

int platform_init(void)
{
	return 0;
}

int platform_has_scroll(void)
{
	return 0;
}

int platform_get_x_scroll(void)
{
	return 0;
}

int platform_get_y_scroll(void)
{
	return 0;
}

void platform_set_x_scroll(int scroll UNUSED) { }

void platform_set_y_scroll(int scroll UNUSED) { }

void platform_update_scroll(void) { }

void platform_set_palette_entry(int idx, uint16_t color)
{
	s_palette[idx] = color;
}

uint16_t platform_get_palette_entry(int idx)
{
	return s_palette[idx];
}

void platform_set_map_entry(int x, int y, int palette)
{
	s_mapData[x + y * 32] = palette;
}

uint16_t platform_get_map_entry(int x, int y)
{
	return s_mapData[x + y * 32];
}

void platform_set_text_map_entry(int x, int y, int palette)
{
	s_textMapData[x + y * 32] = palette;
}

void platform_set_tile_entry(int offset, int value)
{
	s_tileData[offset] = value;
}

void platform_load_tile_data(int offset, const void *data, int size)
{
	memcpy(&s_tileData[offset], data, size);
}

void platform_load_text_tile_data(int offset, const void *data, int size)
{
	memcpy(&s_textTileData[offset], data, size);
}

int platform_get_tile_entry(int offset)
{
	return s_tileData[offset];
}

void platform_set_fade_level(int fade UNUSED) {}
void platform_update_keys(void) { }

int platform_key_pressed(int key UNUSED)
{
	return 0;
}

int platform_key_pressed_repeat(unsigned int key UNUSED)
{
	return 0;
}

void platform_load_sprite_palette(int offset, const uint16_t *data, int size)
{
	memcpy(&s_spritePalette[offset], data, size);
}

void platform_load_sprite_data(int offset, const uint16_t *data)
{
	memcpy(&s_spritePalette[offset], data, 32);
}

void platform_set_sprite_enabled(int enabled UNUSED)
{
	/* do nothing */
}

void platform_play_soundfx(int soundid UNUSED)
{
	/* meep */
}

void platform_move_sprite(int spriteid UNUSED, int x UNUSED, int y UNUSED)
{
}

void platform_dprint(const char *s)
{
	printf("%s", s);
}

int platform_line_fudge_factor(void)
{
	return 40;
}

const char *platform_name(void)
{
	return "Dummy";
}

int platform_touch_x(void)
{
	return -1;
}

int platform_touch_y(void)
{
	return -1;
}

void platform_screen_size(int *width, int *height)
{
	*width = 256;
	*height = 192;
}

const char **platform_get_button_names(void)
{
	return s_buttons;
}

int platform_swap_screen(void)
{
	return 1;
}

void platform_exit(void)
{
	exit(0);
}

int running_in_stress_mode(void)
{
	return 0;
}

char *platform_load_options(void) { return 0; }
void platform_save_options(const char *saves UNUSED, unsigned int size UNUSED) {}

int platform_has_saved_game(void)
{
	return 0;
}

void platform_save_game(const char *game UNUSED, unsigned int size UNUSED)
{
}

char *platform_load_game(void)
{
	return 0;
}

const char *platform_get_lang(void)
{
	return 0;
}

void platform_more_options(void) { }

void platform_wait(void) { }
