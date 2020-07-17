/* movement.h */

#ifndef MOVEMENT_H
#define MOVEMENT_H
extern int g_highlight_creations;
extern unsigned char selected_creature;
extern unsigned char tmp_is_flying;
extern unsigned char tmp_engaged_flag;
extern unsigned char tmp_movement_allowance;
extern unsigned char tmp_range_attack;
void start_movement_round(void);

int print_end_touch_message(int clear);
void movement_l(void);
void movement_a(void);
void do_movement_accept(void);
void movement_b(void);
void movement_start(void);
void movement_select(void);
void movement_touch(int x, int y);
void win_contest(void);
void quit_game(void);

#endif

