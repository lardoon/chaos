#ifndef GFX_H
#define GFX_H

#include <stdint.h>

extern unsigned char cursor_x;
extern unsigned char cursor_y;
extern unsigned char fadedup;
#define PALETTE(p) ((p) << 12)
#define UPDATE_PALETTE(v, p) ((v & ~0xF000) | (PALETTE(p) & 0xF000))

/* a silly way to draw graphics... */
/* can offset them by 1 by doing x*2 +1, etc */
/* used for the wizard deaths... */
void draw_gfx8(const uint16_t *gfx, const uint16_t *map, uint8_t x, uint8_t y, uint8_t frame);
/* similar for paletes.. */
void set_palette8(unsigned short x, unsigned short y,
		unsigned char palette);

void draw_creature(unsigned char x, unsigned char y, unsigned char i,
		unsigned char f);
void draw_wizard(unsigned long x, unsigned long y, unsigned long wizardid,
		unsigned long frame, unsigned long playerid);
/* a hacky test for now, tidy up later */
void draw_wizard8(unsigned long x, unsigned long y, unsigned long wizardid,
		unsigned long frame, unsigned long playerid);

void clear_square(unsigned char x, unsigned char y);

void set_player_col(unsigned char playerid, unsigned short colour);

void set_border_col(unsigned char playerid);

void load_bg_palette(uint8_t topal, uint8_t frompal);
void load_all_palettes(void);
void load_sprite_palette(void);

void draw_game_border(void);
void clear_game_border(void);
void draw_decor_border(unsigned char pal, unsigned short col1,
		unsigned short col2);

void scroll_arena_up(void);
void scroll_arena_down(void);
void scroll_arena_left(void);
void scroll_arena_right(void);

void fg_set_colour(unsigned char col);

int get_main_color(int creatid, int frame);

void draw_cursor(uint32_t cursorid);

void clear_palettes(void);
void clear_palette(unsigned char pal);

void anim_selection(uint8_t pal, uint8_t red, uint8_t green, uint8_t blue);
void draw_fight_frame(uint8_t x, uint8_t y, uint8_t frame);

void remove_cursor(void);
void redraw_cursor(void);
/*void set_cursor_position(uint32_t x, uint32_t y); */

/* check pixel set... */
unsigned char is_pixelset(unsigned short x, unsigned short y, int, int);
void draw_pixel_4bpp(unsigned short x, unsigned short y);

void draw_spellcast_frame(uint8_t x, uint8_t y, uint8_t frame);
void draw_splat_frame(uint8_t x, uint8_t y, uint8_t frame);
void draw_splat_frame_2(int idx);
void draw_pop_frame(uint8_t x, uint8_t y, uint8_t frame);
void draw_pop_frame_2(int idx);
void draw_breath_frame(uint8_t x, uint8_t y, uint8_t frame);

void draw_silhouette_gfx(uint8_t arena_index, const uint16_t * gfx, const uint16_t * map,
		uint16_t col, int palette, int negative);

int fading_up(void);
int fading_down(void);
void fade_up(void);
void fade_down(void);
void clear_bg(int text);

#endif
