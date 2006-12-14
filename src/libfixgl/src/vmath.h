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

#ifndef GL_VMATH_H_
#define GL_VMATH_H_

#include <stdio.h>
#include "fixed_point.h"

#ifndef HAVE_INLINE
#define inline
#endif	/* HAVE_INLINE */

typedef struct vec3 {
	fixed x, y, z, w;
} vec3;

inline vec3 vm_add(vec3 a, vec3 b);
inline vec3 vm_sub(vec3 a, vec3 b);
inline vec3 vm_mul(vec3 v, fixed s);
inline fixed vm_dot(vec3 a, vec3 b);
inline vec3 vm_normalized(vec3 v);
inline void vm_normalize(vec3 *v);
inline vec3 vm_transform(vec3 v, fixed *m);
void vm_mult_matrix(fixed *t, fixed *m1, fixed *m2);
void vm_print_matrix(FILE *fp, fixed *mat);

#endif	/* GL_VMATH_H_ */
