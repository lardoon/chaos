#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL.h>
#include <SDL_mixer.h>
#include "chaos/sound_data.h"
#include "chaos/porting.h"


const char *s_filenames[] = {
	"sfx/attack.ogg",
	"sfx/beam.ogg",
	"sfx/chosen.ogg",
	"sfx/cpustart.ogg",
	"sfx/electro.ogg",
	"sfx/fire.ogg",
	"sfx/gooey.ogg",
	"sfx/justice.ogg",
	"sfx/menu.ogg",
	"sfx/range.ogg",
	"sfx/scream.ogg",
	"sfx/spellstep.ogg",
	"sfx/spellsuccess.ogg",
	"sfx/urgh.ogg",
	"sfx/walk.ogg",
};

#define TOTAL_WAV_FILES (sizeof(s_filenames) / sizeof(s_filenames[0]))

Mix_Chunk *s_waves[TOTAL_WAV_FILES] = {};

void init_wav(int nSample)
{
	Mix_Chunk *wave;
	int loops = 0;
	if (nSample < 0 || nSample >= (int)TOTAL_WAV_FILES)
		return;
	const char *filename = s_filenames[nSample];
	if (s_waves[nSample] == 0) {
		wave = Mix_LoadWAV(filename);
		s_waves[nSample] = wave;
	} else {
		wave = s_waves[nSample];
	}
	if (!wave) {
		fprintf(stderr, "Unable to initialize audio: %s (%d)\n", filename, nSample);
		return;
	}
	Mix_PlayChannel(0, wave, loops);
}

static int s_soundOK = 0;
int init_sdl_sound(void)
{
	int audio_rate = 16000;
	Uint16 audio_format = AUDIO_S16SYS;
	int audio_channels = 1;
	int audio_buffers = 4096;

	if(Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers) != 0) {
		fprintf(stderr, "Unable to initialize audio: %s\n", Mix_GetError());
		s_soundOK = 0;
		return s_soundOK;
	}
	memset(s_waves, 0, sizeof(s_waves));
	s_soundOK = 1;
	return s_soundOK;
}

void platform_play_soundfx(int soundid)
{
	if (s_soundOK) {
		init_wav(soundid);
	}
}
