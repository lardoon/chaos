#ifndef utils_h_seen
#define utils_h_seen

char *read_file_as_str(const char *filename);

void write_str_to_file(const char *filename, const char *tmpfilename,
		       const char *data, unsigned int size);
#endif
