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
