#ifndef gamemenu_h_seen
#define gamemenu_h_seen
int show_game_menu(void);
void show_game_board(void);
void continue_game(void);
void game_menu_up(void);
void game_menu_down(void);
void game_menu_a(void);
void game_menu_touch(int x, int y);

void save_game(int screen);
#endif
