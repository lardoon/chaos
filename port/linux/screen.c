#include <SDL.h>
#include <SDL/SDL_mutex.h>
#include <stdio.h>
#include <time.h>
#include <string.h>              /* for memcmp, memcpy, memset */
#include "chaos/porting.h"
#include "chaos/gfx.h"
#include "chaos/chaos.h"
#include "port/linux/sprites.h"
#include "port/linux/wmhandler.h"
#include "port/linux/winsound.h"
#include "port/linux/winkeys.h"
#include "port/linux/screen.h"
#include "chaos/spelldata.h"
#include "chaos/platform.h"

void emscripten_get_window_size(int *width, int *height, int *fullscreen);
void js_scale_height(void);
extern char *g_load_file;
int gfx_scale = DEFAULT_SCALE;
int g_icon = SPELL_GOOEY_BLOB;
enum ScreenType isFullScreen = NORMAL;
int s_fade_level = 0;
static int s_last_fade_level = 0;

SDL_Surface *screen = 0;
static SDL_Surface *screen_copy = 0;
static SDL_Surface *s_black = 0;

/* cache of palette changes */
static uint8_t s_paletteChanged[16];
static uint8_t s_tileDataChanged[0x400];
static uint8_t s_mapChanged[WIN_SCRN_X * WIN_SCRN_Y];


static uint16_t s_palette[256];
static int s_convertedPalette[256];
static uint16_t s_mapData[WIN_SCRN_X * WIN_SCRN_Y];
static uint16_t s_tileData[WIN_SCRN_X * WIN_SCRN_Y * 8 * 8];

uint16_t s_textMapData[WIN_SCRN_X * WIN_SCRN_Y];
uint16_t s_textTileData[WIN_SCRN_X * WIN_SCRN_Y * 64];

static int should_redraw(int idx, int tile, int palette)
{
	return s_mapChanged[idx] || s_tileDataChanged[tile] || s_paletteChanged[palette];
}

static Uint32 sdl_get_color(uint16_t rgb16)
{
	int factor = 8;
	return SDL_MapRGB(screen->format,
			(rgb16&0x1f)*factor,
			((rgb16>>5)&0x1f)*factor,
			((rgb16>>10)&0x1f)*factor);
}

static void draw_8x8_tile(int x, int y, const uint8_t *gfx, int palette, SDL_Surface * sfc)
{
	/* no flip */
	SDL_Rect rect;
	rect.w = 1 * gfx_scale;
	rect.h = 1 * gfx_scale;
	int pixelPair, i, j, nibble;
	Uint32 color = 0;
	int y8 = y * gfx_scale * 8;
	int x8 = x * gfx_scale * 8;
	int *p16 = &s_convertedPalette[palette * 16];
	rect.y = y8;
	for (j = 0; j < 8; j++) {
		rect.x = x8;
		for (i = 0; i < 4; i++) {
			pixelPair = *gfx++;
			nibble = pixelPair & 0xf;
			if (nibble) {
				color = p16[nibble];
				SDL_FillRect(sfc, &rect, color);
			}
			rect.x += gfx_scale;
			nibble = pixelPair & 0xf0;
			if (nibble) {
				color = p16[nibble >> 4];
				SDL_FillRect(sfc, &rect, color);
			}
			rect.x += gfx_scale;
		}
		rect.y += gfx_scale;
	}

}

static void draw_8x8_tile_vert(int x, int y, const uint8_t *gfx, int palette, SDL_Surface * sfc)
{
	/* vertical flip */
	SDL_Rect rect;
	rect.w = 1 * gfx_scale;
	rect.h = 1 * gfx_scale;
	int pixelPair, i, j;
	Uint32 color = 0;
	int y8 = y * gfx_scale * 8;
	int x8 = x * gfx_scale * 8;
	int *p16 = &s_convertedPalette[palette * 16];
	for (j = 0; j < 8; j++) {
		rect.y = (7 - j) * gfx_scale + y8;
		for (i = 0; i < 4; i++) {
			pixelPair = *gfx++;
			rect.x = i * gfx_scale * 2 + x8;
			if ((pixelPair & 0xf)) {
				color = p16[(pixelPair & 0xf)];
				SDL_FillRect(sfc, &rect, color);
			}
			rect.x += gfx_scale;
			if ((pixelPair & 0xf0)) {
				color = p16[((pixelPair >> 4) & 0xf)];
				SDL_FillRect(sfc, &rect, color);
			}
		}
	}

}

static void draw_8x8_tile_horz(int x, int y, const uint8_t *gfx, int palette, SDL_Surface * sfc)
{
	/* horizontal flip */
	SDL_Rect rect;
	rect.w = 1 * gfx_scale;
	rect.h = 1 * gfx_scale;
	int pixelPair, i, j;
	Uint32 color = 0;
	int y8 = y * gfx_scale * 8;
	int x8 = x * gfx_scale * 8;
	int *p16 = &s_convertedPalette[palette * 16];
	for (j = 0; j < 8; j++) {
		rect.y = j * gfx_scale + y8;
		for (i = 0; i < 4; i++) {
			pixelPair = *gfx++;
				/* horizontal tiles start at the end and work back */
				rect.x = (3 - i) * gfx_scale * 2 + x8 + gfx_scale;
				if ((pixelPair & 0xf0)) {
					color = p16[((pixelPair >> 4) & 0xf)];
					SDL_FillRect(sfc, &rect, color);
				}
				rect.x -= gfx_scale;
				if ((pixelPair & 0xf)) {
					color = p16[(pixelPair & 0xf)];
					SDL_FillRect(sfc, &rect, color);
				}
		}
	}
}

static void draw_8x8_tile_superflip(int x, int y, const uint8_t *gfx, int palette, SDL_Surface * sfc)
{
	/* draws a 8*8 tile to the given x,y tile position */
	SDL_Rect rect;
	rect.w = 1 * gfx_scale;
	rect.h = 1 * gfx_scale;
	int pixelPair, i, j;
	Uint32 color = 0;
	int y8 = y * gfx_scale * 8;
	int x8 = x * gfx_scale * 8;
	int *p16 = &s_convertedPalette[palette * 16];
	for (j = 0; j < 8; j++) {
		rect.y = (7 - j) * gfx_scale + y8;
		for (i = 0; i < 4; i++) {
			pixelPair = *gfx++;
			/* horizontal tiles start at the end and work back */
			rect.x = (3 - i) * gfx_scale * 2 + x8 + gfx_scale;
			if ((pixelPair & 0xf0)) {
				color = p16[((pixelPair >> 4) & 0xf)];
				SDL_FillRect(sfc, &rect, color);
			}
			rect.x -= gfx_scale;
			if ((pixelPair & 0xf)) {
				color = p16[(pixelPair & 0xf)];
				SDL_FillRect(sfc, &rect, color);
			}
		}
	}
}

static int gfx_scale_8 = 0;
static void draw8x8Tile(int x, int y, uint16_t *mapData, uint16_t *tileData)
{
	/* draws a 8*8 tile to the given x,y tile position */
	SDL_Surface *draw_here = (s_fade_level != 0) ? screen_copy : screen;
	int idx = x + y * WIN_SCRN_X;
	uint16_t map_val = mapData[idx];
	if (mapData == s_textMapData && map_val == 0)
		return;
	int palette = (map_val >> 12) & 0xf;
	int tile = map_val & 0x3ff;
	if (!should_redraw(idx, tile, palette))
		return;

	/* update here for the next layer */
	s_mapChanged[idx] = 1;
	SDL_Rect clear = {
		x * gfx_scale_8,
		y * gfx_scale_8,
		gfx_scale_8,
		gfx_scale_8
	};
	SDL_FillRect(draw_here, &clear, sdl_get_color(0x0));
	const uint8_t *gfx = (const uint8_t*)&tileData[tile * 16];
	int flags = map_val & (TILE_FLIP_VERT|TILE_FLIP_HORZ);
	if (likely(flags == 0)) {
		draw_8x8_tile(x, y, gfx, palette, draw_here);
	} else {
		if (unlikely(flags == (TILE_FLIP_VERT|TILE_FLIP_HORZ)))
			draw_8x8_tile_superflip(x, y, gfx, palette, draw_here);
		else if (flags == TILE_FLIP_VERT)
			draw_8x8_tile_vert(x, y, gfx, palette, draw_here);
		else
			draw_8x8_tile_horz(x, y, gfx, palette, draw_here);
	}
}

void draw8x8SpriteTile(int x, int y, const uint8_t * gfx)
{
	/* draws a 8*8 tile to the given x,y tile position */
	SDL_Rect rect;
	rect.w = 1 * gfx_scale;
	rect.h = 1 * gfx_scale;
	int pixelPair, i, j;
	for (j = 0; j < 8; j++) {
		for (i = 0; i < 8; i++) {
			rect.x = i * gfx_scale + x * gfx_scale * 8;
			rect.y = j * gfx_scale + y * gfx_scale * 8;
			pixelPair = *gfx++;
			if ((pixelPair & 0xf))
				SDL_FillRect(screen, &rect, sdl_get_color(g_spritePaletteMem[(pixelPair & 0xf)]));
		}
	}
}

SDL_Surface *get_icon(int gfxid)
{
	SDL_Surface *icon_sfc = SDL_CreateRGBSurface(SDL_SWSURFACE, 32, 32,
			screen->format->BitsPerPixel,
			screen->format->Rmask,
			screen->format->Gmask,
			screen->format->Bmask,
			screen->format->Amask);
	if (!icon_sfc) {
		printf("Error creating icon\n");
		return NULL;
	}
	int tmp_scale = gfx_scale;
	gfx_scale = 2;
	/* fill with gfx data for creature gfxid */
	int palette = CHAOS_SPELLS[gfxid].palette;
	load_bg_palette(0, palette);

	platform_set_map_entry(0, 0, 0);
	platform_set_map_entry(1, 0, 0);
	platform_set_map_entry(0, 1, 0);
	platform_set_map_entry(1, 1, 0);

	platform_set_palette_entry(0, 0);

	int frame = 0;
	const uint16_t *gfx = CHAOS_SPELLS[gfxid].pGFX;
	const uint16_t *map = CHAOS_SPELLS[gfxid].pGFXMap;
	uint16_t nTileCount = 4;	/* 4 tiles for each 16*16 gfx  */
	uint16_t index = frame * nTileCount;
	uint16_t id;
	uint8_t loop = 0;
	for (loop = 0; loop < 2; loop++) {
		id = map[index++];
		draw_8x8_tile(loop, 0, (const uint8_t*)&gfx[id * 16], 0, icon_sfc);
	}

	for (loop = 0; loop < 2; loop++) {
		id = map[index++];
		draw_8x8_tile(loop, 1, (const uint8_t*)&gfx[id * 16], 0, icon_sfc);
	}
	gfx_scale = tmp_scale;
	return icon_sfc;
}

uint16_t platform_get_palette_entry(int idx)
{
	return s_palette[idx];
}

void platform_set_palette_entry(int idx, uint16_t color)
{
	if (color != s_palette[idx]) {
		s_paletteChanged[idx / 16] = 1;
		s_palette[idx] = color;
		s_convertedPalette[idx] = sdl_get_color(color);
	}
}

void platform_set_map_entry(int x, int y, int value)
{
	int idx = x + y * WIN_SCRN_X;
	if (s_mapData[idx] != value) {
		s_mapChanged[idx] = 1;
		s_mapData[idx] = value;
	}
}

uint16_t platform_get_map_entry(int x, int y)
{
	return s_mapData[x + y * WIN_SCRN_X];
}

void platform_set_tile_entry(int offset, int value)
{
	if (s_tileData[offset] != value) {
		s_tileDataChanged[offset / 16] = 1;
		s_tileData[offset] = value;
	}
}

void platform_load_tile_data(int offset, const void *data, int size)
{
	int i = memcmp(&s_tileData[offset], data, size);
	if (i != 0) {
		int start = offset / 16;
		int end = (offset + (size / 2)) / 16;
		for (i = start; i <= end; i++)
			s_tileDataChanged[i] = 1;
		memcpy(&s_tileData[offset], data, size);
	}
}

int platform_get_tile_entry(int offset)
{
	return s_tileData[offset];
}

void platform_set_fade_level(int fadelevel)
{
	if (fadelevel == 0)
		s_fade_level = fadelevel;
	else
		s_fade_level = (16 * fadelevel) - 1;
}

int init_screen(int scale)
{
	if (screen) {
		SDL_FreeSurface(screen);
		SDL_FreeSurface(s_black);
		SDL_FreeSurface(screen_copy);
	}
	int flags = SDL_HWSURFACE;
	if (isFullScreen == FULL_SCREEN) {
		flags |= SDL_FULLSCREEN;
	} else {
		flags |= SDL_RESIZABLE;
	}

	if (isFullScreen == HEADLESS)
		screen = SDL_CreateRGBSurface(SDL_SWSURFACE,
					      scale * WIN_SCRN_X * 8,
					      scale * WIN_SCRN_Y * 8,
					      32,
					      0xff0000,
					      0xff00,
					      0xff,
					      0);
	else
		screen = SDL_SetVideoMode(scale * WIN_SCRN_X * 8,
					  scale * WIN_SCRN_Y * 8, 0, flags);


	if (screen == NULL) {
		printf("Couldn't set 16 bit video mode: %s\n",
				SDL_GetError());

		return 2;
	}
	screen_copy = SDL_CreateRGBSurface(SDL_SWSURFACE, screen->w, screen->h,
			screen->format->BitsPerPixel,
			screen->format->Rmask,
			screen->format->Gmask,
			screen->format->Bmask,
			screen->format->Amask);
	s_black = SDL_CreateRGBSurface(SDL_SWSURFACE, screen->w, screen->h,
			screen->format->BitsPerPixel,
			screen->format->Rmask,
			screen->format->Gmask,
			screen->format->Bmask,
			screen->format->Amask);
	SDL_Rect rect;
	rect.w = screen->w;
	rect.h = screen->h;
	rect.x = 0;
	rect.y = 0;
	SDL_FillRect(s_black, &rect, SDL_MapRGB(s_black->format, 0, 0, 0));
	js_scale_height();
	return 0;
}

int platform_init(void)
{
	memset(s_palette, 0, sizeof(s_palette));
	/* Initialize SDL */
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER
#if !defined(NO_SOUND)
				|SDL_INIT_AUDIO
#endif
				/*      |SDL_INIT_NOPARACHUTE */
		    ) < 0) {
		printf("Couldn't initialize SDL: %s\n",
				SDL_GetError());
		return 1;
	}

	int width, height, fullscreen;
	emscripten_get_window_size(&width, &height, &fullscreen);
	if (width != 0 && height != 0) {
		/* resize based on these funky values */
		int newScaleX = width / (8 * WIN_SCRN_X); /* 8 * 32 = 256 */
		int newScaleY = height / (8 * WIN_SCRN_Y); /* 8 * 24 = 192 */
		int newScale = width > height ? newScaleY : newScaleX;
		if (newScale != 0 && newScale != gfx_scale)
			gfx_scale = newScale;
		if (gfx_scale > 3)
			gfx_scale = 3;
		if (gfx_scale < 1)
			gfx_scale = 1;
	}

	int result = init_screen(gfx_scale);
	if (result != 0)
		return result;

	SDL_WM_SetCaption("Chaos", NULL);
	SDL_Surface *iconsurf = get_icon(g_icon);
	SDL_WM_SetIcon(iconsurf, NULL);
	SDL_FreeSurface(iconsurf);

#if !defined(NO_SOUND)
	if (init_sdl_sound() == 0) {
		printf("Couldn't start sound: %s\n",
				SDL_GetError());
	}
#endif
	enable_interrupts();
	sdl_init_keys();
	return 0;
}

static void draw_sprite(void)
{
	int x, y;
	int idx = 0;
	if (!g_cursor_on)
		return;
	for (y = 0; y < 2; y++) {
		for (x = 0; x < 2; x++) {
			draw8x8SpriteTile(g_cursor_x + x, g_cursor_y + y,
					(const uint8_t*)&g_spriteTileData[idx * 32]);
			idx++;
		}
	}
	idx = g_cursor_x + (g_cursor_y) * WIN_SCRN_X;
	s_mapChanged[idx] = 1;
	s_mapChanged[idx + 1] = 1;
	s_mapChanged[idx + WIN_SCRN_X] = 1;
	s_mapChanged[idx + WIN_SCRN_X + 1] = 1;
}

static void render_screen(void)
{
	int x, y;
	gfx_scale_8 = gfx_scale * 8;
	for (y = 0; y < WIN_SCRN_Y; y++) {
		for (x = 0; x < WIN_SCRN_X; x++) {
			draw8x8Tile(x, y, s_mapData, s_tileData);
			draw8x8Tile(x, y, s_textMapData, s_textTileData);
		}
	}
	memset(s_mapChanged, 0, sizeof(s_mapChanged));
	memset(s_paletteChanged, 0, sizeof(s_paletteChanged));
	memset(s_tileDataChanged, 0, sizeof(s_tileDataChanged));
}

void sdl_update_screen(void)
{
	if (!screen || !screen_copy)
		return;
	/* wait for the game_frame to change... */
	if (s_last_fade_level != s_fade_level) {
		if (s_last_fade_level == 0) {
			/* fading out - copy actual screen to screen_copy as will draw to copy now */
			SDL_BlitSurface(screen, 0, screen_copy, 0);
		} else if (s_fade_level == 0) {
			/* fading in - blit working screen_copy to screen as that's used from now on */
			SDL_BlitSurface(screen_copy, 0, screen, 0);
		}
	}
	render_screen();
	if (s_fade_level != 0)
		SDL_BlitSurface(screen_copy, 0, screen, 0);
	draw_sprite();
	if (s_fade_level != 0) {
		SDL_SetAlpha(s_black, SDL_SRCALPHA, s_fade_level);
		SDL_BlitSurface(s_black, 0, screen, 0);
	}
	SDL_Flip(screen);
	s_last_fade_level = s_fade_level;
}

void platform_set_text_map_entry(int x, int y, int palette)
{
	int idx = x + y * WIN_SCRN_X;
	if (idx > (WIN_SCRN_X * WIN_SCRN_Y))
		return;
	if (s_textMapData[idx] != palette) {
		s_mapChanged[idx] = 1;
		s_textMapData[idx] = palette;
	}
}

void platform_load_text_tile_data(int offset, const void *data, int size)
{
	int i;
	int start = offset / 16;
	int end = (offset + size) / 16;
	for (i = start; i <= end; i++)
		s_tileDataChanged[i] = 1;
	if ((size_t)size > sizeof(s_textTileData))
		printf("Uh-oh, oversize\n");
	memcpy(&s_textTileData[offset], data, size);
}

void platform_screen_size(int *width, int *height)
{
	*width = WIN_SCRN_X * 8;
	*height = WIN_SCRN_Y * 8;
}

int platform_swap_screen(void)
{
	return 1;
}
