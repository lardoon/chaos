#include <nds.h>
#include "chaos/porting.h"

static int keys_held = 0;
static int keys_held_rep = 0;
void platform_update_keys(void)
{
	scanKeys();
	keys_held = keysCurrent();
	keys_held_rep = keysDownRepeat();
}

static KEYPAD_BITS chaos_key_to_nds[] = {
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
	KEY_TOUCH,
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
	"TAP",
};

int platform_key_pressed(int key)
{
	return (keys_held & chaos_key_to_nds[key]) != 0;
}

int platform_key_pressed_repeat(unsigned int key)
{
	return (keys_held_rep & chaos_key_to_nds[key]) != 0;
}

int platform_touch_x(void)
{
	touchPosition data;
	touchRead(&data);
	return data.px;
}

int platform_touch_y(void)
{
	touchPosition data;
	touchRead(&data);
	return data.py;
}

const char **platform_get_button_names(void)
{
	return s_buttons;
}
