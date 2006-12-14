#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "image.h"
#include "image_ppm.h"
#include <vmath.h>

#define MIN(a, b)	((a) < (b) ? (a) : (b))
#define MAX(a, b)	((a) > (b) ? (a) : (b))

void blit(Image *dst, int xpos, int ypos, const Image *src)
{
	if(xpos >= dst->x || ypos >= dst->y) return;

	int sx = MAX(xpos, 0);
	int sy = MAX(ypos, 0);

	int xlen = MIN(src->x - (sx - xpos), dst->x - sx);
	int ylen = MIN(src->y - (sy - ypos), dst->y - sy);

	const Color *sptr = src->pixels + (sy - ypos) * src->x + (sx - xpos);
	Color *dptr = dst->pixels + sy * dst->x + sx;

	for(int j=0; j<ylen; j++) {
		memcpy(dptr, sptr, xlen * sizeof *dptr);
		dptr += dst->x;
		sptr += src->x;
	}
}

void blit_ckey(Image *dst, int xpos, int ypos, const Image *src, const Color &key)
{
	if(xpos >= dst->x || ypos >= dst->y) return;

	int sx = MAX(xpos, 0);
	int sy = MAX(ypos, 0);

	int xlen = MIN(src->x - (sx - xpos), dst->x - sx);
	int ylen = MIN(src->y - (sy - ypos), dst->y - sy);

	const Color *sptr = src->pixels + (sy - ypos) * src->x + (sx - xpos);
	Color *dptr = dst->pixels + sy * dst->x + sx;

	for(int j=0; j<ylen; j++) {
		for(int i=0; i<xlen; i++) {
			if(sptr->packed != key.packed) {
				*dptr = *sptr;
			}
			dptr++;
			sptr++;
		}
		dptr += dst->x - xlen;
		sptr += src->x - xlen;
	}
}

void blend(Image *dst, int xpos, int ypos, const Image *src, float t)
{
	if(xpos >= dst->x || ypos >= dst->y) return;

	int fact = (int)(t * 255.0f);
	int inv_fact = 256 - fact;

	int sx = MAX(xpos, 0);
	int sy = MAX(ypos, 0);

	int xlen = MIN(src->x - (sx - xpos), dst->x - sx);
	int ylen = MIN(src->y - (sy - ypos), dst->y - sy);

	const Color *sptr = src->pixels + (sy - ypos) * src->x + (sx - xpos);
	Color *dptr = dst->pixels + sy * dst->x + sx;

	for(int j=0; j<ylen; j++) {
		for(int i=0; i<xlen; i++) {
			dptr->c.r = (sptr->c.r * fact + dptr->c.r * inv_fact) / 256;
			dptr->c.g = (sptr->c.g * fact + dptr->c.g * inv_fact) / 256;
			dptr->c.b = (sptr->c.b * fact + dptr->c.b * inv_fact) / 256;
			dptr++;
			sptr++;
		}
		dptr += dst->x - xlen;
		sptr += src->x - xlen;
	}
}

inline float InvSqrt(float x)
{
	/*
	float xhalf = 0.5f*x;
	nt i = *(int*)&x; // get bits for floating value
	i = 0x5f375a86- (i>>1); // gives initial guess y0
	x = *(float*)&i; // convert bits back to float
	x = x*(1.5f-xhalf*x*x); // Newton step, repeating increases accuracy
	return x;
	*/

	return 1.0f / sqrt(x);
}

inline Color rb(int x, int y, int cx, int cy, int w, int h, int intensity, float depth,
				float inv_depth, Color *pixels)
{
	if (x == cx && y == cy)
	{
		return pixels[x + w * y];	
	}
	static int nx, ny;
	static float px, py, ctx, cty, difx, dify, invd, len2;
	//static Vector2 p, center, dif;
	
	px = x;
	py = y;
	
	ctx = cx;
	cty = cy;
	
	difx = ctx - px;
	dify = cty - py;

	len2 = difx*difx + dify*dify;
	invd = InvSqrt(len2);
		
	// rare case
	if (inv_depth < invd)
	{
		invd = sqrt(len2);
		depth = 1.0f / invd;
	}
	
	difx *= invd;
	dify *= invd;
	
	px += difx * depth;
	py += dify * depth;

	nx = (int)(px + 0.5f);
	ny = (int)(py + 0.5f);
	
	Color orig = pixels[x + w * y];
	Color blend = pixels[nx + w * ny];
	return Lerp(orig, blend, intensity);
}

void radial_blur(Image *img, int cx, int cy, float intensity, float depth)
{
	if (!img) return;
	if (cx < 0) cx = 0;
	if (cy < 0) cy = 0;
	if (cx >= img->x) cx = img->x - 1;
	if (cy >= img->y) cy = img->y - 1;

	int inten = (int)(255.0 * intensity);
	float inv_depth = 1.0f / depth;

	for (int j=cy; j>=0; j--)
	{
		for (int i=cx; i>=0; i--)
		{
			img->pixels[i + img->x * j] = rb(i, j, cx, cy, img->x, img->y, inten, depth,
				inv_depth, img->pixels);
		}
	}
	for (int j=cy+1; j<img->y; j++)
	{
		for (int i=cx; i>=0; i--)
		{
			img->pixels[i + img->x * j] = rb(i, j, cx, cy, img->x, img->y, inten, depth,
				inv_depth, img->pixels);
		}
	}
	for (int j=cy; j>=0; j--)
	{
		for (int i=cx+1; i<img->x; i++)
		{
			img->pixels[i + img->x * j] = rb(i, j, cx, cy, img->x, img->y, inten, depth,
				inv_depth, img->pixels);
		}
	}
	for (int j=cy+1; j<img->y; j++)
	{
		for (int i=cx+1; i<img->x; i++)
		{
			img->pixels[i + img->x * j] = rb(i, j, cx, cy, img->x, img->y, inten, depth,
				inv_depth, img->pixels);
		}
	}
}

bool load_image(Image *img, const char *fname)
{
	int w, h;
	void *pixels = load_ppm(fname, &w, &h);
	if(!pixels) {
		return false;
	}

	try {
		img->pixels = new Color[w * h];
		memcpy(img->pixels, pixels, w * h * 4);
		img->x = w;
		img->y = h;
	}
	catch(...) {
		free_ppm(pixels);
		return false;
	}

	free_ppm(pixels);
	return true;
}

bool save_image(const Image *img, const char *fname)
{
	FILE *fp;

	if(!(fp = fopen(fname, "wb"))) {
		perror("save_image failed");
		return false;
	}

	fprintf(fp, "P6\n%d %d\n255\n", img->x, img->y);
	for(int i=0; i<img->x * img->y; i++) {
		fputc(img->pixels[i].c.r, fp);
		fputc(img->pixels[i].c.g, fp);
		fputc(img->pixels[i].c.b, fp);
	}
	fclose(fp);

	return true;
}
