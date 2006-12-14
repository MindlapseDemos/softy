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

#include <math.h>
#include "shade.h"
#include "state.h"
#include "gl.h"

vec3 gl_shade(vec3 vcs_pos, vec3 vcs_n) {
	int i;
	vec3 col = state.ambient_light;
	vec3 view;
	
	/* this test is pointless, shade() won't be called if lighting is disabled */
	if(!IS_ENABLED((GL_LIGHTING))) {
		return col;
	}

	/* if local-viewer light model is enabled, use the real view vector
	 * (in view coords, it's just the normalized position). Otherwise
	 * use an approximation, the orthographic (0, 0, 1) view vector.
	 */
	if(IS_ENABLED(GL_LIGHT_MODEL_LOCAL_VIEWER)) {
		view = vm_normalized(vcs_pos);
		view.x = -view.x;
		view.y = -view.y;
		view.z = -view.z;
	} else {
		view.x = view.y = 0;
		view.z = fixed_one;
	}

	for(i=0; i<MAX_LIGHTS; i++) {
		vec3 ldir, half;
		fixed ndotl, ndoth;
		vec3 dif, spec;
		float pow_res;
		
		if(!IS_ENABLED(GL_LIGHT0 + i)) {
			continue;
		}

		/* if the light's w coord is 0, treat it as a light direction (dir-light)
		 * otherwise, find the light direction by subtracting the light position
		 * from the surface point.
		 */
		if(state.lpos[i].w == 0) {
			ldir.x = -state.lpos[i].x;
			ldir.y = -state.lpos[i].y;
			ldir.z = -state.lpos[i].z;
			vm_normalize(&ldir);
		} else {
			ldir.x = state.lpos[i].x - vcs_pos.x;
			ldir.y = state.lpos[i].y - vcs_pos.y;
			ldir.z = state.lpos[i].z - vcs_pos.z;
			vm_normalize(&ldir);
		}
		
		/* calculate the diffuse factor (N.L) */
		ndotl = vm_dot(vcs_n, ldir);
		if(ndotl < 0) ndotl = 0;
		dif.x = fixed_mul(state.diffuse.x, ndotl);
		dif.y = fixed_mul(state.diffuse.y, ndotl);
		dif.z = fixed_mul(state.diffuse.z, ndotl);

		/* add diffuse to the current color sum */
		col.x += dif.x;
		col.y += dif.y;
		col.z += dif.z;

		if(state.specular.x > 0 && state.specular.y > 0 && state.specular.z > 0) {
			float sr, sg, sb;
			/* calculate the half vector (useful for calculating specular intensity) */
			half.x = view.x + ldir.x;
			half.y = view.y + ldir.y;
			half.z = view.z + ldir.z;
			vm_normalize(&half);

			/* calculate the specular intensity ([N.H]^S) */
			ndoth = vm_dot(vcs_n, half);
			if(ndoth < 0) ndoth = 0;

			pow_res = pow(fixed_float(ndoth), fixed_float(state.shininess));

			sr = fixed_float(state.specular.x) * pow_res;
			sg = fixed_float(state.specular.y) * pow_res;
			sb = fixed_float(state.specular.z) * pow_res;
			spec.x = fixedf(((sr > 1.0f) ? 1.0f : sr));
			spec.y = fixedf(((sg > 1.0f) ? 1.0f : sg));
			spec.z = fixedf(((sb > 1.0f) ? 1.0f : sb));

			/* add specular to the current color sum */
			col.x += spec.x;
			col.y += spec.y;
			col.z += spec.z;
		}
	}

	if(col.x > fixed_one) col.x = fixed_one;
	if(col.y > fixed_one) col.y = fixed_one;
	if(col.z > fixed_one) col.z = fixed_one;
	col.w = state.diffuse.w;	/* get alpha from the color/material */

	return col;
}

void gl_phong_shade(fixed nx, fixed ny, fixed nz, fixed vx, fixed vy, fixed vz, fixed *r, fixed *g, fixed *b, fixed *a) {
	vec3 view, normal, col;
	normal.x = nx;
	normal.y = ny;
	normal.z = nz;
	normal.w = fixed_one;

	view.x = vx;
	view.y = vy;
	view.z = vz;
	view.w = fixed_one;

	vm_normalize(&normal);

	col = gl_shade(view, normal);
	*r = col.x;
	*g = col.y;
	*b = col.z;
	*a = col.w;
}
