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

#endif	/* _COLOR_H_ */
