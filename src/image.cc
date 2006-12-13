#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "image.h"
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
  float xhalf = 0.5f*x;
  int i = *(int*)&x; // get bits for floating value
  i = 0x5f375a86- (i>>1); // gives initial guess y0
  x = *(float*)&i; // convert bits back to float
  x = x*(1.5f-xhalf*x*x); // Newton step, repeating increases accuracy
  return x;
}

inline Color rb(int x, int y, int cx, int cy, int w, int h, int intensity, float depth,
				float inv_depth, Color *pixels)
{
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
		invd = sqrtf(len2);
		depth = 1.0f / invd;
	}
	
	difx *= invd;
	dify *= invd;
	
	px += difx * depth;
	py += dify * depth;

	int nx, ny;
	nx = px + 0.5f;
	ny = py + 0.5f;
	
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

	int inten = 255 * intensity;
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

static int read_to_wspace(FILE *fp, char *buf, int bsize);

bool load_image(Image *img, const char *fname)
{
	FILE *fp;
	char buf[64];
	int bytes, raw;
	unsigned int w, h, i, sz;
	unsigned int *pixels;
	
	if(!(fp = fopen(fname, "rb"))) {
		fprintf(stderr, "load_ppm(%s): ", fname);
		perror("open failed");
		return 0;
	}
	
	bytes = read_to_wspace(fp, buf, 64);
	if(buf[0] != 'P' || (buf[1] != '6' && buf[1] != '3')) {
		fprintf(stderr, "%s is not a ppm file\n", fname);
		fclose(fp);
		return 0;
	}
	raw = buf[1] == '6';

	if((bytes = read_to_wspace(fp, buf, 64)) == 0) {
		fclose(fp);
		return false;
	}
	if(!isdigit(*buf)) {
		fprintf(stderr, "load_ppm: invalid width: %s\n", buf);
		fclose(fp);
		return false;
	}
	w = atoi(buf);

	if((bytes = read_to_wspace(fp, buf, 64)) == 0) {
		fclose(fp);
		return false;
	}
	if(!isdigit(*buf)) {
		fprintf(stderr, "load_ppm: invalid height: %s\n", buf);
		fclose(fp);
		return false;
	}
	h = atoi(buf);

	if((bytes = read_to_wspace(fp, buf, 64)) == 0) {
		fclose(fp);
		return false;
	}
	if(!isdigit(*buf) || atoi(buf) != 255) {
		fprintf(stderr, "load_ppm: invalid or unsupported max value: %s\n", buf);
		fclose(fp);
		return false;
	}

	if(!(pixels = (unsigned int*)malloc(w * h * sizeof *pixels))) {
		fprintf(stderr, "load_ppm: malloc failed\n");
		fclose(fp);
		return false;
	}

	sz = h * w;
	for(i=0; i<sz; i++) {
		int r = fgetc(fp);
		int g = fgetc(fp);
		int b = fgetc(fp);

		if(r == -1 || g == -1 || b == -1) {
			free(pixels);
			fclose(fp);
			fprintf(stderr, "load_ppm: EOF while reading pixel data\n");
			return false;
		}
		pixels[i] = (r << 16) | (g << 8) | b;
	}
	fclose(fp);

	try {
		img->pixels = new Color[w * h];
		memcpy(img->pixels, pixels, w * h * sizeof *pixels);
		img->x = w;
		img->y = h;
	}
	catch(...) {
		free(pixels);
		return false;
	}

	free(pixels);
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


static int read_to_wspace(FILE *fp, char *buf, int bsize)
{
	int c, count = 0;
	
	while((c = fgetc(fp)) != -1 && !isspace(c) && count < bsize - 1) {
		if(c == '#') {
			while((c = fgetc(fp)) != -1 && c != '\n' && c != '\r');
			c = fgetc(fp);
			if(c == '\n' || c == '\r') continue;
		}
		*buf++ = c;
		count++;
	}
	*buf = 0;
	
	while((c = fgetc(fp)) != -1 && isspace(c));
	ungetc(c, fp);
	return count;
}
