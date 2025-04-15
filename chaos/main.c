/*standard headers and library calls */
#include "chaos/platform.h"
#include "chaos/porting.h"

/* specific includes */
#include "chaos/spelldata.h"
#include "chaos/gfx.h"
#include "chaos/rand.h"
#include "chaos/cursor_defines.h"
#include "chaos/arena.h"
#include "chaos/players.h"
#include "chaos/chaos.h"
#include "chaos/input.h"
#include "chaos/options.h"
#include "chaos/wizards.h"
#include "chaos/splash.h"
#include "chaos/text16.h"
#include "chaos/spellselect.h"
static int s_interrupts_on = 0;

static void load_bg(void)
{
	load_all_palettes();
	setup_text16(0);
	g_message_y = 18;
	if (!platform_has_scroll())
		g_message_y = 22;
}

volatile uint8_t game_frames;

void chaos_main(void)
{
	int exit_code = platform_init();
	if (exit_code)
		return;
	init_arena_table();
	init_player_table();
	load_bg();
	set_default_options();
	load_sprite_palette();
	reset_arena_tables();
	reset_players();
	show_splash();
	fade_up();
}

static void do_animations(void)
{
	if (g_chaos_state.current_screen == SCR_SPLASH) {
		ChurnRand();	/* make sure we are more random */
		animate_splash_screen();
	}
	else if (g_chaos_state.current_screen == SCR_CREATE_PLAYERS)
		animate_player_screen();
	else if (g_chaos_state.current_screen == SCR_SELECT_SPELL)
		anim_spell_select();
	if (s_interrupts_on && (g_chaos_state.current_screen == SCR_EXAMINE_BOARD
	    || g_chaos_state.current_screen == SCR_CASTING
	    || g_chaos_state.current_screen == SCR_MOVEMENT)) {
		countdown_anim();
		animate_arena();
	}
}

void chaos_one_frame(void)
{
	handle_keys();
	do_animations();
}

/* delay a given number of vblanks */
/* 60 waits 1 second. */
void delay(int d)
{
#ifdef _HEADLESS
	d = 0;
#endif
#ifdef HAS_STRESS_TEST
	int running_in_stress_mode(void);
	if (running_in_stress_mode())
		d = 1;
#endif
	enable_interrupts();
	int i;
	for (i = 0; i < d; i++) {
		platform_wait();
		do_animations();
	}
}

int disable_interrupts(void)
{
	s_interrupts_on = 0;
	return 0;
}

int enable_interrupts(void)
{
	s_interrupts_on = 1;
	return 0;
}

