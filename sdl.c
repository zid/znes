#include <SDL/SDL.h>
#include "main.h"
#include "sdl.h"

static SDL_Surface *screen, *backbuf;

void init_sdl(void)
{
	SDL_Init(SDL_INIT_VIDEO);
	screen = SDL_SetVideoMode(256, 240, 0, SDL_HWSURFACE | SDL_DOUBLEBUF);
	backbuf = SDL_CreateRGBSurface(SDL_SWSURFACE, 256, 240, 24, 0xFF0000, 0xFF00, 0xFF, 0);
}
unsigned int sdl_get_buttons(void)
{
	Uint8 *keys;
	int nkeys;
	unsigned int buttons = 0;

	SDL_PumpEvents();
	keys = SDL_GetKeyState(&nkeys);

	if(keys[SDLK_a])
		buttons |= 0x1;
	if(keys[SDLK_s])
		buttons |= 0x2;
	if(keys[SDLK_d])
		buttons |= 0x4;
	if(keys[SDLK_f])
		buttons |= 0x8;
	if(keys[SDLK_UP])
		buttons |= 0x10;
	if(keys[SDLK_DOWN])
		buttons |= 0x20;
	if(keys[SDLK_LEFT])
		buttons |= 0x40;
	if(keys[SDLK_RIGHT])
		buttons |= 0x80;

	return buttons;
}

unsigned char *sdl_get_buffer(void)
{
	return backbuf->pixels;
}

int sdl_update(void)
{
	SDL_Event ev;

	while(SDL_PollEvent(&ev))
	{
		if(ev.type == SDL_QUIT){
			main_quit();
			/* Although main has been informed we need to quit,
			 * we can return false here to skip the rest of this frame.
			 */
			return 0;   
		}
	}

	return 1;
}

void sdl_frame(void)
{
	SDL_BlitSurface(backbuf, NULL, screen, NULL);
	SDL_Flip(screen);
}
