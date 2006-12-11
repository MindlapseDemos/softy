#include <stdio.h>
#include <stdlib.h>
#include <SDL/SDL.h>
#include "color.h"

// svn comment

bool init();
void redraw();
void handle_event(SDL_Event *event);

SDL_Surface *fb;
bool quit = false;

int main(int argc, char **argv)
{
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
	if(!(fb = SDL_SetVideoMode(640, 480, 32, SDL_SWSURFACE))) {
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
	SDL_FillRect(fb, 0, 0xff0000);
	SDL_Flip(fb);
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
