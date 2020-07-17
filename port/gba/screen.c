#include <string.h>
#include <gba_systemcalls.h>
#include <gba_video.h>
#include <gba_dma.h>
#include "chaos/porting.h"
#include "port/gba/vba.h"

#define BLDMOD_BG0  BIT(0)
#define BLDMOD_BG1  BIT(1)
#define BLDMOD_BG2  BIT(2)
#define BLDMOD_BG3  BIT(3)
#define BLDMOD_OBJ  BIT(4)
#define BLDMOD_BD   BIT(5)
#define BLDMOD_MODE(n)    (n<<6)

uint16_t *level_map_data = 0;
uint16_t *level_char_data = 0;

uint16_t *text_map_data = 0;
uint16_t *text_char_data = 0;

uint16_t platform_get_palette_entry(int idx)
{
	return BG_PALETTE[idx];
}

void platform_set_palette_entry(int idx, uint16_t color)
{
	BG_PALETTE[idx] = color;
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

void platform_set_tile_entry(int offset, int value)
{
	level_char_data[offset] = value;
}

void platform_load_tile_data(int offset, const void *data, int size)
{
	dmaCopy(data, &level_char_data[offset], size);
}

int platform_get_tile_entry(int offset)
{
	return level_char_data[offset];
}

void platform_set_fade_level(int level)
{
	if (level > 16 || level < 0) {
		return;
	}
	REG_BLDCNT = BLDMOD_BG1 | BLDMOD_BG0 |
		BLDMOD_OBJ | (BLDMOD_BD << 8) | BLDMOD_MODE(3);
	REG_BLDY = level;
}

void platform_wait(void)
{
	VBlankIntrWait();
}

extern void gba_init_sprites(void);
extern void gba_init_irq(void);
extern void gba_init_sound(void);
int platform_init(void)
{
	uint16_t flags;
	int map_base = 28;
	RegisterRamReset(RESET_VRAM | RESET_PALETTE | RESET_OAM);
	level_map_data = (uint16_t*)MAP_BASE_ADR(map_base);
	level_char_data = (uint16_t*)CHAR_BASE_ADR(0);
	flags = BG_16_COLOR | TEXTBG_SIZE_512x256 |
		BG_MAP_BASE(map_base) | BG_TILE_BASE(0) |
		BG_PRIORITY(2);
	SetMode(MODE_1 | OBJ_ENABLE | OBJ_1D_MAP);
	/* init level to background 0, text to background 1 */
	BGCTRL[0] = flags;

	map_base = 30;
	text_map_data = (uint16_t*)MAP_BASE_ADR(map_base);
	text_char_data = (uint16_t*)CHAR_BASE_ADR(0);
	flags = BG_16_COLOR | TEXTBG_SIZE_256x256 |
		BG_MAP_BASE(map_base) | BG_TILE_BASE(0) |
		BG_PRIORITY(1);
	BGCTRL[1] = flags;
	REG_DISPCNT |= BG0_ENABLE | BG1_ENABLE;
	gba_init_sprites();
	gba_init_irq();
	gba_init_sound();
#ifdef GBA_PROFILE
	monstartup(0x080002bc, 0x0800c0d4);
#endif
	return 0;
}

void platform_dprint(const char *s)
{
	vbalog(s);
}

void platform_screen_size(int *width, int *height)
{
	*width = 240;
	*height = 160;
}

int platform_swap_screen(void)
{
	return 1;
}
