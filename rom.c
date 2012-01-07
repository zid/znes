#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mem.h"

#define fatal(x) {printf("Fatal error: %s", x); exit(0);}

struct ROM {
	int PRGsize;
	int CHRsize;
	unsigned char *prgbytes;
	unsigned char *chrbytes;
	unsigned int prgbanks;
	unsigned int chrbanks;
	unsigned char *rombytes;
	int flags;
	int flags2;
	unsigned int mapper;
	int mirror;

};

static struct ROM r;

unsigned int rom_chr_is_readonly(void)
{
	return r.CHRsize;
}

unsigned int rom_get_mirror(void)
{
	return r.mirror;
}

unsigned int rom_get_mapper(void)
{
	return r.mapper;
}

unsigned char *rom_getbytes(void)
{
	return r.rombytes;
}

unsigned char *rom_get_bank(signed int bank, unsigned int sub)
{
	if(bank < 0)
	{
		bank = (r.prgbanks) + bank;
	}
	if(bank >= r.prgbanks)
	{
		bank = 0;
	}

	return &r.rombytes[(bank * 0x4000) + (sub * 0x1000)];
}

void rom_load_bank(unsigned int addr, unsigned int bank, unsigned int size)
{
//	printf("Loaded bank %02X into %1X000\n", bank, addr);

	mem_set_bank(addr, &r.rombytes[bank*0x4000]);
}

unsigned char *rom_get_chr(unsigned int bank)
{
	if(bank >= r.chrbanks)
		return NULL;

	return &r.chrbytes[bank * 0x1000];
}

void init_rom(unsigned char *b)
{
	r.rombytes = b+0x10;

	if(memcmp(b, "NES\x1A", 4) != 0)
	    fatal("NES header not found.\n");

	r.PRGsize = b[4] * 16384;
	r.CHRsize = b[5] * 8192;
	r.prgbanks = b[4];
	r.chrbanks = b[5];

	printf("%d prg banks, %d chr banks\n", r.prgbanks, r.chrbanks);
	r.prgbytes = &b[16];
	r.chrbytes = &b[16 + r.PRGsize];
	r.flags = b[6];


	printf("PRG: 0x%X, CHR: 0x%X\n", r.PRGsize, r.CHRsize);
	printf("PRG: %04X, CHR: %04X\n", 16, 16 + r.PRGsize);
	printf("Flags:\n");

	if(r.flags & 0x8){
		printf("\tFour-screen mirroring\n");
		r.mirror = 4;
	} else {
		if(r.flags & 0x1)
		{
			printf("\tVertical mirroring\n");
			r.mirror = 2;
		}
		else
		{
			printf("\tHorizontal mirroring\n");
			r.mirror = 1;
		}
	}

	if(r.flags & 0x2)
		printf("\tSRAM is battery backed\n");

	if(r.flags & 0x4)
		printf("\tTrainer present\n");

	r.flags2 = b[7];
	/* Meh, my test rom doesn't use this */

	r.mapper = (r.flags2 & 0xF) | ((r.flags & 0xF0) >> 4);
	printf("Mapper: %02X\n", r.mapper);
}
