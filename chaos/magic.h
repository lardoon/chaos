#ifndef MAGIC_H
#define MAGIC_H
#include <stdint.h>

void cast_disbelieve(void);
void cast_meditate(void);
void cast_creature(void);
void cast_chaos_law(void);
void cast_trees_castles(void);
void cast_fire_goo(void);
void cast_wall(void);
void cast_magic_missile(void);
void cast_justice(void);
void cast_raise_dead(void);
void cast_subversion(void);
void cast_magic_shield(void);
void cast_magic_armour(void);
void cast_magic_sword(void);
void cast_magic_knife(void);
void cast_magic_bow(void);
void cast_magic_wings(void);
void cast_shadow_form(void);
void cast_turmoil(void);

/*void cast_teleport(void); */
void increase_world_chaos(void);
void do_wall_cast(void);
void do_magic_missile(void);
void do_justice_cast(void);
void do_raisedead_cast(void);
void do_turmoil_cast(void);

/* event callback for ending creature spells */
void do_creature_spell_if_ok(intptr_t arg);
#endif
