#include <stdio.h>
#include "mmc.h"

static int reg3_shift = 3;
static int reg3_value = 0x0;

void mmc_reg3_reset(void)
{
	reg3_value = 0;
	reg3_shift = 3;
}

void mmc_reg3_sendbit(unsigned int b)
{
	reg3_value |= (b&1)<<reg3_shift;
	reg3_shift--;
	if(reg3_shift < 0)
		printf("Reg3 value %0d\n", reg3_value);
	reg3_value = 0;
}
