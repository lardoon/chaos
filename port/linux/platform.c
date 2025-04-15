#include <SDL.h>
#include <stdlib.h>
#include <stdio.h>           /* for printf*/
#include "chaos/porting.h"
#include "port/linux/screen.h"

#ifdef EMSCRIPTEN
void emscripten_sleep(unsigned int ms);
#else
#define emscripten_sleep SDL_Delay
#endif

static const char *s_buttons[] = {
	"ACCEPT",
	"BACK",
	"SELECT",
	"RETURN",
	"RIGHT",
	"LEFT",
	"UP",
	"DOWN",
	"I",
	"K",
	"CLICK",
};

void platform_dprint(const char *s)
{
	printf("%s\n", s);
}

int platform_line_fudge_factor(void)
{
	return 42;
}

const char **platform_get_button_names(void)
{
	return s_buttons;
}

void platform_exit(void)
{
	exit(0);
}

void platform_more_options(void) { }

void platform_wait(void)
{
#ifndef _HEADLESS
	emscripten_sleep(20);
#endif
	sdl_update_screen();
}
