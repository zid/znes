#include <SDL/SDL.h>
#include "main.h"
#include "sdl.h"

static SDL_Surface *screen;

void init_sdl(void)
{
	SDL_Init(SDL_INIT_VIDEO);
	screen = SDL_SetVideoMode(640, 480, 0, SDL_HWSURFACE | SDL_DOUBLEBUF);
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
	/* Called from the PPU at the start of vblank */
	SDL_Flip(screen);
}
