#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include "softy.h"

// parts
#include "p_tunnel.h"
#include "p_eclipse.h"

bool init();
void redraw();
void handle_event(SDL_Event *event);
void cleanup();

SDL_Surface *fbsurf;
Color *fb;
Image *fbimg;

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
	fbimg = new Image;
	fbimg->x = 640;
	fbimg->y = 480;

	if(!tunnel_init()) return false;
	if(!eclipse_init()) return false;

	return true;
}

void redraw()
{
	unsigned int msec = SDL_GetTicks();

	SDL_FillRect(fbsurf, 0, 0);
	if(SDL_MUSTLOCK(fbsurf)) SDL_LockSurface(fbsurf);
	fbimg->pixels = fb = (Color*)fbsurf->pixels;
	
	// --- call any part functions ---
	//tunnel_render(msec / 1000.0f);
	if(msec >= S_ECLIPSE && msec < E_ECLIPSE) {
		eclipse_run(msec);
	}

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
