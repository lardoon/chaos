#include <maxmod9.h>
#include <string.h>
#include "chaos/porting.h"
#include "sfx/soundbankmm.h"
#include "sfx/soundbank.h"

static int loaded_samples[MSL_NSAMPS];

void platform_play_soundfx(int soundid)
{
	if (loaded_samples[soundid] == 0) {
		mmLoadEffect(soundid);
		loaded_samples[soundid] = 1;
	}
	int current_sound = mmEffect(soundid);
	mmEffectRelease(current_sound);
}

void ds_init_sound(void)
{
	memset(loaded_samples, 0, sizeof(loaded_samples));
	mmInitDefaultMem((mm_addr) soundbank);
}
