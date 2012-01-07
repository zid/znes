#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "rom.h"
#include "ppu.h"
#include "cpu.h"
#include "sdl.h"

extern unsigned char palette[64][3];

struct PPU {
	int vblank_nmi;
	int scanline;
	unsigned char *mem[4];
	unsigned char *tiles;
	unsigned char *nametable;
	unsigned int increment;
	unsigned int addr;
	unsigned int addr_count;
	unsigned int bg;
	unsigned int vblank;    /* NMI ACK bit, 0x2000 bit 7 */
};

static struct PPU p;

void ppu_write_reg1(unsigned int val)
{
	/* Possible addresses for name tables */
	unsigned int name_tables[] = {0x2000, 0x2400, 0x2800, 0x2C00};

	/* nametable selection, bottom 2 bits are which of the four tables */
	p.nametable = &p.mem[2][name_tables[val&0x3]&0xFFF];

	p.increment = val & 0x4 ? 32 : 1;
	p.tiles = val & 0x10 ? p.mem[1] : p.mem[0];
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

unsigned int ppu_get_addr(){
	return p.addr;
}

unsigned int ppu_read_reg2(void)
{
	int t = 0;

	t |= (p.vblank << 7);

	p.vblank = 0;
	p.addr_count = 0;

	return t;
}

/* Various mirrors between 0x3F00 and 0x4000 exist. Take an address in that
 * range and return the lowest address which it is a mirror of.
 */
unsigned int ppu_mem_palette_mirror(unsigned int addr)
{
	unsigned int newaddr;

	newaddr = 0x3F00 + (addr & 0x1F);

	switch(newaddr){
		case 0x3F10:
			return 0x3F00;
		break;
		case 0x3F14:
			return 0x3F04;
		break;
		case 0x3F18:
			return 0x3F08;
		break;
		case 0x3F1C:
			return 0x3F0C;
		break;
		default:
			return newaddr;
		break;
	}
}

void ppu_write_data(unsigned int val)
{
	unsigned int addr;

	addr = p.addr;

	if(p.addr < 0x2000 && rom_chr_is_readonly()){
		goto nowrite;
	}
	if(p.addr >= 0x3F00 && p.addr < 0x4000)
		addr = ppu_mem_palette_mirror(p.addr);

	p.mem[(addr&0xF000)>>12][addr&0xFFF] = val;
	if(rom_get_mirror() == 1)
	{
		int newaddr = (addr & 0x3FF) + 0x2400;
		if(addr >= 0x2000 && addr < 0x2400)
			p.mem[(newaddr&0xF000)>>12][newaddr&0xFFF] = val;
		else if(addr >= 0x2400 && addr < 0x2800)
		{
			p.mem[(newaddr&0xF000)>>12][newaddr&0xFFF] = val;
		}
	}
nowrite:
	p.addr += p.increment;
}

void init_ppu(void)
{
	unsigned char *chr;

	init_sdl();
	printf("SDL OK\n");
	p.scanline = 0;
	p.addr_count = 0;
	p.vblank = 0;

	chr = rom_get_chr(0);
	p.mem[0] = chr ? chr : calloc(1, 0x1000);

	chr = rom_get_chr(1);
	p.mem[1] = chr ? chr : calloc(1, 0x1000);

	p.mem[2] = calloc(1, 0x1000);
	p.mem[3] = calloc(1, 0x1000);
}

unsigned int ppu_get_palette_address(unsigned int pnum)
{
	unsigned int palettes[] = {0x3F00, 0x3F04, 0x3F08, 0x3F0C};

	return palettes[pnum];
}

static void ppu_draw_tile(unsigned char *b, unsigned int ty, unsigned int tx, unsigned int tile, unsigned int pnum)
{
	unsigned int y, x, px, py, paddr;
	unsigned char *tiledata, *out;
	unsigned int pindex[8]; /* 8 palette entries for a row of a tile */

	tiledata = &p.tiles[tile*16];

	paddr = ppu_get_palette_address(pnum);
//	printf("paddr: %04X\n", paddr);
	for(y = 0; y < 8; y++){
		/* First 8 bytes of tile data contain the lower order bit of the
		 * palette index for 64 pixels. The second 8 bytes contain the
		 * high order bit. So to find a palette index for a tile: combine
		 * the byte number y and y+8's xth bit. For example, y=3, x=2
		 * you would take byte 3 and 11, and take bit 2 of each byte.
		 */
		pindex[7] = (!!(tiledata[y] & 0x01)) | (!!(tiledata[y+8] & 0x01))<<1;
		pindex[6] = (!!(tiledata[y] & 0x02)) | (!!(tiledata[y+8] & 0x02))<<1;
		pindex[5] = (!!(tiledata[y] & 0x04)) | (!!(tiledata[y+8] & 0x04))<<1;
		pindex[4] = (!!(tiledata[y] & 0x08)) | (!!(tiledata[y+8] & 0x08))<<1;
		pindex[3] = (!!(tiledata[y] & 0x10)) | (!!(tiledata[y+8] & 0x10))<<1;
		pindex[2] = (!!(tiledata[y] & 0x20)) | (!!(tiledata[y+8] & 0x20))<<1;
		pindex[1] = (!!(tiledata[y] & 0x40)) | (!!(tiledata[y+8] & 0x40))<<1;
		pindex[0] = (!!(tiledata[y] & 0x80)) | (!!(tiledata[y+8] & 0x80))<<1;

		/* Calculate what pixel on screen this represents */
		py = 256 * ((ty * 8) + y);

		for(x = 0; x < 8; x++){
			unsigned int addr, red, green, blue;

			/* Convert from a palette index (0-3) and a palette address
			 * into an address suitable for reading from, because of the
			 * internal mirroring of various palette adresses.
			 */
			addr = ppu_mem_palette_mirror(paddr + pindex[x]);
//			printf("addr: %04X, ", addr);
			/* Convert NES colour value to an RGB value using a LUT. */
			red   = palette[p.mem[(addr&0xF000)>>12][addr&0xFFF]][0];
			green = palette[p.mem[(addr&0xF000)>>12][addr&0xFFF]][1];
			blue  = palette[p.mem[(addr&0xF000)>>12][addr&0xFFF]][2];
//			printf("R: 02%X, G: %02X, B: 02%X\n", red, green, blue);


			/* Calc offset into the output texture to write our RBG triplet */
			px = (tx * 8) + x;
			out = b + (px + py) * 3; /* 3 bytes per pixel */

			out[2] = red;
			out[1] = green;
			out[0] = blue;
		}
	}
}

unsigned int palette_for_tile(unsigned int x, unsigned int y, unsigned char *attrib)
{
	unsigned int offset, shift;

	/* Each byte of the attribute table contains the palette index for four
	 * 2x2 tile areas. The attribute table thus needs to convert a 32*30
	 * style coordinate into a 8*8 (less 2 rows) table.
	 */

	/* Clear the bottom two bits and use the 2D array formula y*width + x
	 * to calculate the offset byte. This works because the nametable is
	 * at x4 scale compared to the nametable, and 2^2 = 4.
	 */
	/* offset = (8*y/4) + (x/4) */
	offset = ((y>>2)<<3) + (x>>2);

	/* Each byte in the nametable represents a 4x4 tile block like this:
	 * _______
	 * |AA|BB|
	 * |AA|BB|
	 * ---+---
	 * |CC|DD|
	 * |CC|DD|
	 * -------
	 * Where AA is bit 0-1, BB is bit 2-3, CC is bit 4-5, DD is bit 6-7.
	 * Because 4 tiles are indexed by a single 2 bit entry, the coordinates
	 * are only needed to 1/4th the precision. So (once rounded to the nearest
	 * multiple of 2), x=0-1 is represented by AA, x=2-3 is represented by BB.
	 * Which half byte (4 bits) to use is given by y/2. Which 2 bits within that
	 * half byte is given by x/2.
	 */

	shift = 2 * ((y>>1)&1) + ((x>>1)&1);

	return (attrib[offset] >> shift) & 0x3; /* 0x3 is 0b11 */
}

static void ppu_draw_frame(unsigned char *b)
{
	int y, x;

	for(y = 0; y < 30; y++){
		for(x = 0; x < 32; x++){
			unsigned int tile, palette;

			tile = p.nametable[y*32+x];

			palette = palette_for_tile(x, y, &p.nametable[0x3C0]);
//			printf("x: %02d, y: %02d, tileno: %02X, addr: %04X, palette: %02X\n", x, y, tile, 0x2000 + &p.nametable[y*32+x] - &p.nametable[0], palette);

			ppu_draw_tile(b, y, x, tile, palette);
		}
	}
//	exit(0);
}

static void ppu_vblank_starts(void)
{
	unsigned char *b;
	if(p.bg){
		/* Get pointer from sdl to a buffer for rendering */
		b = sdl_get_buffer();

		ppu_draw_frame(b);

		/* We're done with the buffer, it can be rendered */
		sdl_frame();
	}

	if(p.vblank_nmi)
		cpu_nmi(0xFFFA);
}

void ppu(unsigned int cycles)
{
	int scanline;

	if(!sdl_update())   /* returns false if SDL caught a quit event */
		return;

	scanline = cycles / 113.6f;
	scanline %= 262;

	if(p.scanline == 260 && scanline == 261){
		p.vblank = 0;
	}

	if(scanline == 242 && p.scanline == 241){
		p.vblank = 1;
		ppu_vblank_starts();
	}

	p.scanline = scanline;
}
