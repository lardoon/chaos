#include <gba_dma.h>
#include "chaos/porting.h"

extern uint16_t *text_map_data;
extern uint16_t *text_char_data;

void platform_load_text_tile_data(int offset, const void *data, int size)
{
	/* each 8 by 8 tile takes up 16 bytes */
	/* the font consists of 84 letters */
	/* each one is 16*8, 4bpp = 16*4 pixels = 64 uint8_t transfers, 32 uint16_t transfers per char */
	/* total = 32*84 */
	offset = 605 * 16;
	dmaCopy(data, &text_char_data[offset], size);
}

void platform_set_text_map_entry(int x, int y, int palette)
{
	if (palette == 0)
		text_map_data[x + y * 32] = 0;
	else
		text_map_data[x + y * 32] = palette + 605;
}
