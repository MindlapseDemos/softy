// tunnel.cc

#include "tunnel.h"

static Color tunnel_fog_color;
static Matrix3x3 tunnel_rot;
static float tunnel_shift;
static const Image *tunnel_tex;
static Vector3 tunnel_vecs[640 * 480];

void InitTunnel()
{
	for (unsigned int j=0; j<480; j++)
	{
		for (unsigned int i=0; i<640; i++)
		{
			float x = ((float)i - 320) / 320.0f;
			float y = ((float)j - 240) / 240.0f;
			x *= 4.0f / 3.0f;
			tunnel_vecs[i + 640 * j] = Vector3(x, y, 1.0f).normalized();
		}
	}
}

void TunnelFogColor(const Color &clr)
{
	tunnel_fog_color = clr;
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

inline Color TunnelPixel(unsigned int i)
{
	Vector3 view = tunnel_vecs[i];//.transformed(tunnel_rot);

	const float r = 2.0f;
	const float fog_start = 2.0f;
	const float fog_end = 8.0f;
	float cs = dot_product(view, Vector3(0, 0, 1));
	float ang = acos(cs);
	float sn = sin(ang);

	// avoid division with zero
	if (sn < 0.001f) return tunnel_fog_color;
	float dist = r / sn;
	
	Vector3 pt = dist * view;
	Vector3 pt_nor = pt / r;

	float u = dot_product(pt_nor, Vector3(0, 1, 0));
	u = u * 0.5f + 0.5f;
	u = fmod(u, 1);

	float v = pt.z;
	v += tunnel_shift;
	//v = fmod(v, 1);

	unsigned int cu, cv;
	cu = (unsigned int) (255 * u);
	cu %= 256;
	cv = (unsigned int) (255 * v);
	cv %= 256;

	Color texel = tunnel_tex->pixels[cu + 256 * cv];


	// apply fog
	float fog_dist = dist - fog_start;
	if (fog_dist > fog_end) return tunnel_fog_color;
	float fog_factor = fog_dist / (fog_end - fog_start);
	int cfc = (int) (255 * fog_factor);


	//printf("%f, %d\n", dist, cfc);

	return Lerp(texel, tunnel_fog_color, cfc);
	


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

	Color *pixels = dst.pixels;
	for (unsigned int i=0; i<640 * 480; i++)
	{
		*pixels++ = TunnelPixel(i);
	}
}

