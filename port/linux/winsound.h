#ifndef winsound_h_seen
#define winsound_h_seen
typedef struct _SOUNDMAP  {
	const unsigned char *pSample;	/* pointer to the sample file that makes up the wav */
	const unsigned char *pSampleEnd;	/* pointer to the end of sample file */
} SoundMap;
int init_sdl_sound(void);

#endif
