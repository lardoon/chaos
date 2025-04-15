#include <SDL.h>
#include <SDL/SDL_thread.h>
#include <SDL/SDL_mutex.h>
#include <sys/time.h>
#include "chaos/porting.h"
#include "chaos/rand.h"
#include "port/linux/cmdline.h"
#include "port/linux/screen.h"

extern void emscripten_set_main_loop(void (*func)(), int fps, int simulate_infinite_loop);
extern SDL_cond *g_vsync_cond;
extern SDL_mutex *g_vsync_mutex;
extern int g_stress_mode;
int g_debug_mode;

#ifdef _HEADLESS
int game_running = 1;
#endif

#ifdef EMSCRIPTEN
#include <stdio.h>
Uint32 s_last_time = 0;
int s_counter = 0;
int s_total = 0;
#endif
static void one()
{
#ifdef EMSCRIPTEN
	Uint32 ticks = SDL_GetTicks();
	int taken = ticks - s_last_time;
	s_total += taken;
	s_counter++;
	if (s_counter == 100) {
		printf("taken %d\n", s_total / s_counter);
		s_total = 0;
		s_counter = 0;
	}
	s_last_time = ticks;
#endif
	chaos_one_frame();
	sdl_update_screen();
}

int main(int argc, char *argv[])
{
	if (parse_opts(argc, argv)) {
		/* some weird option that we don't support. */
		return 122;
	}
	if (g_stress_mode) {
		struct timeval tv;
		gettimeofday(&tv, NULL);
		uint32_t val = (tv.tv_usec << 8) | (tv.tv_sec & 0xff);
		setSeed(val);
	}
	chaos_main();
#ifdef EMSCRIPTEN
	s_last_time = SDL_GetTicks();
#endif
	emscripten_set_main_loop(one, 50, 0);
	return 0;
}
