#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include "perlin.h"
#include "softy.h"

#define IMG_X	480
#define IMG_Y	480

static Image foo;

bool eclipse_init(void)
{
	if(!load_image(&foo, "foo.ppm")) {
		return false;
	}
	return true;
}

void eclipse_run(unsigned int msec)
{
	int x, y;
	SDL_GetMouseState(&x, &y);

	blit(fbimg, x - foo.x / 2, y - foo.y / 2, &foo);
}
