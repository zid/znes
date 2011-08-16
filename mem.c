#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "mem.h"
#include "mmc.h"
#include "rom.h"

static unsigned char *mem;
static int banksize;
static int swapbank;

unsigned char *mem_getaddr(unsigned int b)
{
	return &mem[b];
}
unsigned int get_short_at(unsigned int addr)
{
	return mem[addr+1] << 8 | mem[addr];
}

static unsigned char readb(unsigned int addr)
{
	switch(addr)
	{
		case 0x2002:
			return 0x80; /* Always vblank, for now */
		break;
	}

	return mem[addr];
}
unsigned char get_byte_at(unsigned int addr)
{
	return readb(addr);
}

void init_mem(void)
{
	unsigned char *b = rom_getbytes();

	mem = calloc(0x10000, 1); /* 0 - FFFF */
	memcpy(&mem[0x8000], b+0x38000, 16384);
	memcpy(&mem[0xC000], b+0x3C000, 16384);
//  memcpy(&mem[0x6000], sram from file);
}

void writeb(unsigned int addr, unsigned char val)
{
	/* If mapper == 01 */
	if(addr >= 0x8000 && (val & 0x80))
		mmc_shift_reset();
	else if(addr >= 0x8000 && addr < 0xA000)
		mmc_reg0_sendbit(val);
	else if(addr >= 0xA000 && addr < 0xC000)
		mmc_reg1_sendbit(val);
	else if(addr >= 0xC000 && addr < 0xE000)
		mmc_reg2_sendbit(val);
	else if(addr >= 0xE000 && addr <= 0xFFFF)
		mmc_reg3_sendbit(val);

	switch(addr)
	{
		case 0x2000:
			if((val & 0x80) == 0)
				ppu_disable_nmis();
			else
				ppu_enable_nmis();
		break;
		case 0x2001:
			if(val & 0x1)
				printf("Monochrome mode.\n");
			else
				printf("Color mode.\n");

			if(val & 0x2)
				printf("Background clipping disabled.\n");
			else
				printf("Background clipping set to left.\n");

			if(val & 0x4)
				printf("Sprite clipping disabled.\n");
			else
				printf("Sprite clipping at left.\n");

			if(val & 0x8)
				printf("Background displayed.\n");
			else
				printf("Background disabled.\n");

			if(val & 0x10)
				printf("Sprites displayed.\n");
			else
				printf("Sprites not displayed.\n");
		break;
		default:
			mem[addr] = val;
		break;
	}
}


