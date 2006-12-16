// p_radial.cc

#include "image.h"
#include <string.h>
#include <math.h>

extern Color *fb;
static Image img;

bool radial_init()
{
	// load the image to apply radial blur to
	if (!load_image(&img, "data/mlfc.ppm")) return false;
	return true;
}
void radial_render(float secs)
{
	Image dst;
	dst.x = 640;
	dst.y = 480;
	dst.pixels = fb;
	
	memcpy(fb, img.pixels, 640 * 480 * 4);
	radial_blur(&dst, 320 + 100 * sin(secs*12), 240 + 100 * cos(secs*5),
		 0.8f, 15.0f + 14 * sin(secs * 8));
}

void radial_run(unsigned int msec, int param)
{
	radial_render(msec / 1000.0f);
}
