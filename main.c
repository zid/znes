#include <windows.h>
#include <stdio.h>
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

static void read_header(unsigned char *b, struct ROM *r)
{
	if(memcmp(b, "NES\x1A", 4) != 0)
	    fatal("NES header not found.\n");

	r->PRGsize = b[4] * 16384;
	r->CHRsize = b[5] * 8192;

	r->prgbytes = &b[16];
	r->chrbytes = &b[16 + r->PRGsize];
	r->flags = b[6];

	printf("PRG: 0x%X, CHR: 0x%X\n", r->PRGsize, r->CHRsize);
	printf("Flags:\n");

	if(r->flags & 0x8){
		printf("\tFour-screen mirroring\n");
	} else {
		if(r->flags & 0x1)
			printf("\tVertical mirroring\n");
		else
			printf("\tHorizontal mirroring\n");
	}

	if(r->flags & 0x2)
		printf("\tSRAM is battery backed\n");

	if(r->flags & 0x4)
		printf("\tTrainer present\n");

	r->flags2 = b[7];
	/* Meh, my test rom doesn't use this */

	r->mapper = (r->flags2 & 0xF) | ((r->flags & 0xF0) >> 4);
	printf("Mapper: %02X\n", r->mapper);
}

int main(void)
{
	HANDLE f, map;
	unsigned char *rombytes;
	struct ROM rom;

	f = CreateFile("ff1.nes", GENERIC_READ, FILE_SHARE_READ, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(f == INVALID_HANDLE_VALUE)
	    fatal("Unable to open rom file.\n");

	map = CreateFileMapping(f, NULL, PAGE_READONLY, 0, 0, NULL);
	if(!map)
		fatal("Couldn't create file map.\n");

	rombytes = MapViewOfFile(map, FILE_MAP_READ, 0, 0, 0);
	if(!rombytes)
	    fatal("Couldn't map view of file.\n");

	read_header(rombytes, &rom);
	init_mem(rom.prgbytes);
	init_cpu();
	init_ppu();

	while(1){
		cpu_cycle();
		ppu(cpu_getcycles());
	}

	return 0;
}
