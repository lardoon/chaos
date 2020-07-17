/* options.h */

#ifndef OPTIONS_H
#define OPTIONS_H

#define OPT_ROUND_LIMIT 0
#define OPT_OLD_BUGS 1
#define OPT_NEW_FEATURES 2
#define OPT_SOUND_ENABLED 3
#define OPT_SOUND 4
#define OPT_LANGUAGE 5

#define OPTION_COUNT 6
#define BACK_OPTION (OPTION_COUNT + 1)
#define MORE_OPTION (OPTION_COUNT + 2)

#define OPT_CHEAT 6

#define DEFAULT_ROUNDS 1
extern unsigned int Options[OPTION_COUNT + 1];
void set_default_options(void);
int show_options(void);
void options_up(void);
void options_down(void);
void options_a(void);
void options_left(void);
void options_right(void);
void options_back(void);
void options_touch(int x, int y);

#endif
