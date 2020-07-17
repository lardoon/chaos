/* creature.h */
#ifndef CREATURE_H
#define CREATURE_H
/* bit 6 is undead flag */
#define IS_UNDEAD(n) ((n & BIT06) == BIT06)
/* bit 4 is illusion */
#define IS_ILLUSION(n) ((n & BIT04) == BIT04)
/* bit 4 is asleep */
#define IS_ASLEEP(n) ((n & BIT03) == BIT03)

#include "chaos/spellenums.h"
#define IS_FLYING_MOUNT(c) ( (c) >= SPELL_PEGASUS && (c) <= SPELL_MANTICORE)

void creature_spell_succeeds(unsigned char target);
int draw_all_creatures(void);
void spread_fire_blob(void);
void clear_arena(void);
void liberate_from_blob(int idx);
/**
 * Invalidate the drawn-creature cache
 */
void invalidate_cache(void);

#endif				/* CREATURE_H */
