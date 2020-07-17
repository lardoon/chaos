#include "chaos/touch.h"

void pixel_xy_to_square_xy(int x, int y, int *sqx, int *sqy)
{
	*sqx = -1;
	*sqy = -1;
	if (x < 0 || y < 0)
		return;
	*sqx = x / 8;
	*sqy = y / 8;
}
