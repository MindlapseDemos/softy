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
	radial_blur(&dst, 320 + 100 * sin(secs*2), 240 + 100 * cos(secs*2),
		 0.8f, 25.0f);//  + 4 * sin(secs * 13));
}
