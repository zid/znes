#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdio.h>
#include "rom.h"
#include "mem.h"
#include "cpu.h"
#include "ppu.h"

#define fatal(x) {printf("Fatal error: %s", x); exit(0);}

static int running = 1;

void main_quit(void)
{
	running = 0;
}

int main(void)
{
	int fd, len;
	unsigned char *rombytes;
	struct stat st;
	fd = open("nestest.nes", O_RDONLY);
	fstat(fd, &st);

	rombytes = mmap(0, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

	init_rom(rombytes);
	printf("ROM OK\n");
	init_mem();
	printf("Mem OK\n");
	init_cpu();
	printf("CPU OK\n");
	init_ppu();
	printf("PPU OK\n");

	while(running){
		/* Emulates one opcode of the cpu per call */
		cpu_cycle();

		/* Updates the ppu (graphics). This needs to be called last for
		 * consistency, and so if the GUI tells us to quit, we don't continue
		 * emulating. ppu() will call main_quit() to end this while loop
		 */
		ppu(cpu_getcycles());
	}

	return 0;
}
