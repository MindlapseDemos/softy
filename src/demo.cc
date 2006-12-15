// demo.cc
#include <map>
#include <vector>
#include "demo.h"
#include <vmath.h>
#include <SDL.h>
#include "image.h"

extern SDL_Surface *fbsurf;
Color *fb;
extern Image *fbimg;
unsigned int *cfb;

static unsigned int demo_end;
Color second_buffer[640*480];

struct PStruct
{
	Part p;
	std::vector<unsigned int> start;
	std::vector<unsigned int> stop;
	std::vector<bool> local;
	PStruct()
	{
		
	}
};

static std::map<std::string, unsigned int> part_names;
static std::vector<PStruct> parts;

void add_part(const Part &p, std::string name)
{
	part_names[name] = parts.size();
	PStruct np;
	np.p = p;
	parts.push_back(np);
}

void add_part_inst(std::string which, unsigned int start_msec,
	unsigned int stop_msec, bool local_time)
{
	unsigned int i = part_names[which];
	parts[i].start.push_back(start_msec);
	parts[i].stop.push_back(stop_msec);
	parts[i].local.push_back(local_time);
}

bool init_demo()
{
	demo_end = 0;
	for (unsigned int i=0; i<parts.size(); i++)
	{
		if (!parts[i].p.init || !parts[i].p.run) return false;
		if (!parts[i].p.init()) return false;
		
		// determine end of demo
		for (unsigned int j=0; j<parts[i].stop.size(); j++)
		{
			if (parts[i].stop[j] > demo_end)
			{
				demo_end = parts[i].stop[j];
			}
		}
	}
	return true;
}

#include "p_radial.h"

void run_demo(unsigned int msec)
{
	if (msec > demo_end)
	{
		SDL_Quit();
		exit(0);
	}
	// decide what parts to render
	std::vector<Part> rpart;
	std::vector<unsigned int> segment_start;
	std::vector<unsigned int> segment_stop;
	std::vector<bool> slocal;

	for (unsigned int i=0; i<parts.size(); i++)
	{
		// determine which segment is inside this time
		for (unsigned int j=0; j<parts[i].start.size(); j++)
		{
			// allow only two parts at the same time
			if (rpart.size() == 2) break;
			unsigned int start, stop;
			start = parts[i].start[j];
			stop = parts[i].stop[j];
			if (msec >= start && msec < stop)
			{
				// this part is in
				rpart.push_back(parts[i].p);
				segment_start.push_back(start);
				segment_stop.push_back(stop);
				slocal.push_back(parts[i].local[j]);
			}
		}
	}

	// render
	if(SDL_MUSTLOCK(fbsurf)) SDL_LockSurface(fbsurf);
	cfb = (unsigned int*)(fbimg->pixels = fb = (Color*)fbsurf->pixels);
	
	if (!rpart.size())
	{
		// no part here - clear to black
		memset(fb, 0xff, 640 * 480 * 4);
	}
	else if (rpart.size() == 1)
	{
		if (slocal[0])
			rpart[0].run(msec - segment_start[0]);
		else
			rpart[0].run(msec);
	}
	else
	{
		// two parts - crossfade
		float fade_factor;	
		int first=0, second=1;
		if (segment_start[0] > segment_start[1])
		{
			first = 1;
			second = 0;
		}
		
		// determine full overlap
		if (segment_stop[second] < segment_stop[first])
		{
			// full overlap. divide second part in 3 equal subparts:
			// ascending alpha
			unsigned int second_dur = segment_stop[second] - segment_start[second];
			if (msec < segment_start[second] + second_dur / 3)
			{
				fade_factor = (float)(msec - segment_start[second]) /
					(second_dur / 3);
			}
			else if (msec < segment_start[second] + (2 * second_dur) / 3)
			{
				fade_factor = 1.0f;
			}
			else
			{
				fade_factor = 1.0f - 
					(msec - (segment_start[second] + (2 * second_dur) / 3)) /
					(float) (second_dur / 3);
			}
		}
		else
		{
			// partial overlap
			unsigned int ol_start = segment_start[second];
			unsigned int ol_stop = segment_stop[first];
			unsigned int ol_dur = ol_stop - ol_start;
			fade_factor = (float) (msec - ol_start) / (float) ol_dur;
		}

		// render first part
		if (slocal[first])
			rpart[first].run(msec - segment_start[first]);
		else
			rpart[first].run(msec);

		// render second part to second buffer
		cfb = (unsigned int*)(fbimg->pixels = fb = second_buffer);
		if (slocal[second])
			rpart[second].run(msec - segment_start[second]);
		else
			rpart[second].run(msec);

		// blend parts
		Color *dst = (Color*)fbsurf->pixels;
		Color *src = second_buffer;
		int fade = (int) (255 * fade_factor);
		for (unsigned int i=0; i<640*480; i++)
		{
			*dst++ = Lerp(*dst, *src++, fade);
		}
		fbimg->pixels = fb = (Color*) fbsurf->pixels;;
	}

	// fade out for 1 sec before demo ends
	if (demo_end)
	{
		unsigned int fade_dur = demo_end > 2000 ? 2000 : demo_end;
		if (msec > (demo_end - fade_dur))
		{
			Color *dst = (Color*) fbsurf->pixels;
			Color fade_to;
			fade_to.packed = 0;
			int t = ((msec - (demo_end - fade_dur)) * 255) / fade_dur;
			for (unsigned int i=0; i<640*480; i++)
			{
				*dst++ = Lerp(*dst, fade_to, t);
			}
		}
	}
	
	if(SDL_MUSTLOCK(fbsurf)) SDL_UnlockSurface(fbsurf);
	SDL_Flip(fbsurf);
}

void end_demo_at(unsigned int msec)
{
	demo_end = msec;
}



