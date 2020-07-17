#include <gba_video.h>
#include "chaos/porting.h"

bg_scroll bg_level;

void gba_init_scroll(void)
{
	bg_level.x = 0;
	bg_level.y = 0;
}

int platform_has_scroll(void)
{
	return 1;
}

int platform_get_y_scroll(void)
{
	return bg_level.y;
}

int platform_get_x_scroll(void)
{
	return bg_level.x;
}

void platform_set_y_scroll(int yscroll)
{
	bg_level.y = yscroll;
}

void platform_set_x_scroll(int xscroll)
{
	bg_level.x = xscroll;
}

void platform_update_scroll(void)
{
	/* copy to hardware */
	BG_OFFSET[0].x = bg_level.x;
	BG_OFFSET[0].y = bg_level.y;
}
