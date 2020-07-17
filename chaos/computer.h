/* computer.h */

#ifndef CPU_H
#define CPU_H

#include <stdint.h>
extern unsigned char target_square_found;
extern unsigned char move_table_created;
extern unsigned char has_wizard_moved;
extern unsigned char flying_target_found;
extern unsigned short LUT_index;
void do_ai_spell(void);
void do_ai_movement(void);
void ai_cast_disbelieve(void);
void ai_cast_creature(void);
void ai_cast_magic_missile(void);
void ai_cast_subversion(void);
void ai_cast_wall(void);
void ai_cast_justice(void);
void ai_cast_raisedead(void);
void ai_cast_meditate(void);
int create_range_table(int target, int range);
int get_best_index(void);
void create_all_enemies_table(void);
void order_table(int count, uint8_t * table);

/* check if the cpu could dismount */
int cpu_dismount(void);

#endif
