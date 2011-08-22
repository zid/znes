#include <stdlib.h>
#include <stdio.h>
#include "ppu.h"
#include "cpu.h"
#include "sdl.h"

struct PPU {
	int vblank_nmi;
	int scanline;
	unsigned char *mem;
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
	p.nametable = &p.mem[name_tables[val&0x3]];

	p.increment = val & 0x4 ? 32 : 1;
	p.tiles = val & 0x10 ? &p.mem[0x1000] : &p.mem[0x0000];
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

void ppu_write_data(unsigned int val)
{
	if(p.addr == 0x2000)
	{
		printf("Written %02X to 0x2000\n", val);
	}
	p.mem[p.addr] = val;
	p.addr += p.increment;
}

void init_ppu(void)
{
	init_sdl();
	p.scanline = 0;
	p.addr_count = 0;
	p.vblank = 0;
	p.mem = calloc(1, 0x4000);
}

/* Test palette! */
static int paletter[4] = {0x00, 0x50, 0xA0, 0xFF};
static int paletteg[4] = {0x00, 0x50, 0xA0, 0xFF};
static int paletteb[4] = {0x00, 0x50, 0xA0, 0xFF};

static void ppu_draw_tile(unsigned char *b, unsigned int ty, unsigned int tx, unsigned int tile)
{
	unsigned int y, x, px, py;
	unsigned char *tiledata, *out;
	unsigned int pindex[8]; /* 8 palette entries for a row of a tile */

//	printf("Tile: %02X\n", tile);
	tiledata = &p.tiles[tile*16];

	for(y = 0; y < 8; y++){
		/* First 8 bytes of tile data contain the lower order bit of the
		 * palette index for 64 pixels. The second 8 bytes contain the
		 * high order bit. So to find a palette index for a tile: combine
		 * the byte number y and y+8's xth bit. For example, y=3, x=2
		 * you would take byte 3 and 11, and take bit 2 of each byte.
		 */
		pindex[7] = !!(tiledata[y] & 0x01) | (!!(tiledata[y+8] & 0x01))<<1;
		pindex[6] = !!(tiledata[y] & 0x02) | (!!(tiledata[y+8] & 0x02))<<1;
		pindex[5] = !!(tiledata[y] & 0x04) | (!!(tiledata[y+8] & 0x04))<<1;
		pindex[4] = !!(tiledata[y] & 0x08) | (!!(tiledata[y+8] & 0x08))<<1;
		pindex[3] = !!(tiledata[y] & 0x10) | (!!(tiledata[y+8] & 0x10))<<1;
		pindex[2] = !!(tiledata[y] & 0x20) | (!!(tiledata[y+8] & 0x20))<<1;
		pindex[1] = !!(tiledata[y] & 0x40) | (!!(tiledata[y+8] & 0x40))<<1;
		pindex[0] = !!(tiledata[y] & 0x80) | (!!(tiledata[y+8] & 0x80))<<1;

		/* Calculate what pixel on screen this represents */
		py = 256 * ((ty * 8) + y);

		for(x = 0; x < 8; x++){
			px = (tx * 8) + x;
			out = b + (px + py) * 3; /* 3 bytes per pixel, onscreen */

			if(pindex[x] > 3){ printf("Pallete calculation disorder.\n"); exit(0);}
			out[0] = paletter[pindex[x]];
			out[1] = paletteg[pindex[x]];
			out[2] = paletteb[pindex[x]];

		}
	}
}

static void ppu_draw_frame(unsigned char *b)
{
	int y, x;

	for(y = 0; y < 30; y++){
		for(x = 0; x < 32; x++){
			unsigned int tile;

			FILE *f = fopen("nametable.bin", "ab");
			fwrite(p.nametable, 0x3C0, 1, f);
			fclose(f);
			tile = p.nametable[y*32+x];
			ppu_draw_tile(b, y, x, tile);
		}
	}

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
