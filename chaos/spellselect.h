/* spellselect.h */

#ifndef SPELLSELECT_H
#define SPELLSELECT_H
extern char cast_chance_needed;
extern int g_examining_only;
int show_spell_screen(void);
void spell_select_up(void);
void spell_select_down(void);
void spell_select_left(void);
void spell_select_right(void);
void spell_select_a(void);
void spell_select_b(void);
void spell_select_r(void);
void spell_select_touch(int x, int y);
void anim_spell_select(void);
void remove_null_spells(void);
void get_yes_no(const char *question, int x, int y, int pal,
		int main_col, int not_selected_col,
		void (*yes_cb)(void),
		void (*no_cb)(void));

#endif

