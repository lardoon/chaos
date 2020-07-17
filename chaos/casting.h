/* casting.h */

#ifndef CASTING_H
#define CASTING_H
extern unsigned char current_spell;
extern signed char current_spell_chance;
extern unsigned char temp_illusion_flag;
extern unsigned char temp_cast_amount;
extern unsigned char temp_success_flag;
void start_cast_round(void);

/*void casting_up(void); */
/*void casting_down(void); */
/*void casting_left(void); */
/*void casting_right(void); */
void casting_a(void);
void casting_b(void);
void casting_r(void);
void set_current_spell_chance(void);
void print_success_status(void);
void print_name_spell(void);
unsigned char is_spell_in_range(unsigned char index1,
		unsigned char index2,
		unsigned char range);
unsigned char player_cast_ok(void);
void spell_animation(void);
void splat_animation(void);
void pop_animation(void);
void set_spell_success(void);
int los_blocked(unsigned char index);
int draw_line(int index, int gfx);
int is_wall_adjacent(int index);
int is_tree_adjacent(int index);
void ai_cast_turmoil(void);
void casting_touch(int x, int y);

#endif				/* CASTING_H */

