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

/* This isn't supposed to be used directly by the user.
 * It's an internal mini3d interface.
 */
#ifndef GL_RASTERIZER_H_
#define GL_RASTERIZER_H_

#include "fixed_point.h"

struct frame_buffer {
	uint32_t *color_buffer;
	uint32_t *depth_buffer;
	/*uint8_t *stencil_buffer;*/
	int x, y;
};

struct tex2d {
	uint32_t *pixels;
	int x, y, pix_count;
	int xpow, xmask, ymask;
	unsigned int type;
};

struct vertex {
	fixed x, y, z, w;
	fixed nx, ny, nz;
	fixed vx, vy, vz;
	fixed r, g, b, a;
	fixed u, v;
};

int gl_rasterizer_setup(struct frame_buffer *fbuf);

void gl_draw_point(struct vertex *pt);
void gl_draw_line(struct vertex *points);
void gl_draw_polygon(struct vertex *points, int count);

#endif	/* GL_RASTERIZER_H_ */
