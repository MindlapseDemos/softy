// image.h

#ifndef _IMAGE_H_
#define _IMAGE_H_

#include "color.h"

struct Image
{
	int x, y;
	Color *pixels;
};

void blit(Image *dst, int xpos, int ypos, const Image *src);

#endif // ndef _IMAGE_H_
