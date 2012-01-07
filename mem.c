#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "mem.h"
#include "mmc.h"
#include "ppu.h"
#include "rom.h"
#include "joypad.h"
#include <io.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

static unsigned char *mem[16];

unsigned char *mem_getaddr(unsigned int b)
{
	return &mem[(b & 0xF000)>>24][b&0x0FFF];
}

static unsigned char readb(unsigned int addr)
{
	switch(addr)
	{
		case 0x2002:
			return ppu_read_reg2();
		break;
		case 0x4016:
			return joypad_read();
		break;
	}

	if(addr > 0x7FF && addr < 0x2000)
		addr &= 0x7FF;

	return mem[(addr & 0xF000)>>12][addr&0x0FFF];
}

unsigned int get_short_at(unsigned int addr)
{
	return readb((addr+1)&0xFFFF) << 8 | readb(addr);
}

unsigned char get_byte_at(unsigned int addr)
{
	return readb(addr);
}

void mem_set_bank(unsigned int bank, unsigned char *data)
{
	printf("Mem_set_bank %d\n", bank);
	mem[bank+0] = data+0x0000;
	mem[bank+1] = data+0x1000;
	mem[bank+2] = data+0x2000;
	mem[bank+3] = data+0x3000;
}

void init_mem(void)
{
	unsigned int mapper = rom_get_mapper();

	mem[0x0] = calloc(0x1000, 1);
	mem[0x1] = calloc(0x1000, 1);
	mem[0x2] = calloc(0x1000, 1);
	mem[0x3] = calloc(0x1000, 1);
	mem[0x4] = calloc(0x1000, 1);
	mem[0x5] = calloc(0x1000, 1);
	mem[0x6] = calloc(0x1000, 1);
	mem[0x7] = calloc(0x1000, 1);

	switch(mapper)
	{
		case 0x00:
			mem[0x8] = rom_get_bank(0, 0);
			mem[0x9] = rom_get_bank(0, 1);
			mem[0xA] = rom_get_bank(0, 2);
			mem[0xB] = rom_get_bank(0, 3);
			mem[0xC] = rom_get_bank(1, 0);
			mem[0xD] = rom_get_bank(1, 1);
			mem[0xE] = rom_get_bank(1, 2);
			mem[0xF] = rom_get_bank(1, 3);
		break;
		case 0x01:
			mem[0x8] = rom_get_bank(0, 0);
			mem[0x9] = rom_get_bank(0, 1);
			mem[0xA] = rom_get_bank(0, 2);
			mem[0xB] = rom_get_bank(0, 3);
			mem[0xC] = rom_get_bank(-1, 0);
			mem[0xD] = rom_get_bank(-1, 1);
			mem[0xE] = rom_get_bank(-1, 2);
			mem[0xF] = rom_get_bank(-1, 3);
		break;
	}

	int fd;
	int i;

	fd = open("mem2.bin", O_RDWR | O_CREAT | O_TRUNC, S_IRWXU);
	_setmode(fd, _O_BINARY);

	for(i = 0; i < 0x10; i++)
	{
		printf("mem[%02X]: %p\n", i, mem[i]);
		printf("%d\n", write(fd, mem[i], 0x1000));
	}
	close(fd);
}

void writeb(unsigned int addr, unsigned char val)
{
	unsigned int mapper;
	unsigned int filtered = 0;

	if(addr > 0x7FF && addr < 0x2000)
		addr &= 0x7FF;

	mapper = rom_get_mapper();

	switch(mapper)
	{
		case 01: /* MMC1 */
			filtered = mmc1_write(addr, val);
		break;
	}

	if(filtered == 1)
		return;

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


