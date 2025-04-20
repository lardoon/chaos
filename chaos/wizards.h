/* wizards.h */

/* the wizard definitions */
/* the ser colour to change.. */
#ifndef WIZARDS_H
#define WIZARDS_H

#define WIZARD_COLOUR 0x1

/* shadow from is bit 3 */
#define HAS_SHADOWFORM(flag) ( (flag & 0x8) == 0x8 )
/* if bit 4 set, wizard is dead */
#define IS_WIZARD_DEAD(flag) ( (flag & 0x10) == 0x10 )
/* wings is bit 5 */
#define HAS_MAGICWINGS(flag) ( (flag & 0x20) == 0x20 )

#define GFX_MAGIC_SHIELD  8
#define GFX_MAGIC_ARMOUR  9
#define GFX_MAGIC_SWORD   10
#define GFX_MAGIC_KNIFE   11
#define GFX_MAGIC_BOW     12
#define GFX_MAGIC_WINGS   13

/* human and cpu are obvious */
#define PLYR_HUMAN        0
#define PLYR_CPU          1

#define IS_CPU(n) ((players[n].plyr_type&PLYR_CPU)>0)
extern unsigned char current_player;
typedef struct  {
	char name[12];
	unsigned char combat;
	unsigned char ranged_combat;
	unsigned char range;
	unsigned char defence;
	unsigned char movement_allowance;
	unsigned char manoeuvre_rating;
	unsigned char magic_resistance;
	unsigned char spell_count;
	unsigned char spells[42];	/* 2 byte pairs, one for the priority, the other for the spell id */
	unsigned char ability;

	/*animationspeed */
	unsigned char image;	/* this is different... */
	unsigned short colour;

	unsigned char plyr_type;
	unsigned char modifier_flag;
	unsigned char illusion_cast;
	unsigned char selected_spell;
	unsigned char last_spell;
	unsigned char timid;
	unsigned char team;
} player_data;		/* 60 bytes * 8 = 480 bytes of wizard data */
extern player_data *players;

/*
   meaning of bits in modifier flag...
   bit 0
   bit 1    magic knife
   bit 2    magic sword
   bit 3    shadow form
   bit 4    is dead
   bit 5    magic wings
   bit 6    magic shield
   bit 7    magic armour

*/

struct WIZARD_DATA  {
	const unsigned short *pGFX;
	const unsigned short *pMap;
};
extern const struct WIZARD_DATA WizardGFX[];
int creature_count(int player_id, int include_blob);
int update_creaturecount(void);

/* kills the wizard at the target_index */
/* updates flags, etc */
void kill_wizard(void);
void reset_players(void);
void init_player_table(void);

#endif				/* WIZARDS_H */

