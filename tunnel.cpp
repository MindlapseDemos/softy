// tunnel.cpp

#include "tunnel.h"

statix Matrix3x3 tunnel_rot;
static float tunnel_shift;
static const Image *tunnel_tex;
static Vector3 tunnel_vecs[640 * 480];

void InitTunnel()
{
	for (unsigned int j=0; j<480; j++)
	{
		for (unsigned int i=0; i<640; i++)
		{
			float x = (i - 320) / 320.0f;
			float y = (j - 240) / 240.0f;
			x *= 4.0f / 3.0f;
			tunnel_vecs[i + 640 * j] = Vector3(x, y, 1.0f).Normalized();
		}
	}
}


void TunnelRot(const Matrix3x3 &rot)
{
	tunnel_rot = rot;
}

void TunnelTex(const Image *tex)
{
	tunnel_tex = tex;
}

void TunnelShift(float shift)
{
	tunnel_shift = shift;
}

inline unsigned int TunnelPixel(unsigned int i)
{
	
}
	
void Tunnel(Image &dst)
{
	// init the tunnel
	static bool first_time = true;
	if (first_time)
	{
		first_time = false;
		InitTunnel();
	}

	unsigned int *pixels = dst.pixels;
	for (unsigned int i=0; i<640 * 480; i++)
	{
		*pixels++ = TunnelPixel(i);
	}
}

