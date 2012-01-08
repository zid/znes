#include "mapper.h"
#include "rom.h"

#include "nrom.h"
#include "mmc.h"

void init_mapper(void)
{
	int mapper = rom_get_mapper();

	switch(mapper)
	{
		case 0x00:
			init_nrom();
		break;
		case 0x01:
			init_mmc1();
		break;
	}
}
