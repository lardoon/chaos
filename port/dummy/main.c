#include "chaos/porting.h"

int main(void)
{
	chaos_main();
	while (1) {
		chaos_one_frame();
	}
	return 0;
}
