// p_tunnel.cc

#include "tunnel.h"

extern Color *fb;

static Image screen;
static Image texture;


bool tunnel_init()
{

	texture.x = texture.y = 256;
	texture.pixels = new Color[256 * 256];

	// create checker
	unsigned int *pixels = (unsigned int *) texture.pixels;
	for (unsigned int j=0; j<256; j++)
	{
		for (unsigned int i=0; i<256; i++)
		{
			if ((i < 128 && j < 128) || (i > 127 && j > 127))
			{
				*pixels++ = 0xFF0000;
			}
			else
			{
				*pixels++ = 0xFF;
			}
		}
	}

	TunnelTex(&texture);

	return true;
}

void tunnel_render(float secs)
{
	screen.x = 640;
	screen.y = 480;
	screen.pixels = fb;

	Matrix3x3 rot;
	rot.rotate(Vector3(secs / 8, secs / 3, 0));

	Color fog_color;
	fog_color.packed = 0x221132;
	TunnelFogColor(fog_color);
	TunnelRot(rot);
	TunnelShift(4 * secs);
	Tunnel(screen);
}

void tunnel_cleanup()
{
	delete [] texture.pixels;
}






