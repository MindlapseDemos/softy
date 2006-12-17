// demo.cc
#include <map>
#include <vector>
#include "demo.h"
#include <vmath.h>
#include <SDL.h>
#include "image.h"

extern unsigned int music_volume;
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
	std::vector<unsigned int> param_time;
	std::vector<int> param;
	PStruct()
	{
		
	}
};

struct Slide
{
	std::string filename;
	Image img;
	unsigned int start;
	unsigned int stop;
	unsigned int stay;
	int sx, sy, dx, dy;
	Slide(){}
	Slide(std::string filename, unsigned int start, unsigned int stop,
		unsigned int stay, int sx, int sy, int dx, int dy)
	{
		this->filename = filename;
		this->start = start;
		this->stop = stop;
		this->stay = stay;
		this->sx = sx;
		this->sy = sy;
		this->dx = dx;
		this->dy = dy;
	}
};

static std::map<std::string, unsigned int> part_names;
static std::vector<PStruct> parts;

// effects
static std::vector<unsigned int> flash_time;
static std::vector<unsigned int> flash_dur;
static std::vector<Color> flash_color;

// slides
std::vector<Slide> slides;
std::map<std::string, Image*> slide_cache;

Image *get_slide(std::string filename)
{
	if (slide_cache[filename])
	{
		return slide_cache[filename];
	}

	// try to load the image
	Image *img = new Image;
	if (!load_image(img, filename.c_str()))
	{
		delete img;
		return 0;
	}

	slide_cache[filename] = img;
	return img;
}

inline float Lerp(float a, float b, float t)
{
	return a * (1.0f - t) + b * t;
}

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


void progress(float);

bool init_demo()
{
	float prog = 0.0;

	demo_end = 0;
	for (unsigned int i=0; i<parts.size(); i++)
	{
		if (!parts[i].p.init || !parts[i].p.run) return false;
		if (!parts[i].p.init()) return false;

		if(i < parts.size() - 1) {
			prog += 0.05;
			progress(prog);
		} else {
			progress(1.0);
		}
		
		// determine end of demo
		for (unsigned int j=0; j<parts[i].stop.size(); j++)
		{
			if (parts[i].stop[j] > demo_end)
			{
				demo_end = parts[i].stop[j];
			}
		}
	}

	// load slide images
	for (unsigned int i=0; i<slides.size(); i++)
	{
		Image *img = get_slide(slides[i].filename);
		if (!img)
		{
			printf("Failed loading image: %s\n", slides[i].filename.c_str());
			return false;
		}
		slides[i].img = *img;
	}
	
	return true;
}

int get_part_param(unsigned int p, unsigned int msec)
{
	if (!parts[p].param_time.size()) return 0;
	unsigned int max_time = 0;
	unsigned int max_idx = 0;
	bool def = true;
	for (unsigned int i=0; i<parts[p].param_time.size(); i++)
	{
		if (parts[p].param_time[i] <= msec)
		{
			if (parts[p].param_time[i] >= max_time)
			{
				def = false;
				max_time = parts[p].param_time[i];
				max_idx = i;
			}
		}
	}
	if (def)
		return 0;
	return parts[p].param[max_idx];
}

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
	std::vector<int> param;
	
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
				param.push_back(get_part_param(i, msec));
			}
		}
	}

	// render
	if(SDL_MUSTLOCK(fbsurf)) SDL_LockSurface(fbsurf);
	cfb = (unsigned int*)(fbimg->pixels = fb = (Color*)fbsurf->pixels);
	
	//if (!rpart.size())
	//{
		// no part here - clear to black
	// forget it - clear anyway!	
	memset(fb, 0, 640 * 480 * 4);
	memset(second_buffer, 0, 640 * 480 * 4);
	//}
	if (rpart.size() == 1)
	{
		if (slocal[0])
			rpart[0].run(msec - segment_start[0], param[0]);
		else
			rpart[0].run(msec, param[0]);
	}
	else if (rpart.size() == 2)
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
			rpart[first].run(msec - segment_start[first], param[first]);
		else
			rpart[first].run(msec, param[first]);

		// render second part to second buffer
		cfb = (unsigned int*)(fbimg->pixels = fb = second_buffer);
		if (slocal[second])
			rpart[second].run(msec - segment_start[second], param[second]);
		else
			rpart[second].run(msec, param[second]);

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

	// add slides
	for (unsigned int i=0; i<slides.size(); i++)
	{
		if ((msec > slides[i].start) && (msec <= slides[i].stop))
		{
			if (slides[i].start != slides[i].stop) 
			{   
				int t = msec - slides[i].start;
				float ft = (float) t / (float) (slides[i].stop - slides[i].start);
				int x = (int) Lerp((float) slides[i].sx, (float) slides[i].dx, ft);
				int y = (int) Lerp((float) slides[i].sy, (float) slides[i].dy, ft);
				Color ckey;
				ckey.packed = 0x00FF0000;
				blit_ckey(fbimg, x, y, &slides[i].img, ckey);
			}
		}
		if ((msec > slides[i].stop) && 
			(msec <= slides[i].stop + slides[i].stay))
		{
			Color ckey;
			ckey.packed = 0x00FF0000;
			blit_ckey(fbimg, slides[i].dx, slides[i].dy, &slides[i].img, ckey);
		}
	}
	
	// add effects
	// flashes
	for (unsigned int f=0; f<flash_time.size(); f++)
	{
		if ((msec <  flash_time[f] + flash_dur[f] / 2) &&
			(msec >=  flash_time[f] - flash_dur[f] / 2))
		{
			Color *dst = (Color*) fbsurf->pixels;
			int flash_t = msec - (flash_time[f] - flash_dur[f] / 2);
			float t = sin((float)flash_t * 3.14159265 / (float) flash_dur[f]);
			int it = (int) (255 * t);
			for (unsigned int i=0; i<640*480; i++)
			{
				*dst++ = Lerp(*dst, flash_color[f], it);
			}
		}
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
			music_volume = 255 - t;
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


void set_part_param(std::string which, unsigned int msec, int param)
{
	parts[part_names[which]].param_time.push_back(msec);
	parts[part_names[which]].param.push_back(param);
}

void add_flash(unsigned int msec, unsigned int dur, Color c)
{
	flash_time.push_back(msec);
	flash_dur.push_back(dur);
	flash_color.push_back(c);
}

void add_slide(std::string filename, unsigned int msec_begin,
			 unsigned int msec_end, unsigned int msec_stay,
			 int sx, int sy, int dx, int dy)
{
	slides.push_back(Slide(filename, msec_begin, msec_end, msec_stay,
		sx, sy, dx, dy));
}
