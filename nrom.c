#include <stdlib.h>
#include "nrom.h"
#include "rom.h"
#include "mem.h"
#include "cpu.h"
#include "ppu.h"

void init_nrom(void)
{
	cpu_set_writeb(NULL);
	cpu_set_readb(NULL);

	ppu_set_writeb(NULL);
	ppu_set_readb(NULL);

	mem_set_bank(0x0, calloc(0x1000, 1));
	mem_set_bank(0x1, calloc(0x1000, 1));
	mem_set_bank(0x2, calloc(0x1000, 1));
	mem_set_bank(0x3, calloc(0x1000, 1));
	mem_set_bank(0x4, calloc(0x1000, 1));
	mem_set_bank(0x5, calloc(0x1000, 1));
	mem_set_bank(0x6, calloc(0x1000, 1));
	mem_set_bank(0x7, calloc(0x1000, 1));

	mem_set_bank(0x8, rom_get_bank(0, 0));
	mem_set_bank(0x9, rom_get_bank(0, 1));
	mem_set_bank(0xA, rom_get_bank(0, 2));
	mem_set_bank(0xB, rom_get_bank(0, 3));

	mem_set_bank(0xC, rom_get_bank(-1, 0));
	mem_set_bank(0xD, rom_get_bank(-1, 1));
	mem_set_bank(0xE, rom_get_bank(-1, 2));
	mem_set_bank(0xF, rom_get_bank(-1, 3));

	ppu_set_bank_rom(0x0, rom_get_chr(0));
	ppu_set_bank_rom(0x1, rom_get_chr(1));
	ppu_set_bank_rom(0x2, rom_get_chr(2));
	ppu_set_bank_rom(0x3, rom_get_chr(3));
	ppu_set_bank_rom(0x4, rom_get_chr(4));
	ppu_set_bank_rom(0x5, rom_get_chr(5));
	ppu_set_bank_rom(0x6, rom_get_chr(6));
	ppu_set_bank_rom(0x7, rom_get_chr(7));

	ppu_set_bank(0x8, 0x2000);
	ppu_set_bank(0x9, 0x2000);
	ppu_set_bank(0xA, 0x2400);
	ppu_set_bank(0xB, 0x2400);
	ppu_set_bank(0xC, 0x3000);
	ppu_set_bank(0xD, 0x3400);
	ppu_set_bank(0xE, 0x3800);
	ppu_set_bank(0xF, 0x3C00);
}
