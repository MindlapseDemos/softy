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

/* color bit shift and mask values for color packing/unpacking
 * in a cross-endianess manner.
 *
 * author: John Tsiombikas 2004
 */
#ifndef _COLOR_BITS_H_
#define _COLOR_BITS_H_

#include "types.h"

#if !defined(LITTLE_ENDIAN) && !defined(BIG_ENDIAN)
#if  defined(XBOX) || defined(__i386__) || defined(__ia64__) || defined(WIN32) || \
    (defined(__alpha__) || defined(__alpha)) || \
     defined(__arm__) || \
    (defined(__mips__) && defined(__MIPSEL__)) || \
     defined(__SYMBIAN32__) || \
     defined(__x86_64__) || \
     defined(__LITTLE_ENDIAN__)

/* little endian */
#define LITTLE_ENDIAN

#else
/* big endian */	
#define BIG_ENDIAN

#endif	/* endian check */
#endif	/* !defined(LITTLE_ENDIAN) && !defined(BIG_ENDIAN) */

/* 32bit color shift values */
#ifdef LITTLE_ENDIAN
#define ALPHA_SHIFT32	24
#define RED_SHIFT32		16
#define GREEN_SHIFT32	8
#define BLUE_SHIFT32	0
#else /* BIG_ENDIAN */
#define ALPHA_SHIFT32	24
#define RED_SHIFT32		0
#define GREEN_SHIFT32	8
#define BLUE_SHIFT32	16
#endif	/* LITTLE_ENDIAN */

/* 32bit color mask values */
#define ALPHA_MASK32	(0xff << ALPHA_SHIFT32)
#define RED_MASK32		(0xff << RED_SHIFT32)
#define GREEN_MASK32	(0xff << GREEN_SHIFT32)
#define BLUE_MASK32		(0xff << BLUE_SHIFT32)

/* 16bit color shift values */
#ifdef LITTLE_ENDIAN
#define RED_SHIFT16		11
#define GREEN_SHIFT16	5
#define BLUE_SHIFT16	0
#else	/* BIG_ENDIAN */
#define RED_SHIFT16		0
#define GREEN_SHIFT16	5
#define BLUE_SHIFT16	11
#endif	/* LITTLE_ENDIAN */

/* 16bit color mask values */
#define RED_MASK16		(0x1f << RED_SHIFT16)
#define GREEN_MASK16	(0x3f << GREEN_SHIFT16)
#define BLUE_MASK16		(0x1f << BLUE_SHIFT16)

/* 15bit color shift values */
#ifdef LITTLE_ENDIAN
#define RED_SHIFT15		10
#define GREEN_SHIFT15	5
#define BLUE_SHIFT15	0
#else	/* BIG_ENDIAN */
#define RED_SHIFT15		0
#define GREEN_SHIFT15	5
#define BLUE_SHIFT15	10
#endif	/* LITTLE_ENDIAN */

/* 15bit color mask values */
#define RED_MASK15		(0x1f << RED_SHIFT15)
#define GREEN_MASK15	(0x1f << GREEN_SHIFT15)
#define BLUE_MASK15		(0x1f << BLUE_SHIFT15)


/* color packing macros */
#define PACK_COLOR32(a,r,g,b) \
	(((uint32_t)(a) << ALPHA_SHIFT32) & ALPHA_MASK32) | \
	(((uint32_t)(r) << RED_SHIFT32) & RED_MASK32) | \
	(((uint32_t)(g) << GREEN_SHIFT32) & GREEN_MASK32) | \
	(((uint32_t)(b) << BLUE_SHIFT32) & BLUE_MASK32)

#define PACK_COLOR24(r,g,b)		PACK_COLOR32(0xff,r,g,b)

#define PACK_COLOR16(r,g,b) \
	(((uint16_t)(r) << RED_SHIFT16) & RED_MASK16) | \
	(((uint16_t)(g) << GREEN_SHIFT16) & GREEN_MASK16) | \
	(((uint16_t)(b) << BLUE_SHIFT16) & BLUE_MASK16)

#define PACK_COLOR15(r,g,b) \
	(((uint16_t)(r) << RED_SHIFT15) & RED_MASK15) | \
	(((uint16_t)(g) << GREEN_SHIFT15) & GREEN_MASK15) | \
	(((uint16_t)(b) << BLUE_SHIFT15) & BLUE_MASK15)


#endif	/* _COLOR_BITS_H_ */
