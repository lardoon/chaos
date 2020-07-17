#include "chaos/porting.h"
#include "sfx/soundbank.h"
#include <maxmod.h>


void platform_play_soundfx(int soundid)
{
	mm_sfxhand current_sound = mmEffect(soundid);
	mmEffectRelease(current_sound);
}

void gba_init_sound(void)
{
	mmInitDefault((mm_addr) soundbank, 8);
}
