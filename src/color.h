#ifndef _COLOR_H_
#define _COLOR_H_

#ifndef VISUALC

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

#else
union Color {
	unsigned int packed;
	struct {
		unsigned char b, g, r, a;
	}c;
};

#endif // ndef VISUALC

inline Color Lerp(const Color &c1, const Color &c2, int p)
{
	if (p < 0) p = 0;
	if (p > 255) p = 255;
	unsigned char omp = 255 - p;
	//unsigned int r, g, b;//, a;
	Color ret;
	ret.c.r = (c1.c.r * omp + c2.c.r * p) >> 8;// / 256;
	ret.c.g = (c1.c.g * omp + c2.c.g * p) >> 8;// / 256;
	ret.c.b = (c1.c.b * omp + c2.c.b * p) >> 8;// / 256;
	//a = (c1.c.a * omp + c2.c.a * p) / 256;
	ret.c.a = 255;//a;

	return ret;
}

#endif	/* _COLOR_H_ */
