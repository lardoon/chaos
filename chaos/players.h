/* players.h */

#ifndef PLAYERS_H
#define PLAYERS_H
int show_create_players(void);
int redraw_create_players(void);
void create_players_up(void);
void create_players_down(void);
void create_players_left(void);
void create_players_right(void);
void create_players_l(void);
void create_players_r(void);
void create_players_accept(void);
void create_players_start(void);
void create_players_touch(int x, int y);
void create_players_back(void);
void animate_player_screen(void);
unsigned char get_next_human(unsigned char id);
void new_random_spell(intptr_t arg, int index);
void init_players(void);
void create_default_wizs(void);

void players_to_str(char *buffer);
void parse_players(char *plystr);

#endif				/* PLAYERS_H */

