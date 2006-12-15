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
#define LIBFIXGL_SOURCE

#include "gl.h"
#include "state.h"
#include "shade.h"
#include "types.h"

static const int prim_vertices[] = {
	1,		/* GL_POINTS */
	2,		/* GL_LINES */
	-1,		/* GL_LINE_STRIP */
	-1,		/* GL_LINE_LOOP */
	3,		/* GL_TRIANGLES */
	-1,		/* GL_TRIANGLE_STRIP */
	-1,		/* GL_TRIANGLE_FAN */
	4,		/* GL_QUADS */
	-1,		/* GL_QUAD_STRIP */
	-1		/* GL_POLYGON */
};


void glBegin(GLenum primitive) {
	if(primitive < GL_POINTS || primitive > GL_POLYGON) {
		state.gl_error = GL_INVALID_ENUM;
		return;
	}
	CHECK_BEG_END();

	state.prim = primitive;
	state.cur_vert = 0;
	state.prim_elem = prim_vertices[primitive - GL_POINTS];

	if(!state.mvp_valid) {
		int mvtop = state.stack_top[MODE_MODELVIEW] - 1;
		int ptop = state.stack_top[MODE_PROJECTION] - 1;
		vm_mult_matrix(state.mvp_mat, state.mstack[MODE_PROJECTION][ptop], state.mstack[MODE_MODELVIEW][mvtop]);
		state.mvp_valid = 1;
	}

	state.in_beg_end = 1;
}

void glEnd(void) {
	if(!state.in_beg_end) {
		state.gl_error = GL_INVALID_OPERATION;
		return;
	}

	if(state.cur_vert) {
		int i;
		struct vertex add_v[2];
		int vcount = state.cur_vert;
		
		switch(state.prim) {
		case GL_LINE_LOOP:
			add_v[0] = state.v[vcount - 1];
			add_v[1] = state.v[0];
			gl_draw_line(add_v);

		case GL_LINE_STRIP:
			for(i=0; i<vcount - 1; i++) {
				gl_draw_line(state.v + i);
			}
			break;

		case GL_POLYGON:
			gl_draw_polygon(state.v, vcount);

		default:
			break;
		}
	}

	state.in_beg_end = 0;
}


void glVertex3x(GLfixed x, GLfixed y, GLfixed z) {
	fixed *row;
	fixed half_width, half_height;
	struct vertex *v = state.v + state.cur_vert;

	v->r = state.r;
	v->g = state.g;
	v->b = state.b;
	v->a = state.a;
	v->nx = state.nx;
	v->ny = state.ny;
	v->nz = state.nz;
	v->u = state.tu;
	v->v = state.tv;
	/*v->w = state.tw;*/
	
	/* if lighting is enabled, modify the color */
	if(IS_ENABLED(GL_LIGHTING)) {
		vec3 pos, normal, col;
		int mvtop = state.stack_top[MODE_MODELVIEW] - 1;
		
		pos.x = x;
		pos.y = y;
		pos.z = z;
		pos = vm_transform(pos, state.mstack[MODE_MODELVIEW][mvtop]);

		v->vx = pos.x;
		v->vy = pos.y;
		v->vz = pos.z;

		if(!(state.s & (1 << GL_PHONG))) {
			normal.x = v->nx;
			normal.y = v->ny;
			normal.z = v->nz;

			col = gl_shade(pos, normal);
			v->r = col.x;
			v->g = col.y;
			v->b = col.z;
			v->a = col.w;
		}
	}

	/* transform into post-projective homogeneous clip-space */
	row = state.mvp_mat;
	v->x = fixed_mul(row[0], x) + fixed_mul(row[1], y) + fixed_mul(row[2], z) + row[3]; row += 4;
	v->y = fixed_mul(row[0], x) + fixed_mul(row[1], y) + fixed_mul(row[2], z) + row[3]; row += 4;
	v->z = fixed_mul(row[0], x) + fixed_mul(row[1], y) + fixed_mul(row[2], z) + row[3]; row += 4;
	v->w = fixed_mul(row[0], x) + fixed_mul(row[1], y) + fixed_mul(row[2], z) + row[3];

	if(state.prim == GL_POINTS && v->z < 0) {
		return;
	}

	/* divide with W */
	if(v->w) {
		/*fixed z = v->z;*/
		v->x = fixed_div(v->x, v->w);
		v->y = fixed_div(v->y, v->w);
		v->z = fixed_div(v->z, v->w);
		/*v->w = -z;*/
	}

	/* viewport transformation */
	half_width = fixed_mul(fixedi(state.fb.x), fixed_half);
	half_height = fixed_mul(fixedi(state.fb.y), fixed_half);
	v->x = fixed_mul(half_width, v->x + fixed_one);
	v->y = fixed_mul(half_height, fixed_one - v->y);

	if(state.prim_elem != -1) {
		state.cur_vert = (state.cur_vert + 1) % state.prim_elem;
	} else {
		state.cur_vert++;
	}

	if(!state.cur_vert) {
		switch(state.prim) {
		case GL_POINTS:
			gl_draw_point(state.v);
			break;

		case GL_LINES:
			gl_draw_line(state.v);
			break;

		case GL_TRIANGLES:
		case GL_QUADS:
			gl_draw_polygon(state.v, state.prim_elem);
			break;

		default:
			break;
		}
	}
}

void glColor4x(GLfixed r, GLfixed g, GLfixed b, GLfixed a) {
	state.r = r;
	state.g = g;
	state.b = b;
	state.a = a;
}

void glNormal3x(GLfixed x, GLfixed y, GLfixed z) {
	vec3 normal;

	int mvtop = state.stack_top[MODE_MODELVIEW] - 1;
	fixed *row = state.mstack[MODE_MODELVIEW][mvtop];
	
	normal.x = fixed_mul(row[0], x) + fixed_mul(row[1], y) + fixed_mul(row[2], z); row += 4;
	normal.y = fixed_mul(row[0], x) + fixed_mul(row[1], y) + fixed_mul(row[2], z); row += 4;
	normal.z = fixed_mul(row[0], x) + fixed_mul(row[1], y) + fixed_mul(row[2], z);

	if(IS_ENABLED(GL_NORMALIZE)) {
		vm_normalize(&normal);
	}
	state.nx = normal.x;
	state.ny = normal.y;
	state.nz = normal.z;
}

void glTexCoord3x(GLfixed s, GLfixed t, GLfixed r) {
	state.tu = s;
	state.tv = t;
	state.tw = r;
}
