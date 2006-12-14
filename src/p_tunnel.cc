// p_tunnel.cc

#include "tunnel.h"

extern Color *fb;

static Image screen;
static Image texture;


bool tunnel_init()
{

	texture.x = texture.y = 256;
	texture.pixels = new Color[256 * 256];
	
	if(!load_image(&texture, "data/tunnel.ppm")) {
		return false;
	}
	
	TunnelTex(&texture);

	return true;
}

#define psin(t)	(sinf(t) * 0.5f + 0.5f)

void tunnel_render(float secs)
{
	screen.x = 640;
	screen.y = 480;
	screen.pixels = fb;

	TunnelFogStart(2.5f + 2.0f * sinf(10 * secs));
	TunnelFogEnd(6.0f + sinf(10 * secs));

	//TunnelFogStart(8);
	//TunnelFogEnd(9);

	Color fog1, fog2;
	fog1.packed = 0x22FFAA;
	fog2.packed = 0xAA00FF;
	TunnelFogColor(Lerp(fog1, fog2, 255* psin(10 * secs)));
	TunnelFogAmp(1);
	TunnelRot(Vector2(secs / 8, secs / 3));
	TunnelShift(4 * secs);
	
	//TunnelRot(Vector2(0, 0));
	
	Tunnel(screen);
}

void tunnel_run(unsigned int msec)
{
	tunnel_render(msec / 1000.0f);
}

void tunnel_cleanup()
{
	//delete [] texture.pixels;
}






