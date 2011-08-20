#include <stdlib.h>
#include "ppu.h"
#include "cpu.h"
#include "sdl.h"

struct PPU {
	int vblank_nmi;
	int frame;
	unsigned char *mem;
	unsigned char *tiles;
	unsigned char *nametable;
	unsigned int increment;
	unsigned int addr;
	unsigned int addr_count;
	unsigned int bg;
};

static struct PPU p;

void ppu_write_reg1(unsigned int val)
{
	/* Possible addresses for name tables */
	unsigned int name_tables[] = {0x2000, 0x2400, 0x2800, 0x2C00};

	/* Creates a pointer into p.mem at the correct offset */
	p.nametable = &p.mem[name_tables[val&0x3]];

	p.increment = val & 0x4 ? 32 : 1;

	if(val & 0x10)
		p.tiles = &p.mem[0x1000];
	else
		p.tiles = &p.mem[0x0000];

	p.vblank_nmi = !!(val & 0x80);
}

void ppu_write_reg2(unsigned int val)
{
	p.bg = !!(val & 0x8);
}

void ppu_write_addr(unsigned int val)
{
	if(p.addr_count == 0){
		p.addr = (val & 0x3F)<<8;
		p.addr_count = 1;
	} else {
		p.addr |= val;
		p.addr_count = 0;
	}
}

void ppu_write_data(unsigned int val)
{
	p.mem[p.addr] = val;
	p.addr += p.increment;
}

void init_ppu(void)
{
	init_sdl();
	p.frame = 0;
	p.addr_count = 0;
	p.mem = calloc(1, 0x4000);
}

static void ppu_vblank_starts(void)
{
	sdl_frame();

	if(p.vblank_nmi)
		cpu_nmi(0xFFFA);
}

void ppu(unsigned int cycles)
{
	int frame;

	if(!sdl_update())   /* returns false if SDL caught a quit event */
		return;

	frame = cycles / 113.6f;
	frame %= 262;
	if(frame == 242 && p.frame == 241)
		ppu_vblank_starts();
	p.frame = frame;
}
