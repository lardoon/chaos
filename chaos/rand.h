#ifndef rand_h_seen
#define rand_h_seen

#include <stdint.h>

int GetRand(int i);
void ChurnRand(void);
void setSeed(unsigned int newSeed);
unsigned int GetSeed(void);

#endif
