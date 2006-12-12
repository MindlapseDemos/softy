#ifndef _SOFTY_H_
#define _SOFTY_H_

#include <limits.h>
#include "color.h"
#include "image.h"

#define SCR_X	640
#define SCR_Y	480

extern Color *fb;
extern Image *fbimg;

/* demo sequence */
#define S_ECLIPSE	0
#define E_ECLIPSE	UINT_MAX

#endif	/* _SOFTY_H_ */
