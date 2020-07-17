#include <SDL.h>
#include <stdio.h>           /* for printf */
#include "chaos/platform.h"
#include "port/linux/wmhandler.h"
#include "chaos/gfx.h"

extern int gfx_scale;
extern SDL_Surface *screen;
int init_screen(int scale);
void do_resize(SDL_ResizeEvent * event)
{
	/* screen resized... hmmm.. */
	int newScaleX = event->w / (8 * WIN_SCRN_X);
	int newScaleY = event->h / (8 * WIN_SCRN_Y);
	int newScale = newScaleX > newScaleY ? newScaleX : newScaleY;
	setNewScale(newScale);
}

/*
 * Make a deep copy of the surface and pixel data
 */
SDL_Surface *copy_surface(SDL_Surface * src)
{
	SDL_Surface *oldSurface = SDL_CreateRGBSurface(SDL_SWSURFACE,
			src->w, src->h,
			16,
			src->format->Rmask,
			src->format->Gmask,
			src->format->Bmask,
			src->format->Amask);
	/* copy pixels from old screen to copy... */
	SDL_BlitSurface(src, NULL, oldSurface, NULL);
	return oldSurface;
}

/*
 * Copy from dest to src surfaces, applying the scale factor Screens must be a
 * properly scaled size, cos I don't check anything.
 */
void scale_surface(SDL_Surface * dest, int dest_scale,
		SDL_Surface * src, int src_scale)
{
	/*  */
	if (dest_scale == src_scale) {
		SDL_BlitSurface(src, NULL, dest, NULL);
	} else {
		/* not the same scales. */
		SDL_Rect r;
		int x, y;
		r.w = dest_scale;
		r.h = dest_scale;
		int width = dest->w / dest_scale;
		int height = dest->h / dest_scale;
		Uint32 c;
		/* this is a bit hacky, but works? */
		Uint16 *pixels = (Uint16 *) src->pixels;
		for (x = 0; x < width; x++) {
			for (y = 0; y < height; y++) {
				r.x = x * dest_scale;
				r.y = y * dest_scale;
				c = pixels[x * src_scale +
					y * src_scale * (width *
							src_scale)];
				SDL_FillRect(dest, &r, c);
			}
		}


	}
}

void setNewScale(int newScale)
{
	int oldScale = gfx_scale;
	if (newScale >= 1 && newScale != gfx_scale) {
		gfx_scale = newScale;
	}
	int flags = SDL_SWSURFACE /*| SDL_DOUBLEBUF */ ;

	if (isFullScreen) {
		flags |= SDL_FULLSCREEN;
	} else {
		flags |= SDL_RESIZABLE;
	}
	int isOK = SDL_VideoModeOK(gfx_scale * WIN_SCRN_X * 8,
			gfx_scale * WIN_SCRN_Y * 8,
			16,
			flags);
	if (!isOK) {
		printf("Couldn't set 16 bit video mode: %s\n",
				SDL_GetError());
		gfx_scale = oldScale;
	}
	SDL_Surface *oldSurface = copy_surface(screen);
	init_screen(gfx_scale);
	scale_surface(screen, gfx_scale, oldSurface, oldScale);
	SDL_FreeSurface(oldSurface);

}

void toggleFullScreen(void)
{
	int flags = SDL_SWSURFACE /*| SDL_DOUBLEBUF */ ;
	isFullScreen = !isFullScreen;
	if (isFullScreen) {
		flags |= SDL_FULLSCREEN;
	} else {
		flags |= SDL_RESIZABLE;
	}
	SDL_ShowCursor(!isFullScreen);
	/* backup the original pixel data: */
	SDL_Surface *oldSurface = copy_surface(screen);
	SDL_FreeSurface(screen);
	screen = SDL_SetVideoMode(gfx_scale * WIN_SCRN_X * 8,
			gfx_scale * WIN_SCRN_Y * 8, 16, flags);
	/* copy the original data to the screen */
	scale_surface(screen, gfx_scale, oldSurface, gfx_scale);
	SDL_FreeSurface(oldSurface);
}
