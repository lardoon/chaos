/* Parse command line options */
#include "chaos/platform.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include "port/linux/cmdline.h"

#include "chaos/options.h"
#include "port/linux/wmhandler.h"
#include "chaos/gfx.h"
#include "chaos/spelldata.h"
#include "chaos/chaos.h"

extern int gfx_scale;
extern int g_stress_mode;
extern int g_debug_mode;
extern char *g_load_file;
extern int g_icon;
struct option cmdline_opts[] = {
	{"load", required_argument, 0, 'l'},
	{"debug", required_argument, 0, 'd'},
	{"help", no_argument, 0, '?'},
	{"off", no_argument, (int *) &Options[OPT_OLD_BUGS], 0},
	{"on", no_argument, (int *) &Options[OPT_OLD_BUGS], 1},
	{"old-bugs", required_argument, 0, 'o'},
	{"rounds", required_argument, 0, 'r'},
	{"forever", no_argument, (int *) &Options[OPT_ROUND_LIMIT], 0},
	{"stress", no_argument, 0, 's'},
	{"scale", no_argument, 0, 'x'},
	{"scale-1", no_argument, &gfx_scale, 1},
	{"scale-2", no_argument, &gfx_scale, 2},
	{"scale-3", no_argument, &gfx_scale, 3},
	{"icon", required_argument, 0, 'i'},
	{"full-screen", no_argument, 0, 'f'},
	{"headless", no_argument, 0, 'n'},
	{"port", required_argument, 0, 'p'},
	{NULL, no_argument, NULL, 0}
};
void print_usage(void)
{
	printf("Usage: chaos [OPTIONS]...\n\n");

	printf("Chaos - The Battle of Wizards by Julian Gollop\n");
	printf("Turn-based magical combat for up to 8 players\n\n");

	printf("Examples: \n");
	printf(" chaos -sx2 --full-screen # full screen mode, 2x scale, host a game\n\n");

	printf("If a long option shows an argument as mandatory, then it is mandatory\n");
	printf("for the equivalent short option also.  Similarly for optional arguments.\n\n");

	printf("Display options: \n");
	printf("-f, --full-screen       Run in full screen mode\n");
	printf("-x, --scale=SCALE       Set scale 1x - 3x\n");
	printf("    --scale-1\n");
	printf("    --scale-2\n");
	printf("    --scale-3\n");
	printf("-n, --headless          Do not create a window at all (debug mode)\n");
	printf("-i, --icon=SPELL        Set application icon to the spell number.\n");
	printf("                        Accepts values from %d to %d inclusive.\n\n",
		 SPELL_KING_COBRA, SPELL_WALL);
	printf("Gameplay options: \n");
	printf("-o, --old-bugs=on       Set old bugs 'on' or 'off'\n");
	printf("    --on\n");
	printf("    --off\n");
	printf("-s,--stress             Stress test mode (8 CPUs, keep going)\n");
	printf("-l,--load=FILE          Load this file and continue\n");
	printf("-d,--debug=MODE         Debug mode: 0 spells fail, 1 spells "
		       "succeed\n always\n");
	printf("-r, --rounds=ROUNDS     Set number of rounds played:  \n");
	printf("    --forever            0    infinite rounds\n");
	printf("                         1    players*2+15 rounds (default)\n");
	printf("                         any  maximum rounds (e.g. 25)\n\n");
}

int parse_opts(int argc, char *argv[])
{

	int op = -1;
	int usage = 0;
	while ((op = getopt_long(argc, argv,
					"sd:l:fni:o:r:x:",
					cmdline_opts, NULL)) != -1) {
		switch (op) {
			case 'f':
				isFullScreen = FULL_SCREEN;
				break;
			case 'n':
				isFullScreen = HEADLESS;
				break;
			case 'i':
				g_icon = atoi(optarg);
				if (g_icon < SPELL_KING_COBRA
						|| g_icon > SPELL_WALL)
					g_icon = SPELL_GOOEY_BLOB;
				break;
			case 'o':
				if (strcmp(optarg, "on") == 0) {
					Options[OPT_OLD_BUGS] = 1;
				} else {
					Options[OPT_OLD_BUGS] = 0;
				}
				break;
			case 'r':
				Options[OPT_ROUND_LIMIT] = atoi(optarg);
				break;
			case 's':
				g_stress_mode = 1;
				break;
			case 'x':
				gfx_scale = atoi(optarg);
				switch (gfx_scale) {
					case 1:
					case 2:
					case 3:
						break;
					default:
						printf("Unknown graphics scale: %s\n",
								optarg);
						gfx_scale = 1;
						break;
				}
				break;
			case 'l':
				g_load_file = strdup(optarg);
				break;
			case 'd':
				g_debug_mode = atoi(optarg) + 1;
				break;
			case '?':
				usage = 1;
				break;
			default:
				break;
		}
	}
#ifdef _HEADLESS
	isFullScreen = HEADLESS;
#endif
	if (usage) {
		print_usage();
		return 1;
	}
	return 0;
}

#ifdef HAS_STRESS_TEST
int running_in_stress_mode(void)
{
	return g_stress_mode;
}
#endif

