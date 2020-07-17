#ifndef porting_h_seen
#define porting_h_seen

#include <stdint.h>

#define UNUSED  __attribute__((unused))
#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)     __builtin_expect((x),0)

/**
 * A big list of the functions that need to be implemented to
 * port Chaos to a new platform.
 */

/**
 * Initialise platform specific things.
 * @return 0 on success, anything else for a failure.
 */
int platform_init(void);

/**
 * Get the screen size in pixels.
 * @param width the width in pixels
 * @param height the height in pixels
 * @returns the screen dimenstions in width height args.
 */
void platform_screen_size(int *width, int *height);

/**
 * Swap screen that is being drawn to.
 *@return 0 if succeeds, 1 if fails
 */
int platform_swap_screen(void);

/**
 * @return the name of the platform, for use on the title screen.
 */
const char *platform_name(void);

/**
 * Exit from the game. Called when you press back/B on splash.
 */
void platform_exit(void);

/**
 * @return non-zero if scroll is used, or 0 if there is no scrolling.
 */
int platform_has_scroll(void);

/**
 * Get the X scroll position of the bg level.
 * On platforms with no scroll, return 0.
 * @return x scroll value
 */
int platform_get_x_scroll(void);

/**
 * Get the Y scroll position of the bg level.
 * On platforms with no scroll, return 0.
 * @return y scroll value
 */
int platform_get_y_scroll(void);

/**
 * Update the x scroll.
 * @param scroll the new x scroll value
 */
void platform_set_x_scroll(int scroll);

/**
 * Update the y scroll.
 * @param scroll the new y scroll value
 */
void platform_set_y_scroll(int scroll);

/**
 * Write the scroll changes to hardware.
 */
void platform_update_scroll(void);

/**
 * Set the given palette entry.
 * @param idx the palette index to set
 * @param color 15-bit RGB color.
 */
void platform_set_palette_entry(int idx, uint16_t color);

/**
 * Get the given palette entry.
 * @param idx palette index to get
 * @return the 16-bit color at the given index
 */
uint16_t platform_get_palette_entry(int idx);

/**
 * Set the map index at the given location.
 * @param x the x pos
 * @param y the y pos
 * @param palette the palette index to set at this tile
 */
void platform_set_map_entry(int x, int y, int palette);

/**
 * Get the map data at a given location
 */
uint16_t platform_get_map_entry(int x, int y);

/**
 * Set the map index at the given location for text.
 * @param x the x pos
 * @param y the y pos
 * @param palette the palette index to set at this tile
 */
void platform_set_text_map_entry(int x, int y, int palette);

/**
 * Set the tile data at the given offset.
 * @param offset the offset into tile data
 * @param value the value of the data
 */
void platform_set_tile_entry(int offset, int value);

/**
 * Load multi-byte data chunk into tile data.
 * @param offset the offset to load at.
 * @param data the data to load
 * @param size the size of the data in bytes
 */
void platform_load_tile_data(int offset, const void *data, int size);

/**
 * Load multi-byte data chunk into tile data for text.
 * Text layer should be drawn *on top* of the other layer.
 * @param offset the offset to load at.
 * @param data the data to load
 * @param size the size of the data in bytes
 */
void platform_load_text_tile_data(int offset, const void *data, int size);

/**
 * Get the tile data at the given offset.
 * @param offset the offset into tile data
 * @param value the value of the data
 */
int platform_get_tile_entry(int offset);

/**
 * Set the fade level. Fade the screen down to black.
 * @param fade 16 for max fade (dark), 0 for min fade (normal)
 */
void platform_set_fade_level(int fade);

/**
 * Update the state of the input (poll events).
 */
void platform_update_keys(void);

/**
 */
typedef enum CHAOS_PORT_KEYS {
	CHAOS_KEY_A,
	CHAOS_KEY_B,
	CHAOS_KEY_SELECT,
	CHAOS_KEY_START,
	CHAOS_KEY_RIGHT,
	CHAOS_KEY_LEFT,
	CHAOS_KEY_UP,
	CHAOS_KEY_DOWN,
	CHAOS_KEY_R,
	CHAOS_KEY_L,
	CHAOS_KEY_TOUCH,
} CHAOS_PORT_KEYS;

/**
 * Is a key pressed. See CHAOS_PORT_KEYS for details.
 * @param key the key to get, one of CHAOS_PORT_KEYS
 * @return true if the key has been pressed, false otherwise
 */
int platform_key_pressed(int key);

/** is a key pressed with repeat.
 * @param key the key pressed
 * @return true if the key is pressed, false otherwise
 */
int platform_key_pressed_repeat(unsigned int key);

/** @return the touch screen x coordinate, or -1 if touch is not supported */
int platform_touch_x(void);

/** @return the touch screen x coordinate */
int platform_touch_y(void);

/**
 * Load 16-bit palette information to the sprite palette.
 * @param offset the offset in the palette to load to
 * @param data the data to load
 * @param size the number of 16-bit entries
 */
void platform_load_sprite_palette(int offset, const uint16_t *data, int size);

/**
 * Load a single "tile" of data into sprite memory.
 * @param offset the offset in sprite memory
 * @param data the data to load - 32 words of it.
 */
void platform_load_sprite_data(int offset, const uint16_t *data);

/**
 * Enable or disable the sprites completely.
 * @param enabled if non-zero then enable the sprites, otherwise disable
 */
void platform_set_sprite_enabled(int enabled);

/**
 * Play a sound effect..
 * @param soundid the sound index to play.
 */
void platform_play_soundfx(int soundid);

/**
 * Update the position of a sprite.
 * @param spriteid the sprite id
 * @param x the new x position
 * @param y the new y position
 */
void platform_move_sprite(int spriteid, int x, int y);

/**
 * This is the start of the whole thing. Do not implement this, instead call it
 * from the port's main() routine.
 */
void chaos_main(void);

/**
 * Execute one frame. After this wait for a vblank, draw a frame, etc.
 */
void chaos_one_frame(void);

/**
 * Debug printing output. Sometimes usefull.
 * @param s the text to print.
 */
void platform_dprint(const char *s);

/**
 * A "wait" is called after the return value number of pixels is drawn
 * in the line routine. 42 is a good value.
 * @return a fudge factor for refreshing the screen.
 */
int platform_line_fudge_factor(void);

/**
 * wait vsync
 */
void platform_wait(void);

/**
 * Get the names of the buttons.
 *@return the names of the buttons.
 */
const char **platform_get_button_names(void);

/**
 * Load the options data.
 * @return a char array with the text data for the options, this will be free'd
 * by the main code.
 */
char *platform_load_options(void);

/**
 * Save the options data.
 * @param saves the data to save to some safe place, NULL terminated.
 * @param size the size of the array, just in case.
 */
void platform_save_options(const char *saves, unsigned int size);

/**
 * load the game data.
 * @return a char array with the text data for the game, this will be free'd
 */
char *platform_load_game(void);

/**
 * Is there a saved game available to load.
 * @return 1 if there is a save game, 0 otherwise.
 */
int platform_has_saved_game(void);

/**
 * Save the game data.
 * @param game the data to save to some safe place, NULL terminated.
 * @param size the size of the array, just in case.
 */
void platform_save_game(const char *game, unsigned int size);

/**
 * Show more extended options. For Android.
 */
void platform_more_options(void);

/** get the language */
const char *platform_get_lang(void);

#endif
