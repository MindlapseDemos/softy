#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include <SDL_thread.h>
#include <gl.h>
#include "softy.h"
#include "demo.h"
#include "demoscript.h"
#include "sdlvf.h"

// parts
#include "p_tunnel.h"
#include "p_eclipse.h"
#include "p_radial.h"
#include "p_slimy.h"
#include "p_amiga.h"
#include "p_glow.h"

bool init();
void redraw();
void handle_event(SDL_Event *event);
void cleanup();
int music_func(void *data);

SDL_Surface *fbsurf;
Image *fbimg;
unsigned int music_volume = 128;

unsigned long start_time;
bool music = false;	/* TODO: change this to true! */

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

	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO);
	if(!(fbsurf = SDL_SetVideoMode(640, 480, 32, SDL_SWSURFACE | (fullscreen ? SDL_FULLSCREEN : 0)))) {
		fprintf(stderr, "failed to init video\n");
		return EXIT_FAILURE;
	}
	SDL_WM_SetCaption("MLFC demo", 0);
	SDL_ShowCursor(false);
	
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
bool glow_init_wrapper() {return glow_init() != -1;}

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
	add_part(Part(glow_init_wrapper, glow_run), "glow");
	//add_part(Part(eclipse_init, eclipse_run), "eclipse");

	//add_slide("data/manga.ppm", 0, 2000, 10000, -100, -100, 100, 100);
	
	if (!process_demo_script("demoscript"))
	{
		printf("ERROR: bad demoscript\n");
		return false;
	}

	// call this after demoscript
	if (!init_demo()) return false;

	if(music) {
		if(sdlvf_init("data/music.ogg") == -1) {
			fprintf(stderr, "failed to load data/music.ogg, use -m to run without music\n");
			return false;
		}
		atexit(sdlvf_done);
		//SDL_CreateThread(music_func, 0);
	}

	start_time = SDL_GetTicks();
	
	return true;
}

// frame interval = 1000 / fps
#define FRAME_INTERVAL		25

#define SHOW_FPS
void redraw()
{
	static unsigned int prev_frame = -100;
	unsigned int msec = SDL_GetTicks() - start_time;

	// exo logo pou ton bazo ton gamo-framelimiter :)
	// to slimy effect einai framerate dependent
	if(msec - prev_frame < FRAME_INTERVAL) {
		return;
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

	if(music)
	{
		sdlvf_check();
		sdlvf_volume(music_volume);
	}
}

void handle_event(SDL_Event *event)
{
	static int esc_pressed;
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

	default:
		break;
	}
}

void cleanup()
{
	tunnel_cleanup();
}

/*
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
*/
