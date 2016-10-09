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
