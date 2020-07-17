/* options.c */
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "chaos/platform.h"
#include "chaos/porting.h"
#include "chaos/translate.h"

#include "chaos/sound_data.h"
#include "chaos/options.h"
#include "chaos/spelldata.h"
#include "chaos/chaos.h"
#include "chaos/gfx.h"
#include "chaos/text16.h"
#include "chaos/input.h"
#include "chaos/splash.h"
#include "chaos/creature.h"
#include "chaos/touch.h"

#define ON_OFF_OPTION 0
#define NUMBER_OPTION 1
/*#define NO_OPTION 2*/
#define LANG_OPTION 3

/* the options... used to vary in game stuff */
struct option {
	char *name;		/* text value of the option on screen */
	uint8_t type;		/* on off, etc */
	uint8_t minimum;		/* min value */
	uint16_t maximum;		/* max value */
};

static const struct option s_option_menu[] = {
	{ T("Round Limit"), NUMBER_OPTION, 0, 500 },
	{ T("Old Bugs"), ON_OFF_OPTION, 0, 59 },
	{ T("New Features"), ON_OFF_OPTION, 0, 59 },
	{ T("Sound On "), ON_OFF_OPTION, 0, 59 },
	{ T("Sound Test"), NUMBER_OPTION, 0, 14 },
	{ T("Language"), LANG_OPTION, 0, 2 },
};

static const char *s_languages[] = {
	"EN", "ES"
};

unsigned int Options[OPTION_COUNT + 1];
static int s_load_options = 1;

static int is_android(void)
{
	return (strcmp(platform_name(), "Android") == 0);
}

static int get_step_size(void)
{
	int width, height;
	int step = 3;
	platform_screen_size(&width, &height);
	if (height < 190)
		step = 2;
	if (is_android())
		step = 2;
	return step;
}

static void deselect_option(uint8_t item)
{
	set_text16_colour(item + 1, RGB16(0, 14, 0));
}

static void select_option(uint8_t item)
{
	set_text16_colour(item + 1, RGB16(0, 31, 0));
	set_text16_colour(BACK_OPTION, RGB16(20, 0, 0));
	set_text16_colour(MORE_OPTION, RGB16(20, 0, 0));
}

static void handle_on_off(int nOpt, int x, int y, int step)
{
	const char *what = Options[nOpt] ? _("ON ") : _("OFF");
	print_text16(what, x + 14, y + nOpt * step, nOpt + 1);
}

static void handle_number(int nOpt, int x, int y, int step)
{
	char str[9];
	const char *def = _("DEFAULT");
	size_t blotout = strlen(def);
	if (nOpt == OPT_ROUND_LIMIT && Options[nOpt] < 2) {
		if (Options[nOpt] == 0) {
			strcpy(str, _("OFF  "));
		} else if (Options[nOpt] == 1) {
			strcpy(str, def);
		}
	} else {
		int2a(Options[nOpt], str, 10);
	}
	size_t len = strlen(str);
	while (len < blotout) {
		str[len++] = ' ';
	}
	str[len] = 0;
	print_text16(str, x + 14, y + nOpt * step, nOpt + 1);
}

static void handle_language(int nOpt, int x, int y, int step)
{
	if (Options[nOpt] > 2) {
		Options[nOpt] = 0;
	}
	const char *lang = _("DEFAULT");
	if (Options[nOpt])
		lang = s_languages[Options[nOpt] - 1];
	print_text16("       ", x + 14, y + nOpt * step, nOpt + 1);
	print_text16(lang, x + 14, y + nOpt * step, nOpt + 1);
}

static void draw_option(int nOpt)
{

	/* used to draw the value of a particular option after changing it... */
	if (nOpt >= OPTION_COUNT)
		return;

	int step = get_step_size();
	int x = 3;
	int y = 5;
	print_text16(_(s_option_menu[nOpt].name), x, y + nOpt * step,
		     nOpt + 1);
	int type = s_option_menu[nOpt].type;
	switch (type) {
	case ON_OFF_OPTION:
		handle_on_off(nOpt, x, y, step);
		break;
	case NUMBER_OPTION:
		handle_number(nOpt, x, y, step);
		break;
	case LANG_OPTION:
		handle_language(nOpt, x, y, step);
		break;
	default: break;
	}
}


/* change this so it uses a scroller... */
static void draw_options(void)
{
	int j;
	for (j = 0; j < OPTION_COUNT; j++) {
		draw_option(j);
		deselect_option(j);
	}
}

void parse_options(char *optstr)
{
	if (optstr == NULL)
		return;
	/* read "lines" from optstr */
	/* OPTION = VALUE */
	char *tmp = optstr;
	int exitloop = 0;
	while (exitloop == 0 && *tmp != 0) {
		char *namebegin, *nameend;
		char *valbegin;
		while (isspace((unsigned char)*tmp)) {
			tmp++;
		}
		namebegin = tmp;
		while (*tmp != '=' && !isspace((unsigned char)*tmp)) {
			tmp++;
		}
		nameend = tmp;
		while (isspace((unsigned char)*tmp) || *tmp == '=') {
			tmp++;
		}
		valbegin = tmp;
		while (*tmp != 0 && *tmp != '\n' && !isspace((unsigned char)*tmp)) {
			tmp++;
		}
		if (*tmp != 0) {
			*tmp = 0;
			tmp++;
		} else {
			exitloop = 1;
		}

		int namesz = nameend - namebegin;
		int optentry = -1;
		if (strncmp(namebegin, "ROUND_LIMIT", namesz) == 0)
			optentry = OPT_ROUND_LIMIT;
		if (strncmp(namebegin, "NEW_FEATURES", namesz) == 0)
			optentry = OPT_NEW_FEATURES;
		if (strncmp(namebegin, "SOUND_ENABLED", namesz) == 0)
			optentry = OPT_SOUND_ENABLED;
		if (strncmp(namebegin, "OLD_BUGS", namesz) == 0)
			optentry = OPT_OLD_BUGS;
		if (strncmp(namebegin, "LANG", namesz) == 0)
			optentry = OPT_LANGUAGE;
		/* 0 for off, 1 for always fail casts, 2 for always succeeds casts */
		if (strncmp(namebegin, "CHEAT", namesz) == 0)
			optentry = OPT_CHEAT;

		if (optentry < 0)
			continue;
		int val = atoi(valbegin);
		Options[optentry] = val;
	}

}

void set_default_options(void)
{
	Options[OPT_ROUND_LIMIT] = DEFAULT_ROUNDS;
	Options[OPT_OLD_BUGS] = 1;	/* on */
	Options[OPT_SOUND_ENABLED] = 1;	/* on */
	Options[OPT_NEW_FEATURES] = 1;	/* on */
	Options[OPT_CHEAT] = 0;	/* no cheat */
	Options[OPT_LANGUAGE] = 0;	/* default */
	if (s_load_options) {
		char *res = platform_load_options();
		parse_options(res);
		free(res);
		s_load_options = 0;
	}
}

static int has_back(void)
{
	return strcmp(platform_name(), "GBA") != 0;
}

int show_options(void)
{
	clear_palettes();
	clear_text();
	clear_arena();
	anim_col = -31;
	anim_col_grad = -8;
	hilite_item = 0;
	g_chaos_state.current_screen = SCR_OPTIONS;
	print_text16(_("GAME OPTIONS"), 4, 2, 14);

	if (has_back()) {
		/* On GBA this doesn't fit, can use B */
		print_text16(_("BACK"), 23, 2, BACK_OPTION);
		set_text16_colour(BACK_OPTION, RGB16(20, 0, 0));
	}
	draw_options();
	if (is_android()) {
		print_text16(_("MORE OPTIONS"), 5, 20, MORE_OPTION);
		set_text16_colour(MORE_OPTION, RGB16(20, 0, 0));
	}

	set_text16_colour(14, RGB16(31, 30, 0));
	draw_decor_border(15, RGB16(0, 0, 0), RGB16(31, 0, 0));
	select_option(hilite_item);
	return 0;
}

void options_a(void)
{
	if (hilite_item == OPT_SOUND) {
		platform_play_soundfx(Options[OPT_SOUND]);
	}
	if (hilite_item == -1) {
		return options_back();
	}
	if (hilite_item == OPTION_COUNT) {
		platform_more_options();
		return;
	}
}

void options_up(void)
{
	if (hilite_item > 0) {
		if (hilite_item > OPTION_COUNT) {
			set_text16_colour(MORE_OPTION, RGB16(20, 0, 0));
		}
		play_soundfx(SND_MENU);
		deselect_option(hilite_item);
		hilite_item--;
		select_option(hilite_item);
	} else if (hilite_item == 0 && has_back()) {
		deselect_option(hilite_item);
		hilite_item--;
		set_text16_colour(BACK_OPTION, RGB16(31, 0, 0));
	}
}

void options_down(void)
{
	if (hilite_item == -1) {
		set_text16_colour(BACK_OPTION, RGB16(20, 0, 0));
		hilite_item++;
		select_option(hilite_item);
	} else if (hilite_item < OPTION_COUNT - 1) {
		play_soundfx(SND_MENU);
		deselect_option(hilite_item);
		hilite_item++;
		select_option(hilite_item);
	} else if (is_android() && hilite_item == (OPTION_COUNT - 1)) {
		deselect_option(hilite_item);
		hilite_item++;
		set_text16_colour(MORE_OPTION, RGB16(31, 0, 0));
	}
}

static void check_option_change_update(void)
{
	if (hilite_item == OPT_LANGUAGE) {
		int tmp = hilite_item;
		show_options();
		deselect_option(0);
		hilite_item = tmp;
		select_option(tmp);
	} else {
		draw_option(hilite_item);
	}
}

void options_left(void)
{
	if (s_option_menu[hilite_item].type == ON_OFF_OPTION) {
		if (!Options[hilite_item]) {
			Options[hilite_item] = 1;
			draw_option(hilite_item);
		}
	} else if (s_option_menu[hilite_item].type == NUMBER_OPTION ||
		   s_option_menu[hilite_item].type == LANG_OPTION) {

		/* number type, decrease */
		if (Options[hilite_item] >
				s_option_menu[hilite_item].minimum) {
			Options[hilite_item]--;
			check_option_change_update();
		}
	}
}

void options_right(void)
{
	if (s_option_menu[hilite_item].type == ON_OFF_OPTION) {
		if (Options[hilite_item]) {
			Options[hilite_item] = 0;
			draw_option(hilite_item);
		}
	} else if (s_option_menu[hilite_item].type == NUMBER_OPTION ||
		   s_option_menu[hilite_item].type == LANG_OPTION) {

		/* number type, increase */
		if (Options[hilite_item] <
				s_option_menu[hilite_item].maximum) {
			Options[hilite_item]++;
			check_option_change_update();
		}
	}
}

static void append_opt(const char *name, int optnum, char *buffer)
{
	char tmp[9];
	int2a(Options[optnum], tmp, 10);
	strcat(buffer, name);
	strcat(buffer, " = ");
	strcat(buffer, tmp);
	strcat(buffer, "\n");
}

char *options_to_str(void)
{
	char *buffer = malloc(OPTION_COUNT * 30);
	buffer[0] = 0;
	append_opt("ROUND_LIMIT", OPT_ROUND_LIMIT, buffer);
	append_opt("NEW_FEATURES", OPT_NEW_FEATURES, buffer);
	append_opt("SOUND_ENABLED", OPT_SOUND_ENABLED, buffer);
	append_opt("OLD_BUGS", OPT_OLD_BUGS, buffer);
	append_opt("LANG", OPT_LANGUAGE, buffer);
	return buffer;
}

void options_back(void)
{
	play_soundfx(SND_CHOSEN);
	char *optstr = options_to_str();
	platform_save_options(optstr, strlen(optstr));
	free(optstr);
	wait_for_letgo();
	fade_down();
	show_splash();
	fade_up();
}

void options_touch(int x, int y)
{
	int sqx, sqy;
	pixel_xy_to_square_xy(x, y, &sqx, &sqy);
	if (sqx > 22 && (sqy == 2 || sqy == 3 || sqy == 4)) {
		if (hilite_item >= 0)
			deselect_option(hilite_item);
		set_text16_colour(BACK_OPTION, RGB16(31, 0, 0));
		return options_back();
	}
	if (sqx > 4 && (sqy > 19 && sqy < 22) && is_android()) {
		if (hilite_item < OPTION_COUNT)
			deselect_option(hilite_item);
		set_text16_colour(MORE_OPTION, RGB16(31, 0, 0));
		platform_more_options();
		return;
	}
	int step = get_step_size();
	int opt = (sqy - 5) / step;
	if (opt < 0 || opt >= OPTION_COUNT)
		return;
	deselect_option(hilite_item);
	hilite_item = opt;
	select_option(hilite_item);
	int type = s_option_menu[opt].type;
	if (type == ON_OFF_OPTION) {
		Options[opt] = !Options[opt];
		draw_option(hilite_item);
	} else if (type == NUMBER_OPTION || type == LANG_OPTION) {
		if (sqx <= 14)
			options_left();
		else
			options_right();
		if (hilite_item == OPT_SOUND)
			platform_play_soundfx(Options[OPT_SOUND]);
	} else {
		options_back();
	}
}
