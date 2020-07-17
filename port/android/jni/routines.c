#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "chaos/porting.h"
#include "chaos/platform.h"
#include "chaos/translate.h"
#include "chaos/chaos.h"
#include "port/android/jni/screen.h"
#include "chaos/wizards.h"
#ifdef HAS_STRESS_TEST
#include "chaos/splash.h"
#include "chaos/players.h"
#endif
#define KEY_COUNT       11

static uint16_t s_palette[256];
static int s_converted_palette[256];
static uint16_t s_tileData[SCREEN_WIDTH * SCREEN_HEIGHT * 8 * 8];
static uint16_t s_spritePalette[256];
static uint16_t s_mapData[SCREEN_WIDTH * SCREEN_HEIGHT];
static uint16_t s_textMapData[SCREEN_WIDTH * SCREEN_HEIGHT];
static uint16_t s_textTileData[SCREEN_WIDTH * SCREEN_HEIGHT * 8 * 8];
uint32_t s_repeated[KEY_COUNT];
int g_keys_down[KEY_COUNT];
int g_last_keys[KEY_COUNT];
int g_touch_x = -1;
int g_touch_y = -1;

static uint16_t s_spriteTileData[8 * 64];
static int s_cursor_on = 0;
static int s_cursor_x = 0;
static int s_cursor_y = 0;

/*#define ANDROID_RGB(r, g, b) ( ((r) << 16) | ((g) << 8) | (b))*/
#define ANDROID_RGB(r, g, b) ( ((r) << (5+6)) | ((g) << 6) | (b))

uint16_t *g_pixels = 0;
static int android_get_color(uint16_t rgb16)
{
	return ANDROID_RGB((rgb16&0x1f),
			((rgb16>>5)&0x1f),
			((rgb16>>10)&0x1f));
}

#undef TEXTURE_WIDTH
#undef TEXTURE_HEIGHT
#define TEXTURE_WIDTH (SCREEN_WIDTH * 8 * 2)
#define TEXTURE_HEIGHT (SCREEN_HEIGHT * 8)

static void draw_8x8_tile_to(int x, int y, const uint8_t *gfx, int palette)
{
	/* draws a 8*8 tile to the given x,y tile position */
	int pixelPair, i, j;
	int pix_y;
	int pix_x;
	const int x8 = x * 8;
	const int y8 = y * 8;
	int *pal = &s_converted_palette[palette * 16];
	pix_y = y8 * TEXTURE_WIDTH;
	for (j = 0; j < 8; j++) {
		pix_x = x8 + pix_y;
		for (i = 0; i < 4; i++) {
			pixelPair = *gfx++;
			int nibble = pixelPair & 0xf;
			if (nibble) {
				int val = pal[nibble];
				g_pixels[pix_x++] = val;
			} else {
				pix_x += 1;
			}
			nibble = pixelPair & 0xf0;
			if (nibble) {
				int val = pal[nibble >> 4];
				g_pixels[pix_x++] = val;
			} else {
				pix_x += 1;
			}
		}
		pix_y += TEXTURE_WIDTH;
	}
}

static void draw_8x8_tile_vert(int x, int y, const uint8_t *gfx, int palette)
{
	/* draws a 8*8 tile to the given x,y tile position */
	int pixelPair, i, j;
	int pix_y;
	int pix_x;
	int color = 0;
	const int x8 = x * 8;
	const int y8 = y * 8;
	int *pal = &s_converted_palette[palette * 16];
	for (j = 7; j >= 0; j--) {
		pix_y =  j + y8;
		pix_y *= TEXTURE_WIDTH;
		pix_x = x8 + pix_y;
		for (i = 0; i < 4; i++) {
			pixelPair = *gfx++;
			if ((pixelPair & 0xf)) {
				color = pal[(pixelPair & 0xf)];
				g_pixels[pix_x] = color;
			}
			pix_x++;
			if ((pixelPair & 0xf0)) {
				color = pal[((pixelPair >> 4) & 0xf)];
				g_pixels[pix_x] = color;
			}
			pix_x++;
		}
	}
}

static void draw_8x8_tile_horz(int x, int y, const uint8_t *gfx, int palette)
{
	/* draws a 8*8 tile to the given x,y tile position */
	int pixelPair, i, j;
	int pix_y;
	int pix_x;
	int color = 0;
	const int x8 = x * 8;
	const int y8 = y * 8;
	int *pal = &s_converted_palette[palette * 16];
	for (j = 0; j < 8; j++) {
		pix_y = j + y8;
		pix_y *= TEXTURE_WIDTH;
		for (i = 3; i >= 0; i--) {
			pixelPair = *gfx++;
			/* horizontal tiles start at the end and work back */
			pix_x = i * 2 + x8 + 1;
			pix_x += pix_y;
			if ((pixelPair & 0xf0)) {
				color = pal[((pixelPair >> 4) & 0xf)];
				g_pixels[pix_x] = color;
			}
			pix_x--;
			if ((pixelPair & 0xf)) {
				color = pal[(pixelPair & 0xf)];
				g_pixels[pix_x++] = color;
			}
		}
	}
}

static void draw_8x8_tile_superflip(int x, int y, const uint8_t *gfx, int palette)
{
	/* draws a 8*8 tile to the given x,y tile position */
	int pixelPair, i, j;
	int pix_y;
	int pix_x;
	int color = 0;
	const int x8 = x * 8;
	const int y8 = y * 8;
	int *pal = &s_converted_palette[palette * 16];
	for (j = 7; j >= 0; j--) {
		pix_y = j + y8;
		pix_y *= TEXTURE_WIDTH;
		for (i = 3; i >= 0; i--) {
			pixelPair = *gfx++;
			/* horizontal tiles start at the end and work back */
			pix_x = pix_y + (i * 2 + x8 + 1);
			if ((pixelPair & 0xf0)) {
				color = pal[((pixelPair >> 4) & 0xf)];
				g_pixels[pix_x] = color;
			}
			pix_x--;
			if ((pixelPair & 0xf)) {
				color = pal[(pixelPair & 0xf)];
				g_pixels[pix_x++] = color;
			}
		}
	}
}

#if 0
static void clear_rect(int x, int y)
{
	int i, j;
	/* clear a 8x8 tile at the x, y coord */
	for (i = y * 8; i < (y * 8 + 8); i++) {
		for (j = x * 8; j < (x * 8 + 8); j++) {
			g_pixels[j + (i * SCREEN_WIDTH)] = 0;
		}
	}
}
#endif

static void draw_8x8_tile(int x, int y, uint16_t *mapData, uint16_t *tileData)
{
	/* draws a 8*8 tile to the given x,y tile position */
	int idx = x + y * SCREEN_WIDTH;
	int map_val = mapData[idx];
	if (map_val == 0)
		return;
	int palette = (map_val >> 12) & 0xf;
	uint16_t tile = map_val & 0x3ff;
	const uint8_t *gfx = (const uint8_t*)&tileData[tile * 16];
	int flags = map_val & (TILE_FLIP_VERT|TILE_FLIP_HORZ);
	if (likely(flags == 0)) {
		draw_8x8_tile_to(x, y, gfx, palette);
	} else {
		if (unlikely(flags == (TILE_FLIP_VERT|TILE_FLIP_HORZ)))
			draw_8x8_tile_superflip(x, y, gfx, palette);
		else if (flags == TILE_FLIP_VERT)
			draw_8x8_tile_vert(x, y, gfx, palette);
		else
			draw_8x8_tile_horz(x, y, gfx, palette);
	}
}

static void draw_8x8_sprite_tile(int x, int y, const uint8_t * gfx)
{
	/* draws a 8*8 tile to the given x,y tile position */
	int pix_x, pix_y;
	int pixelPair, i, j;
	int y8 = y * 8;
	int x8 = x * 8;
	for (j = 0; j < 8; j++) {
		pix_y = (j + y8) * TEXTURE_WIDTH;
		pix_x = x8 + pix_y;
		for (i = 0; i < 8; i++) {
			pixelPair = *gfx++;
			if ((pixelPair & 0xf)) {
				int color = android_get_color(s_spritePalette[(pixelPair & 0xf)]);
				g_pixels[pix_x] = color;
			}
			pix_x++;
		}
	}
}

static void draw_sprite(void)
{
	int x, y;
	int idx = 0;
	if (!s_cursor_on)
		return;
	for (y = 0; y < 2; y++) {
		for (x = 0; x < 2; x++) {
			draw_8x8_sprite_tile(s_cursor_x + x, s_cursor_y + y,
					(const uint8_t*)&s_spriteTileData[idx * 32]);
			idx++;
		}
	}
}

void android_render_screen(uint16_t *pixels)
{
	int x, y;
	g_pixels = pixels;
	for (y = 0; y < SCREEN_HEIGHT; y++) {
		for (x = 0; x < SCREEN_WIDTH; x++) {
			draw_8x8_tile(x, y, s_mapData, s_tileData);
			draw_8x8_tile(x, y, s_textMapData, s_textTileData);
		}
	}
	draw_sprite();
}

void platform_set_palette_entry(int idx, uint16_t color)
{
	if (color != s_palette[idx]) {
		s_palette[idx] = color;
		s_converted_palette[idx] = android_get_color(color);
	}
}

uint16_t platform_get_palette_entry(int idx)
{
	return s_palette[idx];
}

void platform_set_map_entry(int x, int y, int value)
{
	int idx = x + y * SCREEN_WIDTH;
	if (s_mapData[idx] != value) {
		s_mapData[idx] = value;
	}
}

uint16_t platform_get_map_entry(int x, int y)
{
	return s_mapData[x + y * SCREEN_WIDTH];
}

void platform_set_text_map_entry(int x, int y, int palette)
{
	int idx = x + y * SCREEN_WIDTH;
	if (idx > (SCREEN_WIDTH * SCREEN_HEIGHT))
		return;
	if (s_textMapData[idx] != palette) {
		s_textMapData[idx] = palette;
	}
}

void platform_set_tile_entry(int offset, int value)
{
	if (s_tileData[offset] != value) {
		s_tileData[offset] = value;
	}
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

void platform_update_keys(void)
{
	int i;
	for (i = 0; i < KEY_COUNT; i++)
		g_last_keys[i] = g_keys_down[i];
}

int platform_key_pressed(int key)
{
	return g_keys_down[key] != 0;
}

int platform_key_pressed_repeat(unsigned int key)
{
	if (platform_key_pressed(key)) {
		/* if the recent poll has the key pressed */
		if (!g_last_keys[key] || (g_last_keys[key] && 5 < s_repeated[key])) {
			/* and it wasn't pressed last time.. */
			s_repeated[key] = 0;
			return 1;
		} else if (g_last_keys[key]) {
			s_repeated[key]++;
		}
	}
	return 0;
}

void platform_load_sprite_palette(int offset, const uint16_t *data, int size)
{
	int i;
	for (i = 0; i < size; i++) {
		s_spritePalette[offset + i] = data[i];
	}
}

void platform_load_sprite_data(int offset, const uint16_t *data)
{
	memcpy(&s_spriteTileData[offset], data, 64);
}

void platform_set_sprite_enabled(int enabled)
{
	s_cursor_on = enabled;
}

void platform_move_sprite(int spriteid, int x, int y)
{
	if (spriteid != 0)
		return;
	s_cursor_x = x / 8;
	s_cursor_y = y / 8;
}

#ifdef HAS_STRESS_TEST
int running_in_stress_mode(void)
{
	return 1;
}

static int in_stress_mode = 0;

void stress_mode(void)
{
	if (pending_events()) {
		g_keys_down[CHAOS_KEY_TOUCH] = 0;
		return;
	}
	in_stress_mode = 1;
	if (g_chaos_state.current_screen == SCR_SPLASH) {
		splash_start();
		splash_start();
	} else if (g_chaos_state.current_screen == SCR_CREATE_PLAYERS) {
		players[0].plyr_type = PLYR_CPU | (4 << 4);
		g_chaos_state.playercount = 8;
		create_players_start();
	} else {
		g_keys_down[CHAOS_KEY_TOUCH] = 0;
	}
	in_stress_mode = 0;
}

#endif

int platform_line_fudge_factor(void)
{
	return 40;
}

const char *platform_name(void)
{
	return "Android";
}

int platform_touch_x(void)
{
	return g_touch_x;
}

int platform_touch_y(void)
{
	return g_touch_y;
}

void platform_screen_size(int *width, int *height)
{
	*width = SCREEN_WIDTH * 8;
	*height = SCREEN_HEIGHT * 8;
}

static const char *s_buttons[] = {
	T("OK"),
	T("BACK"),
	T("S"),
	T("MENU"),
	T("RIGHT"),
	T("LEFT"),
	T("UP"),
	T("DOWN"),
	T("SEARCH"),
	T("L"),
	T("TOUCH"),
};

const char **platform_get_button_names(void)
{
	return s_buttons;
}

int platform_swap_screen(void)
{
	return 1;
}

void android_wait_vsync(void);
void platform_wait(void)
{
	android_wait_vsync();
}
