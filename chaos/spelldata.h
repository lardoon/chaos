#ifndef spelldata_h_seen
#define spelldata_h_seen
#include "chaos/spellenums.h"

struct spell_data {
	const char *spellName;
	const signed char chaosRating;
	const unsigned char castChance;
	const unsigned char castRange;
	const unsigned char castPriority;
	const unsigned char palette;
	const unsigned char combat;
	const unsigned char rangedCombat;
	const unsigned char rangedCombatRange;
	const unsigned char defence;
	const unsigned char movement;
	const unsigned char manvr;
	const unsigned char magicRes;
	const unsigned char frequency;
	/* pointer to the spell casting function */
	void (*pFunc) (void);
	/* description for non-creature spells */
	const char *description;
	const unsigned char flags;
	/* pointer to the graphics array (extern u8 _binary_GFX_raw_start[];) */
	const unsigned short *pGFX;
	/* pointer to the map file that makes up the graphics (extern u8 _binary_GFX_map_start[];) */
	const unsigned short *pGFXMap;
};

extern const struct spell_data CHAOS_SPELLS[];
#define NEW_FEATURE 1
#endif
