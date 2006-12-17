// image.h

#ifndef _IMAGE_H_
#define _IMAGE_H_

#include "color.h"

struct Image {
	int x, y;
	Color *pixels;
};

void blit(Image *dst, int xpos, int ypos, const Image *src);
void blit_ckey(Image *dst, int xpos, int ypos, const Image *src, const Color &key);
void blit_hack(Image *dst, int xpos, int ypos, const Image *src, const Color &key, int skip_lines);
void blend(Image *dst, int xpos, int ypos, const Image *src, float t);
void radial_blur(Image *img, int cx, int cy, float intensity, float depth);

bool load_image(Image *img, const char *fname);
bool save_image(const Image *img, const char *fname);

#endif // ndef _IMAGE_H_
