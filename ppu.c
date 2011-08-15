#include "ppu.h"
#include "cpu.h"

struct PPU {
	int vblank_nmi;
	int frame;
};

static struct PPU p;

void ppu_disable_nmis(void)
{
	p.vblank_nmi = 0;
}

void init_ppu(void)
{
	p.frame = 0;
}

void ppu_enable_nmis(void)
{
	p.vblank_nmi = 1;
}

static void ppu_vblank_starts(void)
{
	if(p.vblank_nmi)
		cpu_nmi(0xFFFA);
}

void ppu(unsigned int cycles)
{
	int frame;

	frame = cycles / 113.6f;
	frame %= 262;
	if(frame == 242 && p.frame == 241)
		ppu_vblank_starts();
	p.frame = frame;
}
