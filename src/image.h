// image.h

#ifndef _IMAGE_H_
#define _IMAGE_H_

#include "color.h"

struct Image {
	int x, y;
	Color *pixels;
};

void blit(Image *dst, int xpos, int ypos, const Image *src);

bool load_image(Image *img, const char *fname);
bool save_image(const Image *img, const char *fname);

#endif // ndef _IMAGE_H_
