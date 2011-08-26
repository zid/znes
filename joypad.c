#include "sdl.h"

static unsigned int reload;
static unsigned int shift;

unsigned int joypad_read(void)
{
	unsigned int buttons, button;

	if(reload)
		shift = 0;

	buttons = sdl_get_buttons();
	button = (buttons >> shift) & 1;
	printf("Returned %d for shift %d\n", button, shift);

	shift++;
	if(shift == 8)
		shift = 0;

	return button;
}

void joypad_write(unsigned int val)
{
	reload = val & 1;
	printf("Shift cleared\n");

	if(reload)
		shift = 0;
}

