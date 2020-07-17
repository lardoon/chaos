#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include "chaos/options.h"

void parse_options(const char *optstr);
char *options_to_str(void);

void test_parse_options(void)
{
	parse_options(NULL);
	assert("did not crash");

	Options[OPT_ROUND_LIMIT] = 0;
	Options[OPT_OLD_BUGS] = 0;	/* on */
	Options[OPT_SOUND_ENABLED] = 1;	/* on */
	Options[OPT_NEW_FEATURES] = 1;	/* on */
	const char *t1 = ("ROUND_LIMIT = 21\nSOUND_ENABLED = 0\n"
			  "OLD_BUGS = 1\nNEW_FEATURES = 0\n");
	char *tmp = strdup(t1);
	parse_options(tmp);
	free(tmp);
	assert(Options[OPT_ROUND_LIMIT] == 21);
	assert(Options[OPT_OLD_BUGS] == 1);
	assert(Options[OPT_SOUND_ENABLED] == 0);
	assert(Options[OPT_NEW_FEATURES] == 0);

	char *optstr = options_to_str();
	assert(optstr != NULL);
	const char *expected = ("ROUND_LIMIT = 21\n"
				"NEW_FEATURES = 0\n"
				"SOUND_ENABLED = 0\n"
				"OLD_BUGS = 1\n"
				"LANG = 0\n");
	int diff = strcmp(optstr, expected);
	assert(diff == 0);
}

int main(void)
{
	test_parse_options();
	return 0;
}
