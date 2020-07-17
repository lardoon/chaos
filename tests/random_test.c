#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "chaos/rand.h"
#include "chaos/spellenums.h"

#define GETRAND_TEST_VAL 256
extern uint8_t *new_spell_bag(int minspell, int maxspell, int count);

void dump_spell(int size)
{
	/* test shuffle bag */
	uint8_t *bag = new_spell_bag(SPELL_KING_COBRA, SPELL_RAISE_DEAD, size);
	int i;
	for (i = 0; i < size; i++) {
		printf("%d. %d\n", i, bag[i]);
	}
	free(bag);
}

void dump_xpm(int size)
{
	printf("/* XPM */\n"
	       "static char *icon[] = {\n"
	       "/* columns rows colors chars-per-pixel */\n"
	       "\"%d %d 2 1\",\n"
	       "\"  c #000000\",\n"
	       "\". c #FFFFFF\",\n"
	       , size, size);
	int x, y;
	for (y = 0; y < size; y++) {
		printf("\"");
		for (x = 0; x < size; x++) {
			int r = GetRand(GETRAND_TEST_VAL);
			printf("%c", r > (GETRAND_TEST_VAL / 2) ? '.': ' ');
		}
		printf("\"%s\n", y == (size - 1) ? "" : ",");
	}
	printf("}\n");
}

void dump_stream(int size)
{
	int i;
	for (i = 0; i < size; i++) {
		printf("%d\n", GetRand(GETRAND_TEST_VAL));
	}
}

void print_usage(void)
{
	printf("random_test [-x] [size]\n"
	       " -x        dump out a size x size xpm file\n"
	       "size       the amount of random numbers to dump out, default 96\n");
}

int main(int argc, const char *argv[])
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	uint32_t val = (tv.tv_usec << 8) | (tv.tv_sec & 0xff);
	setSeed(val);

	int size = 96;
	int xpm = 0;
	int spell = 0;
	if (argc > 1) {
		int i;
		for (i = 1; i < argc; i++) {
			int tmp = strtol(argv[i], 0, 0);
			if (tmp != 0)
				size = tmp;
			if (strcmp("-x", argv[i]) == 0)
				xpm = 1;
			if (strcmp("-s", argv[i]) == 0)
				spell = 1;
			if (strcmp("-h", argv[i]) == 0) {
				print_usage();
				return 1;
			}
		}
	}

	if (xpm) {
		dump_xpm(size);
	} else if (spell) {
		dump_spell(size);
	} else {
		dump_stream(size);
	}

	return 0;
}

