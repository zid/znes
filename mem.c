#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "mem.h"
#include "mmc.h"
#include "ppu.h"
#include "rom.h"
#include "joypad.h"

static unsigned char *mem[16];

void mem_set_bank(unsigned int bank, unsigned char *data)
{
	mem[bank] = data;
}

void init_mem(void)
{

}

unsigned char cpu_readb_unsafe(unsigned int addr)
{
	switch(addr)
	{
		case 0x2002:
			return ppu_read_reg2();
		break;
		case 0x2007:
			return ppu_readb_buffered();
		break;
		case 0x4016:
			return joypad_read();
		break;
	}

	return mem[(addr & 0xF000)>>12][addr&0x0FFF];
}

void cpu_writeb_unsafe(unsigned int addr, unsigned char val)
{
	switch(addr)
	{
		case 0x2000:
			ppu_write_reg1(val);
		break;
		case 0x2001:
			ppu_write_reg2(val);
		break;
		case 0x2006:
			ppu_write_addr(val);
		break;
		case 0x2007:
			ppu_write_data(val);
		break;
		case 0x4016:
			joypad_write(val);
		break;
		default:
			mem[(addr & 0xF000)>>12][addr&0x0FFF] = val;
		break;
	}
}


