#include <stdio.h>
#include "mmc.h"
#include "rom.h"

static unsigned int reg0_value;
static unsigned int reg1_value;
static unsigned int reg2_value;
static unsigned int reg3_value;
static unsigned int reg4_value; /* shift register */
static int count;

/* MCC1 register 0 */
static int bankselect = 8;
static int banksize;

static void mmc1_shift_reset(void)
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

static void mmc1_reg0_sendbit(unsigned int b)
{
	if(!sendbit(b, &reg0_value))
		return;

	bankselect = (reg0_value & 0x4) ? 0x8 : 0xC;
	banksize   = (reg0_value & 0x8) ? 16384 : 32768;
}

static void mmc1_reg1_sendbit(unsigned int b)
{
	sendbit(b, &reg1_value);
}

static void mmc1_reg2_sendbit(unsigned int b)
{
	sendbit(b, &reg2_value);
}

static void mmc1_reg3_sendbit(unsigned int b)
{
	if(!sendbit(b, &reg3_value))
		return;

	rom_load_bank(bankselect, reg3_value & 0x1F, banksize);
}

int mmc1_write(unsigned int addr, unsigned char val)
{
	if(addr >= 0x8000 && (val & 0x80))
		mmc1_shift_reset();
	else if(addr >= 0x8000 && addr < 0xA000)
		mmc1_reg0_sendbit(val);
	else if(addr >= 0xA000 && addr < 0xC000)
		mmc1_reg1_sendbit(val);
	else if(addr >= 0xC000 && addr < 0xE000)
		mmc1_reg2_sendbit(val);
	else if(addr >= 0xE000 && addr <= 0xFFFF)
		mmc1_reg3_sendbit(val);
	else
		return 0;

	return 1;
}
