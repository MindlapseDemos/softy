#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "image.h"

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
