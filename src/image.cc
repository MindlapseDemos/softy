#include "image.h"

void blit(Image *dst, int xpos, int ypos, const Image *src)
{
	const Color *sptr = src->pixels;
	Color *dptr = dst->pixels + ypos * dst->x;

	for(int j=0; j<src->y; j++) {
	}
}
