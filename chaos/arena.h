/* arena.h  */
/* the arena is stored in IWRAM and holds references to creatures */

#ifndef ARENA_H
#define ARENA_H
#include <stdint.h>

#define MESSAGE_X   0
extern int g_message_y;

/* check bit 7 for movement/spreading */
#define HAS_MOVED(n) ((n & 0x80) == 0x80)
#define HAS_SPREAD(n) ((n & 0x80) == 0x80)
#define ARENA_SIZE 0x9f
extern unsigned char **arena;

/*
   arena 0 - the creature index
   arena 1 - the timer square - when this reaches 0, update the frame in arena 2
   arena 2 - the anim frame, 4 is dead.
   arena 3 - modifier flag - bits are important
   arena 4 - the creature index for creatures under the thing in arena 0
   arena 5 - modifier flag for things in arena 4

   arena 3 bits
   bit 0    |
   bit 1    |    the owner (or player id for wizards)
   bit 2    |
   bit 3    is asleep
   bit 4    is illusionary (i.e. bit 4 1 = illusion, bit 4 0 = real)
   bit 5    has had disbelieve cast on it
   bit 6    undead
   bit 7    "has been spread" for gooey blob and fire

*/
extern unsigned char wizard_index;	/* index into arena of current player */
extern unsigned char start_index;	/* index into arena for start square of current spell */
extern unsigned char target_index;	/* index into arena for target square of current spell */
int get_owner(int square);
int is_dead(int square);
int is_asleep(int square);
int is_blind(int square);
void move_cursor_left(void);
void move_cursor_right(void);
void move_cursor_up(void);
void move_cursor_down(void);
void countdown_anim(void);
void animate_arena(void);
void clear_message(void);
void get_yx(uint16_t arena_index, uint8_t * H, uint8_t * L);
void get_yx2(uint16_t arena_index, uint8_t * H, uint8_t * L);
void get_yx_upper(unsigned char arena_index, unsigned char *H,
		unsigned char *L);

/* get distance between 2 x,y arena coords */
void get_chaosdistance(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint16_t * diff);

/* get distance between 2 arena indices */
void get_distance(uint16_t square1, uint16_t square2, uint16_t * distance);

/* return a new index based on the old one by applying a surround index value */
uint8_t apply_position_modifier(uint8_t square, uint8_t i);
void get_cursor_pos(unsigned char *x, unsigned char *y);
void move_cursor_to(uint32_t x, uint32_t y);
/* just calls move_screen_to with an int... */
void move_screen_to(int index);
void set_current_player_index(void);
void unset_moved_flags(void);
void destroy_all_creatures(unsigned char playerid);

/* random castle crushing... */
void destroy_castles(void);

/* get a new spell from the wood */
void random_new_spell(void);
void reset_arena_tables(void);
void highlight_players_stuff(unsigned char playerid);
void display_arena(void);

void set_cursor_position(uint32_t x, uint32_t y, int32_t bgx, int32_t bgy);

void display_cursor_contents(uint8_t index);
void init_arena_tables(void);
void init_arena_table(void);
void update_sleepers(void);
/** really does the sleep or blind spell */
void sleep_blind(void);
/** annoy meditating wizards */
void freeze_meditating_wizards(void);

/** Get the player at the given index
 * fixes a bug if the player there is > player 8. What a hack */
int get_player_at(int idx);

void arena_to_str(int layer, char *arenastr);
int str_to_arena(int layer, const char *data);
/* returns 0 on success, anything else on error */
int load_arenas(const char *arenastr);

#endif				/* ARENA_H */

