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

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "vmath.h"

#define M(i, j)	((i << 2) + j)

inline vec3 vm_add(vec3 a, vec3 b) {
	vec3 res;
	res.x = a.x + b.x;
	res.y = a.y + b.y;
	res.z = a.z + b.z;
	res.w = a.w;
	return res;
}

inline vec3 vm_sub(vec3 a, vec3 b) {
	vec3 res;
	res.x = a.x - b.x;
	res.y = a.y - b.y;
	res.z = a.z - b.z;
	res.w = a.w;
	return res;
}

inline vec3 vm_mul(vec3 v, fixed s) {
	vec3 res;
	res.x = fixed_mul(v.x, s);
	res.y = fixed_mul(v.y, s);
	res.z = fixed_mul(v.z, s);
	res.w = v.w;
	return res;
}

inline fixed vm_dot(vec3 a, vec3 b) {
	return fixed_mul(a.x, b.x) + fixed_mul(a.y, b.y) + fixed_mul(a.z, b.z);
}

#define SQ(x)	((x) * (x))
#define FIXED_SQ(x)	fixed_mul((x), (x))
#define MAX(a, b)	((a) > (b) ? (a) : (b))

inline vec3 vm_normalized(vec3 v) {
	vm_normalize(&v);
	return v;
}

inline void vm_normalize(vec3 *v) {
	float x = fixed_float(v->x);
	float y = fixed_float(v->y);
	float z = fixed_float(v->z);
	float len = sqrt(SQ(x) + SQ(y) + SQ(z));
	v->x = fixedf(x / len);
	v->y = fixedf(y / len);
	v->z = fixedf(z / len);
}

inline vec3 vm_transform(vec3 v, fixed *m) {
	vec3 out;
	fixed *row = m;
	out.x = fixed_mul(row[0], v.x) + fixed_mul(row[1], v.y) + fixed_mul(row[2], v.z) + row[3]; row += 4;
	out.y = fixed_mul(row[0], v.x) + fixed_mul(row[1], v.y) + fixed_mul(row[2], v.z) + row[3]; row += 4;
	out.z = fixed_mul(row[0], v.x) + fixed_mul(row[1], v.y) + fixed_mul(row[2], v.z) + row[3]; row += 4;
	out.w = fixed_mul(row[0], v.x) + fixed_mul(row[1], v.y) + fixed_mul(row[2], v.z) + row[3];
	return out;
}


void vm_mult_matrix(fixed *t, fixed *m1, fixed *m2) {
	int i, j;
	fixed res[16];

	for(i=0; i<4; i++) {
		for(j=0; j<4; j++) {
			res[M(i,j)] = fixed_mul(m1[M(i,0)], m2[M(0,j)]) +
						fixed_mul(m1[M(i,1)], m2[M(1,j)]) +
						fixed_mul(m1[M(i,2)], m2[M(2,j)]) +
						fixed_mul(m1[M(i,3)], m2[M(3,j)]);
		}
	}

	memcpy(t, res, 16 * sizeof *t);
}

void vm_print_matrix(FILE *fp, fixed *mat) {
	int i;
	for(i=0; i<16; i++) {
		fprintf(fp, "%9.3f", fixed_float(mat[i]));
		fputc(i % 4 == 3 ? '\n' : ' ', fp);
	}
}
