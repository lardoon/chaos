#include "chaos/porting.h"
#include <emscripten.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

extern void js_save(const char *key, const char *data);
extern char *js_load(const char *key);
extern int js_has_saved_game(const char *key);
extern int js_get_window_width(void);
extern int js_get_window_height(void);
extern char *js_get_lang(void);

char *s_language = 0;

const char *platform_name(void)
{
	return "Web";
}

char *platform_load_options(void)
{
	return js_load("options.cfg");
}

void platform_save_options(const char *saves, unsigned int size)
{
	js_save("options.cfg", saves);
}

int platform_has_saved_game(void)
{
	return js_has_saved_game("game.txt");
}

void platform_save_game(const char *game, unsigned int size)
{
	js_save("game.txt", game);
}

char *platform_load_game(void)
{
	return js_load("game.txt");
}

void emscripten_get_window_size(int *width, int *height, int *fullscreen)
{
	*width = js_get_window_width();
	*height = js_get_window_height();
}

const char *platform_get_lang(void)
{
	if (s_language != 0)
		return s_language;

	/* would have to reload to change language */
	char *lang = js_get_lang();
	s_language = lang;
	return s_language;
}
