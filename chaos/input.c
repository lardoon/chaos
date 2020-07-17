#include "chaos/platform.h"
#include "chaos/porting.h"

#include "chaos/input.h"
#include "chaos/gamemenu.h"
#include "chaos/arena.h"
#include "chaos/wizards.h"
#include "chaos/gfx.h"
#include "chaos/examine.h"
#include "chaos/options.h"
#include "chaos/editname.h"
#include "chaos/movement.h"
#include "chaos/splash.h"
#include "chaos/players.h"
#include "chaos/spellselect.h"
#include "chaos/casting.h"

/* also need to include all the screens,  */
/* as we use calls to the specific up, down, etc here */
#include "chaos/chaos.h"

#define CVK_RIGHT       0
#define CVK_LEFT        1
#define CVK_UP          2
#define CVK_DOWN        3
#define CVK_A           4
#define CVK_B           5
#define CVK_L           6
#define CVK_R           7
#define CVK_TOUCH       8

#define CVK_COUNT       9

/*#define CVK_MAX              128*/
#define CVK_KEY_REPEAT_DEFAULT       20

static unsigned char virtual_keys[CVK_COUNT] = { 0 };
static const int key_map[] = {
	CHAOS_KEY_RIGHT,
	CHAOS_KEY_LEFT,
	CHAOS_KEY_UP,
	CHAOS_KEY_DOWN,
	CHAOS_KEY_A,
	CHAOS_KEY_B,
	CHAOS_KEY_L,
	CHAOS_KEY_R,
	CHAOS_KEY_TOUCH,
};

static void get_key_presses(void)
{
	platform_update_keys();
	int i;
	for (i = 0; i < CVK_COUNT; i++) {
		if (platform_key_pressed(key_map[i])) {
			virtual_keys[i]++;
			if (virtual_keys[i] > 128)
				virtual_keys[i] = 128;
		} else {
			virtual_keys[i] = 0;
		}
	}
}

static void up_pressed(void)
{
	switch (g_chaos_state.current_screen) {
		case SCR_SPLASH:
			splash_up();
			break;
		case SCR_EDIT_NAME:
			edit_name_up();
			break;
		case SCR_CREATE_PLAYERS:
			create_players_up();
			break;
		case SCR_GAME_MENU:
			game_menu_up();
			break;
		case SCR_SELECT_SPELL:
			spell_select_up();
			break;
		case SCR_OPTIONS:
			options_up();
			break;

		case SCR_CASTING:
		case SCR_MOVEMENT:
		case SCR_EXAMINE_BOARD:
			move_cursor_up();
			break;
	}
}

static void down_pressed(void)
{
	switch (g_chaos_state.current_screen) {
		case SCR_SPLASH:
			splash_down();
			break;
		case SCR_EDIT_NAME:
			edit_name_down();
			break;
		case SCR_SELECT_SPELL:
			spell_select_down();
			break;
		case SCR_CREATE_PLAYERS:
			create_players_down();
			break;
		case SCR_GAME_MENU:
			game_menu_down();
			break;

		case SCR_OPTIONS:
			options_down();
			break;

		case SCR_CASTING:
		case SCR_MOVEMENT:
		case SCR_EXAMINE_BOARD:
			move_cursor_down();
			break;

	}
}

static void left_pressed(void)
{
	switch (g_chaos_state.current_screen) {
		/*    case SCR_GAME_BOARD: move_cursor_left(); break;   */
		case SCR_SPLASH:
			splash_left();
			break;
		case SCR_CREATE_PLAYERS:
			create_players_left();
			break;
		case SCR_EDIT_NAME:
			edit_name_left();
			break;
		case SCR_SELECT_SPELL:
			spell_select_left();
			break;

		case SCR_OPTIONS:
			options_left();
			break;

		case SCR_CASTING:
		case SCR_MOVEMENT:
		case SCR_EXAMINE_BOARD:
			move_cursor_left();
			break;
	}
}

static void right_pressed(void)
{
	switch (g_chaos_state.current_screen) {
		/*    case SCR_GAME_BOARD: move_cursor_right(); break;   */
		case SCR_SPLASH:
			splash_right();
			break;
		case SCR_CREATE_PLAYERS:
			create_players_right();
			break;

		case SCR_EDIT_NAME:
			edit_name_right();
			break;

		case SCR_SELECT_SPELL:
			spell_select_right();
			break;

		case SCR_OPTIONS:
			options_right();
			break;

		case SCR_EXAMINE_BOARD:
		case SCR_CASTING:
		case SCR_MOVEMENT:
			move_cursor_right();
			break;
	}
}

static void a_pressed(void)
{
	switch (g_chaos_state.current_screen) {
		case SCR_SPLASH:
			splash_a();
			break;
		case SCR_CREATE_PLAYERS:
			create_players_accept();
			break;
		case SCR_GAME_MENU:
			game_menu_a();
			break;
		case SCR_SELECT_SPELL:
			spell_select_a();
			break;
		case SCR_EDIT_NAME:
			edit_name_a();
			break;

		case SCR_OPTIONS:
			options_a();
			break;

		case SCR_CASTING:
			casting_a();
			break;
		case SCR_MOVEMENT:
			movement_a();
			break;
	}

}

static void b_pressed(void)
{
	switch (g_chaos_state.current_screen) {
		case SCR_SPLASH:
			splash_b();
			break;

		case SCR_SELECT_SPELL:
			spell_select_b();
			break;
		case SCR_EDIT_NAME:
			edit_name_b();
			break;
		case SCR_CASTING:
			casting_b();
			break;

		case SCR_OPTIONS:
			options_back();
			break;

		case SCR_MOVEMENT:
			movement_b();
			break;

		case SCR_EXAMINE_BOARD:
			examine_back();
			break;

		case SCR_CREATE_PLAYERS:
			create_players_back();
			break;
	}
}

static void l_pressed(void)
{
	switch (g_chaos_state.current_screen) {
		case SCR_EXAMINE_BOARD:
		case SCR_MOVEMENT:
		case SCR_CASTING:
			movement_l();
			break;

		case SCR_CREATE_PLAYERS:
			create_players_l();
			break;
	}

}

static void r_pressed(void)
{
	switch (g_chaos_state.current_screen) {
		case SCR_SELECT_SPELL:
			spell_select_r();
			break;

		case SCR_CREATE_PLAYERS:
			create_players_r();
			break;

		case SCR_EXAMINE_BOARD:
		case SCR_MOVEMENT:
		case SCR_CASTING:
			examine_square(target_index);
			break;

	}

}

static void touch_pressed(void)
{
	int x = platform_touch_x();
	int y = platform_touch_y();
	if (x == -1 || y == -1)
		return;
	switch (g_chaos_state.current_screen) {
		case SCR_SPLASH:
			splash_touch(x, y);
			break;
		case SCR_OPTIONS:
			options_touch(x, y);
			break;
		case SCR_CREATE_PLAYERS:
			create_players_touch(x, y);
			break;
		case SCR_EDIT_NAME:
			edit_name_touch(x, y);
			break;
		case SCR_GAME_MENU:
			game_menu_touch(x, y);
			break;
		case SCR_SELECT_SPELL:
			spell_select_touch(x, y);
			break;
		case SCR_EXAMINE_BOARD:
			examine_board_touch(x, y);
			break;
		case SCR_CASTING:
			casting_touch(x, y);
			break;
		case SCR_MOVEMENT:
			movement_touch(x, y);
			break;
	}
}

static void start_pressed(void)
{
	switch (g_chaos_state.current_screen) {
		case SCR_SPLASH:
			splash_start();
			break;
		case SCR_MOVEMENT:
			movement_start();
			break;
		case SCR_CREATE_PLAYERS:
			create_players_start();
			break;
		case SCR_EDIT_NAME:
			edit_name_start();
			break;
			/*    case SCR_OPTIONS: options_start(); break; */
	}
}

static void select_pressed(void)
{
	switch (g_chaos_state.current_screen) {
		case SCR_MOVEMENT:
			movement_select();
			break;
	}
}

void keypress_waiting(intptr_t arg)
{
	enum wait_type waiting = (enum wait_type)arg;
	if (waiting == NO_WAIT)
		return;
	platform_update_keys();
	for (;;) {
		int i, prsd;
		prsd = 0;
		for (i = 0; i <= CHAOS_KEY_TOUCH; i++) {
			if (platform_key_pressed(i)) {
				prsd = 1;
				break;
			}
		}
		if ((waiting == WAIT_FOR_LETGO && prsd == 0) || (waiting == WAIT_FOR_KEYPRESS && prsd == 1))
			break;
		platform_wait();
		platform_update_keys();
	}
}

void wait_for_keypress(void)
{
	if (IS_CPU(g_chaos_state.current_player))
		return;
	keypress_waiting(WAIT_FOR_KEYPRESS);
}

void wait_for_letgo(void)
{
	if (IS_CPU(g_chaos_state.current_player))
		return;
	keypress_waiting(WAIT_FOR_LETGO);
}

#undef CVK_KEY_PRESSED
#define CVK_KEY_PRESSED(n)      ( (virtual_keys[n]==1)|| (virtual_keys[n]>CVK_KEY_REPEAT))
void handle_keys(void)
{
	get_key_presses();
	int CVK_KEY_REPEAT = CVK_KEY_REPEAT_DEFAULT;
	/* cut the delay for the game board */
	if (g_chaos_state.current_screen == SCR_CASTING ||
	    g_chaos_state.current_screen == SCR_MOVEMENT ||
	    g_chaos_state.current_screen == SCR_EXAMINE_BOARD) {
		CVK_KEY_REPEAT /= 2;
	}

	if (CVK_KEY_PRESSED(CVK_UP)) {
		up_pressed();
	}
	if (CVK_KEY_PRESSED(CVK_DOWN)) {
		down_pressed();
	}
	if (CVK_KEY_PRESSED(CVK_LEFT)) {
		left_pressed();
	}
	if (CVK_KEY_PRESSED(CVK_RIGHT)) {
		right_pressed();
	}
	if (CVK_KEY_PRESSED(CVK_A)) {
		a_pressed();
	}
	if (CVK_KEY_PRESSED(CVK_B)) {
		b_pressed();
	}
	if (CVK_KEY_PRESSED(CVK_L)) {
		l_pressed();
	}
	if (CVK_KEY_PRESSED(CVK_R)) {
		r_pressed();
	}
	if (CVK_KEY_PRESSED(CVK_TOUCH)) {
		touch_pressed();
	}

	if (platform_key_pressed_repeat(CHAOS_KEY_START)) {
		start_pressed();
	}

	if (platform_key_pressed_repeat(CHAOS_KEY_SELECT)) {
		select_pressed();
	}
}
#undef CVK_KEY_PRESSED

