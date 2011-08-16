#include <stdio.h>
#include "mmc.h"

static unsigned int reg0_value;
static unsigned int reg1_value;
static unsigned int reg2_value;
static unsigned int reg3_value;
static unsigned int reg4_value; /* shift register */
static int count;

/* MCC1 register 0 */
static int bankselect;
static int banksize;

void mmc_shift_reset(void)
{
	reg4_value = 0;
	count = 0;
}

static int sendbit(unsigned int b, unsigned int *v)
{
	reg4_value = (reg4_value >> 1) | ((b&1) << 4);
	count++;
	if(count == 5){
		*v = reg4_value;
		count = 0;
		return 1;
	}
	return 0;
}

void mmc_reg0_sendbit(unsigned int b)
{
	if(!sendbit(b, &reg0_value))
		return;

	bankselect = (reg0_value & 0x4) ? 0x8000 : 0xC000;
	banksize   = (reg0_value & 0x8) ? 16384 : 32768;
}

void mmc_reg1_sendbit(unsigned int b)
{
	if(sendbit(b, &reg1_value))
		printf("Written %02X to mmc register 1\n", reg4_value);
}

void mmc_reg2_sendbit(unsigned int b)
{
	if(sendbit(b, &reg2_value))
		printf("Written %02X to mmc register 2\n", reg4_value);
}

void mmc_reg3_sendbit(unsigned int b)
{
	if(!sendbit(b, &reg3_value))
		return;

	rom_load_bank(bankselect, reg3_value & 0x1F, banksize);
}
