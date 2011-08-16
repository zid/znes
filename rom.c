#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mem.h"

#define fatal(x) {printf("Fatal error: %s", x); exit(0);}

struct ROM {
	int PRGsize;
	int CHRsize;
	unsigned char *chrbytes;
	unsigned char *prgbytes;
	unsigned char *rombytes;
	int flags;
	int flags2;
	int mapper;
};

static struct ROM r;

unsigned char *rom_getbytes(void)
{
	return r.rombytes;
}

void rom_load_bank(unsigned int addr, unsigned int bank, unsigned int size)
{
	memcpy(mem_getaddr(addr), &r.rombytes[bank*size], size);
}

void init_rom(unsigned char *b)
{
	r.rombytes = b+0x10;

	if(memcmp(b, "NES\x1A", 4) != 0)
	    fatal("NES header not found.\n");

	r.PRGsize = b[4] * 16384;
	r.CHRsize = b[5] * 8192;

	r.prgbytes = &b[16];
	r.chrbytes = &b[16 + r.PRGsize];
	r.flags = b[6];

	printf("PRG: 0x%X, CHR: 0x%X\n", r.PRGsize, r.CHRsize);
	printf("Flags:\n");

	if(r.flags & 0x8){
		printf("\tFour-screen mirroring\n");
	} else {
		if(r.flags & 0x1)
			printf("\tVertical mirroring\n");
		else
			printf("\tHorizontal mirroring\n");
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
