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
#include <math.h>
#include <stddef.h>
#include "gl.h"
#include "gl_rasterizer.h"
#include "types.h"
#include "color_bits.h"
#include "vmath.h"
#include "state.h"
#include "shade.h"

#define DEG_TO_RADF(a) 	(((float)a) * (3.1415926535897932 / 180.0))
#define DEG_TO_RADX(a)	(fixed_mul((a), fixedf(3.141592653 / 180.0)))
#define CLAMP(x, a, b)	((x) < (a) ? (a) : ((x) > (b) ? (b) : (x)))

/* this holds the global OpenGL state */
struct state state;

static int which_pow2(int n);

void fglCreateContext(void) {
	int i;

	state.s = 0;
	state.in_beg_end = 0;
	
	glEnable(GL_SMOOTH);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_DEPTH_WRITE);
	
	for(i=0; i<MODE_COUNT; i++) {
		state.stack_top[i] = 1;
	}
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, 1.333333, 1.0, 1000.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	memset(state.tex, 0, MAX_TEXTURES * sizeof(void*));
	state.btex = 0;
	
	state.prim = GL_TRIANGLES;

	state.mvp_valid = 0;

	state.r = state.g = state.b = 0;
	state.a = fixedi(1);
	state.nx = state.ny = state.nz = 0;
	state.tu = state.tv = 0;

	state.lpos[0].x = 0;
	state.lpos[0].y = 0;
	state.lpos[0].z = -fixed_one;
	state.lpos[0].w = 0;

	state.ambient.x = state.ambient.y = state.ambient.z = 0;
	state.diffuse.x = state.diffuse.y = state.diffuse.z = fixed_half;
	state.diffuse.w = fixed_one;
	state.specular.x = state.specular.y = state.specular.z = 0;
	state.shininess = fixed_one;

	state.ambient_light.x = state.ambient_light.y = state.ambient_light.z = state.ambient_light.w = 0;

	state.src_blend = GL_ONE;
	state.dst_blend = GL_ZERO;

	state.gl_error = 0;

	state.fb.color_buffer = state.fb.depth_buffer = 0;
}

void fglDestroyContext(void) {
	free(state.fb.color_buffer);
	free(state.fb.depth_buffer);
}

void glViewport(GLint x, GLint y, GLsizei w, GLsizei h) {
	CHECK_BEG_END();

	/* NOTE: for the moment ignore x/y */
	state.fb.x = w;
	state.fb.y = h;

	free(state.fb.color_buffer);
	free(state.fb.depth_buffer);
	
	if(!(state.fb.color_buffer = malloc(w * h * sizeof(uint32_t)))) {
		return;
	}

	if(!(state.fb.depth_buffer = malloc(w * h * sizeof(uint32_t)))) {
		free(state.fb.color_buffer);
		state.fb.color_buffer = 0;
		return;
	}

	gl_rasterizer_setup(&state.fb);
}

GLuint *fglGetFrameBuffer(void) {
	return state.fb.color_buffer;
}

/* clear */
void glClearColor(GLclampf r, GLclampf g, GLclampf b, GLclampf a) {
	CHECK_BEG_END();
	state.clear_r = fixedf(CLAMP(r, 0.0f, 1.0f));
	state.clear_g = fixedf(CLAMP(g, 0.0f, 1.0f));
	state.clear_b = fixedf(CLAMP(b, 0.0f, 1.0f));
	state.clear_a = fixedf(CLAMP(a, 0.0f, 1.0f));
}

void glClearColorx(GLfixed r, GLfixed g, GLfixed b, GLfixed a) {
	CHECK_BEG_END();
	state.clear_r = CLAMP(r, 0, fixed_one);
	state.clear_g = CLAMP(g, 0, fixed_one);
	state.clear_b = CLAMP(b, 0, fixed_one);
	state.clear_a = CLAMP(a, 0, fixed_one);
}

void glClearDepth(GLclampd d) {
	CHECK_BEG_END();
	state.clear_depth = fixedf(CLAMP(d, 0.0, 1.0));
}

void glClearDepthx(GLfixed d) {
	CHECK_BEG_END();
	state.clear_depth = CLAMP(d, 0, fixed_one);
}

void glClear(GLbitfield what) {
	int i, sz;
	uint32_t *cptr, *zptr, col, zval;
	fixed r, g, b, a;
	GLbitfield all;

	all =	GL_COLOR_BUFFER_BIT |
			GL_DEPTH_BUFFER_BIT |
			GL_ACCUM_BUFFER_BIT |
			GL_STENCIL_BUFFER_BIT;

	if(what & ~all) {
		GL_ERROR(GL_INVALID_VALUE);
		return;
	}
	CHECK_BEG_END();

	sz = state.fb.x * state.fb.y;
	cptr = state.fb.color_buffer;
	zptr = state.fb.depth_buffer;
	r = fixed_int(fixed_mul(state.clear_r, fixed_255));
	g = fixed_int(fixed_mul(state.clear_g, fixed_255));
	b = fixed_int(fixed_mul(state.clear_b, fixed_255));
	a = fixed_int(fixed_mul(state.clear_a, fixed_255));
	col = PACK_COLOR32(a, r, g, b);
	zval = state.clear_depth;
	
	for(i=0; i<sz; i++) {
		if(what & GL_COLOR_BUFFER_BIT) {
			*cptr++ = col;
		}

		if(what & GL_DEPTH_BUFFER_BIT) {
			*zptr++ = zval;
		}
	}
}


/* general state */
void glEnable(GLenum what) {
	CHECK_BEG_END();
	state.s |= 1 << what;
}

void glDisable(GLenum what) {
	CHECK_BEG_END();
	state.s &= ~(1 << what);
}

GLboolean glIsEnabled(GLenum what) {
	if(what >= _STATE_BITS_COUNT) {
		GL_ERROR(GL_INVALID_ENUM);
		return 0;
	}
	if(state.in_beg_end) {
		GL_ERROR(GL_INVALID_OPERATION);
		return 0;
	}
	return IS_ENABLED(what) ? GL_TRUE : GL_FALSE;
}

void glLightModeli(GLenum pname, GLint val) {
	CHECK_BEG_END();

	switch(pname) {
	case GL_LIGHT_MODEL_LOCAL_VIEWER:
		if(val) {
			glEnable(GL_LIGHT_MODEL_LOCAL_VIEWER);
		} else {
			glDisable(GL_LIGHT_MODEL_LOCAL_VIEWER);
		}
		break;

	case GL_LIGHT_MODEL_COLOR_CONTROL:
		if(val == GL_SINGLE_COLOR) {
			/* TODO */
		} else if(val == GL_SEPARATE_SPECULAR_COLOR) {
			/* TODO */
		} else {
			GL_ERROR(GL_INVALID_ENUM);
		}
		break;

	case GL_LIGHT_MODEL_TWO_SIDE:
		/* TODO */
		break;

	default:
		GL_ERROR(GL_INVALID_ENUM);
		break;
	}
}

void glLightModelf(GLenum pname, GLfloat param) {
	glLightModeli(pname, (GLint)param);
}

void glLightModelfv(GLenum pname, GLfloat *params) {
	CHECK_BEG_END();

	switch(pname) {
	case GL_LIGHT_MODEL_AMBIENT:
		state.ambient_light.x = fixedf(params[0]);
		state.ambient_light.y = fixedf(params[1]);
		state.ambient_light.z = fixedf(params[2]);
		state.ambient_light.w = fixedf(params[3]);
		break;

	case GL_LIGHT_MODEL_COLOR_CONTROL:
	case GL_LIGHT_MODEL_LOCAL_VIEWER:
	case GL_LIGHT_MODEL_TWO_SIDE:
		glLightModeli(pname, (GLint)params[0]);
		break;

	default:
		GL_ERROR(GL_INVALID_ENUM);
		break;
	}
}

void glLightModeliv(GLenum pname, GLint *params) {
	CHECK_BEG_END();

	switch(pname) {
	case GL_LIGHT_MODEL_AMBIENT:
		state.ambient_light.x = params[0] >> (FIXED_SHIFT - 1);
		state.ambient_light.y = params[1] >> (FIXED_SHIFT - 1);
		state.ambient_light.z = params[2] >> (FIXED_SHIFT - 1);
		state.ambient_light.w = params[3] >> (FIXED_SHIFT - 1);
		break;

	case GL_LIGHT_MODEL_COLOR_CONTROL:
	case GL_LIGHT_MODEL_LOCAL_VIEWER:
	case GL_LIGHT_MODEL_TWO_SIDE:
		glLightModeli(pname, params[0]);
		break;

	default:
		GL_ERROR(GL_INVALID_ENUM);
		break;
	}
}

void glBlendFunc(GLenum src, GLenum dst) {
	if(src < GL_ZERO || src > GL_ONE_MINUS_DST_ALPHA ||
			dst < GL_ZERO || dst > GL_ONE_MINUS_DST_ALPHA) {
		GL_ERROR(GL_INVALID_ENUM);
		return;
	}
	CHECK_BEG_END();

	state.src_blend = src;
	state.dst_blend = dst;
}

void glShadeModel(GLenum mode) {
	CHECK_BEG_END();
	switch(mode) {
	case GL_FLAT:
		glDisable(GL_SMOOTH);
		break;

	case GL_SMOOTH:
		glEnable(GL_SMOOTH);
		break;

	case GL_PHONG:
		glEnable(GL_PHONG);
		break;

	default:
		GL_ERROR(GL_INVALID_ENUM);
		break;
	}
}

/* zbuffer state */
void glDepthMask(GLboolean boolval) {
	CHECK_BEG_END();
	if(boolval) {
		glEnable(GL_DEPTH_WRITE);
	} else {
		glDisable(GL_DEPTH_WRITE);
	}
}

/* lights and materials */
void glLightf(GLenum light, GLenum pname, GLfloat param) {
	glLightx(light, pname, fixedf(param));
}

void glLightx(GLenum light, GLenum pname, GLfixed param) {
	CHECK_BEG_END();

	switch(pname) {
	case GL_SPOT_EXPONENT:
	case GL_SPOT_CUTOFF:
	case GL_CONSTANT_ATTENUATION:
	case GL_LINEAR_ATTENUATION:
	case GL_QUADRATIC_ATTENUATION:
		/* TODO */
		break;

	default:
		GL_ERROR(GL_INVALID_ENUM);
	}
}

void glLighti(GLenum light, GLenum pname, GLint param) {
	glLightx(light, pname, fixedi(param));
}

void glLightfv(GLenum light, GLenum pname, GLfloat *params) {
	fixed xparams[4];
	xparams[0] = fixedf(params[0]);
	xparams[1] = fixedf(params[1]);
	xparams[2] = fixedf(params[2]);
	xparams[3] = fixedf(params[3]);
	glLightxv(light, pname, xparams);
}
void glLightxv(GLenum light, GLenum pname, GLfixed *params) {
	int index = light - GL_LIGHT0;
	vec3 *tmp = (vec3*)params;

	if(index >= MAX_LIGHTS) {
		GL_ERROR(GL_INVALID_ENUM);
		return;
	}

	CHECK_BEG_END();
	
	switch(pname) {
	case GL_AMBIENT:
	case GL_DIFFUSE:
	case GL_SPECULAR:
		/* TODO */
		break;

	case GL_POSITION:
		{
			int mvtop = state.stack_top[MODE_MODELVIEW] - 1;
			state.lpos[index] = vm_transform(*tmp, state.mstack[MODE_MODELVIEW][mvtop]);
			if(tmp->w == 0) {
				vm_normalize(&state.lpos[index]);
			}
		}
		break;

	case GL_SPOT_CUTOFF:
	case GL_SPOT_DIRECTION:
	case GL_SPOT_EXPONENT:
	case GL_CONSTANT_ATTENUATION:
	case GL_LINEAR_ATTENUATION:
	case GL_QUADRATIC_ATTENUATION:
		/* TODO */
		break;

	default:
		GL_ERROR(GL_INVALID_ENUM);
	}
}

void glLightiv(GLenum light, GLenum pname, GLint *params) {
	fixed xparams[4];
	xparams[0] = fixedi(params[0]);
	xparams[1] = fixedi(params[1]);
	xparams[2] = fixedi(params[2]);
	xparams[3] = fixedi(params[3]);
	glLightxv(light, pname, xparams);
}

void glMaterialf(GLenum face, GLenum pname, GLfloat param) {
	glMaterialx(face, pname, fixedf(param));
}

void glMateriali(GLenum face, GLenum pname, GLint param) {
	glMaterialx(face, pname, fixedi(param));
}

void glMaterialx(GLenum face, GLenum pname, GLfixed param) {
	if(face < GL_FRONT || face > GL_FRONT_AND_BACK) {
		GL_ERROR(GL_INVALID_ENUM);
		return;
	}

	if(pname == GL_SHININESS) {
		if(param > fixedi(128)) {
			GL_ERROR(GL_INVALID_VALUE);
			return;
		}
		state.shininess = param;
	} else {
		GL_ERROR(GL_INVALID_ENUM);
	}
}


void glMaterialfv(GLenum face, GLenum pname, GLfloat *params) {
	fixed xparams[4];
	xparams[0] = fixedf(params[0]);
	xparams[1] = fixedf(params[1]);
	xparams[2] = fixedf(params[2]);
	xparams[3] = fixedf(params[3]);
	glMaterialxv(face, pname, xparams);
}

void glMaterialiv(GLenum face, GLenum pname, GLint *params) {
	GLfixed xparams[4];
	xparams[0] = fixedi(params[0]);
	xparams[1] = fixedi(params[1]);
	xparams[2] = fixedi(params[2]);
	xparams[3] = fixedi(params[3]);
	glMaterialxv(face, pname, xparams);
}

void glMaterialxv(GLenum face, GLenum pname, GLfixed *params) {
	if(face < GL_FRONT || face > GL_FRONT_AND_BACK) {
		GL_ERROR(GL_INVALID_ENUM);
		return;
	}

	switch(pname) {
	case GL_AMBIENT_AND_DIFFUSE:
		state.ambient.x = state.diffuse.x = params[0];
		state.ambient.y = state.diffuse.y = params[1];
		state.ambient.z = state.diffuse.z = params[2];
		state.ambient.w = state.diffuse.w = params[3];
		break;
		
	case GL_AMBIENT:
		state.ambient.x = params[0];
		state.ambient.y = params[1];
		state.ambient.z = params[2];
		state.ambient.w = params[3];
		break;

	case GL_DIFFUSE:
		state.diffuse.x = params[0];
		state.diffuse.y = params[1];
		state.diffuse.z = params[2];
		state.diffuse.w = params[3];
		break;

	case GL_SPECULAR:
		state.specular.x = params[0];
		state.specular.y = params[1];
		state.specular.z = params[2];
		state.specular.w = params[3];
		break;

	case GL_EMISSION:
		/* TODO */
		break;

	case GL_SHININESS:
		if(params[0] > fixedi(128)) {
			GL_ERROR(GL_INVALID_VALUE);
			return;
		}
		state.shininess = params[0];
		break;

	case GL_COLOR_INDEXES:
		/* TODO (NOT) */
		break;

	default:
		GL_ERROR(GL_INVALID_ENUM);
	}
}


/* texture state */
void glGenTextures(GLsizei n, GLuint *tex) {
	int ti = 1;

	CHECK_BEG_END();

	memset(tex, 0, n * sizeof *tex);

	while(n-- > 0) {
		while(ti < MAX_TEXTURES && state.tex[ti]) ti++;
		if(ti >= MAX_TEXTURES) return;

		state.tex[ti] = malloc(sizeof(struct tex2d));
		memset(state.tex[ti], 0, sizeof(struct tex2d));
		*tex++ = ti;
	}
}

void glDeleteTextures(GLsizei n, const GLuint *tex) {
	CHECK_BEG_END();

	while(n-- > 0) {
		if(state.tex[*tex]) {
			free(state.tex[*tex]->pixels);
			free(state.tex[*tex]);
		}
		state.tex[*tex++] = 0;
	}
}

GLboolean glIsTexture(GLuint tex) {
	if(state.in_beg_end) {
		GL_ERROR(GL_INVALID_OPERATION);
		return 0;
	}
	return tex > 0 && tex < MAX_TEXTURES && state.tex[tex] != 0;
}

void glBindTexture(GLenum targ, GLuint tex) {
	if(targ < GL_TEXTURE_1D || targ > GL_TEXTURE_3D) {
		GL_ERROR(GL_INVALID_ENUM);
		return;
	}
	/* TODO: if tex-card != targ-card GL_INVALID_OPERATION */
	CHECK_BEG_END();

	state.btex = glIsTexture(tex) ? tex : 0;
}

/* lvl, ifmt and border are ignored...
 * - textures are always stored as 32bit images internally.
 * - no border
 * - no mipmaps
 * TODO: handle various pixel formats correctly (not internal).
 */
void glTexImage2D(GLenum targ, GLint lvl, GLint ifmt, GLsizei w, GLsizei h, GLint border, GLenum fmt, GLenum type, const GLvoid *pixels) {
	struct tex2d *tex;
	int xp2;

	if(targ != GL_TEXTURE_2D && targ != GL_PROXY_TEXTURE_2D) {
		GL_ERROR(GL_INVALID_ENUM);
		return;
	}
	if(lvl < 0 || (ifmt < 1 || ifmt > 4) || (border < 0 || border > 1)) {
		GL_ERROR(GL_INVALID_VALUE);
		return;
	}
	CHECK_BEG_END();

	if(!state.btex || (xp2 = which_pow2(w)) == -1) {
		return;
	}
	tex = state.tex[state.btex];
	
	tex->type = targ;
	tex->pixels = malloc(w * h * 4);
	if(pixels) {
		memcpy(tex->pixels, pixels, w * h * 4);
	}
	tex->x = w;
	tex->y = h;
	tex->xpow = xp2;
	tex->xmask = w - 1;
	tex->ymask = h - 1;
}


/* matrix manipulation */
void glMatrixMode(GLenum mode) {
	unsigned int mmode = mode - GL_MODELVIEW;
	int top;

	if(mmode >= MODE_COUNT) {
		GL_ERROR(GL_INVALID_ENUM);
		return;
	}
	CHECK_BEG_END();

	top = state.stack_top[mmode];
	state.matrix_mode = mmode;
}

void glLoadIdentity(void) {
	int top = state.stack_top[state.matrix_mode] - 1;
	fixed *mat = state.mstack[state.matrix_mode][top];

	CHECK_BEG_END();
	memset(mat, 0, 16 * sizeof *mat);
	mat[0] = mat[5] = mat[10] = mat[15] = fixed_one;
	state.mvp_valid = 0;
}

void glLoadMatrixf(GLfloat *mat) {
	int i;
	int top = state.stack_top[state.matrix_mode] - 1;
	fixed *stmat = state.mstack[state.matrix_mode][top];
	
	CHECK_BEG_END();

	for(i=0; i<16; i++) {
		stmat[i] = fixedf(mat[i]);
	}
	state.mvp_valid = 0;
}

void glLoadMatrixx(GLfixed *mat) {
	int top = state.stack_top[state.matrix_mode] - 1;
	fixed *stmat = state.mstack[state.matrix_mode][top];
	
	CHECK_BEG_END();
	memcpy(stmat, mat, 16 * sizeof *stmat);
	state.mvp_valid = 0;
}

#define M(i, j)	((i << 2) + j)

void glMultMatrixf(GLfloat *m2) {
	int i;
	fixed xm2[16];
	int top = state.stack_top[state.matrix_mode] - 1;
	fixed *stmat = state.mstack[state.matrix_mode][top];

	CHECK_BEG_END();

	for(i=0; i<16; i++) {
		xm2[i] = fixedf(m2[i]);
	}
	vm_mult_matrix(stmat, stmat, xm2);
	state.mvp_valid = 0;
}

void glMultMatrixx(GLfixed *m2) {
	int top = state.stack_top[state.matrix_mode] - 1;
	fixed *stmat = state.mstack[state.matrix_mode][top];
	CHECK_BEG_END();

	vm_mult_matrix(stmat, stmat, m2);
	state.mvp_valid = 0;
}

void glPushMatrix(void) {
	int mmode = state.matrix_mode;
	int top = state.stack_top[mmode] - 1;

	if(top >= MATRIX_STACK_SIZE) {
		GL_ERROR(GL_STACK_OVERFLOW);
		return;
	}
	CHECK_BEG_END();

	memcpy(state.mstack[mmode][top + 1], state.mstack[mmode][top], 16 * sizeof(fixed));
	state.stack_top[mmode]++;
	state.mvp_valid = 0;
}

void glPopMatrix(void) {
	int mmode = state.matrix_mode;
	int top = state.stack_top[mmode];

	if(top <= 1) {	/* there must always be at least one current matrix */
		GL_ERROR(GL_STACK_UNDERFLOW);
		return;
	}
	CHECK_BEG_END();

	state.stack_top[mmode]--;
	state.mvp_valid = 0;
}

void glTranslatef(GLfloat x, GLfloat y, GLfloat z) {
	glTranslatex(fixedf(x), fixedf(y), fixedf(z));
}

void glTranslatex(GLfixed x, GLfixed y, GLfixed z) {
	fixed tmat[16] = {fixedi(1), 0, 0, 0, 0, fixedi(1), 0, 0, 0, 0, fixedi(1), 0, 0, 0, 0, fixedi(1)};
	CHECK_BEG_END();

	tmat[3] = x;
	tmat[7] = y;
	tmat[11] = z;
	glMultMatrixx(tmat);
}

void glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z) {
	float sina, cosa, invcosa, nxsq, nysq, nzsq, len;
	float rmat[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};

	CHECK_BEG_END();

	len = (float)sqrt(x * x + y * y + z * z);
	x /= len;
	y /= len;
	z /= len;

	sina = (float)sin(DEG_TO_RADF(angle));
	cosa = (float)cos(DEG_TO_RADF(angle));
	invcosa = 1.0f - cosa;
	nxsq = x * x;
	nysq = y * y;
	nzsq = z * z;
	
	rmat[M(0, 0)] = nxsq + (1.0f - nxsq) * cosa;
	rmat[M(0, 1)] = x * y * invcosa - z * sina;
	rmat[M(0, 2)] = x * z * invcosa + y * sina;
	rmat[M(1, 0)] = x * y * invcosa + z * sina;
	rmat[M(1, 1)] = nysq + (1.0f - nysq) * cosa;
	rmat[M(1, 2)] = y * z * invcosa - x * sina;
	rmat[M(2, 0)] = x * z * invcosa - y * sina;
	rmat[M(2, 1)] = y * z * invcosa + x * sina;
	rmat[M(2, 2)] = nzsq + (1.0f - nzsq) * cosa;

	glMultMatrixf(rmat);
}

void glRotatex(GLfixed angle, GLfixed x, GLfixed y, GLfixed z) {
	glRotatef(fixed_float(angle), fixed_float(x), fixed_float(y), fixed_float(z));
}

void glRotateEulerf(GLfloat x, GLfloat y, GLfloat z) {
	float mat[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};

	x = DEG_TO_RADF(x);
	y = DEG_TO_RADF(y);
	z = DEG_TO_RADF(z);

	mat[M(1,1)] = (float)cos(x);
	mat[M(1,2)] = -(float)sin(x);
	mat[M(2,1)] = (float)sin(x);
	mat[M(2,2)] = (float)cos(x);
	glMultMatrixf(mat);

	mat[M(1,1)] = 1.0f;
	mat[M(1,2)] = mat[M(2,1)] = 0.0f;
	mat[0] = (float)cos(y);
	mat[M(0,2)] = (float)sin(y);
	mat[M(2,0)] = -(float)sin(y);
	mat[M(2,2)] = (float)cos(y);
	glMultMatrixf(mat);

	mat[M(2,0)] = mat[M(0,2)] = 0.0f;
	mat[M(2,2)] = 1.0f;
	mat[0] = (float)cos(z);
	mat[1] = -(float)sin(z);
	mat[M(1,0)] = (float)sin(z);
	mat[M(1,1)] = (float)cos(z);
	glMultMatrixf(mat);
}

void glRotateEulerx(GLfixed x, GLfixed y, GLfixed z) {
	glRotateEulerf(fixed_float(x), fixed_float(y), fixed_float(z));
}

void glScalef(GLfloat x, GLfloat y, GLfloat z) {
	glScalex(fixedf(x), fixedf(y), fixedf(z));
}

void glScalex(GLfixed x, GLfixed y, GLfixed z) {
	fixed smat[16] = {fixedi(1), 0, 0, 0, 0, fixedi(1), 0, 0, 0, 0, fixedi(1), 0, 0, 0, 0, fixedi(1)};
	smat[0] = x;
	smat[5] = y;
	smat[10] = z;
	glMultMatrixx(smat);
}

void gluPerspective(GLfloat fovy, GLfloat aspect, GLfloat znear, GLfloat zfar) {
	float m[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
	float f, dz;
	
	fovy = DEG_TO_RADF(fovy);
	f = 1.0f / tan(fovy * 0.5f);
	dz = znear - zfar;

	m[M(0, 0)] = f / aspect;
	m[M(1, 1)] = f;
	m[M(2, 2)] = (zfar + znear) / dz;
	m[M(3, 2)] = -1.0f;
	m[M(2, 3)] = 2.0f * zfar * znear / dz;
	m[M(3, 3)] = 0.0f;
	
	glMultMatrixf(m);
}

#define CROSS(rx, ry, rz, x1, y1, z1, x2, y2, z2) \
	do { \
		(rx) = (y1) * (z2) - (z1) * (y2); \
		(ry) = (z1) * (x2) - (x1) * (z2); \
		(rz) = (x1) * (y2) - (y1) * (x2); \
	} while(0)

void gluLookAt(GLfloat x, GLfloat y, GLfloat z, GLfloat tx, GLfloat ty, GLfloat tz, GLfloat ux, GLfloat uy, GLfloat uz)
{
	float m[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
	float len, vx, vy, vz, rx, ry, rz;

	vx = tx - x;
	vy = ty - y;
	vz = tz - z;

	len = sqrt(vx * vx + vy * vy + vz * vz);
	vx /= len; vy /= len; vz /= len;

	len = sqrt(ux * ux + uy * uy + uz * uz);
	ux /= len; uy /= len; uz /= len;

	CROSS(rx, ry, rz, vx, vy, vz, ux, uy, uz);
	CROSS(ux, uy, uz, rx, ry, rz, vx, vy, vz);

	m[0] = rx; m[1] = ry; m[2] = rz;
	m[4] = ux; m[5] = uy; m[6] = uz;
	m[8] = -vx; m[9] = -vy; m[10] = -vz;;

	/*m[0] = rx; m[4] = ry; m[8] = rz;
	m[1] = ux; m[5] = uy; m[9] = uz;
	m[2] = -vx; m[6] = -vy; m[10] = -vz;*/

	glMultMatrixf(m);
	glTranslatef(-x, -y, -z);
}


void glPointSize(GLfloat sz) {
	glPointSizex(fixedf(sz));
}

void glPointSizex(GLfixed sz) {
	state.point_sz = sz;
}

GLenum glGetError(void) {
	unsigned int tmp = state.gl_error;

	if(state.in_beg_end) {
		state.gl_error = GL_INVALID_OPERATION;
		return 0;
	}

	state.gl_error = 0;
	return tmp;
}

void glFlush(void) {}
void glFinish(void) {}

static int which_pow2(int x) {
	int p = 0;
	int tmpx = 1;
	
	while(p < MAX_TEXTURE_SIZE && tmpx < x) {
		p++;
		tmpx <<= 1;
	}

	return x != tmpx ? 0 : p;
}
