// demo.h

#ifndef _DEMO_H_
#define _DEMO_H_

#include <string>

typedef bool (*part_init_func)();
typedef void (*part_run_func)(unsigned int msec);

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

#endif // ndef _DEMO_H_
