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

#ifndef FIXED_GL_H_
#define FIXED_GL_H_

#include <limits.h>
#include "types.h"
#include "fixed_point.h"

enum {
	GL_SMOOTH,
	GL_LIGHTING,
	GL_LIGHT0,
	GL_LIGHT1,
	GL_LIGHT2,
	GL_LIGHT3,
	GL_TEXTURE_1D,
	GL_TEXTURE_2D,
	GL_TEXTURE_3D,
	GL_TEXTURE_CUBE,
	GL_DEPTH_TEST,
	GL_DEPTH_WRITE, /* set through glDepthMask() rather than glEnable/glDisable */
	GL_BLEND,
	GL_LIGHT_MODEL_LOCAL_VIEWER,	/* set through glLightModel() */
	GL_LIGHT_MODEL_TWO_SIDE,		/* set through glLightModel() */
	GL_NORMALIZE,
	GL_POINT_SMOOTH,
	GL_LINE_SMOOTH,
	GL_POLYGON_SMOOTH,
	GL_PHONG,	/* oh yeah :) */
	/* -- end of bit fields (acually bit offsets) -- */
	_STATE_BITS_COUNT,

	GL_PROXY_TEXTURE_1D,
	GL_PROXY_TEXTURE_2D,
	GL_PROXY_TEXTURE_3D,
	GL_PROXY_TEXTURE_CUBE,
	GL_FLAT,
	GL_LIGHT_MODEL_AMBIENT,
	GL_LIGHT_MODEL_COLOR_CONTROL,
	GL_SINGLE_COLOR,
	GL_SEPARATE_SPECULAR_COLOR,

	GL_MODELVIEW = 100,
	GL_PROJECTION,
	GL_TEXTURE,

	GL_POINTS = 200,
	GL_LINES,
	GL_LINE_STRIP,
	GL_LINE_LOOP,
	GL_TRIANGLES,
	GL_TRIANGLE_STRIP,	/* XXX: not implemented */
	GL_TRIANGLE_FAN,	/* XXX: not implemented */
	GL_QUADS,
	GL_QUAD_STRIP,		/* XXX: not implemented */
	GL_POLYGON,

	GL_COLOR_BUFFER_BIT = 300,
	GL_DEPTH_BUFFER_BIT,
	GL_ACCUM_BUFFER_BIT,
	GL_STENCIL_BUFFER_BIT,

	GL_POSITION = 400,
	GL_AMBIENT,
	GL_DIFFUSE,
	GL_AMBIENT_AND_DIFFUSE,
	GL_SPECULAR,
	GL_EMISSION,
	GL_SHININESS,
	GL_COLOR_INDEXES,

	GL_SPOT_EXPONENT,
	GL_SPOT_CUTOFF,
	GL_SPOT_DIRECTION,
	GL_CONSTANT_ATTENUATION,
	GL_LINEAR_ATTENUATION,
	GL_QUADRATIC_ATTENUATION,

	GL_FRONT = 450,
	GL_BACK,
	GL_FRONT_AND_BACK,

	GL_ZERO = 500,
	GL_ONE,
	GL_SRC_COLOR,
	GL_ONE_MINUS_SRC_COLOR,
	GL_DST_COLOR,
	GL_ONE_MINUS_DST_COLOR,
	GL_SRC_ALPHA,
	GL_ONE_MINUS_SRC_ALPHA,
	GL_DST_ALPHA,
	GL_ONE_MINUS_DST_ALPHA,

	GL_RGB = 700,
	GL_RGBA,
	GL_BGR,
	GL_BGRA,
	
	GL_UNSIGNED_BYTE = 800,
	GL_UNSIGNED_SHORT
};

enum {
	GL_NO_ERROR,
	GL_INVALID_ENUM,
	GL_INVALID_VALUE,
	GL_INVALID_OPERATION,
	GL_STACK_OVERFLOW,
	GL_STACK_UNDERFLOW,
	GL_OUT_OF_MEMORY,
	GL_TABLE_TOO_LARGE
};

#ifdef __cplusplus
extern "C" {
#endif	/* __cplusplus */

void fglCreateContext(void);
void fglDestroyContext(void);
GLuint *fglGetFrameBuffer(void);

void glViewport(GLint x, GLint y, GLsizei w, GLsizei h);

/* clear */
void glClearColor(GLclampf r, GLclampf g, GLclampf b, GLclampf a);
void glClearColorx(GLfixed r, GLfixed g, GLfixed b, GLfixed a);
void glClearDepth(GLclampd d);
void glClearDepthx(GLfixed d);
void glClear(GLbitfield what);

/* general state */
void glEnable(GLenum what);
void glDisable(GLenum what);
GLboolean glIsEnabled(GLenum what);

void glLightModeli(GLenum pname, GLint val);
void glLightModelf(GLenum pname, GLfloat val);
void glLightModeliv(GLenum pname, GLint *val);
void glLightModelfv(GLenum pname, GLfloat *val);

void glBlendFunc(GLenum src, GLenum dst);
void glShadeModel(GLenum mode);

/* zbuffer state */
void glDepthMask(GLboolean boolval);

/* lights and materials */
void glLightf(GLenum light, GLenum pname, GLfloat param);
void glLightx(GLenum light, GLenum pname, GLfixed param);
void glLighti(GLenum light, GLenum pname, GLint param);
void glLightfv(GLenum light, GLenum pname, GLfloat *params);
void glLightxv(GLenum light, GLenum pname, GLfixed *params);
void glLightiv(GLenum light, GLenum pname, GLint *params);

void glMaterialf(GLenum face, GLenum pname, GLfloat param);
void glMateriali(GLenum face, GLenum pname, GLint param);
void glMaterialx(GLenum face, GLenum pname, GLfixed param);
void glMaterialfv(GLenum face, GLenum pname, GLfloat *params);
void glMaterialiv(GLenum face, GLenum pname, GLint *params);
void glMaterialxv(GLenum face, GLenum pname, GLfixed *params);

/* texture state */
void glGenTextures(GLsizei n, GLuint *tex);
void glDeleteTextures(GLsizei n, const GLuint *tex);
GLboolean glIsTexture(GLuint tex);
void glBindTexture(GLenum targ, GLuint tex);
void glTexImage2D(GLenum targ, GLint lvl, GLint ifmt, GLsizei w, GLsizei h, GLint border, GLenum fmt, GLenum type, const GLvoid *pixels);

/* matrix manipulation */
void glMatrixMode(GLenum mode);
void glLoadIdentity(void);
void glLoadMatrixf(GLfloat *mat);
void glLoadMatrixx(GLfixed *mat);
void glMultMatrixf(GLfloat *mat);
void glMultMatrixx(GLfixed *mat);

void glPushMatrix(void);
void glPopMatrix(void);

void glTranslatef(GLfloat x, GLfloat y, GLfloat z);
void glTranslatex(GLfixed x, GLfixed y, GLfixed z);
void glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
void glRotatex(GLfixed angle, GLfixed x, GLfixed y, GLfixed z);
void glRotateEulerf(GLfloat x, GLfloat y, GLfloat z);
void glRotateEulerx(GLfixed x, GLfixed y, GLfixed z);
void glScalef(GLfloat x, GLfloat y, GLfloat z);
void glScalex(GLfixed x, GLfixed y, GLfixed z);

void gluPerspective(GLfloat fovy, GLfloat aspect, GLfloat znear, GLfloat zfar);

/* rendering */
void glBegin(GLenum primitive);
void glEnd(void);


/* ----- vertices ----- */
#define glVertex2f(x, y)		glVertex3x(fixedf(x), fixedf(y), 0)
#define glVertex2d(x, y)		glVertex3x(fixedf(x), fixedf(y), 0)
#define glVertex2x(x, y)		glVertex3x(x, y, 0)
#define glVertex2s(x, y)		glVertex3x(fixedi(x), fixedi(y), 0)
#define glVertex2i(x, y)		glVertex3x(fixedi(x), fixedi(y), 0)
#define glVertex3f(x, y, z)		glVertex3x(fixedf(x), fixedf(y), fixedf(z))
#define glVertex3d(x, y, z)		glVertex3x(fixedf(x), fixedf(y), fixedf(z))
void glVertex3x(GLfixed x, GLfixed y, GLfixed z);
#define glVertex3s(x, y, z)		glVertex3x(fixedi(x), fixedi(y), fixedi(z))
#define glVertex3i(x, y, z)		glVertex3x(fixedi(x), fixedi(y), fixedi(z))
#define glVertex4f(x, y, z, w)	glVertex3x(fixedf(x), fixedf(y), fixedf(z))
#define glVertex4d(x, y, z, w)	glVertex3x(fixedf(x), fixedf(y), fixedf(z))
#define glVertex4x(x, y, z, w)	glVertex3x(x, y, z)
#define glVertex4s(x, y, z, w)	glVertex3x(fixedi(x), fixedi(y), fixedi(z))
#define glVertex4i(x, y, z, w)	glVertex3x(fixedi(x), fixedi(y), fixedi(z))

/* These can't be implemented as macros, due to multiple evaluation concerns.
 * For instance #define glVertex2fv(v) glVertex2f((v)[0], (v)[1]) evaluates v twice.
 * With GNU C or any C99 compiler it should be the same anyway
 */
static inline void glVertex2fv(const GLfloat *v)	{ glVertex2f(v[0], v[1]); }
static inline void glVertex2dv(const GLdouble *v)	{ glVertex2d(v[0], v[1]); }
static inline void glVertex2xv(const GLfixed *v)	{ glVertex2x(v[0], v[1]); }
static inline void glVertex2sv(const GLshort *v)	{ glVertex2s(v[0], v[1]); }
static inline void glVertex2iv(const GLint *v)		{ glVertex2i(v[0], v[1]); }
static inline void glVertex3fv(const GLfloat *v)	{ glVertex3f(v[0], v[1], v[2]); }
static inline void glVertex3dv(const GLdouble *v)	{ glVertex3d(v[0], v[1], v[2]); }
static inline void glVertex3xv(const GLfixed *v)	{ glVertex3x(v[0], v[1], v[2]); }
static inline void glVertex3sv(const GLshort *v)	{ glVertex3s(v[0], v[1], v[2]); }
static inline void glVertex3iv(const GLint *v)		{ glVertex3i(v[0], v[1], v[2]); }
static inline void glVertex4fv(const GLfloat *v)	{ glVertex4f(v[0], v[1], v[2], v[3]); }
static inline void glVertex4dv(const GLdouble *v)	{ glVertex4d(v[0], v[1], v[2], v[3]); }
static inline void glVertex4xv(const GLfixed *v)	{ glVertex4x(v[0], v[1], v[2], v[3]); }
static inline void glVertex4sv(const GLshort *v)	{ glVertex4s(v[0], v[1], v[2], v[3]); }
static inline void glVertex4iv(const GLint *v)		{ glVertex4i(v[0], v[1], v[2], v[3]); }

/* due to the limited range of fixed point variables, short and int variants are done with floating point math */
#define _MAP_SBYTE(x)	fixed_div(fixedi((int)(x) * 2 + 1), fixed_255)
#define _MAP_UBYTE(x)	fixed_div(fixedi(x), fixed_255)
#define _MAP_SSHRT(x)	fixedf((float)(2 * (int)(x) + 1) / 65535.0f)
#define _MAP_USHRT(x)	fixedf((float)(x) / 65535.0f)
#define _MAP_SINT(x)	fixedf((2.0 * (float)(x) + 1.0) / (float)INT_MAX)
#define _MAP_UINT(x)	fixedf((float)(x) / (float)UINT_MAX)

/* ----- colors ----- */
#define glColor3f(r, g, b)			glColor4x(fixedf(r), fixedf(g), fixedf(b), fixed_one)
#define glColor3d(r, g, b)			glColor4x(fixedf(r), fixedf(g), fixedf(b), fixed_one)
#define glColor3x(r, g, b)			glColor4x(r, g, b, fixed_one)
#define glColor3b(r, g, b)			glColor4x(_MAP_SBYTE(r), _MAP_SBYTE(g), _MAP_SBYTE(b), fixed_one)
#define glColor3ub(r, g, b)			glColor4x(_MAP_UBYTE(r), _MAP_UBYTE(g), _MAP_UBYTE(b), fixed_one)
#define glColor3s(r, g, b)			glColor4x(_MAP_SSHRT(r), _MAP_SSHRT(g), _MAP_SSHRT(b), fixed_one)
#define glColor3us(r, g, b)			glColor4x(_MAP_USHRT(r), _MAP_USHRT(g), _MAP_USHRT(b), fixed_one)
#define glColor3i(r, g, b)			glColor4x(_MAP_SINT(r), _MAP_SINT(g), _MAP_SINT(b), fixed_one)
#define glColor3ui(r, g, b)			glColor4x(_MAP_UINT(r), _MAP_UINT(g), _MAP_UINT(b), fixed_one)
#define glColor4f(r, g, b, a)		glColor4x(fixedf(r), fixedf(g), fixedf(b), fixedf(a))
#define glColor4d(r, g, b, a)		glColor4x(fixedf(r), fixedf(g), fixedf(b), fixedf(a))
void glColor4x(GLfixed r, GLfixed g, GLfixed b, GLfixed a);
#define glColor4b(r, g, b, a)		glColor4x(_MAP_SBYTE(r), _MAP_SBYTE(g), _MAP_SBYTE(b), _MAP_SBYTE(a))
#define glColor4ub(r, g, b, a)		glColor4x(_MAP_UBYTE(r), _MAP_UBYTE(g), _MAP_UBYTE(b), _MAP_UBYTE(a))
#define glColor4s(r, g, b, a)		glColor4x(_MAP_SSHRT(r), _MAP_SSHRT(g), _MAP_SSHRT(b), _MAP_SSHRT(a))
#define glColor4us(r, g, b, a)		glColor4x(_MAP_USHRT(r), _MAP_USHRT(g), _MAP_USHRT(b), _MAP_USHRT(a))
#define glColor4i(r, g, b, a)		glColor4x(_MAP_SINT(r), _MAP_SINT(g), _MAP_SINT(b), _MAP_SINT(a))
#define glColor4ui(r, g, b, a)		glColor4x(_MAP_UINT(r), _MAP_UINT(g), _MAP_UINT(b), _MAP_UINT(a))

static inline void glColor3bv(const GLbyte *v)		{ glColor3b(v[0], v[1], v[2]); }
static inline void glColor3dv(const GLdouble *v)	{ glColor3d(v[0], v[1], v[2]); }
static inline void glColor3fv(const GLfloat *v)		{ glColor3f(v[0], v[1], v[2]); }
static inline void glColor3xv(const GLfixed *v)		{ glColor3x(v[0], v[1], v[2]); }
static inline void glColor3iv(const GLint *v)		{ glColor3i(v[0], v[1], v[2]); }
static inline void glColor3sv(const GLshort *v)		{ glColor3s(v[0], v[1], v[2]); }
static inline void glColor3ubv(const GLubyte *v)	{ glColor3ub(v[0], v[1], v[2]); }
static inline void glColor3uiv(const GLuint *v)		{ glColor3ui(v[0], v[1], v[2]); }
static inline void glColor3usv(const GLushort *v)	{ glColor3us(v[0], v[1], v[2]); }
static inline void glColor4bv(const GLbyte *v)		{ glColor4b(v[0], v[1], v[2], v[3]); }
static inline void glColor4dv(const GLdouble *v)	{ glColor4d(v[0], v[1], v[2], v[3]); }
static inline void glColor4fv(const GLfloat *v)		{ glColor4f(v[0], v[1], v[2], v[3]); }
static inline void glColor4xv(const GLfixed *v)		{ glColor4x(v[0], v[1], v[2], v[3]); }
static inline void glColor4iv(const GLint *v)		{ glColor4i(v[0], v[1], v[2], v[3]); }
static inline void glColor4sv(const GLshort *v)		{ glColor4s(v[0], v[1], v[2], v[3]); }
static inline void glColor4ubv(const GLubyte *v)	{ glColor4ub(v[0], v[1], v[2], v[3]); }
static inline void glColor4uiv(const GLuint *v)		{ glColor4ui(v[0], v[1], v[2], v[3]); }
static inline void glColor4usv(const GLushort *v)	{ glColor4us(v[0], v[1], v[2], v[3]); }


/* ----- normals ----- */
#define glNormal3f(x, y, z)		glNormal3x(fixedf(x), fixedf(y), fixedf(z))
#define glNormal3d(x, y, z)		glNormal3x(fixedf(x), fixedf(y), fixedf(z))
void glNormal3x(GLfixed x, GLfixed y, GLfixed z);
#define glNormal3b(x, y, z)		glNormal3x(_MAP_SBYTE(x), _MAP_SBYTE(y), _MAP_SBYTE(z))
#define glNormal3s(x, y, z)		glNormal3x(_MAP_SSHRT(x), _MAP_SSHRT(y), _MAP_SSHRT(z))
#define glNormal3i(x, y, z)		glNormal3x(_MAP_SINT(x), _MAP_SINT(y), _MAP_SINT(z))

static inline void glNormal3bv(const GLbyte *v)		{ glNormal3b(v[0], v[1], v[2]); }
static inline void glNormal3dv(const GLdouble *v)	{ glNormal3d(v[0], v[1], v[2]); }
static inline void glNormal3fv(const GLfloat *v)	{ glNormal3f(v[0], v[1], v[2]); }
static inline void glNormal3xv(const GLfixed *v)	{ glNormal3x(v[0], v[1], v[2]); }
static inline void glNormal3iv(const GLint *v)		{ glNormal3i(v[0], v[1], v[2]); }
static inline void glNormal3sv(const GLshort *v)	{ glNormal3s(v[0], v[1], v[2]); }

/* ----- texture coordinates ----- */
#define glTexCoord1f(s)				glTexCoord3x(fixedf(s), 0, 0)
#define glTexCoord1d(s)				glTexCoord3x(fixedf(s), 0, 0)
#define glTexCoord1x(s)				glTexCoord3x(s, 0, 0)
#define glTexCoord1i(s)				glTexCoord3x(fixedi(s), 0, 0)
#define glTexCoord1s(s)				glTexCoord3x(fixedi(s), 0, 0)
#define glTexCoord2f(s, t)			glTexCoord3x(fixedf(s), fixedf(t), 0)
#define glTexCoord2d(s, t)			glTexCoord3x(fixedf(s), fixedf(t), 0)
#define glTexCoord2x(s, t)			glTexCoord3x(s, t, 0)
#define glTexCoord2i(s, t)			glTexCoord3x(fixedi(s), fixedi(t), 0)
#define glTexCoord2s(s, t)			glTexCoord3x(fixedi(s), fixedi(t), 0)
#define glTexCoord3f(s, t, r)		glTexCoord3x(fixedf(s), fixedf(t), fixedf(r))
#define glTexCoord3d(s, t, r)		glTexCoord3x(fixedf(s), fixedf(t), fixedf(r))
void glTexCoord3x(GLfixed s, GLfixed t, GLfixed r);
#define glTexCoord3i(s, t, r)		glTexCoord3x(fixedi(s), fixedi(t), fixedi(r))
#define glTexCoord3s(s, t, r)		glTexCoord3x(fixedi(s), fixedi(t), fixedi(r))
#define glTexCoord4f(s, t, r, q)	glTexCoord3x(fixedf(s), fixedf(t), fixedf(r))
#define glTexCoord4d(s, t, r, q)	glTexCoord3x(fixedf(s), fixedf(t), fixedf(r))
#define glTexCoord4x(s, t, r, q)	glTexCoord3x(s, t, r)
#define glTexCoord4i(s, t, r, q)	glTexCoord3x(fixedi(s), fixedi(t), fixedi(r))
#define glTexCoord4s(s, t, r, q)	glTexCoord3x(fixedi(s), fixedi(t), fixedi(r))

static inline void glTexCoord1dv(const GLdouble *v)	{ glTexCoord1d(v[0]); }
static inline void glTexCoord1fv(const GLfloat *v)	{ glTexCoord1f(v[0]); }
static inline void glTexCoord1xv(const GLfixed *v)	{ glTexCoord1x(v[0]); }
static inline void glTexCoord1iv(const GLint *v)	{ glTexCoord1i(v[0]); }
static inline void glTexCoord1sv(const GLshort *v)	{ glTexCoord1s(v[0]); }
static inline void glTexCoord2dv(const GLdouble *v)	{ glTexCoord2d(v[0], v[1]); }
static inline void glTexCoord2fv(const GLfloat *v)	{ glTexCoord2f(v[0], v[1]); }
static inline void glTexCoord2xv(const GLfixed *v)	{ glTexCoord2x(v[0], v[1]); }
static inline void glTexCoord2iv(const GLint *v)	{ glTexCoord2i(v[0], v[1]); }
static inline void glTexCoord2sv(const GLshort *v)	{ glTexCoord2s(v[0], v[1]); }
static inline void glTexCoord3dv(const GLdouble *v)	{ glTexCoord3d(v[0], v[1], v[2]); }
static inline void glTexCoord3fv(const GLfloat *v)	{ glTexCoord3f(v[0], v[1], v[2]); }
static inline void glTexCoord3xv(const GLfixed *v)	{ glTexCoord3x(v[0], v[1], v[2]); }
static inline void glTexCoord3iv(const GLint *v)	{ glTexCoord3i(v[0], v[1], v[2]); }
static inline void glTexCoord3sv(const GLshort *v)	{ glTexCoord3s(v[0], v[1], v[2]); }
static inline void glTexCoord4dv(const GLdouble *v)	{ glTexCoord4d(v[0], v[1], v[2], v[3]); }
static inline void glTexCoord4fv(const GLfloat *v)	{ glTexCoord4f(v[0], v[1], v[2], v[3]); }
static inline void glTexCoord4xv(const GLfixed *v)	{ glTexCoord4x(v[0], v[1], v[2], v[3]); }
static inline void glTexCoord4iv(const GLint *v)	{ glTexCoord4i(v[0], v[1], v[2], v[3]); }
static inline void glTexCoord4sv(const GLshort *v)	{ glTexCoord4s(v[0], v[1], v[2], v[3]); }



void glPointSize(GLfloat sz);
void glPointSizex(GLfixed sz);

GLenum glGetError(void);
void glFlush(void);
void glFinish(void);

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif	/* FIXED_GL_H_ */
