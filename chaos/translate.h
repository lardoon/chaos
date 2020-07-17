#ifndef translate_h_seen
#define translate_h_seen

/* Use _() to translate strings. */
#define _(text) translate(text)

/* Use T() for strings that need to be translated dynamically later, not where
 * they are declared.  This is just a marker that is picked up by xgettext to
 * extract the strings into the pot file. */
#define T(text) text

const char *translate(const char *user_input);

#endif
