#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <nds.h>
#include "chaos/porting.h"

static uint16_t *level_map_data = 0;
static uint16_t *level_char_data = 0;

static uint16_t *main_level_map_data = 0;
static uint16_t *main_level_char_data = 0;
static uint16_t *sub_level_map_data = 0;
static uint16_t *sub_level_char_data = 0;

static uint16_t *text_map_data = 0;
static uint16_t *text_char_data = 0;
static uint16_t *sprite_gfx = 0;

static uint16_t *main_text_map_data = 0;
static uint16_t *sub_text_map_data = 0;

static uint16_t *main_text_char_data = 0;
static uint16_t *sub_text_char_data = 0;

extern void ds_init_sound(void);

static void clear_vram(void)
{
	int i;
	u16 *bgpal = BG_PALETTE;
	u16 *sppal = SPRITE_PALETTE;
	u16 *gfx = BG_GFX;

	for (i = 0; i < 256; i++) {
		bgpal[i] = 0;
		sppal[i] = 0;
	}
	/* Clear VRAM */
	for (i = 0; i < 0x40000; ++i)
		gfx[i] = 0;

	bgpal = BG_PALETTE_SUB;
	gfx = BG_GFX_SUB;

	for (i = 0; i < 256; i++)
		bgpal[i] = 0;
	/* Clear VRAM */
	for (i = 0; i < 0x40000; ++i)
		gfx[i] = 0;
}

int platform_init(void)
{
	videoSetMode(MODE_0_2D | DISPLAY_SPR_ACTIVE);
	videoSetModeSub(MODE_0_2D);
	vramSetBankA(VRAM_A_MAIN_BG);
	vramSetBankB(VRAM_B_MAIN_SPRITE_0x06400000);
	vramSetBankC(VRAM_C_SUB_BG);
	clear_vram();
        oamInit(&oamMain, SpriteMapping_1D_32, false);
	lcdMainOnBottom();
	int map_base = 28;
	int tile_base = 0;
	int bg = bgInit(0, BgType_Text4bpp, BgSize_T_512x256, map_base, tile_base);
	bgSetPriority(bg, 2);
	main_level_char_data = bgGetGfxPtr(bg);
	main_level_map_data = bgGetMapPtr(bg);

	bg = bgInitSub(0, BgType_Text4bpp, BgSize_T_512x256, map_base, tile_base);
	bgSetPriority(bg, 2);
	sub_level_char_data = bgGetGfxPtr(bg);
	sub_level_map_data = bgGetMapPtr(bg);

	level_char_data = main_level_char_data;
	level_map_data = main_level_map_data;

	map_base = 30;
	bg = bgInit(1, BgType_Text4bpp, BgSize_T_256x256, map_base, tile_base);
	bgSetPriority(bg, 1);
	main_text_char_data = bgGetGfxPtr(bg);
	main_text_map_data = bgGetMapPtr(bg);

	bg = bgInitSub(1, BgType_Text4bpp, BgSize_T_256x256, map_base, tile_base);
	bgSetPriority(bg, 1);
	sub_text_char_data = bgGetGfxPtr(bg);
	sub_text_map_data = bgGetMapPtr(bg);

	text_map_data = main_text_map_data;
	text_char_data = main_text_char_data;

	sprite_gfx = oamAllocateGfx(&oamMain, SpriteSize_16x16, SpriteColorFormat_256Color);
	ds_init_sound();
	return 0;
}

void platform_set_palette_entry(int idx, uint16_t color)
{
	if (text_map_data == main_text_map_data)
		BG_PALETTE[idx] = color;
	else
		BG_PALETTE_SUB[idx] = color;
}

uint16_t platform_get_palette_entry(int idx)
{
	if (text_map_data == main_text_map_data)
		return BG_PALETTE[idx];
	else
		return BG_PALETTE_SUB[idx];
}

void platform_set_map_entry(int x, int y, int value)
{
	/* the second bank of 256*256 start at tile 32*32... */
	if (x < 32)
		level_map_data[x + y * 32] = value;
	else
		level_map_data[(x - 32) + (32 * 32) + y * 32] = value;
}

uint16_t platform_get_map_entry(int x, int y)
{
	return level_map_data[x + y * 32];
}

void platform_set_text_map_entry(int x, int y, int palette)
{
	if (palette == 0)
		text_map_data[x + y * 32] = 0;
	else
		text_map_data[x + y * 32] = palette + 605;
}

void platform_set_tile_entry(int offset, int value)
{
	level_char_data[offset] = value;
}

void platform_load_tile_data(int offset, const void *data, int size)
{
	memcpy(&level_char_data[offset], data, size);
}

void platform_load_text_tile_data(int offset, const void *data, int size)
{
	offset = 605 * 16;
	memcpy(&text_char_data[offset], data, size);
}

int platform_get_tile_entry(int offset)
{
	return level_char_data[offset];
}

#define BLDMOD_FADE (3<<6)
#define BLDMOD_BG0 1
#define BLDMOD_BG1 2
#define BLDMOD_BG2 4
#define BLDMOD_BG3 8
#define BLDMOD_OBJ 16
#define BLDMOD_BD  32

static void blend(int first, int second)
{
	int blnd = (first)|((second)<<8)|(BLDMOD_FADE);
	REG_BLDCNT = blnd;
}

static void set_fade(int level)
{
	swiWaitForVBlank();
	blend(BLDMOD_BG0 | BLDMOD_BG1 | BLDMOD_BG2| BLDMOD_OBJ,
		BLDMOD_BD );
	REG_BLDY = level;
}

void platform_set_fade_level(int level)
{
	if (level < 0 || level > 16)
		return;
	set_fade(level);
}

void platform_load_sprite_data(int offset, const uint16_t *data)
{
	DC_FlushAll();
	dmaCopy(data, &sprite_gfx[offset], 64);
}

void platform_load_sprite_palette(int offset, const uint16_t *data, int size)
{
	DC_FlushAll();
	dmaCopy(data, &SPRITE_PALETTE[offset], size * 2);
}

void platform_set_sprite_enabled(int enabled)
{
	if (enabled) {
		oamEnable(&oamMain);
	} else {
		oamDisable(&oamMain);
	}
}

void platform_move_sprite(int spriteid, int x, int y)
{
	oamSet(&oamMain,
		spriteid,	/* oam index */
		x, y,		/* x and y location of the sprite*/
		0,		/* priority, lower renders last (on top) */
		0,		/* alpha value of bmp sprite */
		SpriteSize_16x16,
		SpriteColorFormat_256Color,
		sprite_gfx,	/* pointer to the loaded graphics */
		-1,		/* sprite rotation data */
		false,		/* double the size when rotating? */
		false,		/* hide the sprite? */
		false, false,	/* vflip, hflip */
		false		/* apply mosaic */
		);
	oamUpdate(&oamMain);
}

void platform_dprint(const char *s)
{
	iprintf("%s", s);
}

int platform_line_fudge_factor(void)
{
	return 30;
}

const char *platform_name(void)
{
	return "NDS";
}

void platform_screen_size(int *width, int *height)
{
	*width = 256;
	*height = 192;
}

int platform_swap_screen(void)
{
	if (text_map_data == sub_text_map_data) {
		text_map_data = main_text_map_data;
		text_char_data = main_text_char_data;
		level_char_data = main_level_char_data;
		level_map_data = main_level_map_data;
	} else {
		text_map_data = sub_text_map_data;
		text_char_data = sub_text_char_data;
		level_char_data = sub_level_char_data;
		level_map_data = sub_level_map_data;
	}
	return 0;
}

void platform_exit(void)
{
	exit(0);
}

static const char *s_langs[] = {
	"ja",
	"en",
	"fr",
	"de",
	"it",
	"es",
	"ch",
	"??"
};

const char *platform_get_lang(void)
{
	int lang = PersonalData->language;
	if (lang > 7)
		return "en";
	return s_langs[lang];
}

void platform_more_options(void) { }

void platform_wait(void)
{
	swiWaitForVBlank();
}
