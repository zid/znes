#include <windows.h>
#include <stdio.h>
#include "rom.h"
#include "mem.h"
#include "cpu.h"
#include "ppu.h"

#define fatal(x) {printf("Fatal error: %s", x); exit(0);}

int main(void)
{
	HANDLE f, map;
	unsigned char *rombytes;

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

	init_rom(rombytes);
	init_mem();
	init_cpu();
	init_ppu();

	while(1){
		cpu_cycle();
		ppu(cpu_getcycles());
	}

	return 0;
}
