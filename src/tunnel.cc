// tunnel.cc

#include "tunnel.h"

#define SINE_TABLE 1000
#define TEXSIZE 512

static Color tunnel_fog_color;
static Vector2 tunnel_rot;
static Matrix3x3 rot_mat;
static float tunnel_shift;
static const Image *tunnel_tex;
static Vector3 tunnel_vecs[640 * 480];
static unsigned int replica = 1;
static float fog_amp = 1;

static float fog_start = 1.0f;
static float inv_fog_start;
static float fog_end = 2.5f;
static float inv_fog_end;


float sines[SINE_TABLE];

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
	
	// init sine table
	for (unsigned int i=0; i<SINE_TABLE; i++)
	{
		float p = (float) i / (float) (SINE_TABLE - 1);
		sines[i] = sinf(acosf(p));
	}
}

void TunnelReplica(unsigned int n)
{
	if (n == 0) n = 1;
	replica = n;	
}

void TunnelFogColor(const Color &clr)
{
	tunnel_fog_color = clr;
}

void TunnelRot(const Vector2 &rot)
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

void TunnelFogStart(float start)
{
	fog_start = start / fog_amp;
	inv_fog_start = 1.0f / fog_start;	
}
void TunnelFogEnd(float end)
{
	fog_end = end / fog_amp;
	inv_fog_end = 1.0f / fog_end;
}

void TunnelFogAmp(float amp)
{
	fog_amp = amp;	
}

inline Color TunnelPixel(unsigned int i)
{
	static Vector3 view;
	view = tunnel_vecs[i];
	view.transform(rot_mat);

	// OPT:
	//float cs = dot_product(view, Vector3(0, 0, 1));
	// NEW:
	float cs = view.z;
	// ENDOPT
	
	// OPT:
	//float sn = sinf(acosf(cs));
	// NEW:
	static float sn;
	static int index;
	index = cs * SINE_TABLE;
	if (index < 0) index = -index;
	sn = sines[index];
	
	// avoid division with zero
	if (sn < inv_fog_end) return tunnel_fog_color;
	float dist = 1.0f / sn;
	
	Vector3 pt = dist * view;

	// OPT:
	//float u = dot_product(pt_nor, Vector3(0, 1, 0));
	// NEW:
	float u = pt.y;
	u *= 2.0f;
	// ENDOPT

	float v = pt.z;
	v += tunnel_shift;
	v *= 0.5f;

	unsigned int cu, cv;
	cu = (unsigned int) (255 * u);
	cu %= TEXSIZE;
	cv = (unsigned int) (255 * v);
	cv %= TEXSIZE;

	Color texel = tunnel_tex->pixels[cu + TEXSIZE * cv];

	if (sn > inv_fog_start) return texel;

	// apply fog
	float fog_dist = dist - fog_start;
	float fog_factor = fog_dist / (fog_end - fog_start);
	int cfc = (int) (255 * fog_factor);
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
	
	rot_mat.set_rotation(tunnel_rot);

	Color *pixels = dst.pixels;
	
	for (unsigned int i=0; i<640 * 480; i+=replica)
	{
		*pixels++ = TunnelPixel(i);
	}
	
	// render replicas
	unsigned int rbytes = 640 * 480 * 4 / replica;
	char *screen = (char*)dst.pixels;
	for (unsigned int i=1; i<replica; i++)
	{
		screen += rbytes;
		memcpy(screen, dst.pixels, rbytes);
	}
}

