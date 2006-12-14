#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "image_ppm.h"

static int read_to_wspace(FILE *fp, char *buf, int bsize);

void *load_ppm(const char *fname, int *xsz, int *ysz)
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
		return 0;
	}
	if(!isdigit(*buf)) {
		fprintf(stderr, "load_ppm: invalid width: %s\n", buf);
		fclose(fp);
		return 0;
	}
	w = atoi(buf);

	if((bytes = read_to_wspace(fp, buf, 64)) == 0) {
		fclose(fp);
		return 0;
	}
	if(!isdigit(*buf)) {
		fprintf(stderr, "load_ppm: invalid height: %s\n", buf);
		fclose(fp);
		return 0;
	}
	h = atoi(buf);

	if((bytes = read_to_wspace(fp, buf, 64)) == 0) {
		fclose(fp);
		return 0;
	}
	if(!isdigit(*buf) || atoi(buf) != 255) {
		fprintf(stderr, "load_ppm: invalid or unsupported max value: %s\n", buf);
		fclose(fp);
		return 0;
	}

	if(!(pixels = (unsigned int*)malloc(w * h * sizeof *pixels))) {
		fprintf(stderr, "load_ppm: malloc failed\n");
		fclose(fp);
		return 0;
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
			return 0;
		}
		pixels[i] = (r << 16) | (g << 8) | b;
	}
	fclose(fp);

	if(xsz) *xsz = w;
	if(ysz) *ysz = h;

	return pixels;
}

void free_ppm(void *img)
{
	free(img);
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
