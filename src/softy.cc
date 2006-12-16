#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include <SDL_thread.h>
#include <mikmod.h>
#include <gl.h>
#include "softy.h"
#include "demo.h"
#include "demoscript.h"

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
int music_func(void *data);

SDL_Surface *fbsurf;
Image *fbimg;
MODULE *mod;
unsigned int music_volume = 128;

unsigned long start_time;
bool music = true;	/* TODO: change this to true! */

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

			case 'm':
				music = !music;
				break;

			default:
				fprintf(stderr, "unrecognized argument: %s\n", argv[i]);
				return EXIT_FAILURE;
			}
		}
	}

	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
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

	// demoscript
	//set_part_param("tunnel", 5000, 2);
	//add_part_inst("eclipse", 0, 21032141, true);
	//add_part_inst("amiga", 2000, 5000, true);
	//add_part_inst("tunnel", 0, 2500, true);
	//add_part_inst("radial", 4500, 10000, true);
	//add_part_inst("slimy", 7000, 15000, true);
	if (!process_demo_script("demoscript"))
	{
		printf("ERROR: bad demoscript\n");
		return false;
	}

	// call this after demoscript
	if (!init_demo()) return false;

	if(music) {
		MikMod_RegisterAllDrivers();
		MikMod_RegisterAllLoaders();

		md_mode |= DMODE_SOFT_MUSIC;
		if(MikMod_Init("") != 0) {
			fprintf(stderr, "mikmod init failed: %s\n", MikMod_strerror(MikMod_errno));
			fprintf(stderr, "to run without music use -m\n");
			return false;
		}
		atexit(MikMod_Exit);

		if(!(mod = Player_Load("data/music.mod", 4, 0))) {
			fprintf(stderr, "failed to load %s: %s\n", "data/music.mod", MikMod_strerror(MikMod_errno));
			fprintf(stderr, "to run without music use -m\n");
			return false;
		}
		Player_Start(mod);
		// start music thread
		SDL_CreateThread(music_func, 0);
	}

	start_time = SDL_GetTicks();
	
	return true;
}

// frame interval = 1000 / fps
#define FRAME_INTERVAL		5

#define SHOW_FPS
void redraw()
{
	static unsigned int prev_frame = -100;
	unsigned int msec = SDL_GetTicks() - start_time;

	if(msec - prev_frame < FRAME_INTERVAL) {
		//return;
	}
	prev_frame = msec;

	run_demo(msec);

#ifdef SHOW_FPS
	static unsigned int prev_fps_msec = 0;
	static int frames;
	if(msec - prev_fps_msec >= 2000) {
		float fps = (float)frames / ((float)(msec - prev_fps_msec) / 1000.0);
		printf("\rfps: %.2f    ", fps);
		fflush(stdout);
		frames = 0;
		prev_fps_msec = msec;
	} else {
		frames++;
	}
#endif

}

void handle_event(SDL_Event *event)
{
	static int esc_pressed;
	static int repl = 1;
	switch(event->type) {
	case SDL_KEYDOWN:
		if(event->key.keysym.sym == SDLK_ESCAPE) {
			//SDL_Quit();
			//exit(0);
			if(!esc_pressed) {
				end_demo_at(SDL_GetTicks() - start_time + 2000);
				esc_pressed = 1;
			}
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

int music_func(void *data)
{
	while (true)
	{
		if(mod) 
		{
			Player_SetVolume(music_volume);
			MikMod_Update();
		}
		SDL_Delay(5);
	}
}
