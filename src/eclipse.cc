#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <SDL.h>
#include "perlin.h"
#include "softy.h"

#define FLAME_FRAMES	128

static Image grad;
static Image flame[FLAME_FRAMES];

float sum_abs_noise(float x, float y, float z, int factors)
{
	double vec[] = {x, y, z};
	float mul = 1.0;
	float sum = 0.0;

	for(int i=0; i<factors; i++) {
		sum += fabs(noise3(vec)) * mul;
		mul *= 0.5;
		vec[0] *= 2.0;
		vec[1] *= 2.0;
		vec[2] *= 2.0;
	}

	return sum;
}

#define MIN(a, b)	((a) < (b) ? (a) : (b))
#define MAX(a, b)	((a) > (b) ? (a) : (b))

#define DISC_RAD	0.5

bool eclipse_init(void)
{
	if(!load_image(&grad, "data/ecl_grad.ppm")) {
		return false;
	}

	for(int k=0; k<FLAME_FRAMES; k++) {
		float scale = k * 0.1;
		flame[k].x = flame[k].y = 400;
		flame[k].pixels = new Color[flame[k].x * flame[k].y];

		Color *ptr = flame[k].pixels;
		for(int j=0; j<flame[0].y; j++) {
			for(int i=0; i<flame[0].x; i++) {
				float dx = (float)(i - flame[0].x / 2) / (float)(flame[0].x / 2);
				float dy = (float)(j - flame[0].y / 2) / (float)(flame[0].y / 2);
				float len = sqrt(dx * dx + dy * dy);

				if(len < DISC_RAD) {
					ptr->packed = 0;
				} else {
					float x = dx * 2.0;
					float y = dy * 2.0;

					int gx = (int)(grad.x * (len - 0.05 + sum_abs_noise(x, y, k * 0.045, 6) * 0.5));

					gx = MAX(MIN(gx, grad.x), 0);
					ptr->c.r = grad.pixels[gx].c.r;
					ptr->c.g = grad.pixels[gx].c.g;
					ptr->c.b = grad.pixels[gx].c.b;
				}
				ptr++;
			}
		}
	}

	return true;
}

static float pingpong(float x)
{
	return (((int)x % 2 == 0) ? (x - floor(x)) : 1.0f - (x - floor(x)));
}

void eclipse_run(unsigned int msec, int param)
{
	memset(fb, 0, fbimg->x * fbimg->y * 4);

	int x = (fbimg->x - flame[0].x) / 2;
	int y = (fbimg->y - flame[0].y) / 2;

	float t = fmod((float)msec / 20000.0, 1.0);

	int first = (int)(t * (float)FLAME_FRAMES);
	int second = (first + 1) % FLAME_FRAMES;

	float t_first = (float)first / (float)FLAME_FRAMES;
	float t_sec = (float)second / (float)FLAME_FRAMES;
	float t_inter = (t - t_first) / (t_sec - t_first);

	blit(fbimg, x, y, &flame[first]);
	blend(fbimg, x, y, &flame[second], t_inter);
}
