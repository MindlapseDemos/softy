/*
This file is part of libfixgl, a fixed point implementation of OpenGL
Copyright (C) 2006 John Tsiombikas <nuclear@siggraph.org>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include <math.h>
#include "fixed_point.h"

const fixed fixed_one = fixedi(1);
const fixed fixed_half = fixedf(0.5);
const fixed fixed_tenth = fixedf(0.1);
const fixed fixed_255 = fixedi(255);

#ifndef DBG_USE_FLOAT

#define PI			3.1415927
#define TWO_PI		6.2831853

#define LUT_SIZE	256

static fixed sin_lut[LUT_SIZE], cos_lut[LUT_SIZE];
static int initialized;

static void precalc_lut(void) {
	int i;
	
	for(i=0; i<LUT_SIZE; i++) {
		float angle = TWO_PI * (float)i / (float)LUT_SIZE;
		
		sin_lut[i] = FLOAT_TO_FIXED(sin(angle));
		cos_lut[i] = FLOAT_TO_FIXED(cos(angle));
	}
	
	initialized = 1;
}

static const fixed fix_two_pi = FLOAT_TO_FIXED(TWO_PI);

fixed fixed_sin(fixed angle) {
	int a;
	
	if(!initialized) precalc_lut();
	a = FIXED_INT_PART(fixed_div(angle, fix_two_pi) * 255) % 256;
	return a >= 0 ? sin_lut[a] : -sin_lut[-a];
}

fixed fixed_cos(fixed angle) {
	int a;
	
	if(!initialized) precalc_lut();
	a = FIXED_INT_PART(fixed_div(angle, fix_two_pi) * 255) % 256;
	return a >= 0 ? cos_lut[a] : cos_lut[-a];
}

#endif
