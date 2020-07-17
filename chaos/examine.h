/* examine.h */

#ifndef EXAMINE_H
#define EXAMINE_H
int examine_board(void);
void examine_back(void);

/* examine creature at this square,  */
/* when done retrun to board */
void examine_square(int index);

/* examine spell at this square */
/* when done, does nothing */
void examine_spell(unsigned char index);

/* touch screen controls */
void examine_board_touch(int x, int y);

void clear_top_screen(void);

void print_description(const char *description, int x, int y);

#endif

