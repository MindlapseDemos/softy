#ifndef _COLOR_H_
#define _COLOR_H_

union Color {
	unsigned int packed;
	struct {
#ifdef LITTLE_ENDIAN
		unsigned char b, g, r, a;
#else
		unsigned char r, g, b, a;
#endif
	} __attribute__((packed)) c;
};

inline Color Lerp(const Color &c1, const Color &c2, unsigned char p)
{
	unsigned char omp = 255 - p;
	unsigned int r, g, b, a;
	r = (c1.c.r * omp + c2.c.r * p) / 256;
	g = (c1.c.g * omp + c2.c.g * p) / 256;
	b = (c1.c.b * omp + c2.c.b * p) / 256;
	a = (c1.c.a * omp + c2.c.a * p) / 256;

	Color ret;

	ret.c.r = r;
	ret.c.g = g;
	ret.c.b = b;
	ret.c.a = a;

	return ret;
}

#endif	/* _COLOR_H_ */
