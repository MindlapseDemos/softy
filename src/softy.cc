#include <stdio.h>
#include <stdlib.h>
#include <SDL/SDL.h>
#include "color.h"
#include "p_tunnel.h"

bool init();
void redraw();
void handle_event(SDL_Event *event);
void cleanup();

SDL_Surface *fbsurf;
Color *fb;
bool quit = false;

int main(int argc, char **argv)
{
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
	if(!(fbsurf = SDL_SetVideoMode(640, 480, 32, SDL_SWSURFACE))) {
		fprintf(stderr, "failed to init video\n");
		return EXIT_FAILURE;
	}
	SDL_WM_SetCaption("MLFC demo", 0);

	if(!init()) {
		SDL_Quit();
		return EXIT_FAILURE;
	}

	for(;;) {
		SDL_Event event;
		while(SDL_PollEvent(&event)) {
			handle_event(&event);
		}
		
		redraw();
	}

	cleanup();

	return 0;
}

bool init()
{
	tunnel_init();

	return true;
}


void redraw()
{
	SDL_FillRect(fbsurf, 0, 0);
	if(SDL_MUSTLOCK(fbsurf)) SDL_LockSurface(fbsurf);
	fb = (Color*)fbsurf->pixels;
	
	// call any part functions
	tunnel_render(SDL_GetTicks() / 1000.0f);


	if(SDL_MUSTLOCK(fbsurf)) SDL_UnlockSurface(fbsurf);
	SDL_Flip(fbsurf);
}

void handle_event(SDL_Event *event)
{
	switch(event->type) {
	case SDL_KEYDOWN:
		if(event->key.keysym.sym == SDLK_ESCAPE) {
			SDL_Quit();
			exit(0);
		}
		break;

	default:
		break;
	}
}

void cleanup()
{
	tunnel_cleanup();
}
