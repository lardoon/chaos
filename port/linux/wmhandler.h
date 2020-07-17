#ifndef wmhandler_h_seen
#define wmhandler_h_seen

#include <SDL/SDL_events.h>

#define WIN_SCRN_X  32
#define WIN_SCRN_Y  24
#ifdef EMSCRIPTEN
#define DEFAULT_SCALE 2
#else
#define DEFAULT_SCALE 1
#endif

enum ScreenType {
	NORMAL,
	FULL_SCREEN,
	HEADLESS
};

extern enum ScreenType isFullScreen;
void do_resize(SDL_ResizeEvent * event);
void toggleFullScreen(void);
void setNewScale(int newScale);

#endif
