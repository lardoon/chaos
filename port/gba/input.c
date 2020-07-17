#include "chaos/porting.h"
#include <gba_input.h>

static KEYPAD_BITS chaos_key_to_gba[] = {
	KEY_A,
	KEY_B,
	KEY_SELECT,
	KEY_START,
	KEY_RIGHT,
	KEY_LEFT,
	KEY_UP,
	KEY_DOWN,
	KEY_R,
	KEY_L,
};

static const char *s_buttons[] = {
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
	"",
};

static uint16_t s_keys_down = 0;
static uint16_t s_keys_down_rep = 0;

int platform_key_pressed(int key)
{
	if (key <= KEY_L)
		return (s_keys_down & chaos_key_to_gba[key]) != 0;
	return 0;
}

int platform_key_pressed_repeat(unsigned int key)
{
	if (key <= KEY_L) {
		return (s_keys_down_rep & chaos_key_to_gba[key]) != 0;
	}
	return 0;
}

void platform_update_keys(void)
{
	scanKeys();
	s_keys_down = keysHeld();
	s_keys_down_rep = keysDownRepeat();
}

int platform_touch_x(void)
{
	return -1;
}

int platform_touch_y(void)
{
	return -1;
}

const char **platform_get_button_names(void)
{
	return s_buttons;
}
