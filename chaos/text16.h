#ifndef TEXT16_H
#define TEXT16_H
#include <stdint.h>

/*

   The map of the characters is as follows:

   ' ( ) * + , - . / 0-9 : ; < = > ? @ A-Z [ \ ] ^ _ ` a-z

   The changes are:

   ASCII   |   Chaos
   ------------+-------------
   |
 *      |   CHAOS symbol
 +      |   LAW symbol
 /      |   %
 <      |   back arrow
 >      |   return/enter arrow
 \      |   #

 e.g. "\\<>" would print "#" followed by back arrow, return arrow on screen


*/

#define MAX_LETTERS       95
#define FIRST_CHAR_INDEX  '\''
#define TRANSP_CHAR_INDEX  ('z'+1)

void setup_text16(uint16_t offset);

void print_char16(int ascii, int x, int y, int pal);
void print_text16(const char *text, uint16_t x, uint16_t y, uint8_t pal);
void set_text16_colour(uint8_t pal, uint16_t col);
int clear_text(void);
char *trim_whitespace(char *str);
void int2a(int n, char s[], int b);

#endif				/*TEXT16_H */

