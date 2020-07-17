#ifndef chaos_h_seen
#define chaos_h_seen
/* chaos.h */

#include <stdint.h>

#define CHAOS_SAVE_VERSION       1

#define SCR_SPLASH               10
#define SCR_OPTIONS              20
#define SCR_CREATE_PLAYERS       30
#define SCR_EDIT_NAME            40
#define SCR_GAME_MENU            50
#define SCR_SELECT_SPELL         60
#define SCR_EXAMINE_SPELL        70
#define SCR_EXAMINE_BOARD        80

#define SCR_CASTING              100
#define SCR_MOVEMENT             110
#define SCR_INFO_SCREEN          120
#define SCR_EXAMINE              130

/* should be last arena spell + 1 */
#define WIZARD_INDEX              43

/* defines for status_flag */
#define UNDEAD 1
#define ENGAGED 2
#define MOVELEFT 4
#define TURNOVER 8
#define ISHUMAN 16
#define SPELL_IS_ILLUSION 32
#define ISHIDDEN 64
#define ALIVE 128
struct chaos_state {
	signed char world_chaos;	/* Chaos level in the world */
	unsigned char round_count;	/* total rounds played so far */
	unsigned char dead_wizards;	/* number of dead wizards */
	unsigned char playercount;	/* number of total players 2-8 */
	unsigned char current_player;	/* current player to move/cast */
	unsigned long int current_screen;
};

extern struct chaos_state g_chaos_state;

extern const unsigned short int chaos_cols[];
extern int hilite_item;
extern signed char anim_col;
extern signed char anim_col_grad;
extern volatile uint8_t game_frames;
extern const signed char surround_table[8][2];
extern const unsigned char key_table[8];
int enable_interrupts(void);
int disable_interrupts(void);
void delay(int d);

void play_soundfx(int);

/* appends a stat to the buffer with the given name */
void append_stat(char *buffer, const char *name, int value);

/** convert g_chaos_state to a string.*/
void chaos_state_to_str(char *buffer);
/** given a string, fill out g_chaos_state. */
void str_to_chaos_state(char *data);

/** From the save data, get save version. This is the first thing in the file. */
int get_save_version(const char *data);
/** Add the VERSION = version string to the buffer. @see CHAOS_SAVE_VERSION. */
void set_save_version(char *buffer, int version);

#endif
