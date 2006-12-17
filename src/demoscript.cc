// demoscript.cc
#include <stdio.h>
#include <string>
#include <vector>
#include "demo.h"

static unsigned int time_frame = 0;

using namespace std;

vector<string> read_line(FILE *fp);
bool process_cmd(vector<string> cmd);

bool process_demo_script(const char *filename)
{
	FILE *fp = fopen(filename, "rb");
	if (!fp) return false;

	while (!feof(fp))
	{
		vector<string> line = read_line(fp);
		if (!process_cmd(line))
		{
			fclose(fp);
			return false;
		}
	}

	fclose(fp);
	return true;
}

int classify_char(char c)
{
	// newline
	if (c == 0x0A) return 0;
	if (c == 0x0D) return 0;

	// whitespace
	if (c == '\t') return 1;
	if (c == ' ')  return 1;
	if (c == (char)0xFF) return 1;

	// all other chars
	return 2;
}

vector<string> read_line(FILE *fp)
{
	vector<string> ret;
	string token;
	while (!feof(fp))
	{
		char c = getc(fp);
		int type = classify_char(c);
		if (type == 0) 
		{
			if (token.size()) ret.push_back(token);
			break;
		}
		if (type == 1)
		{
			if (token.size())
			{
				ret.push_back(token);
				token = "";
			}
		}
		if (type == 2) token += c;
	}

	// ignore comments
	if (ret.size())
	{
		if (ret[0].size())
			if (ret[0][0] == '#') return vector<string>();
	}

	return ret;
}

unsigned int read_time(string t, bool frame = false)
{
	// check for suffix s (seconds)
	unsigned int ret;
	if (t[t.size() - 1] == 's')
	{
		float time;
		sscanf(t.c_str(), "%fs", &time);
		ret = (unsigned int) ((time * 1000.0f) + 0.5f);
	}
	else
	{
		ret = (unsigned int) atoi(t.c_str());
	}

	if (frame) ret += time_frame;
	return ret;
}

Color read_color(string c)
{
	float r, g, b;
	sscanf(c.c_str(), "r%fg%fb%f", &r, &g, &b);
	Color ret;
	ret.c.r = (unsigned char) (255 * r);
	ret.c.g = (unsigned char) (255 * g);
	ret.c.b = (unsigned char) (255 * b);
	
	return ret;
}

bool process_inst_cmd(vector<string> cmd)
{
	if (cmd.size() < 4) 
	{
		printf("ERROR: inst command does not take %d arguments\n", (int)(cmd.size() - 1));
		return false;
	}
	bool local = false;
	if (cmd.size() > 4)
	{
		if (cmd[4] == "local")
		{
			local = true;
		}
		else
		{
			printf("WARNING: %s: ignoring unexpected token\n", cmd[4].c_str());
		}

		for (unsigned int i=5; i<cmd.size(); i++)
		{
			printf("WARNING: %s: ignoring unexpected token\n", cmd[i].c_str());
		}

	}

	add_part_inst(cmd[1], read_time(cmd[2], true), read_time(cmd[3], true), local);
	return true;
}

bool process_param_cmd(vector<string> cmd)
{
	if (cmd.size() != 4)
	{
		printf("ERROR: param command does not take %d arguments\n", cmd.size() - 1);
		return false;
	}

	string name = cmd[1];
	unsigned int msec = read_time(cmd[2], true);
	int param = atoi(cmd[3].c_str());
	set_part_param(name, msec, param);
}

bool process_flash_cmd(vector<string> cmd)
{
	if (cmd.size() != 4)
	{
		printf("ERROR: flash command does not take %d arguments\n", cmd.size() - 1);
		return false;
	}

	Color c = read_color(cmd[1]);
	unsigned int msec = read_time(cmd[2], true);
	unsigned int dur = read_time(cmd[3]);
	
	add_flash(msec, dur, c);
	
	return true;
}

bool process_tframe_cmd(vector<string> cmd)
{
	if (cmd.size() != 2)
	{
		printf("ERROR: tframe command does not take %d arguments\n", cmd.size() - 1);
		return false;
	}
	time_frame = read_time(cmd[1]);
	return true;
}

bool process_cmd(vector<string> cmd)
{
	if (!cmd.size()) return true;

	if (cmd[0] == "inst")
	{
		return process_inst_cmd(cmd);
	}
	if (cmd[0] == "param")
	{
		return process_param_cmd(cmd);
	}
	if (cmd[0] == "flash")
	{
		return process_flash_cmd(cmd);
	}
	if (cmd[0] == "tframe")
	{
		return process_tframe_cmd(cmd);
	}
	if (cmd[0] == "include")
	{
		if (cmd.size() != 2)
		{
			printf("ERROR: include c command does not take %d arguments\n", cmd.size() - 1);
			return false;
		}
		return process_demo_script(cmd[1].c_str());
	}
	
	printf("WARNING: ignoring unknown command %s\n", cmd[0].c_str());

	return true;
}
