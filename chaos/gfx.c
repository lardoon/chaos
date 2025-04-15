#include <string.h>

#include "chaos/platform.h"
#include "chaos/porting.h"
#include "chaos/palettes.h"

#include "chaos/gfx.h"
#include "chaos/images.h"
/* specific includes */
#include "chaos/spelldata.h"
#include "chaos/arena.h"
#include "chaos/creature.h"
#include "chaos/cursor_defines.h"
#include "chaos/chaos.h"
#include "chaos/wizards.h"
#include "chaos/text16.h"

#define SPRITEPALETTE_DATA palettebPal
#define SPRITEPALETTE_SIZE (palettebPalLen/2)

#define BG_CORNER_TILE 601
#define BG_VERT_EDGE_TILE 602
#define BG_HORZ_EDGE_TILE 603
#define BG_SOLID_TILE 604
#define BG_PALETTE (11<<12)


const uint16_t *PALETTES[10] = {
	palette0Pal,
	palette1Pal,
	palette2Pal,
	palette3Pal,
	palette4Pal,
	palette5Pal,
	palette6Pal,
	palette7Pal,
	palette8Pal,
	palette9Pal,
};

static const unsigned short *const s_cursor_gfx[][2] = {
  {cursor_spellTiles, cursor_spellMap},
  {cursor_engagedTiles, cursor_engagedMap},
  {cursor_fireTiles, cursor_fireMap},
  {cursor_flyTiles, cursor_flyMap},
  {cursor_groundTiles, cursor_groundMap},
  {cursorTiles, cursorMap},
};

/*unsigned char bg_colour; // the colour of the pixel to draw in the rotation layer */

unsigned char cursor_x = 0;
unsigned char cursor_y = 0;

unsigned char fadedup;

static void load_sprite_data(uint32_t spriteid)
{
	/* load a sprite from the sprite arrays */
	/* check we are in bounds */
	if (spriteid > CURSOR_NORMAL_GFX)
		return;

	/* get the size of the object and from this work out how many tiles need to be read in */
	/*k = <start of frame in map file> = Frame*number of tiles per sprite */
	/* tile count = (sprites*width*height / 64) */
	const uint16_t *tile_data = s_cursor_gfx[spriteid][0];
	const uint16_t *map_data = s_cursor_gfx[spriteid][1];

	int tile_count = 4;
	int i;
	/* need to load it in blocks of 64... */
	for (i = 0; i < tile_count; i++) {
		int tile_id = map_data[i];
		int offset = i * 32;
		platform_load_sprite_data(offset, &tile_data[tile_id * 32]);
	}
}

static void set_palette(unsigned short x, unsigned short y, unsigned char palette)
{
	/* sets the arena square's palette index... */
	/* mask out the old palette and set the current one */
	/* get the tile ids for the 4 gba tiles at the ARENA x,y */
	set_palette8(x * 2, y * 2, palette);
}

static void draw_gfx(const uint16_t *gfx, const uint16_t *map,
		uint8_t x, uint8_t y, uint8_t frame)
{
	draw_gfx8(gfx, map, x * 2, y * 2, frame);
}

void draw_creature(unsigned char x, unsigned char y, unsigned char i,
		unsigned char f)
{
	/* draw the creature i at position x,y with frame f */
	set_palette(x, y, CHAOS_SPELLS[i].palette);

	draw_gfx(CHAOS_SPELLS[i].pGFX,
			CHAOS_SPELLS[i].pGFXMap, x, y, f);
}

void clear_square(unsigned char x, unsigned char y)
{
	/* clears a square in the arena */
	int i;
	int startOAM = 16 + x * 2 * 16 + 2 * y * 30 * 16;
	/* bug here in -O3 ?  */
	/* clear  */
	for (i = 0; i < 32; i++) {
		platform_set_tile_entry(startOAM++, 0);
	}

	startOAM = 16 + x * 2 * 16 + (2 * y + 1) * 30 * 16;	/* x*2*16+(2*y+1)*30*16; */
	for (i = 0; i < 32; i++) {
		platform_set_tile_entry(startOAM++, 0);
	}
}

static void draw_decor_border_internal(unsigned char pal,
		unsigned short col1, unsigned short col2,
		int x_limit, int y_limit)
{
	uint16_t x, y;
	/* set the colours */
	platform_set_palette_entry(16 * pal + 2, col1);
	platform_set_palette_entry(16 * pal + 1, col2);

	platform_load_tile_data(16 + (19200 / 2), bgTiles, bgTilesLen);

	/* top left corner */
	platform_set_map_entry(0, 0, BG_CORNER_TILE | PALETTE(pal));
	/* top right corner */
	platform_set_map_entry(x_limit, 0, BG_CORNER_TILE | TILE_FLIP_HORZ | PALETTE(pal));
	/* bottom left corner */
	platform_set_map_entry(0, y_limit, BG_CORNER_TILE | TILE_FLIP_VERT | PALETTE(pal));
	/* bottom right corner */
	platform_set_map_entry(x_limit, y_limit, BG_CORNER_TILE | TILE_FLIP_HORZ | TILE_FLIP_VERT | PALETTE(pal));
	/* now fill the sides */
	/* top edge */
	for (x = 1; x < x_limit; x++) {
		platform_set_map_entry(x, 0, BG_HORZ_EDGE_TILE | PALETTE(pal));
	}

	/* bottom edge */
	for (x = 1; x < x_limit; x++) {
		platform_set_map_entry(x, y_limit, BG_HORZ_EDGE_TILE | TILE_FLIP_VERT | PALETTE(pal));

	}

	/* left edge */
	for (y = 1; y < y_limit; y++) {
		platform_set_map_entry(0, y, BG_VERT_EDGE_TILE | PALETTE(pal));
	}

	/* right edge */
	for (y = 1; y < y_limit; y++) {
		platform_set_map_entry(x_limit, y, BG_VERT_EDGE_TILE | TILE_FLIP_HORZ | PALETTE(pal));
	}
	platform_set_x_scroll(0);
	platform_set_y_scroll(0);
	platform_update_scroll();
}

void draw_game_border(void)
{
	if (!platform_has_scroll()) {
		draw_decor_border_internal(11, RGB16(31, 0, 0), RGB16(21, 0, 0),
				31, 21);
		return;
	}
	uint16_t x, y;

	/* draws the game border, which is tiles in bg */
	/* place the tiles at the end of the creature data, i.e. 150 creatures, 4 tiles each = 600 */
	/* use tile 600 to 607 = 0x4b00 19200 to 19392 (= 32 bytes per tile */
	/*  */

	platform_load_tile_data(16 + (19200 / 2), bgTiles, bgTilesLen);

	/* top left corner */
	platform_set_map_entry(0, 0, BG_CORNER_TILE | BG_PALETTE);
	/* top right corner */
	platform_set_map_entry(33, 0, BG_CORNER_TILE | TILE_FLIP_HORZ | BG_PALETTE);
	/* bottom left corner */
	platform_set_map_entry(0, 23, BG_CORNER_TILE | TILE_FLIP_VERT | BG_PALETTE);
	/* bottom right corner */
	platform_set_map_entry(33, 23, BG_CORNER_TILE | TILE_FLIP_HORZ | TILE_FLIP_VERT | BG_PALETTE);

	/* now fill in the sides */
	/* top / bottom edges */
	for (x = 1; x < 32; x++) {
		/* top */
		platform_set_map_entry(x, 0, BG_HORZ_EDGE_TILE | BG_PALETTE);
		platform_set_map_entry(x, 1, BG_SOLID_TILE | BG_PALETTE);
		/* bottom edge */
		platform_set_map_entry(x, 23, BG_HORZ_EDGE_TILE | TILE_FLIP_VERT | BG_PALETTE);
		platform_set_map_entry(x, 22, BG_SOLID_TILE | BG_PALETTE);
	}
	/* left over top edge tile... */
	platform_set_map_entry(32, 0, BG_HORZ_EDGE_TILE | BG_PALETTE);
	/* left over bottom edge tile... */
	platform_set_map_entry(32, 23, BG_HORZ_EDGE_TILE | TILE_FLIP_VERT | BG_PALETTE);

	/* left and right edges */
	for (y = 1; y < 23; y++) {
		/* left */
		platform_set_map_entry(0, y, BG_VERT_EDGE_TILE | BG_PALETTE);
		platform_set_map_entry(1, y, BG_SOLID_TILE | BG_PALETTE);

		/* right edge */
		platform_set_map_entry(33, y, BG_VERT_EDGE_TILE | TILE_FLIP_HORZ | BG_PALETTE);
		platform_set_map_entry(32, y, BG_SOLID_TILE | BG_PALETTE);
	}
}

void clear_game_border(void)
{
	uint16_t x, y;
	/* the second bank of 256*256 start at tile 32*32... */
	uint16_t SECOND_TILEBANK_0 = 32 * 32;

	/* clear the game border from the map data */

	/* top left corner */
	platform_set_tile_entry(0, 0);
	/* top right corner */
	platform_set_tile_entry(SECOND_TILEBANK_0 + 1, 0);
	/* bottom left corner */
	platform_set_tile_entry(23 * 32, 0);
	/* bottom right corner */
	platform_set_tile_entry(SECOND_TILEBANK_0 + 1 + 23 * 32, 0);

	/* now fill in the sides */
	/* top/bottom edge */
	for (x = 1; x < 32; x++) {
		/*top */
		platform_set_tile_entry(x, 0);
		platform_set_tile_entry(x + 32, 0);
		/*bottom */
		platform_set_tile_entry(x + 23 * 32, 0);
		platform_set_tile_entry(x + 22 * 32, 0);
	}
	/* left over top edge tile... */
	platform_set_tile_entry(SECOND_TILEBANK_0 + 0, 0);
	/* left over bottom edge tile... */
	platform_set_tile_entry(SECOND_TILEBANK_0 + 23 * 32, 0);

	/* left and right edge */
	for (y = 1; y < 23; y++) {
		/*l */
		platform_set_tile_entry(y * 32, 0);
		platform_set_tile_entry(1 + y * 32, 0);
		/*r */
		platform_set_tile_entry(SECOND_TILEBANK_0 + 1 + y * 32, 0);
		platform_set_tile_entry(SECOND_TILEBANK_0 + y * 32, 0);
	}
}

/* draw a "decorative" border... within the given palette with the given colours */
void draw_decor_border(unsigned char pal, unsigned short col1,
		unsigned short col2)
{

	/* this border is smaller than the game border, goes from 0 to 30 and is only 1 deep */
	int width, height;
	int x_limit, y_limit;
	platform_screen_size(&width, &height);
	x_limit = (width / 8) - 1;
	y_limit = (height / 8) - 1;
	draw_decor_border_internal(pal, col1, col2, x_limit, y_limit);
}

void load_all_palettes(void)
{
	int i;
	for (i = 0; i < 10; i++) {
		load_bg_palette(i, i);
	}
	for (i = 0; i < 8; i++) {
		set_player_col(i, players[i].colour);
	}
	platform_set_palette_entry(0, 0);
}

void load_bg_palette(uint8_t topal, uint8_t frompal)
{
	uint16_t palindex = topal * 16;
	int i;
	for (i = 0; i < 16; i++) {
		uint16_t col = PALETTES[frompal][i];
		if (i == 0)
			col = 0;
		platform_set_palette_entry(palindex + i, col);
	}
}

void load_sprite_palette(void)
{
	platform_load_sprite_palette(0, (const uint16_t*) SPRITEPALETTE_DATA, SPRITEPALETTE_SIZE);
}

void draw_cursor(uint32_t cursorid)
{
	/* draw the given cursor at the given coordinates... */
	load_sprite_data(cursorid);
	/*platform_move_sprite(0, 0, 0);*/
}

/*//////////////////////////////////////////  */

static uint16_t munge_pixels(uint8_t thisVal, int newCol_l, int newCol_h)
{
	static uint8_t defaultCol_l = WIZARD_COLOUR;
	static uint8_t defaultCol_h = WIZARD_COLOUR << 4;
	/* 4 bits per pixel... */
	/* mask the lower 4 bits */
	if ((thisVal & 0x0f) == defaultCol_l) {
		thisVal &= ~defaultCol_l;
		thisVal |= newCol_l;
	}
	/* mask the higher 4 bits */
	if ((thisVal & 0xf0) == defaultCol_h) {
		thisVal &= ~defaultCol_h;
		thisVal |= newCol_h;
	}
	return thisVal;
}

void draw_wizard8(unsigned long x, unsigned long y, unsigned long wizardid,
		unsigned long frame, unsigned long playerid)
{
	set_palette8(x, y, 9);

	uint16_t pNewGfx[64];
	uint16_t nTileCount = 4;	/* 4 tiles for each wizard */
	uint16_t k = frame * nTileCount;

	uint16_t id;
	uint16_t index = k;

	/* need to load it in blocks of 16 for 4bpp gfx */
	uint16_t loop = 0;

	/* copy the actual graphics into the temp array */
	for (loop = 0; loop < 4; loop++) {
		/* id = <the tile id in the map file at this position> */
		id = WizardGFX[wizardid].pMap[index++];
		memcpy(&pNewGfx[loop * 16], &WizardGFX[wizardid].pGFX[id * 16], 32);
	}

	/* replace the temp array index WIZARD_COLOUR (which is the colour that should be replaced) */
	/* with index playerid, which is the colour index for this player. */
	uint8_t newCol_l = (uint8_t) playerid + 8;
	uint8_t newCol_h = (uint8_t) (playerid + 8) << 4;
	for (loop = 0; loop < 64; loop++) {
		uint16_t thisVal = pNewGfx[loop];
		pNewGfx[loop] = munge_pixels(thisVal & 0xff, newCol_l, newCol_h) |
			(munge_pixels(thisVal >> 8, newCol_l, newCol_h) << 8);
	}
	/* each 8*8 tile takes up 32 bytes. */
	uint16_t startOAM = 16 + x * 16 + y * 30 * 16;
	platform_load_tile_data(startOAM, pNewGfx, 64);
	startOAM = 16 + x * 16 + (y + 1) * 30 * 16 - 32;	/* x*2*16+(2*y+1)*30*16; */
	platform_load_tile_data(32 + startOAM, &pNewGfx[32], 64);
}

void draw_wizard(unsigned long x, unsigned long y, unsigned long wizardid,
		unsigned long frame, unsigned long playerid)
{
	/* draw the given wizard id...  */
	/* take from list of wizard gfx pointers */
	/* WizardGFX[wizardid].pWizardData->pGFX is the pointer to the graphics */
	/* WizardGFX[wizardid].pWizardData->pMap is the pointer to the map file */
	/* to draw a given frame use similar code to the creature drawing routine */

	if ((players[playerid].modifier_flag & 0x8) && frame == 3) {
		/* shadow form, do not draw this frame */
		clear_square(x, y);
		return;
	}

	draw_wizard8(x * 2, y * 2, wizardid, frame, playerid);
}

void set_palette8(unsigned short x, unsigned short y,
		unsigned char palette)
{
	/* 8 bit resolution, 4 square size setting...  mask out the old palette
	 * and set the current one. Get the tile ids for the 4 gba tiles at the
	 * ARENA x,y */

	/* there are 2 rows before hand and 2 blocks at the start of each row */
	if (platform_has_scroll()) {
		y += 2;
		x += 2;
	} else {
		y += 1;
		x += 1;
	}
	platform_set_map_entry(x, y, UPDATE_PALETTE(platform_get_map_entry(x, y), palette));
	platform_set_map_entry(x + 1, y, UPDATE_PALETTE(platform_get_map_entry(x + 1, y), palette));
	platform_set_map_entry(x, y + 1, UPDATE_PALETTE(platform_get_map_entry(x, y + 1), palette));
	platform_set_map_entry(x + 1, y + 1, UPDATE_PALETTE(platform_get_map_entry(x + 1, y + 1), palette));
}

void set_player_col(unsigned char playerid, unsigned short colour)
{
	/* set a player to be this colour */
	if (playerid < 8) {
		platform_set_palette_entry(16 * 9 + playerid + 8, colour);
	}
}

void set_border_col(unsigned char playerid)
{
	/* set the border colour based on the player colour */
	if (playerid < 8) {
		uint16_t col = platform_get_palette_entry(16 * 9 + playerid + 8);
		int8_t red = col & 0x1F;
		int8_t green = (col >> 5) & 0x1F;
		int8_t blue = (col >> 10) & 0x1F;
		red -= 8;
		green -= 8;
		blue -= 8;
		if (red < 0)
			red = 0;
		if (green < 0)
			green = 0;
		if (blue < 0)
			blue = 0;

		platform_set_palette_entry(16 * 11 + 2, col);
		platform_set_palette_entry(16 * 11 + 1, RGB16(red, green, blue));
	}

}

void clear_palette(unsigned char pal)
{
	if (pal > 15)
		return;
	int i;
	for (i = 0; i < 16; i++) {
		platform_set_palette_entry(pal * 16 + i, 0);
	}
}

void clear_palettes(void)
{
	int i;
	for (i = 0; i < 16; i++) {
		clear_palette(i);
	}
}

/* this is a bit of a problem... just changes the palette */
/* on win, how to update the text? */
/* could search palette info, get the square with this palette */
/* then redraw it in place... */
void anim_selection(uint8_t pal, uint8_t red, uint8_t green, uint8_t blue)
{
	int8_t r = red;
	int8_t g = green;
	int8_t b = blue;

	r = r + anim_col;
	g = g + anim_col;
	b = b + anim_col;

	r = r > 31 ? 31 : r;
	g = g > 31 ? 31 : g;
	b = b > 31 ? 31 : b;

	r = r < 0 ? 0 : r;
	g = g < 0 ? 0 : g;
	b = b < 0 ? 0 : b;

	set_text16_colour(pal, RGB16(r, g, b));

	anim_col += anim_col_grad;

	if (anim_col >= 31) {
		anim_col_grad = -8;
		anim_col = 31;
	} else if (anim_col <= -15) {
		anim_col_grad = 8;
		anim_col = -15;
	}

}

void draw_gfx8(const uint16_t *gfx, const uint16_t *map, uint8_t x, uint8_t y, uint8_t frame)
{
	/* draws graphics on the 8 by 8 tile boundaries */
	/* ie, to draw on 16*16 tiles, multiply x, y by 2 */
	uint16_t nTileCount = 4;	/* 4 tiles for each 16*16 gfx  */
	uint16_t index = frame * nTileCount;
	uint16_t id;
	uint8_t loop = 0;
	/* each 8*8 tile takes up 32 bytes. */
	uint16_t startOAM = 16 + x * 16 + y * 30 * 16;
	/* get start address in screen */
	for (loop = 0; loop < 2; loop++) {
		/* id = <the tile id in the map file at this position> */
		id = map[index++];
		platform_load_tile_data(loop * 16 + startOAM, &gfx[id * 16], 32);
	}

	startOAM = 16 + x * 16 + (y + 1) * 30 * 16 - 32;
	for (loop = 2; loop < 4; loop++) {
		/* id = <the tile id in the map file at this position> */
		id = map[index++];
		platform_load_tile_data(loop * 16 + startOAM, &gfx[id * 16], 32);
	}
}

void draw_fight_frame(uint8_t x, uint8_t y, uint8_t frame)
{
	set_palette(x, y, 8);
	draw_gfx(fightTiles, fightMap, x, y, frame);
}

void draw_spellcast_frame(uint8_t x, uint8_t y, uint8_t frame)
{
	set_palette(x, y, 0);
	draw_gfx(spellTiles, spellMap, x, y, frame);
}

void draw_splat_frame(uint8_t x, uint8_t y, uint8_t frame)
{
	set_palette(x, y, 8);
	draw_gfx(bolt_animTiles, bolt_animMap, x, y, frame);
}

void draw_pop_frame(uint8_t x, uint8_t y, uint8_t frame)
{
	set_palette(x, y, 8);
	draw_gfx(disappearTiles, disappearMap, x, y, frame);
}

void draw_pop_frame_2(int idx)
{
	uint8_t x, y;
	get_yx(idx, &y, &x);
	x--;
	y--;
	set_palette(x, y, 8);
	int frame;
	disable_interrupts();
	for (frame = 0; frame < 7 ; frame++) {
		draw_gfx(disappearTiles, disappearMap, x, y, frame);
		platform_wait();
	}
	enable_interrupts();
}

void draw_splat_frame_2(int idx)
{
	uint8_t x, y;
	get_yx(idx, &y, &x);
	x--;
	y--;
	set_palette(x, y, 8);
	int frame;
	disable_interrupts();
	for (frame = 0; frame < 8 ; frame++) {
		draw_gfx(bolt_animTiles, bolt_animMap, x, y, frame);
		platform_wait();
	}
	enable_interrupts();
}

void draw_breath_frame(uint8_t x, uint8_t y, uint8_t frame)
{
	set_palette(x, y, 8);
	draw_gfx(fire_animTiles, fire_animMap, x, y,
			frame);
}

void remove_cursor(void)
{
	platform_set_sprite_enabled(0);
}

void redraw_cursor(void)
{
	platform_wait();
	platform_set_sprite_enabled(1);
}

void draw_pixel_4bpp(unsigned short x, unsigned short y)
{
	/* draw a pixel to the non rot bg, assume the tiles are set up already */
	/* use "1" as the colour */

	/* convert the x position to the x,y position to the 8*8 tile */
	/* each tile is 8 pixels wide, but these are stored in quadruples  */
	/* (pairs of 8 bit values, each 8 bit value holds 2 pixels) */
	/* therefore each 8*8 block is just 16 "reads" big. */
	/* 16 * 16 = 256 bits. We have 4 bits per pixel, so 256 bits = 64 pixels */

	/* the pixel could be either one of the 4 4bit blocks  */
	/*       1111 0000 0000 0000    F000 -> 0FFF */
	/*       0000 1111 0000 0000    0F00 -> F0FF */
	/*       0000 0000 1111 0000    00F0 -> FF0F */
	/*       0000 0000 0000 1111    000F -> FFF0 */
	/* to determine the 4 bit block, need to think about the pixel. */
	/* pixel 0 4 is in lower 4 bits   00  100     &3 == 0 */
	/* pixel 1 5 is in next 4 bits    01  101     &3 == 1 */
	/* pixel 2 6 is in next 4 bits    10  110     &3 == 2 */
	/* pixel 3 7 is in next 4 bits    11  111     &3 == 3 */

	/* tile     add           x at top left corner      y
	   1         20          0                         0
	   2         40          8                         0
	   3         60...       16                        0
	   31        3E0         0                         8
	   32        400         8                         8....

	   so to get tile from x,y... need to 1 + (y/8 * 30 + x/8)
	   */

	int tile = 1 + (x / 8) + ((y / 8) * 30);
	/* then from here it's easy (ish) just work out the remainder from /8 */

	int xrem = x & 7;
	int yrem = y & 7;

	int address =
		tile * 0x10 /*16 16-bit vals per tile */  + (xrem / 4) +
		(yrem * 2);

	uint16_t pixel = platform_get_tile_entry(address);
	pixel |= (0x000F << (4 * (x & 3)));
	platform_set_tile_entry(address, pixel);
}

unsigned char is_pixelset(unsigned short x, unsigned short y, int target,
		int start)
{
	/* check a pixel is set in the non rot bg */

	/* fix for disappearing fire/ghost/wraith */
	int chaos_tile = x / 16 + (y / 16) * 16;
	int creature = arena[0][chaos_tile];
	int dead = is_dead(chaos_tile);
	if (chaos_tile == target || chaos_tile == start ||
			creature < SPELL_KING_COBRA ||
			creature == SPELL_WRAITH || creature == SPELL_GHOST ||
			creature == SPELL_MAGIC_FIRE || dead) {
		return 0;
	}
	int tile = 1 + (x / 8) + ((y / 8) * 30);
	/* then from here it's easy (ish) just work out the remainder from /8 */
	int xrem = x & 0x7;
	int yrem = y & 0x7;

	int address =
		tile * 0x10 /*16 16-bit vals per tile */  + (xrem / 4) +
		(yrem * 2);

	uint16_t pixel = platform_get_tile_entry(address);
	return (pixel & (0x000F << (4 * (x & 3)))) != 0;
}

/**
 * mask out the bytes for a solid (justice) or inverted (highlight) image.
 */
static uint8_t solid_or_inverted(int negative, uint8_t thisVal, int mask, int orMask)
{
	/* 4 bits per pixel... */
	/* mask the lower 4 bits */
	if (negative == 0) {
		/* draw solid withno pixel detail */
		if ((thisVal & mask) != 0)
			thisVal |= orMask;
	} else {
		/* draw inversed */
		if ((thisVal & mask) != 0)
			thisVal &= ((~mask) & 0xff);
		else
			thisVal |= orMask;
	}
	return thisVal;
}

static uint8_t solid_or_inverted_twice(int negative, uint8_t thisVal, int lowMask, int hiMask)
{
	uint8_t byte = thisVal;
	byte = solid_or_inverted(negative, byte, 0x0f, lowMask);
	byte = solid_or_inverted(negative, byte, 0xf0, hiMask);
	return byte;
}

/* draw a silhoutte style gfx thing at the given location */
/* used for the justice cast */
/* passing in 1 for negative draws in the reversed "hilighted" style */
void draw_silhouette_gfx(uint8_t arena_index,
		const uint16_t *gfx, const uint16_t *map,
		uint16_t col, int palette, int negative)
{
	uint8_t x, y;
	get_yx(arena_index, &y, &x);
	x--;
	y--;

	uint16_t pNewGfx[64];

	uint16_t nTileCount = 4;	/* 4 tiles for each gfx */

	uint16_t id;
	uint16_t index = arena[2][arena_index] * nTileCount;	/* set the frame... */
	if (palette == -1) {
		palette = CHAOS_SPELLS[arena[0][arena_index]].palette;
	}

	set_palette(x, y, palette);
	if (palette == 11) {
		platform_set_palette_entry(16 * 11 + 0xf, col);
		col = 0xf;
	}
	/* need to load it in blocks of 16 for 4bpp gfx */
	int loop = 0;
	/* copy the actual graphics into the temp array */
	for (loop = 0; loop < 4; loop++) {
		/* id = <the tile id in the map file at this position> */
		id = map[index++];
		memcpy(&pNewGfx[loop * 16], &gfx[id * 16], 32);
	}

	/* replace in the temp array any non zero gfx values with the silhoutte value F */
	/* use palette 11 - the border palette, but the Fth entry isn't used */

	int lowMask = col & 0xf;
	int hiMask = (col & 0xf) << 4;

	for (loop = 0; loop < 64; loop++) {
		uint8_t byte, bytex;
		uint16_t thisVal = pNewGfx[loop];
		bytex = solid_or_inverted_twice(negative, thisVal & 0xff, lowMask, hiMask);
		byte = solid_or_inverted_twice(negative, (thisVal >> 8) & 0xff, lowMask, hiMask);
		pNewGfx[loop] = (byte << 8) | bytex;
	}

	/* each 8*8 tile takes up 32 bytes. */
	uint16_t startOAM;
	startOAM = 16 + x * 2 * 16 + 2 * y * 30 * 16;
	platform_load_tile_data(startOAM, pNewGfx, 64);
	startOAM = 16 + x * 2 * 16 + (2 * y + 1) * 30 * 16 - 32;	/* x*2*16+(2*y+1)*30*16; */
	platform_load_tile_data(32 + startOAM, &pNewGfx[32], 64);
}

int get_main_color(int creatid, int frame)
{
	/* get the main colour for this spell */
	if (creatid > SPELL_WALL || creatid < SPELL_KING_COBRA) {
		return 0;
	}
	if (frame > 3) {
		frame = 0;
	}
	int m, i, id;
	uint16_t pixelPair;

	uint16_t used[16];
	for (i = 0; i < 16; i++)
		used[i] = 0;
	/* use the frame to get the current colour.. */
	const uint16_t *gfx = CHAOS_SPELLS[creatid].pGFX;
	const uint16_t *map = CHAOS_SPELLS[creatid].pGFXMap;
	for (m = frame * 4; m < ((frame + 1) * 4); m++) {
		id = map[m];
		for (i = 0; i < 16; i++) {
			pixelPair = gfx[id * 16 + i];
			used[pixelPair & 0xf]++;
			used[(pixelPair >> 4) & 0xf]++;
			used[(pixelPair >> 8) & 0xf]++;
			used[(pixelPair >> 12) & 0xf]++;
		}
	}
	int maxUsed = 0;
	int maxIndex = 1;
	/* don't count 0, as that is transparent and most likely max used */
	for (i = 1; i < 16; i++) {
		if (used[i] > maxUsed) {
			maxIndex = i;
			maxUsed = used[i];
		}
	}
	return maxIndex;
}

void fade_up(void)
{
#if !defined(_REPLAY)

	int i;
	for (i = 16; i >= 0; i-=2) {
		platform_set_fade_level(i);
		platform_wait();
	}
#endif
	fadedup = 1;
}

void fade_down(void)
{
#if !defined(_REPLAY)
	int i;
	for (i = 0; i <= 16; i+=2) {
		platform_set_fade_level(i);
		platform_wait();
	}
#endif
	fadedup = 0;
}

void clear_bg(int text)
{
	void (*func)(int, int, int) = platform_set_map_entry;
	if (text)
		func = platform_set_text_map_entry;
	int x, y, width, height;
	int xmax = 32;
	int ymax = 24;
	platform_screen_size(&width, &height);
	if (height == 160) /* gba has x2 thick border */
		ymax = 26;
	for (x = 0; x < xmax; x++)
		for (y = 0; y < ymax; y++)
			func(x, y, 0);
}
