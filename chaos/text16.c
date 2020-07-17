#include <ctype.h>
#include <string.h>

#include "chaos/platform.h"
#include "chaos/porting.h"
#include "chaos/arena.h"
#include "chaos/text16.h"
#include "chaos/gfx.h"
#include "gfx/palettec/chaosfont.h"
static uint16_t bgmapoffset16;

/*  Reverses string s[] in place  */
static void reverse(char s[])
{
	int i, j;
	for (i = 0, j = strlen(s) - 1; i < j; i++, j--) {
		int c = s[i];
		s[i] = s[j];
		s[j] = c;
	}
}

/* set up the 16 point letters in the tile memory - used for standard stuff... */
/* the letters are 4bpp */
void setup_text16(uint16_t offset)
{
	bgmapoffset16 = offset;
	/*uint16_t realos = bgmapoffset16 * 16;*/	/* each 8 by 8 tile takes up 16 bytes */

	/* the font consists of 84 letters */
	/* each one is 16*8, 4bpp = 16*4 pixels = 64 uint8_t transfers, 32 uint16_t transfers per char */
	/* total = 32*84 */
	platform_load_text_tile_data(0 /*realos*/, chaosfontTiles, 64 * MAX_LETTERS);
	platform_swap_screen();
	platform_load_text_tile_data(0 /*realos*/, chaosfontTiles, 64 * MAX_LETTERS);
	platform_swap_screen();
	/* the colour is set depending on the palette */
}

/* set the colour for the text in a given palette */
void set_text16_colour(uint8_t pal, uint16_t col)
{
	platform_set_palette_entry(16 * pal + 1, col);
}

void print_char16(int ascii, int x, int y, int pal)
{
	uint32_t letter = ascii - FIRST_CHAR_INDEX;

	/* draw a letter... */
	if (ascii == ' ') {
		letter = TRANSP_CHAR_INDEX - FIRST_CHAR_INDEX;
	}
	platform_set_text_map_entry(x, y, (letter * 2 + bgmapoffset16) | (pal << 12));
	platform_set_text_map_entry(x, y + 1, (letter * 2 + 1 + bgmapoffset16) | (pal << 12));
}

/* print the 16 point text */
/* each letter is 2 tiles big. the first tile goes on row y, */
/* the second on row y+1 */
void print_text16(const char *text, uint16_t x, uint16_t y, uint8_t pal)
{
	uint32_t indx = 0;
	uint32_t newx = x;
	while ((text[indx] != 0)) {
		print_char16(text[indx++] & 0xff, newx, y, pal);
		newx++;
	}
}

int clear_text(void)
{
	clear_bg(1);
	return 0;
}

char *trim_whitespace(char *str)
{
	char *s = str;
	char *d = str;

	/* eat trailing white space */
	while (*s)
		s++;
	while (str < s && isspace((unsigned char)*(s - 1)))
		*s-- = '\0';
	*s = '\0';
	s = str;

	/* has leading space? */
	if (isspace((unsigned char)*s))
	{
		/* eat white space  */
		while (isspace((unsigned char)*s))
			s++;

		/* copy */
		while (*s)
			*d++ = *s++;
		*d = '\0';
	}
	return str;
}

void int2a(int n, char s[], int b)
{
	static const char digits[17] = "0123456789ABCDEF";
	int i, sign;
	if (b < 2 || b > 36) {
		return;
	}
	if ((sign = n) < 0)
		n = -n;
	i = 0;

	do {
		s[i++] = digits[n % b];
	} while ((n /= b) > 0);

	if (sign < 0)
		s[i++] = '-';
	s[i] = '\0';
	reverse(s);
}
