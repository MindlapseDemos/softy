/*
 * sdlvf - the SDL/vorbisfile sound system
 * Copyright (C) 2004  Vasilis Vasaitis <vvas@hal.csd.auth.gr>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _SDLVF_H
#define _SDLVF_H

/*
 * All the possible return values of sdlvf_init() and sdlvf_check().
 */
enum {
	SDLVF_PLAYING,              /* no error, playing file */
	SDLVF_STOPPED,              /* no error, playback has stopped */
	SDLVF_BADFILE,              /* could not open file specified */
	SDLVF_BADOGG,               /* file specified is not valid */
	SDLVF_BADSTREAM,            /* could not query current stream */
	SDLVF_NOAUDIO               /* could not open audio device */
};

/*
 * Make sure that everything works under C++ as well.
 */
#ifdef __cplusplus
extern "C" {
#endif

/*
 * Initialises the SDL/vorbisfile sound system. The audio subsystem of
 * SDL must have been initialised before calling this function. On
 * success, the Ogg Vorbis file specified is played, and zero
 * (SDLVF_PLAYING) is returned. On error, the appropriate error code
 * is returned.
 */
int sdlvf_init(const char *);

/*
 * This function should be called regularly, i.e. multiple times per
 * second. It makes sure that everything is going as it should. It
 * also checks whether playback has ended, returning the appropriate
 * value if this is the case.
 */
int sdlvf_check(void);

/*
 * Seeks to the specified position in seconds.
 */
int sdlvf_seek(double);

/*
 * Stops playback and shuts down the sound system.
 */
void sdlvf_done(void);

/*
 * Returns the equivalent error string for the supplied error code.
 */
char *sdlvf_strerror(int);

/*
 * Close the C++-supporting block if needed.
 */
#ifdef __cplusplus
}
#endif

#endif /* !defined(_SDLVF_H) */
