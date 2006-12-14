#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include <gl.h>
#include "softy.h"
#include "demo.h"

// parts
#include "p_tunnel.h"
#include "p_eclipse.h"
#include "p_radial.h"
#include "p_slimy.h"
#include "p_amiga.h"

bool init();
void redraw();
void handle_event(SDL_Event *event);
void cleanup();

SDL_Surface *fbsurf;
Image *fbimg;

unsigned long start_time;

int main(int argc, char **argv)
{
	bool fullscreen = false;

	for(int i=1; i<argc; i++) {
		if(argv[i][0] == '-' && argv[i][2] == 0) {
			switch(argv[i][1]) {
			case 'f':
				fullscreen = true;
				break;

			case 'w':
				fullscreen = false;
				break;

			default:
				fprintf(stderr, "unrecognized argument: %s\n", argv[i]);
				return EXIT_FAILURE;
			}
		}
	}

	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
	if(!(fbsurf = SDL_SetVideoMode(640, 480, 32, SDL_SWSURFACE | (fullscreen ? SDL_FULLSCREEN : 0)))) {
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

bool slimy_init_wrapper() {return slimy_init() != -1;}
bool amiga_init_wrapper() {return amiga_init() != -1;}

bool init()
{	
	fbimg = new Image;
	fbimg->x = 640;
	fbimg->y = 480;

	fglCreateContext();

	// add parts to demo system
	add_part(Part(slimy_init_wrapper, slimy_run), "slimy");
	add_part(Part(tunnel_init, tunnel_run), "tunnel");
	add_part(Part(radial_init, radial_run), "radial");
	add_part(Part(amiga_init_wrapper, amiga_run), "amiga");
	//add_part(Part(eclipse_init, eclipse_run), "eclipse");
	if (!init_demo()) return false;

	// demoscript
	//add_part_inst("eclipse", 0, 21032141, true);
	add_part_inst("amiga", 2000, 321432, true);
	//add_part_inst("tunnel", 0, 5000, true);
	//add_part_inst("radial", 0, 10000, true);
	//add_part_inst("tunnel", 7000, 15000, true);


	//if(!tunnel_init()) return false;
	//if(!eclipse_init()) return false;
	//if (!radial_init()) return false;

	start_time = SDL_GetTicks();

	return true;
}

void redraw()
{
	unsigned int msec = SDL_GetTicks() - start_time;
	run_demo(msec);
}

void handle_event(SDL_Event *event)
{
	static int repl = 1;
	switch(event->type) {
	case SDL_KEYDOWN:
		if(event->key.keysym.sym == SDLK_ESCAPE) {
			SDL_Quit();
			exit(0);
		}
		break;
	case SDL_KEYUP:
		if(event->key.keysym.sym == SDLK_UP) {
			repl ++;
			TunnelReplica(repl);
		}
		if(event->key.keysym.sym == SDLK_DOWN) {
			if (repl > 1) repl --;
			TunnelReplica(repl);
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
