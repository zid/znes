#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "rom.h"
#include "ppu.h"
#include "cpu.h"
#include "sdl.h"

static unsigned char *ppu_mem;
extern unsigned char palette[64][3];

static void (*ppu_writeb)(unsigned int, unsigned char);
static unsigned char (*ppu_readb)(unsigned int);


struct PPU {
	int vblank_nmi;
	int scanline;
	unsigned char *mem[16];
	unsigned int tiles;
	unsigned int nametable;
	unsigned int increment;
	unsigned int addr;
	unsigned int addr_count;
	unsigned int bg;
	unsigned int vblank;    /* NMI ACK bit, 0x2000 bit 7 */
	unsigned char buffer;
};

static struct PPU p;

void ppu_set_writeb(void (*writebfp)(unsigned int, unsigned char))
{
	ppu_writeb = writebfp;
}

void ppu_set_readb(unsigned char (*readbfp)(unsigned int))
{
	ppu_readb = readbfp;
}

void ppu_writeb_unsafe(unsigned int addr, unsigned char val)
{
	p.mem[(addr & 0xFC00) >> 10][addr & 0x3FF] = val;
}

/* Actually reads from memory, called inside the ppu, and by the mapper */
unsigned char ppu_readb_raw(unsigned int addr)
{
	return p.mem[(addr & 0xFC00) >> 10][addr & 0x3FF];
}

/* Designed to be called from anywhere but inside the ppu */
unsigned char ppu_readb_buffered(void)
{
	unsigned char temp;

	if(p.addr < 0x3F00)
	{
		temp = p.buffer;
		p.buffer = ppu_readb(p.addr);
		p.addr += p.increment;

		return temp;
	}

	p.addr += p.increment;
	return ppu_readb(p.addr);
}

void ppu_set_bank(unsigned int bank, unsigned int addr)
{
	p.mem[bank] = &ppu_mem[addr];
}

/* PPU register 2007 */
void ppu_write_data(unsigned int val)
{
	ppu_writeb(p.addr, val);

	p.addr += p.increment;
}

/* PPU register 2000 */
void ppu_write_reg1(unsigned int val)
{
	/* Possible addresses for name tables */
	unsigned int name_tables[] = {0x2000, 0x2400, 0x2800, 0x2C00};

	/* nametable selection, bottom 2 bits are which of the four tables */
	p.nametable = name_tables[val & 0x3];

	p.increment = val & 0x4 ? 32 : 1;
	p.tiles = !!(val & 0x10) * 0x1000;
	p.vblank_nmi = !!(val & 0x80);
}

/* PPU register 2001 */
void ppu_write_reg2(unsigned int val)
{
	p.bg = !!(val & 0x8);
}

/* PPU register 2006 */
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

unsigned int ppu_read_reg2(void)
{
	int t = 0;

	t |= (p.vblank << 7);

	p.vblank = 0;
	p.addr_count = 0;

	return t;
}

static unsigned int ppu_get_palette_entry(unsigned int pnum, unsigned int pindex)
{
	unsigned int addr;

	if(pindex == 0)
	{
		addr = 0x3F00;
	} else {
		addr = 0x3F00 + (pnum*4) + pindex;
	}

	return addr;
}



void init_ppu(void)
{
	init_sdl();
	printf("SDL OK\n");
	p.scanline = 0;
	p.addr_count = 0;
	p.vblank = 0;

	ppu_mem = calloc(0x4000, 1);
}

static void ppu_draw_tile(unsigned char *b, unsigned int ty, unsigned int tx, unsigned int tile, unsigned int pnum)
{
	unsigned int y, x, px, py, tiledata;
	unsigned char *out;
	unsigned int pindexlo[8], pindexhi[8]; /* 8 palette entries for a row of a tile */

	tiledata = p.tiles + tile*16;

	for(y = 0; y < 8; y++){
		/* First 8 bytes of tile data contain the lower order bit of the
		 * palette index for 64 pixels. The second 8 bytes contain the
		 * high order bit. So to find a palette index for a tile: combine
		 * the byte number y and y+8's xth bit. For example, y=3, x=2
		 * you would take byte 3 and 11, and take bit 2 of each byte.
		 */
		pindexlo[7] = (!!(ppu_readb_raw(tiledata+y+0) & 0x01));
		pindexhi[7] = (!!(ppu_readb_raw(tiledata+y+8) & 0x01))<<1;

		pindexlo[6] = (!!(ppu_readb_raw(tiledata+y+0) & 0x02));
		pindexhi[6] = (!!(ppu_readb_raw(tiledata+y+8) & 0x02))<<1;

		pindexlo[5] = (!!(ppu_readb_raw(tiledata+y+0) & 0x04));
		pindexhi[5] = (!!(ppu_readb_raw(tiledata+y+8) & 0x04))<<1;

		pindexlo[4] = (!!(ppu_readb_raw(tiledata+y+0) & 0x08));
		pindexhi[4] = (!!(ppu_readb_raw(tiledata+y+8) & 0x08))<<1;

		pindexlo[3] = (!!(ppu_readb_raw(tiledata+y+0) & 0x10));
		pindexhi[3] = (!!(ppu_readb_raw(tiledata+y+8) & 0x10))<<1;

		pindexlo[2] = (!!(ppu_readb_raw(tiledata+y+0) & 0x20));
		pindexhi[2] = (!!(ppu_readb_raw(tiledata+y+8) & 0x20))<<1;

		pindexlo[1] = (!!(ppu_readb_raw(tiledata+y+0) & 0x40));
		pindexhi[1] = (!!(ppu_readb_raw(tiledata+y+8) & 0x40))<<1;

		pindexlo[0] = (!!(ppu_readb_raw(tiledata+y+0) & 0x80));
		pindexhi[0] = (!!(ppu_readb_raw(tiledata+y+8) & 0x80))<<1;

		/* Calculate what pixel on screen this represents */
		py = 256 * ((ty * 8) + y);

		for(x = 0; x < 8; x++){
			unsigned int red, green, blue;
			int paddr, pbyte;
			/* Convert from a palette index (0-3) and a palette number
			 * into a palette entry.
			 */
			paddr = ppu_get_palette_entry(pnum, pindexhi[x] | pindexlo[x]);
			pbyte = ppu_readb_raw(paddr);

			/* Convert NES colour value to an RGB value using a LUT. */
			red   = palette[pbyte][0];
			green = palette[pbyte][1];
			blue  = palette[pbyte][2];

			/* Calc offset into the output texture to write our RBG triplet */
			px = (tx * 8) + x;
			out = b + (px + py) * 3; /* 3 bytes per pixel */

			out[2] = red;
			out[1] = green;
			out[0] = blue;
		}
	}
}

static unsigned int palette_for_tile(unsigned int x, unsigned int y, unsigned int attraddr)
{
	int atx, aty, atbyte, subx, suby, index;
	int mask[] = {0x03, 0x0C, 0x30, 0xC0};
	int shift[] = {0, 2, 4, 6};

	aty = y/4;
	atx = x/4;

	/* Which byte in the attribute table our x and y use */
	atbyte = ppu_readb_raw(attraddr + (aty * 8) + atx);

	/* Now we find out which quarter of the attribute byte our x and y use
	 * by taking bit1 of it.
	 */

	subx = (x&2)>>1;
	suby = (y&2)>>1;

	/* Convert our subx/suby into a unique index */
	index = suby * 2 + subx;

	/* Use the index to find the mask and shift values to extract the
	 * palette from the attribute table.
	 */
	return (atbyte & mask[index]) >> shift[index];
}

static void ppu_draw_frame(unsigned char *b)
{
	int y, x;

	for(y = 0; y < 30; y++){
		for(x = 0; x < 32; x++){
			unsigned int tile, palette;

			tile = ppu_readb_raw(p.nametable + (y*32+x));

			palette = palette_for_tile(x, y, p.nametable + 0x3C0);
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
