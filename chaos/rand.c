#include "chaos/rand.h"

static uint32_t s_random = 0x600d5eed;

void ChurnRand(void)
{
	s_random *= 663608941;	/* churn the random number */
}


int GetRand(int i)
{
	unsigned long long nResult = i;
	nResult = nResult * s_random;
	ChurnRand();
	return (uint32_t) (nResult >> 32);
}

unsigned int GetSeed(void)
{
	return s_random;
}

void setSeed(unsigned int newSeed)
{
	s_random = newSeed;
}

