#include <fat.h>                        /* for fatInitDefault */
#include <stdio.h>                      /* for fclose, fopen, FILE, fread, etc */
#include <stdlib.h>                      /* for malloc */
#include <sys/stat.h>                   /* for mkdir */
#include "chaos/porting.h"

static const char *options_file = "/data/chaos/options.cfg";
static const char *game_save_file = "/data/chaos/game.txt";
static const char *tmp_filename = "/data/chaos/tmp.dat";
static int s_fatInit = 0;

char *read_file_as_str(const char *filename);

void write_str_to_file(const char *filename, const char *tmpfilename,
		       const char *data, unsigned int size);

static void file_init(void)
{
	struct stat buf;
	if (!s_fatInit) {
		fatInitDefault();
		s_fatInit = 1;
		if (!(stat("/data", &buf) == 0 && S_ISDIR(buf.st_mode))) {
			mkdir("/data", 0777);
		}
		if (!(stat("/data/chaos", &buf) == 0 && S_ISDIR(buf.st_mode))) {
			mkdir("/data/chaos", 0777);
		}
	}
}

char *platform_load_options(void)
{
	file_init();
	/* load sram saves from disk... */
	return read_file_as_str(options_file);
}


void platform_save_options(const char *saves, unsigned int size)
{
	file_init();
	write_str_to_file(options_file, tmp_filename, saves, size);
}

char *platform_load_game(void)
{
	file_init();
	return read_file_as_str(game_save_file);
}

int platform_has_saved_game(void)
{
	struct stat buf;
	return stat(game_save_file, &buf) == 0 && S_ISREG(buf.st_mode);
}

void platform_save_game(const char *game, unsigned int size)
{
	write_str_to_file(game_save_file, tmp_filename, game, size);
}
