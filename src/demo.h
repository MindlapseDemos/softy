// demo.h

#ifndef _DEMO_H_
#define _DEMO_H_

#include <string>
#include "color.h"

typedef bool (*part_init_func)();
typedef void (*part_run_func)(unsigned int msec, int param);

struct Part
{
	part_init_func init;
	part_run_func run;
	Part()
	{
		init = 0;
		run = 0;
	}
	Part(part_init_func init, part_run_func run)
	{
		this->init = init;
		this->run = run;
	}
};

void add_part(const Part &p, std::string name);
void add_part_inst(std::string which, unsigned int start_msec,
	unsigned int stop_msec, bool local_time);
bool init_demo();
void run_demo(unsigned int msec);
void end_demo_at(unsigned int msec);

void set_part_param(std::string which, unsigned int msec, int param);
void add_flash(unsigned int msec, unsigned int dur, Color c);
void add_slide(std::string filename, unsigned int msec_begin,
			 unsigned int msec_end, unsigned int msec_stay,
			 int sx, int sy, int dx, int dy);

#endif // ndef _DEMO_H_
