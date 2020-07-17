#ifndef touch_h_seen
#define touch_h_seen

/* returns the 8x8 pixel square in which the x, y touch
 * event occurred */
void pixel_xy_to_square_xy(int x, int y, int *sqx, int *sqy);
#endif
