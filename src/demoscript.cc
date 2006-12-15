// demoscript.cc
#include <stdio.h>
#include <string>
#include <vector>
#include "demo.h"

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
	if (c == 0xFF) return 1;

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
		if (type == 0) break;
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

unsigned int read_time(string t)
{
	// check for suffix s (seconds)
	if (t[t.size() - 1] == 's')
	{
		float time;
		sscanf(t.c_str(), "%fs", &time);
		return (unsigned int) ((time * 1000.0f) + 0.5f);
	}
	return (unsigned int) atoi(t.c_str());
}

bool process_inst_cmd(vector<string> cmd)
{
	if (cmd.size() < 4) 
	{
		printf("ERROR: inst command does not take %d arguments\n", cmd.size() - 1);
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

	add_part_inst(cmd[1], read_time(cmd[2]), read_time(cmd[3]), local);
	return true;
}

bool process_cmd(vector<string> cmd)
{
	if (!cmd.size()) return true;

	if (cmd[0] == "inst")
	{
		return process_inst_cmd(cmd);
	}
	
	printf("WARNING: ignoring unknown command %s\n", cmd[0]);

	return true;
}
