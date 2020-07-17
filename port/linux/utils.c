#include <stdio.h>                      /* for fclose, fopen, printf, FILE, etc */
#include <stdlib.h>                     /* for free, malloc, exit, getenv */
#include <unistd.h>                     /* for fsync */
#include "port/linux/utils.h"

char *read_file_as_str(const char *filename)
{
	char *result = NULL;
	/* load sram saves from disk... */
	FILE *fp = fopen(filename, "r");
	if (fp != 0) {
		size_t n;
		fseek(fp, 0, SEEK_END);
		n = ftell(fp);
		n++;
		fseek(fp, 0, SEEK_SET);
		result = malloc(n);
		size_t amount = fread(result, n - 1, 1, fp);
		if (amount > 0)
			result[n - 1] = 0;
		else
			result[0] = 0;
		fclose(fp);
	}
	return result;
}

void write_str_to_file(const char *filename, const char *tmpfilename, const char *data, unsigned int size)
{
	FILE *fd = fopen(tmpfilename, "w");
	if (fd != 0) {
		fwrite(data, size, 1, fd);
		fsync(fileno(fd));
		fclose(fd);
		rename(tmpfilename, filename);
	}
}
