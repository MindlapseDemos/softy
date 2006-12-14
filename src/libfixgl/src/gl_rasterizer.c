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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "gl_rasterizer.h"
#include "gl.h"
#include "state.h"
#include "color_bits.h"

#ifndef HAVE_INLINE
#define inline
#endif	/* HAVE_INLINE */

#define INTERP_COL
#define INTERP_Z
/*#define INTERP_W*/
#define INTERP_TEX
#define INTERP_NORM
#define ENABLE_BLENDING

#define GET_R(p)	(((p) & RED_MASK32) >> RED_SHIFT32)
#define GET_G(p)	(((p) & GREEN_MASK32) >> GREEN_SHIFT32)
#define GET_B(p)	(((p) & BLUE_MASK32) >> BLUE_SHIFT32)
#define GET_A(p)	(((p) & ALPHA_MASK32) >> ALPHA_SHIFT32)

#define MIN(a, b)	((a) < (b) ? (a) : (b))
#define MAX(a, b)	((a) > (b) ? (a) : (b))
#define CLAMP(x, l, h)	MAX((l), MIN((h), (x)))

#define swap(type, a, b)	{type tmp = a; a = b; b = tmp;}

struct edge {
	int x;
	fixed r, g, b, a;
	fixed u, v;
	fixed z, w;
	fixed nx, ny, nz;
	fixed vx, vy, vz;
};

static inline void scan_int(fixed x1, fixed y1, fixed x2, fixed y2, int elem_offset);
static inline void scan_fixed(fixed x1, fixed y1, fixed x2, fixed y2, int elem_offset);
static inline void fill_scanlines(int starty, int endy);

#ifdef ENABLE_BLENDING
static uint32_t blend(int r, int g, int b, int a, int fb_r, int fb_g, int fb_b, int fb_a);
#endif	/* ENABLE_BLENDING */

void gl_phong_shade(fixed nx, fixed ny, fixed nz, fixed vx, fixed vy, fixed vz, fixed *r, fixed *g, fixed *b, fixed *a);

/* ---------- rasterizer state variables and edge tables --------- */
static struct tex2d *tex;
static int phong, affine_tex = 1;
static int gouraud = 1;

static fixed flat_r, flat_g, flat_b, flat_a;

static struct frame_buffer *fb;
static int xres, yres;

static int *scanline_offset;
struct edge *left_edge, *right_edge;

int gl_rasterizer_setup(struct frame_buffer *fbuf) {
	int i;

	fb = fbuf;
	xres = fb->x;
	yres = fb->y;

	free(left_edge);
	free(right_edge);
	free(scanline_offset);

	left_edge = malloc(yres * sizeof(struct edge));
	right_edge = malloc(yres * sizeof(struct edge));
	scanline_offset = malloc(yres * sizeof(int));

	if(!left_edge || !right_edge || !scanline_offset) {
		return -1;
	}

	for(i=0; i<yres; i++) {
		scanline_offset[i] = i * xres;
	}

	return 0;
}


void gl_draw_point(struct vertex *pt) {
	int cx = fixed_int(pt->x);/*fixed_mul(pt->x, fixed_half));*/
	int cy = fixed_int(pt->y);/*fixed_mul(pt->y, fixed_half));*/

	int ia = CLAMP(fixed_int(fixed_mul(pt->a, fixed_255)), 0, 255);
	int ir = CLAMP(fixed_int(fixed_mul(pt->r, fixed_255)), 0, 255);
	int ig = CLAMP(fixed_int(fixed_mul(pt->g, fixed_255)), 0, 255);
	int ib = CLAMP(fixed_int(fixed_mul(pt->b, fixed_255)), 0, 255);
	uint32_t pcol = PACK_COLOR32(ia, ir, ig, ib);
	
	/* TODO: implement smooth (circular) points
	if(IS_ENABLED(GL_POINT_SMOOTH)) {
	}
	*/

	int i, j;
	int sz = MAX(1, fixed_round(state.point_sz));
		
	int y = cy - sz / 2;
	int x = cx - sz / 2;
	int offs = y * fb->x + x;
	uint32_t *cptr = fb->color_buffer + offs;
	uint32_t *zptr = fb->depth_buffer + offs;

	for(j=0; j<sz; j++) {
		if(y >= fb->y) break;
		if(y >= 0) {
			x = cx - sz / 2;

			for(i=0; i<sz; i++) {
				if(x >= fb->x) break;
				if(x >= 0) {
					uint32_t zval = (uint32_t)pt->z;
					if(!IS_ENABLED(GL_DEPTH_TEST) || zval < *zptr) {
						*cptr = pcol;
						if(IS_ENABLED(GL_DEPTH_WRITE)) *zptr = zval;
					}
				}
				x++;
				cptr++;
				zptr++;
			}
		}
		zptr += fb->x - sz;
		y++;
	}
}


void gl_draw_line(struct vertex *points) {
	/* TODO */
}


/* draw_polygon()
 * function to rasterize a convex polygon with gouraud color
 * interpolation, texture coordinate interpolation and zbuffering
 */

void gl_draw_polygon(struct vertex *points, int count) {
	int i, iy1;
	int starty = yres;
	int endy = 0;

	struct vertex *v1 = points - 1;
	struct vertex *v2 = points;

	phong = (IS_ENABLED(GL_PHONG) && IS_ENABLED(GL_LIGHTING)) ? 1 : 0;
	gouraud = IS_ENABLED(GL_SMOOTH) ? 1 : 0;

	if(!gouraud) {
		flat_r = points[0].r;
		flat_g = points[0].g;
		flat_b = points[0].b;
		flat_a = points[0].a;
	}

	/*points[0].w = fixed_div(fixed_one, points[0].w);
	points[1].w = fixed_div(fixed_one, points[1].w);
	points[2].w = fixed_div(fixed_one, points[2].w);*/

	count--;
	for(i=0; i<count; i++) {
		v1 = v2;
		v2++;

		iy1 = fixed_round(v1->y);
		if(iy1 < starty) starty = iy1;
		if(iy1 > endy) endy = iy1;

		scan_int(v1->x, v1->y, v2->x, v2->y, offsetof(struct edge, x));
#ifdef INTERP_Z
		scan_fixed(v1->z, v1->y, v2->z, v2->y, offsetof(struct edge, z));
#endif
#ifdef INTERP_W
		scan_fixed(v1->w, v1->y, v2->w, v2->y, offsetof(struct edge, w));
#endif
#ifdef INTERP_COL
		/* interpolate color */
		if(gouraud) {
			scan_fixed(v1->a, v1->y, v2->a, v2->y, offsetof(struct edge, a));
			scan_fixed(v1->r, v1->y, v2->r, v2->y, offsetof(struct edge, r));
			scan_fixed(v1->g, v1->y, v2->g, v2->y, offsetof(struct edge, g));
			scan_fixed(v1->b, v1->y, v2->b, v2->y, offsetof(struct edge, b));
		}
#endif
#ifdef INTERP_TEX
		/* interpolate texture coordinates */
		if(affine_tex) {
			scan_fixed(v1->u, v1->y, v2->u, v2->y, offsetof(struct edge, u));
			scan_fixed(v1->v, v1->y, v2->v, v2->y, offsetof(struct edge, v));
		} else {
#ifdef INTERP_W
			fixed ru1 = v1->w ? fixed_mul(v1->u, v1->w) : 0;
			fixed ru2 = v2->w ? fixed_mul(v2->u, v2->w) : 0;
			fixed rv1 = v1->w ? fixed_mul(v1->v, v1->w) : 0;
			fixed rv2 = v2->w ? fixed_mul(v2->v, v2->w) : 0;
			scan_fixed(ru1, v1->y, ru2, v2->y, offsetof(struct edge, u));
			scan_fixed(rv1, v1->y, rv2, v2->y, offsetof(struct edge, v));
#endif
		}
#endif
#ifdef INTERP_NORM
		if(phong) {
			scan_fixed(v1->nx, v1->y, v2->nx, v2->y, offsetof(struct edge, nx));
			scan_fixed(v1->ny, v1->y, v2->ny, v2->y, offsetof(struct edge, ny));
			scan_fixed(v1->nz, v1->y, v2->nz, v2->y, offsetof(struct edge, nz));

			scan_fixed(v1->vx, v1->y, v2->vx, v2->y, offsetof(struct edge, vx));
			scan_fixed(v1->vy, v1->y, v2->vy, v2->y, offsetof(struct edge, vy));
			scan_fixed(v1->vz, v1->y, v2->vz, v2->y, offsetof(struct edge, vz));
		}
#endif

	}

	/* unrolled the last loop to avoid the check to setup v2 correctly */
	v1 = v2;
	v2 = points;

	iy1 = fixed_round(v1->y);
	if(iy1 < starty) starty = iy1;
	if(iy1 > endy) endy = iy1;

	scan_int(v1->x, v1->y, v2->x, v2->y, offsetof(struct edge, x));
#ifdef INTERP_Z
	scan_fixed(v1->z, v1->y, v2->z, v2->y, offsetof(struct edge, z));
#endif
#ifdef INTERP_W
	scan_fixed(v1->w, v1->y, v2->w, v2->y, offsetof(struct edge, w));
#endif
#ifdef INTERP_COL
	/* interpolate color */
	if(gouraud) {
		scan_fixed(v1->a, v1->y, v2->a, v2->y, offsetof(struct edge, a));
		scan_fixed(v1->r, v1->y, v2->r, v2->y, offsetof(struct edge, r));
		scan_fixed(v1->g, v1->y, v2->g, v2->y, offsetof(struct edge, g));
		scan_fixed(v1->b, v1->y, v2->b, v2->y, offsetof(struct edge, b));
	}
#endif
#ifdef INTERP_TEX
	/* interpolate texture coordinates */
	if(affine_tex) {
		scan_fixed(v1->u, v1->y, v2->u, v2->y, offsetof(struct edge, u));
		scan_fixed(v1->v, v1->y, v2->v, v2->y, offsetof(struct edge, v));
	} else {
#ifdef INTERP_W
		fixed ru1 = v1->w ? fixed_mul(v1->u, v1->w) : 0;
		fixed ru2 = v2->w ? fixed_mul(v2->u, v2->w) : 0;
		fixed rv1 = v1->w ? fixed_mul(v1->v, v1->w) : 0;
		fixed rv2 = v2->w ? fixed_mul(v2->v, v2->w) : 0;
		scan_fixed(ru1, v1->y, ru2, v2->y, offsetof(struct edge, u));
		scan_fixed(rv1, v1->y, rv2, v2->y, offsetof(struct edge, v));
#endif
	}
#endif
#ifdef INTERP_NORM
	if(phong) {
		scan_fixed(v1->nx, v1->y, v2->nx, v2->y, offsetof(struct edge, nx));
		scan_fixed(v1->ny, v1->y, v2->ny, v2->y, offsetof(struct edge, ny));
		scan_fixed(v1->nz, v1->y, v2->nz, v2->y, offsetof(struct edge, nz));
	
		scan_fixed(v1->vx, v1->y, v2->vx, v2->y, offsetof(struct edge, vx));
		scan_fixed(v1->vy, v1->y, v2->vy, v2->y, offsetof(struct edge, vy));
		scan_fixed(v1->vz, v1->y, v2->vz, v2->y, offsetof(struct edge, vz));
	}
#endif
	
	/* fill the scanlines */
	
	starty = MAX(starty, 0);
	endy = MIN(endy, yres);

#ifndef INTERP_COL
	if(gouraud) {
		int y;
		for(y=starty; y<endy; y++) {
			right_edge[y].a = left_edge[y].a = points->a;
			right_edge[y].r = left_edge[y].r = points->r;
			right_edge[y].g = left_edge[y].g = points->g;
			right_edge[y].b = left_edge[y].b = points->b;
		}
	}
#endif

#ifdef INTERP_TEX
	tex = (IS_ENABLED(GL_TEXTURE_2D) && state.btex) ? state.tex[state.btex] : 0;
#endif

	fill_scanlines(starty, endy);
}

static inline void scan_int(fixed x1, fixed y1, fixed x2, fixed y2, int elem_offset) {
	fixed x, dx, dy, slope;
	int j, y, idy;
	struct edge *ptr, *edge = right_edge;

	if(y1 > y2) {
		edge = left_edge;
		swap(fixed, y1, y2);
		swap(fixed, x1, x2);
	}

	x = x1;
	y = fixed_round(y1);

	dx = x2 - x1;
	dy = y2 - y1;
	idy = fixed_round(y2) - y;

	if(!idy) return;

	slope = dy ? fixed_div(dx, dy) : dx;

	ptr = edge + y;		/* CAUTION: since this is only used for x which has offset 0, we skip it */
	for(j=0; j<idy && y<yres; j++, y++, ptr++) {
		if(y >= 0) {
			*(int*)ptr = fixed_round(x);
		}
		x += slope;
	}
}

static inline void scan_fixed(fixed x1, fixed y1, fixed x2, fixed y2, int elem_offset) {
	fixed x, dx, dy, slope;
	int y, idy, j;
	struct edge *ptr, *edge = right_edge;

	if(y1 > y2) {
		edge = left_edge;
		swap(fixed, y1, y2);
		swap(fixed, x1, x2);
	}

	x = x1;
	y = fixed_round(y1);

	dx = x2 - x1;
	dy = y2 - y1;
	idy = fixed_round(y2) - y;

	if(!idy) return;

	slope = dy ? fixed_div(dx, dy) : dx;

	ptr = (struct edge*)((char*)(edge + y) + elem_offset);
	for(j=0; j<=idy && y<yres; j++, y++, ptr++) {
		if(y >= 0) {
			*(fixed*)ptr = x;
		}
		x += slope;
	}
}


static inline void fill_scanlines(int starty, int endy) {
	int x, y;
	struct edge *left_ptr = left_edge + starty;
	struct edge *right_ptr = right_edge + starty;
	int *sptr = scanline_offset + starty;

	for(y=starty; y<endy; y++, left_ptr++, right_ptr++) {
		uint32_t *cptr, *zptr;
		int startx = left_ptr->x;
		int endx = right_ptr->x;

		fixed dx = fixedi(endx - startx);
		
		/* color, z, w, u, v interpolation */
		fixed a, r, g, b, z;
#ifdef INTERP_COL
		fixed da, dr, dg, db;
#endif
#ifdef INTERP_Z
		fixed dz;
#endif
#ifdef INTERP_W
		fixed w, dw;
#endif
#ifdef INTERP_TEX
		fixed u, v, du, dv;
#endif
#ifdef INTERP_NORM
		fixed nx, ny, nz, dnx, dny, dnz, vx, vy, vz, dvx, dvy, dvz;
#endif
		fixed rslope, gslope, bslope, aslope, zslope, wslope, uslope, vslope;
		fixed nxslope, nyslope, nzslope, vxslope, vyslope, vzslope;
		
		if(gouraud) {
#ifdef INTERP_COL
			a = left_ptr->a;
			r = left_ptr->r;
			g = left_ptr->g;
			b = left_ptr->b;
			da = right_ptr->a - a;
			dr = right_ptr->r - r;
			dg = right_ptr->g - g;
			db = right_ptr->b - b;
#endif
		} else {
			a = flat_a;
			r = flat_r;
			g = flat_g;
			b = flat_b;
		}
#ifdef INTERP_Z
		z = left_ptr->z;
		dz = right_ptr->z - z;
#endif
#ifdef INTERP_W
		w = left_ptr->w;
		dw = right_ptr->w - w;
#endif
#ifdef INTERP_TEX
		u = left_ptr->u;
		v = left_ptr->v;
		du = right_ptr->u - u;
		dv = right_ptr->v - v;
#endif
#ifdef INTERP_NORM
		if(phong) {
			nx = left_ptr->nx;
			ny = left_ptr->ny;
			nz = left_ptr->nz;
			dnx = right_ptr->nx - nx;
			dny = right_ptr->ny - ny;
			dnz = right_ptr->nz - nz;

			vx = left_ptr->vx;
			vy = left_ptr->vy;
			vz = left_ptr->vz;
			dvx = right_ptr->vx - vx;
			dvy = right_ptr->vy - vy;
			dvz = right_ptr->vz - vz;
		}
#endif
		
		if(dx > 0) {
#ifdef INTERP_COL
			if(gouraud) {
				aslope = fixed_div(da, dx);
				rslope = fixed_div(dr, dx);
				gslope = fixed_div(dg, dx);
				bslope = fixed_div(db, dx);
			}
#endif
#ifdef INTERP_Z
			zslope = fixed_div(dz, dx);
#endif
#ifdef INTERP_W
			wslope = fixed_div(dw, dx);
#endif
#ifdef INTERP_TEX
			uslope = fixed_div(du, dx);
			vslope = fixed_div(dv, dx);
#endif
#ifdef INTERP_NORM
			if(phong) {
				nxslope = fixed_div(dnx, dx);
				nyslope = fixed_div(dny, dx);
				nzslope = fixed_div(dnz, dx);

				vxslope = fixed_div(dvx, dx);
				vyslope = fixed_div(dvy, dx);
				vzslope = fixed_div(dvz, dx);
			}
#endif
		} else {
			aslope = rslope = gslope = bslope = zslope = wslope = uslope = vslope = 0;
			if(phong) nxslope = nyslope = nzslope = vxslope = vyslope = vzslope = 0;
		}

		/*startx = MAX(startx, 0);*/
		endx = MIN(endx, xres);
		
		cptr = fb->color_buffer + *sptr + startx;
		zptr = fb->depth_buffer + *sptr++ + startx;
		
		for(x=startx; x<endx; x++) {
			if(x >= 0 && z > 0) {
#ifdef INTERP_Z
				uint32_t zval = (uint32_t)z;

				if(!IS_ENABLED(GL_DEPTH_TEST) || zval < *zptr) {
#endif
				
					static fixed fixed_255 = fixedi(255);
					int ia, ir, ig, ib;

					ia = a < 0 ? 0 : (a > fixed_one ? 255 : fixed_int(fixed_mul(a, fixed_255)));
					ir = r < 0 ? 0 : (r > fixed_one ? 255 : fixed_int(fixed_mul(r, fixed_255)));
					ig = g < 0 ? 0 : (g > fixed_one ? 255 : fixed_int(fixed_mul(g, fixed_255)));
					ib = b < 0 ? 0 : (b > fixed_one ? 255 : fixed_int(fixed_mul(b, fixed_255)));

#ifdef INTERP_NORM
					if(phong) {
						gl_phong_shade(nx, ny, nz, vx, vy, vz, &r, &g, &b, &a);
						ia = fixed_int(fixed_mul(a, fixed_255));
						ir = fixed_int(fixed_mul(r, fixed_255));
						ig = fixed_int(fixed_mul(g, fixed_255));
						ib = fixed_int(fixed_mul(b, fixed_255));
					}
#endif

#ifdef INTERP_TEX
					if(tex) {
						int tx, ty;
						uint32_t texel;

#ifdef INTERP_W
						if(!affine_tex) {
							u = fixed_div(u, w);
							v = fixed_div(v, w);
						}
#endif
						tx = fixed_round(fixed_mul(u, fixedi(tex->x))) & tex->xmask;
						ty = fixed_round(fixed_mul(v, fixedi(tex->y))) & tex->ymask;

						texel = tex->pixels[(ty << tex->xpow) + tx];
						ir = (ir * GET_R(texel)) >> 8;
						ig = (ig * GET_G(texel)) >> 8;
						ib = (ib * GET_B(texel)) >> 8;
						ia = (ia * GET_A(texel)) >> 8;
					}
#endif
				
#ifdef ENABLE_BLENDING
					if(IS_ENABLED(GL_BLEND) && !(state.src_blend == GL_ONE && state.dst_blend == GL_ZERO)) {
						*cptr = blend(ir, ig, ib, ia, GET_R(*cptr), GET_G(*cptr), GET_B(*cptr), GET_A(*cptr));
					} else
#endif
					{
						*cptr = PACK_COLOR32(ia, ir, ig, ib);
					}

#ifdef INTERP_Z
					if(IS_ENABLED(GL_DEPTH_WRITE)) *zptr = zval;
				}
#endif
			}

			cptr++;
#ifdef INTERP_Z
			zptr++;
			z += zslope;
#endif

#ifdef INTERP_COL
			if(gouraud) {
				a += aslope;
				r += rslope;
				g += gslope;
				b += bslope;
			}
#endif
#ifdef INTERP_TEX
			u += uslope;
			v += vslope;
#endif
#ifdef INTERP_W
			w += wslope;
#endif
#ifdef INTERP_NORM
			if(phong) {
				nx += nxslope;
				ny += nyslope;
				nz += nzslope;
			
				vx += vxslope;
				vy += vyslope;
				vz += vzslope;
			}
#endif
		}

	}
}

void gl_draw_rect(int x1, int y1, int x2, int y2) {
}

#ifdef ENABLE_BLENDING
static uint32_t blend(int r, int g, int b, int a, int fb_r, int fb_g, int fb_b, int fb_a) {
	int i;
	int sr, sg, sb, dr, dg, db;	/* source/dest factors */

	for(i=0; i<2; i++) {
		int *fr = i ? &dr : &sr;
		int *fg = i ? &dg : &sg;
		int *fb = i ? &db : &sb;
	
		switch(i ? state.dst_blend : state.src_blend) {
		case GL_ONE: 
			*fr = *fg = *fb = 255;
			break;
		case GL_ZERO:
			*fr = *fg = *fb = 0;
			break;
		case GL_SRC_COLOR:
			*fr = r;
			*fg = g;
			*fb = b;
			break;
		case GL_ONE_MINUS_SRC_COLOR:
			*fr = 255 - r;
			*fg = 255 - g;
			*fb = 255 - b;
			break;
		case GL_DST_COLOR:
			*fr = fb_r;
			*fg = fb_g;
			*fb = fb_b;
			break;
		case GL_ONE_MINUS_DST_COLOR:
			*fr = 255 - fb_r;
			*fg = 255 - fb_g;
			*fb = 255 - fb_b;
			break;
		case GL_SRC_ALPHA:
			*fr = *fg = *fb = a;
			break;
		case GL_ONE_MINUS_SRC_ALPHA:
			*fr = *fg = *fb = 255 - a;
			break;
		case GL_DST_ALPHA:
			*fr = *fg = *fb = fb_a;
			break;
		case GL_ONE_MINUS_DST_ALPHA:
			*fr = *fg = *fb = 255 - fb_a;
			break;
		}
	}

	r = (r * sr + fb_r * dr) >> 8;
	g = (g * sg + fb_g * dg) >> 8;
	b = (b * sb + fb_b * db) >> 8;

	r = CLAMP(r, 0, 255);
	g = CLAMP(g, 0, 255);
	b = CLAMP(b, 0, 255);

	return PACK_COLOR32(a, r, g, b);
}
#endif	/* ENABLE_BLENDING */
