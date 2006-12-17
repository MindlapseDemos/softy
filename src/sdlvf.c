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

#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>
#include <SDL.h>
#include <string.h>
#include "sdlvf.h"

#define SDL_SAMPLES 2048
#define VORBISFILE_BUFFER 4096

#define MIN(x, y) ((x) < (y) ? (x) : (y))

/*
 * The error strings array, which must be kept in sync with the error
 * codes in sdlvf.h.
 */
static char *audio_errors[] = {
	"playing audio",
	"audio playback has been stopped",
	"could not open input file for reading",
	"input file does not appear to be an valid bitstream",
	"unable to get information about the current logical bitstream",
	"unable to open the audio device"
};

/*
 * The static data shared by all SDL/vorbisfile functions. Note that
 * there is never a need for more than one instance of the sound
 * system, so this a reasonable thing to do.
 */
static OggVorbis_File audio_vf;
static volatile int audio_stopped, audio_reopen;

/*
 * This function is called by SDL when more audio data is needed.
 */
static void audio_callback(void *userdata, Uint8 *stream, int len)
{
	/* state information kept across calls */
	static char buf[VORBISFILE_BUFFER];
	static int buflen = 0;
	static int curr = -1;
	static int stopped = 0, reopen = 0;

	/* local variables */
	int filled = 0;

	/* check for exceptional conditions */
	if (stopped || reopen) {
		audio_stopped = stopped;
		audio_reopen = reopen;
		stopped = reopen = 0;
	}
	if (audio_stopped || audio_reopen) return;

	/* check for leftovers in our buffer */
	if (buflen != 0) {
		int copy = MIN(buflen, len);
		memcpy(stream, buf, copy);
		filled += copy;
		buflen -= copy;
		memmove(buf, buf + copy, buflen);
	}

	/* main loop */
	while (filled < len) {
		int needed = MIN(VORBISFILE_BUFFER, len - filled), read;
		int prev = curr;
		do read = ov_read(&audio_vf, stream + filled, needed, 0, 2, 1, &curr);
		while (read < 0);
		if (read == 0) {
			stopped = 1;
			break;
		}
		if (curr != prev && prev != -1) {
			memcpy(buf, stream + filled, buflen = read);
			memset(stream + filled, 0, read);
			reopen = 1;
			break;
		}
		filled += read;
	}
}

/*
 * Opens the audio device based on the parameters of the current
 * logical bitstream of the Ogg Vorbis file.
 */
static int audio_open(void)
{
	vorbis_info *vi;
	SDL_AudioSpec as;

	audio_stopped = 0;
	audio_reopen = 0;

	if ((vi = ov_info(&audio_vf, -1)) == NULL)
		return SDLVF_BADSTREAM;
	as.freq = vi->rate;
	as.format = AUDIO_S16;
	as.channels = vi->channels;
	as.samples = SDL_SAMPLES;
	as.callback = audio_callback;
	as.userdata = NULL;
	if (SDL_OpenAudio(&as, NULL) == -1)
		return SDLVF_NOAUDIO;
	SDL_PauseAudio(0);

	return SDLVF_PLAYING;
}

/*
 * Closes the audio device; provided for naming consistency.
 */
static void audio_close(void)
{
	SDL_CloseAudio();
}

/*
 * Initialises SDL/vorbisfile and starts playing the Ogg Vorbis file
 * <fname>. Returns zero (SDLVF_PLAYING) on success; any other value
 * indicates an error.
 */
int sdlvf_init(const char *fname)
{
	FILE *f;
	int result;
	if ((f = fopen(fname, "rb")) == NULL)
		return SDLVF_BADFILE;
	if (ov_open(f, &audio_vf, NULL, 0) != 0) {
		fclose(f);
		return SDLVF_BADOGG;
	}
	if ((result = audio_open()) != SDLVF_PLAYING)
		ov_clear(&audio_vf);
	return result;
}

/*
 * Checks the status of SDL/vorbisfile, reopening the audio device if
 * necessary. It also reports whether audio playback has stopped.
 */
int sdlvf_check(void)
{
	if (audio_stopped)
		return SDLVF_STOPPED;
	if (audio_reopen) {
		audio_close();
		return audio_open();
	}
	return SDLVF_PLAYING;
}

/*
 * Seeks to the specified <time> in seconds, taking care of locking
 * and avoiding clicks and pops.
 */
int sdlvf_seek(double time)
{
	int result;
	SDL_LockAudio();
	result = ov_time_seek_lap(&audio_vf, time);
	SDL_UnlockAudio();
	return result;
}

/*
 * Shuts down SDL/vorbisfile, closing the audio device and freeing
 * data structures.
 */
void sdlvf_done(void)
{
	audio_close();
	ov_clear(&audio_vf);
}

/*
 * Returns the equivalent error string for the supplied error code.
 */
char *sdlvf_strerror(int error)
{
	return audio_errors[error];
}
