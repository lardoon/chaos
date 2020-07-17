#include <string.h>
#include "chaos/porting.h"
#include "chaos/options.h"

extern const char *translation_es_po[];
extern const char *translation_chaos_pot[];

const char **get_names(void)
{
	return translation_chaos_pot;
}

const char **get_translations(void)
{
	const char *lang = platform_get_lang();
	if (Options[OPT_LANGUAGE] == 1)
		return translation_chaos_pot;
	else if (Options[OPT_LANGUAGE] == 2)
		return translation_es_po;

	/* else use default */
	if (lang && strncmp(lang, "es", 2) == 0)
		return translation_es_po;
	return 0;
}

const char *translate(const char *user_input)
{
	const char **catalog = get_names();
	const char **translations = get_translations();
	if (catalog == 0 || translations == 0)
		return user_input;
	int i;
	size_t search_len = strlen(user_input);
	for (i = 0; catalog[i] != 0; i++) {
		if (strcmp(catalog[i], user_input) == 0 && strlen(catalog[i]) == search_len) {
			return (translations[i][0] == 0) ?  user_input : translations[i];
		}
	}
	return user_input;
}
