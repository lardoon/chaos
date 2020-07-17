#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>

void order_table(uint8_t count, uint8_t * table);

void test_order_table(void)
{
	uint8_t data[] = {
		0, 1,
		6, 5,
		4, 7,
		2, 3,
	};
	uint8_t expected[] = {
		6, 5,
		4, 7,
		2, 3,
		0, 1,
	};
	order_table(4, data);

	assert(memcmp(expected, data, sizeof(expected)) == 0);
}

int main(void)
{
	test_order_table();
	return 0;
}
