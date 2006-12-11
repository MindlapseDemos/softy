#include <stdio.h>
#include <stdlib.h>
#include <SDL/SDL.h>
#include "color.h"

bool init();
void redraw();
void handle_event(SDL_Event *event);

SDL_Surface *fbsurf;
unsigned int *fb;
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
	return 0;
}

bool init()
{
	return true;
}


void redraw()
{
	SDL_FillRect(fbsurf, 0, 0xff0000);
	if(SDL_MUSTLOCK(fbsurf)) SDL_LockSurface(fbsurf);
	fb = (unsigned int*)fbsurf->pixels;
	
	// call any part functions
	
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
