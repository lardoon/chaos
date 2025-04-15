#include <SDL.h>
#include "chaos/porting.h"
#include "chaos/splash.h"
#include "chaos/chaos.h"

extern char *g_load_file;
#ifdef _HEADLESS
extern int game_running;
#endif

const char *platform_name(void)
{
	return "SDL";
}

void emscripten_get_window_size(int *width, int *height, int *fullscreen)
{
	*width = 0;
	*height = 0;
	*fullscreen = 0;
}

/* emulate emscripten_set_main_loop for regular SDL... */
void emscripten_set_main_loop(void (*func)(), int fps, int simulate_infinite_loop UNUSED)
{
	int rate = 20;
	if (fps != 0) {
		rate = 1000 / fps;
	}
	int doload = g_load_file != NULL;
#ifdef _HEADLESS
	while(game_running) {
#else
	while (1) {
#endif
		Uint32 ticksbefore = SDL_GetTicks();
		func();
		/* ensure that if -l option is given, it loads right away */
		if (doload) {
			splash_start();
			splash_down();
			splash_start();
			doload = 0;
		}
		Uint32 ticksafter = SDL_GetTicks();
		/* want 50 fps, so delay for 20 ms max */
		int delay = rate - (ticksafter - ticksbefore);
		if (delay > 0) {
			SDL_Delay(delay);
		}
	}
}

void js_scale_height(void) { }

const char *platform_get_lang(void)
{
	return getenv("LANG");
}
