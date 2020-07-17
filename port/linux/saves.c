#include <stdio.h>                      /* for fclose, fopen, printf, FILE, etc */
#include <stdlib.h>                     /* for free, malloc, getenv */
#include <string.h>                     /* for strcat, strlen, strcpy */
#include <sys/stat.h>                   /* for stat, mkdir, S_ISDIR */
#include <unistd.h>                     /* for fsync */
#include "chaos/porting.h"          /* for prototypes of platform_save_options, etc. */
#include "port/linux/utils.h"          /* for read_file_as_str */

static const char *save_file = "options.cfg";
char *g_load_file = NULL;

char *platform_load_options(void)
{
	char *result = NULL;
	const char *home = getenv("HOME");
	if (!home) {
		printf("Unable to find home\n");
		return 0;
	}
	char *path = malloc(strlen(home) + 10 + strlen(save_file));
	strcpy(path, home);
	strcat(path, "/.chaos");

	struct stat buf;
	int exists = 0;
	if (stat(path, &buf) == 0)
		exists = S_ISDIR(buf.st_mode);

	if (!exists && mkdir(path, 0777) < 0) {
		printf("Unable to create .chaos directory in home\n");
		free(path);
		return 0;
	}
	strcat(path, "/");
	strcat(path, save_file);

	/* load sram saves from disk... */
	result = read_file_as_str(path);
	free(path);
	return result;
}

static char *get_save_filename(const char *bn)
{
	const char *home = getenv("HOME");
	char *path = malloc(strlen(home) + 20 + strlen(bn));
	strcpy(path, home);
	strcat(path, "/.chaos/");
	strcat(path, bn);
	return path;
}

void platform_save_options(const char *saves, unsigned int size)
{
	char *path = get_save_filename(save_file);
	char *tmp = get_save_filename("options.tmp");

	write_str_to_file(path, tmp, saves, size);
	free(path);
	free(tmp);
}

int platform_has_saved_game(void)
{
	struct stat buf;
	char *path = g_load_file == NULL ? get_save_filename("game.txt") : strdup(g_load_file);
	int result = stat(path, &buf) == 0 && S_ISREG(buf.st_mode);
	free(path);
	return result;
}

void platform_save_game(const char *game, unsigned int size)
{
	char *path = get_save_filename("game.txt");
	char *tmp = get_save_filename("game.txt.tmp");
	write_str_to_file(path, tmp, game, size);
	free(path);
	free(tmp);
	/* make sure not to load the -l option again. */
	free(g_load_file);
	g_load_file = NULL;
}

char *platform_load_game(void)
{
	char *path = g_load_file == NULL ? get_save_filename("game.txt") : strdup(g_load_file);
	char *result = read_file_as_str(path);
	free(path);
	return result;
}
