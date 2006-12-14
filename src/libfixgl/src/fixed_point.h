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

#ifndef FIXED_POINT_H_
#define FIXED_POINT_H_

#include "types.h"

#ifdef DBG_USE_FLOAT
typedef float fixed;
#else
typedef int fixed;
#endif

/* valid choices for DECIMAL_BITS
 *      8: for fixed point 24:8
 *     16: for fixed point 16:16
 */
#define DECIMAL_BITS	16

#if DECIMAL_BITS == 8
#define FIXED_SHIFT		8
#define FLT_SCALE		256.0f
#define FRAC_MASK		0xff
#else	/* DECIMAL_BITS == 16 */
#define FIXED_SHIFT		16
#define FLT_SCALE		65536.0f
#define FRAC_MASK		0xffff
#endif	/* DECIMAL_BITS */

extern const fixed fixed_zero;
extern const fixed fixed_one;
extern const fixed fixed_half;
extern const fixed fixed_tenth;
extern const fixed fixed_255;

#ifdef DBG_USE_FLOAT
/* ------- debug mode, use floating point -------- */
#define FIXED_INT_PART(n)	((int)(n))
#define FIXED_FRAC_PART(n)	((n) > 0.0f ? (FIXED_INT_PART(n) - (n)) : (FIXED_INT_PART(n) + (n)))
#define FIXED_ROUND(n)		FIXED_INT_PART((n) + 0.5f)

#define FIXED_TO_FLOAT(n)	(n)
#define FLOAT_TO_FIXED(n)	(n)
#define INT_TO_FIXED(n)		((float)(n))

#else	/* ---- really fixed point ---- */

#define FIXED_INT_PART(n)	((n) >> FIXED_SHIFT)
#define FIXED_FRAC_PART(n)	((n) & FRAC_MASK)
#define FIXED_ROUND(n)		FIXED_INT_PART((n) + fixed_half)
/*#define FIXED_ROUND(n)		FIXED_INT_PART(n)*/

#define FIXED_TO_FLOAT(n)	(float)((n) / FLT_SCALE)
#define FLOAT_TO_FIXED(n)	(fixed)((n) * FLT_SCALE)
#define INT_TO_FIXED(n)		(fixed)((n) << FIXED_SHIFT)

#endif


#define fixed_int(n)		FIXED_INT_PART(n)
#define fixed_frac(n)		FIXED_FRAC_PART(n)
#define fixed_float(n)		FIXED_TO_FLOAT(n)
#define fixed_round(n)		FIXED_ROUND(n)

#define fixedf(n)			FLOAT_TO_FIXED(n)
#define fixedi(n)			INT_TO_FIXED(n)

#define fixed_add(n1, n2)	((n1) + (n2))
#define fixed_sub(n1, n2)	((n1) - (n2))



#ifdef DBG_USE_FLOAT

#define fixed_mul(n1, n2)	((n1) * (n2))
#define fixed_div(n1, n2)	((n1) / (n2))

#define fixed_sin(x)	(fixed)sin(x)
#define fixed_cos(x)	(fixed)cos(x)

#else

#if DECIMAL_BITS == 8
#define fixed_mul(n1, n2)	(fixed)((n1) * (n2) >> FIXED_SHIFT)
#define fixed_div(n1, n2)	(((n1) << FIXED_SHIFT) / (n2))
#else
#define fixed_div(n1, n2)	(((int64_t)(n1) << FIXED_SHIFT) / (int64_t)(n2))
#define fixed_mul(n1, n2)	(((n1) >> 8) * ((n2) >> 8))
#endif	/* DECIMAL_BITS */

#ifdef __cplusplus
extern "C" {
#endif

fixed fixed_sin(fixed angle);
fixed fixed_cos(fixed angle);

#ifdef __cplusplus
}
#endif

#endif

#endif	/* FIXED_POINT_H_ */
