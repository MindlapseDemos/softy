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
#ifndef GL_SHADE_H_
#define GL_SHADE_H_

#ifndef LIBFIXGL_SOURCE
#error "shade.h should not included from user programs, just include gl.h"
#endif

#include "vmath.h"

/* This calculates shading with the phong shading model (half-angle variant)
 * vcs_pos: the surface position in view coordinates
 * vcs_n: the normal in view coordinates
 */
vec3 gl_shade(vec3 vcs_pos, vec3 vcs_n);
void gl_phong_shade(fixed nx, fixed ny, fixed nz, fixed vx, fixed vy, fixed vz, fixed *r, fixed *g, fixed *b, fixed *a);

#endif	/* GL_SHADE_H_ */
