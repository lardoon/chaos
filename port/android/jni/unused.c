#include "chaos/porting.h"

int platform_init(void)
{
	return 0;
}

int platform_has_scroll(void)
{
	return 0;
}

int platform_get_x_scroll(void)
{
	return 0;
}

int platform_get_y_scroll(void)
{
	return 0;
}


void platform_set_x_scroll(int scroll UNUSED) { }

void platform_set_y_scroll(int scroll UNUSED) { }

void platform_update_scroll(void) { }
