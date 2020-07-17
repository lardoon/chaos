#include <gba_dma.h>
#include <gba_sprites.h>
#include <gba_video.h>
#include "chaos/porting.h"

#define MAX_SPRITE_ID 128
static OBJATTR s_sprite[MAX_SPRITE_ID];

void platform_load_sprite_palette(int offset, const uint16_t *data, int size)
{
	dmaCopy(data, &SPRITE_PALETTE[offset], size * 2);
}

void platform_load_sprite_data(int offset, const uint16_t *data)
{
	dmaCopy(data, &SPRITE_GFX[offset], 64);
}

static void gba_update_sprites(void)
{
	int i;
	for(i = 0; i < MAX_SPRITE_ID; i++)
		OAM[i] = s_sprite[i];
}

void platform_move_sprite(int spriteid, int x, int y)
{
	OBJATTR *sp = &s_sprite[spriteid];
	x &= 0x01FF;
	sp->attr1 = sp->attr1 & 0xFE00;
	sp->attr1 = sp->attr1 | x;
	y &= 0x00FF;
	sp->attr0 = sp->attr0 & 0xFF00;
	sp->attr0 = sp->attr0 | y;
	gba_update_sprites();
}

void gba_init_sprites(void)
{
	int i;
	for (i = 0; i < 128; i++) {
		OAM[i].attr0 = OBJ_DISABLE;
	}

	for (i = 0; i < MAX_SPRITE_ID; i++) {
		OBJATTR *sp = &s_sprite[i];
		sp->attr0 = ATTR0_COLOR_256 | ATTR0_SQUARE;
		sp->attr1 = ATTR1_SIZE_16;
		sp->attr2 = ATTR2_PRIORITY(1) | (i * 8);
	}
}

void platform_set_sprite_enabled(int enabled)
{
	if (enabled) {
		REG_DISPCNT |= OBJ_ENABLE;
	} else {
		REG_DISPCNT &= ~OBJ_ENABLE;
	}
}
