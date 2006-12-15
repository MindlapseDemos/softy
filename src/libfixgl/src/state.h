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
#ifndef GL_STATE_H_
#define GL_STATE_H_

#ifndef LIBFIXGL_SOURCE
#error "state.h should not included from user programs, just include gl.h"
#endif

#include "gl_rasterizer.h"
#include "vmath.h"

#define MAX_TEXTURES		128
#define MAX_TEXTURE_SIZE	4096
#define MAX_LIGHTS			4
#define MATRIX_STACK_SIZE	32

enum {MODE_MODELVIEW, MODE_PROJECTION, MODE_TEXTURE};
#define MODE_COUNT		3

#define MAX_POLY		64

#define GL_ERROR(e)	\
	if(!state.gl_error) state.gl_error = (e)

#define CHECK_BEG_END()	\
	if(state.in_beg_end) {\
		GL_ERROR(GL_INVALID_OPERATION);\
		return;\
	}

struct state {
	unsigned int s;

	/* transformation state */
	int matrix_mode;
	fixed mstack[MODE_COUNT][MATRIX_STACK_SIZE][16];
	int stack_top[MODE_COUNT];
	
	fixed mvp_mat[16];
	int mvp_valid;

	/* texture state */
	struct tex2d *tex[MAX_TEXTURES];
	unsigned int btex;		/* texture bound */

	/* rendering state */
	unsigned int prim;
	struct frame_buffer fb;
	fixed clear_r, clear_g, clear_b, clear_a;
	fixed clear_depth;
	fixed point_sz;

	/* lighting state */
	vec3 lpos[MAX_LIGHTS];
	vec3 ambient, diffuse, specular;
	fixed shininess;

	vec3 ambient_light;

	/* blending state */
	unsigned int src_blend, dst_blend;

	/* misc */
	unsigned int gl_error;
	int in_beg_end;

	/* vertex state */
	fixed r, g, b, a;
	fixed nx, ny, nz;
	fixed tu, tv, tw;
	struct vertex v[MAX_POLY];
	int cur_vert;
	int prim_elem;		/* num elements of the current primitive */
};

#define IS_ENABLED(x)	(state.s & (1 << (x)))

extern struct state state;

#endif	/* GL_STATE_H_ */
