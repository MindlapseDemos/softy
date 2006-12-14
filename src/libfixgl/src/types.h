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

#ifndef GL_TYPES_H_
#define GL_TYPES_H_

#include <stdint.h>

/* OpenGL types */
typedef uint8_t 	GLboolean;
typedef int8_t 		GLbyte;
typedef uint8_t 	GLubyte;
typedef int16_t 	GLshort;
typedef uint16_t 	GLushort;
typedef int32_t 	GLint;
typedef uint32_t 	GLuint;
typedef uint32_t 	GLsizei;
typedef uint32_t	GLenum;
typedef uint32_t	GLbitfield;
typedef float		GLfloat;
typedef float		GLclampf;
typedef double		GLdouble;
typedef double		GLclampd;
typedef void		GLvoid;

#define GL_TRUE		1
#define GL_FALSE	0

/* fixed GL types */
#ifdef DBG_USE_FLOAT
typedef float		GLfixed;
typedef float		GLclampx;
#else
typedef int			GLfixed;
typedef int			GLclampx;
#endif


#endif	/* GL_TYPES_H_ */
