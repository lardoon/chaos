#include "chaos/platform.h"
#include "chaos/porting.h"
#include "chaos/chaos.h"
#include "chaos/wizards.h"
#include "chaos/splash.h"

#include <stdlib.h>
#include <SDL.h>
#include "port/linux/winkeys.h"
#include "port/linux/wmhandler.h"
/* "emulates" the keypad */
#define KEY_COUNT       11
int g_winkeys[KEY_COUNT] = {0};
int g_stress_mode = 0;
static int s_mouse_x = 0;
static int s_mouse_y = 0;
int g_last_winkeys[KEY_COUNT] = {0};
static const SDLKey s_keyCodes[KEY_COUNT] = {
	SDLK_s,		/* KEY_A */
	SDLK_BACKSPACE,		/* KEY_B */
	SDLK_n,	/* KEY_SELECT */
	SDLK_RETURN,	/* KEY_START  */
	SDLK_d,	/* KEY_RIGHT  */
	SDLK_a,	/* KEY_LEFT   */
	SDLK_w,	/* KEY_UP     */
	SDLK_x,	/* KEY_DOWN   */
	SDLK_i,		/* KEY_R */
	SDLK_k,		/* KEY_L */
	SDLK_UNDO	/* KEY_TOUCH */
};

static const SDLKey s_keyCodesAlt[KEY_COUNT] = {
	SDLK_SPACE,		/* KEY_A */
	SDLK_BACKSPACE,		/* KEY_B */
	SDLK_n,	/* KEY_SELECT */
	SDLK_RETURN,	/* KEY_START  */
	SDLK_RIGHT,	/* KEY_RIGHT  */
	SDLK_LEFT,	/* KEY_LEFT   */
	SDLK_UP,	/* KEY_UP     */
	SDLK_DOWN,	/* KEY_DOWN   */
	SDLK_i,		/* KEY_R */
	SDLK_k,		/* KEY_L */
	SDLK_UNDO	/* KEY_TOUCH */
};

static void winKeyEvent(SDL_KeyboardEvent * event)
{
	int i;
	if (event->type == SDL_KEYDOWN) {
		/* set in key array   */
		for (i = 0; i < KEY_COUNT; i++) {
			if (s_keyCodes[i] == event->keysym.sym ||
			    s_keyCodesAlt[i] == event->keysym.sym) {
				g_winkeys[i] = 1;
			} else {
				switch (event->keysym.sym) {
				case SDLK_e:
					g_winkeys[4] = g_winkeys[6] = 1;
					break;
				case SDLK_q:
					g_winkeys[5] = g_winkeys[6] = 1;
					break;
				case SDLK_z:
					g_winkeys[5] = g_winkeys[7] = 1;
					break;
				case SDLK_c:
					g_winkeys[4] = g_winkeys[7] = 1;
					break;
				default:
					break;
				}
			}
		}
	} else if (event->type == SDL_KEYUP) {
		/* unset in key array */
		for (i = 0; i < KEY_COUNT; i++) {
			if (s_keyCodes[i] == event->keysym.sym ||
			    s_keyCodesAlt[i] == event->keysym.sym) {
				g_winkeys[i] = 0;
			} else {
				switch (event->keysym.sym) {
				case SDLK_e:
					g_winkeys[4] = g_winkeys[6] = 0;
					break;
				case SDLK_q:
					g_winkeys[5] = g_winkeys[6] = 0;
					break;
				case SDLK_z:
					g_winkeys[5] = g_winkeys[7] = 0;
					break;
				case SDLK_c:
					g_winkeys[4] = g_winkeys[7] = 0;
					break;
				default:
					break;
				}
			}
		}
	}
}

void sdl_init_keys(void)
{
	int i;
	for (i = 0; i < KEY_COUNT; i++) {
		g_winkeys[i] = 0;
		g_last_winkeys[i] = 0;
	}
}

extern void update_players(void);
extern void create_players_start(void);

static void stress_mode(void)
{
	int cs = g_chaos_state.current_screen;
	if (cs == SCR_SPLASH) {
		splash_start();
		splash_start();
	} else if (cs == SCR_CREATE_PLAYERS) {
		players[0].plyr_type = PLYR_CPU | (4 << 4);
		g_chaos_state.playercount = 8;
		create_players_start();
	} else {
		g_winkeys[10] = 0;
	}
}

void platform_update_keys(void)
{
	SDL_Event event;
	int i;
	for (i = 0; i < KEY_COUNT; i++) {
		g_last_winkeys[i] = g_winkeys[i];
	}

	/* Check for events */
	int do_exit = 0;
	g_winkeys[10] = 0;
	s_mouse_x = -1;
	s_mouse_y = -1;
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
			case SDL_KEYUP:

				/* handle window manager keys... */
				if ((event.key.keysym.sym == SDLK_F4
					&& SDL_GetModState() == KMOD_LALT)
					||event.key.keysym.sym == SDLK_ESCAPE ) {
					do_exit = 1;
				}
				if (event.key.keysym.sym == SDLK_F5) {
					toggleFullScreen();
				}
				if (event.key.keysym.sym >= SDLK_1
						&&event.key.keysym.sym <= SDLK_4) {
					setNewScale(event.key.keysym.sym -
							SDLK_1 + 1);
				}

				/* FALL THROUGH */
			case SDL_KEYDOWN:
				winKeyEvent(&event.key);
				break;
			case SDL_QUIT:
				do_exit = 1;
				break;
			case SDL_VIDEORESIZE:
				do_resize(&event. resize);
				break;
			case SDL_MOUSEBUTTONDOWN:
				g_winkeys[10] = 1;
				s_mouse_x = event.button.x;
				s_mouse_y = event.button.y;
				break;
			case SDL_MOUSEBUTTONUP:
				break;
			default:
				break;
		}
	}

	if (!do_exit && g_stress_mode) {
		stress_mode();
		return;
	}

	/* a hacky way to exit this, but better than nothing */
	if (do_exit) {
		if (isFullScreen) {
			toggleFullScreen();
		} else {
			exit(0);
		}
	}
}

int platform_key_pressed(int key)
{
	if ((key <= CHAOS_KEY_TOUCH))
		return (g_winkeys[key] != 0);
	return 0;
}

uint32_t repeated[KEY_COUNT];
int platform_key_pressed_repeat(unsigned int key)
{
	if (platform_key_pressed(key)) {

		/* if the recent poll has the key pressed */
		if (!g_last_winkeys[key] || (g_last_winkeys[key] && 50 < repeated[key])) {
			/* and it wasn't pressed last time.. */
			repeated[key] = 0;
			return 1;
		} else if (g_last_winkeys[key]) {
			repeated[key]++;
		}
	}
	return 0;
}

extern int gfx_scale;
int platform_touch_x(void)
{
	return s_mouse_x / gfx_scale;
}

int platform_touch_y(void)
{
	return s_mouse_y / gfx_scale;
}
