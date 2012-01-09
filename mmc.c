#include <stdio.h>
#include <stdlib.h>
#include "cpu.h"
#include "mem.h"
#include "mmc.h"
#include "rom.h"
#include "ppu.h"

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
	int bank;

	if(!sendbit(b, &reg3_value))
		return;

	bank = reg3_value & 0x1F;

	mem_set_bank(bankselect + 0, rom_get_bank(bank, 0));
	mem_set_bank(bankselect + 1, rom_get_bank(bank, 1));
	mem_set_bank(bankselect + 2, rom_get_bank(bank, 2));
	mem_set_bank(bankselect + 3, rom_get_bank(bank, 3));
}

static void mmc1_cpu_writeb(unsigned int addr, unsigned char val)
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
		cpu_writeb_unsafe(addr, val);
}

static unsigned char mmc1_cpu_readb(unsigned int addr)
{
	return cpu_readb_unsafe(addr);
}

static void mmc1_ppu_writeb(unsigned int addr, unsigned char val)
{
	/* 3000 - 3EFF is mirrored to 2000 - 2EFF */
	if(addr >= 0x3000 && addr <= 0x3EFF)
	{
		addr = 0x2000 + (addr & 0xEFF);
		goto write;
	}

	/* 3F20 - 3FFF is mirrored to 3F00 - 3F1F */
	if(addr >= 0x3F20)
	{
		/* Don't write here, this area can be double mirrored */
		addr &= 0x3F1F;
	}

	switch(addr){
		case 0x3F10:
			addr =  0x3F00;
		break;
		case 0x3F14:
			addr =  0x3F04;
		break;
		case 0x3F18:
			 addr = 0x3F08;
		break;
		case 0x3F1C:
			addr = 0x3F0C;
		break;
	}

	write:
	ppu_writeb_raw(addr, val);
}

static unsigned char mmc1_ppu_readb(unsigned int addr)
{
	/* 3000 - 3EFF is mirrored to 2000 - 2EFF */
	if(addr >= 0x3000 && addr <= 0x3EFF)
	{
		addr = 0x2000 + (addr & 0xEFF);
		goto read;
	}

	/* 3F20 - 3FFF is mirrored to 3F00 - 3F1F */
	if(addr >= 0x3F20)
	{
		/* Not done yet, this area can be double mirrored */
		addr &= 0x3F1F;
	}

	switch(addr)
	{
		case 0x3F10:
			addr = 0x3F00;
		break;
		case 0x3F14:
			addr = 0x3F04;
		break;
		case 0x3F18:
			addr = 0x3F08;
		break;
		case 0x3F1C:
			addr = 0x3F0C;
		break;
	}

	read:
	return ppu_readb_raw(addr);
}

void init_mmc1(void)
{
	cpu_set_writeb(mmc1_cpu_writeb);
	cpu_set_readb(mmc1_cpu_readb);

	ppu_set_writeb(mmc1_ppu_writeb);
	ppu_set_readb(mmc1_ppu_readb);

	mem_set_bank(0x0, calloc(0x1000, 1));
	mem_set_bank(0x1, calloc(0x1000, 1));
	mem_set_bank(0x2, calloc(0x1000, 1));
	mem_set_bank(0x3, calloc(0x1000, 1));
	mem_set_bank(0x4, calloc(0x1000, 1));
	mem_set_bank(0x5, calloc(0x1000, 1));
	mem_set_bank(0x6, calloc(0x1000, 1));
	mem_set_bank(0x7, calloc(0x1000, 1));

	mem_set_bank(0x8, rom_get_bank(0, 0));
	mem_set_bank(0x9, rom_get_bank(0, 1));
	mem_set_bank(0xA, rom_get_bank(0, 2));
	mem_set_bank(0xB, rom_get_bank(0, 3));

	mem_set_bank(0xC, rom_get_bank(-1, 0));
	mem_set_bank(0xD, rom_get_bank(-1, 1));
	mem_set_bank(0xE, rom_get_bank(-1, 2));
	mem_set_bank(0xF, rom_get_bank(-1, 3));

	ppu_set_bank(0x0, 0x0000);
	ppu_set_bank(0x1, 0x0400);
	ppu_set_bank(0x2, 0x0800);
	ppu_set_bank(0x3, 0x0C00);
	ppu_set_bank(0x4, 0x1000);
	ppu_set_bank(0x5, 0x1400);
	ppu_set_bank(0x6, 0x1800);
	ppu_set_bank(0x7, 0x1C00);
	ppu_set_bank(0x8, 0x2000);
	ppu_set_bank(0x9, 0x2400);
	ppu_set_bank(0xA, 0x2000);
	ppu_set_bank(0xB, 0x2400);
	ppu_set_bank(0xC, 0x3000);
	ppu_set_bank(0xD, 0x3400);
	ppu_set_bank(0xE, 0x3800);
	ppu_set_bank(0xF, 0x3C00);
}
