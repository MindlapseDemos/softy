#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <gl.h>
#include <vmath.h>
#include "image_ppm.h"

static void slime(unsigned int *fb, unsigned int *img);
static void draw_cube(float sz, float r, float g, float b, float spow);
static void draw_plane(float xsz, float ysz, int subdiv, float r, float g, float b);
static void draw_starfield(float t);

#define ASPECT	(16.0 / 9.0)

static int xres = 640;
static int yres = 480;
static int vp_x, vp_y, slime_vp_x, slime_vp_y, voffset;

static int hack = 152;

static unsigned int floor_tex, cube_tex;

unsigned int *cfb;

static unsigned int *logo_pix, *foot_pix;
static int logo_x, logo_y, foot_x, foot_y;

#define SLIME_FRAMES	110
static unsigned int *frames[SLIME_FRAMES];

#define FRAME_INTERVAL	27

#define NSTARS	1000
float stars[NSTARS * 3];

int slimy_init(void)
{
	int i;

	vp_x = xres;
	vp_y = (float)xres / ASPECT;
	slime_vp_x = vp_x / 2;
	slime_vp_y = vp_y;
	voffset = (yres - slime_vp_y) / 2 * xres;

	printf("allocating: %d mb for slime buffers\n", (int)(slime_vp_x * slime_vp_y * SLIME_FRAMES * sizeof(unsigned int)) / 1024 / 1024);

	for(i=0; i<SLIME_FRAMES; i++) {
		if(!(frames[i] = calloc(slime_vp_x * slime_vp_y, sizeof *frames[i]))) {
			perror("malloc failure");
			return -1;
		}
	}
	
	fglCreateContext();
	glViewport(0, 0, vp_x, vp_y);

	glClearColor(0, 0, 0, 0);
	glClearDepth(1);
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, (float)vp_x / (float)vp_y, 1.0, 100.0);
	glMatrixMode(GL_MODELVIEW);

	{
		int tex_x, tex_y;
		unsigned int *pixels;

		if(!(pixels = load_ppm("data/overlap.ppm", &tex_x, &tex_y))) {
			return -1;
		}

		glGenTextures(1, &floor_tex);
		glBindTexture(GL_TEXTURE_2D, floor_tex);
		glTexImage2D(GL_TEXTURE_2D, 0, 4, tex_x, tex_y, 0, GL_BGRA, GL_UNSIGNED_BYTE, pixels);
		free(pixels);

		if(!(pixels = load_ppm("data/marble.ppm", &tex_x, &tex_y))) {
			return -1;
		}

		glGenTextures(1, &cube_tex);
		glBindTexture(GL_TEXTURE_2D, cube_tex);
		glTexImage2D(GL_TEXTURE_2D, 0, 4, tex_x, tex_y, 0, GL_BGRA, GL_UNSIGNED_BYTE, pixels);
		free(pixels);
	}

	if(!(logo_pix = load_ppm("data/title.ppm", &logo_x, &logo_y))) {
		return -1;
	}

	if(!(foot_pix = load_ppm("data/foot.ppm", &foot_x, &foot_y))) {
		return -1;
	}

	/* init starfield */
	for(i=0; i<NSTARS; i++) {
		float x = 2.0 * ((float)rand() / (float)RAND_MAX) - 1.0;
		float y = 2.0 * ((float)rand() / (float)RAND_MAX) - 1.0;
		float z = 2.0 * ((float)rand() / (float)RAND_MAX) - 1.0;

		stars[i * 3] = x * 20.0;
		stars[i * 3 + 1] = y * 20.0;
		stars[i * 3 + 2] = z * 20.0;
	}

	return 0;
}

void slimy_free(void)
{
	int i;
	for(i=0; i<SLIME_FRAMES; i++) {
		free(frames[i]);
	}
}

#define SZ		4.5
#define HSZ		(SZ / 2.0)
void slimy_run(unsigned int msec, int param)
{
	static float amb[] = {0.3, 0.3, 0.3, 0.0};
	static float lpos[] = {-100, 100, 100, 1.0};

	int xoffs = (xres - slime_vp_x) / 2;
	float t = (float)msec / 9.0;


	glClearColor(0, 0, 0, 0);
	glClearDepth(1);

	glDisable(GL_DEPTH_TEST);
	glShadeModel(GL_FLAT);

	glEnable(GL_LIGHT0);
	glLoadIdentity();
	glLightfv(GL_LIGHT0, GL_POSITION, lpos);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, amb);

	/* --------- draw the background --------- */
	glClear(GL_COLOR_BUFFER_BIT);


	glLoadIdentity();
	glTranslatef(0, -1, -3.5);
	glRotatef(t / 4.0, 0, 1, 0);

	draw_starfield((float)msec / 1000.0);
	
	glDisable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, floor_tex);
	draw_plane(SZ, SZ, 7, 0.8, 0.8, 0.8);
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_LIGHTING);

	memcpy(cfb + voffset, fglGetFrameBuffer(), vp_x * vp_y * sizeof(unsigned int));

	/* --------- draw the slimy bit ---------- */
	glClear(GL_COLOR_BUFFER_BIT);

	glLoadIdentity();
	glTranslatef(0, 0, -3.5);
	glRotatef(cos(t/150.0) * 45.0, 1, 0, 0);
	glRotatef(cos(t/200.0) * 90.0 + sin(t / 100), 0, 1, 0);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, cube_tex);
	/*draw_cube(1.0, 1, 0.6, 0.4, 20);*/
	draw_cube(1.0, 1, 1, 1, 20);
	glDisable(GL_TEXTURE_2D);

	slime(cfb + voffset + xoffs, fglGetFrameBuffer() + xoffs);

	memcpy(cfb, logo_pix, logo_x * logo_y * sizeof(unsigned int));
	memcpy(cfb + (yres - foot_y) * xres, foot_pix, foot_x * foot_y * sizeof(unsigned int));
}

#define HANDLE_COLORKEY
static void slime(unsigned int *fb, unsigned int *img)
{
	static int current;
	int i, j;

	for(i=0; i<slime_vp_y; i++) {
		memcpy(frames[current] + i * slime_vp_x, img, slime_vp_x * sizeof(unsigned int));
		img += vp_x;
	}

	for(i=0; i<slime_vp_y; i++) {
		int k = (i + hack) / 2;
#ifdef HANDLE_COLORKEY
		unsigned int *src = frames[(current + k) % SLIME_FRAMES] + i * slime_vp_x;

		for(j=0; j<slime_vp_x; j++) {
			if(*src) {
				fb[j] = *src;
			}
			src++;
		}
#else
		memcpy(fb, frames[(current + k) % SLIME_FRAMES] + i * slime_vp_x, slime_vp_x * sizeof(unsigned int));
#endif
		fb += xres;
	}

	current = (current + 1) % SLIME_FRAMES;
}


static void draw_cube(float sz, float r, float g, float b, float spow)
{
	float color[4];
	float white[] = {1, 1, 1, 1};
	sz *= 0.5;

	color[0] = r;
	color[1] = g;
	color[2] = b;
	color[3] = 1.0;

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, white);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, spow);
	
	glBegin(GL_QUADS);
	
	/* face +Z */
	glNormal3f(0, 0, 1);
	glColor3f(1, 0, 0);
	glTexCoord2f(0, 0);
	glVertex3f(sz, -sz, sz);
	glTexCoord2f(1, 0);
	glVertex3f(-sz, -sz, sz);
	glTexCoord2f(1, 1);
	glVertex3f(-sz, sz, sz);
	glTexCoord2f(0, 1);
	glVertex3f(sz, sz, sz);

	/* face -X */
	glNormal3f(-1, 0, 0);
	glColor3f(1, 0, 0);
	glTexCoord2f(0, 0);
	glVertex3f(-sz, -sz, sz);
	glTexCoord2f(1, 0);
	glVertex3f(-sz, -sz, -sz);
	glTexCoord2f(1, 1);
	glVertex3f(-sz, sz, -sz);
	glTexCoord2f(0, 1);
	glVertex3f(-sz, sz, sz);

	/* face -Z */
	glNormal3f(0, 0, -1);
	glColor3f(1, 0, 0);
	glTexCoord2f(0, 1);
	glVertex3f(-sz, sz, -sz);
	glTexCoord2f(0, 0);
	glVertex3f(-sz, -sz, -sz);
	glTexCoord2f(1, 0);
	glVertex3f(sz, -sz, -sz);
	glTexCoord2f(1, 1);
	glVertex3f(sz, sz, -sz);

	/* face +X */
	glNormal3f(1, 0, 0);
	glColor3f(1, 0, 0);
	glTexCoord2f(0, 1);
	glVertex3f(sz, sz, -sz);
	glTexCoord2f(0, 0);
	glVertex3f(sz, -sz, -sz);
	glTexCoord2f(1, 0);
	glVertex3f(sz, -sz, sz);
	glTexCoord2f(1, 1);
	glVertex3f(sz, sz, sz);

	/* face +Y */
	glNormal3f(0, 1, 0);
	glColor3f(1, 0, 0);
	glTexCoord2f(1, 0);
	glVertex3f(sz, sz, sz);
	glTexCoord2f(0, 0);
	glVertex3f(-sz, sz, sz);
	glTexCoord2f(0, 1);
	glVertex3f(-sz, sz, -sz);
	glTexCoord2f(1, 1);
	glVertex3f(sz, sz, -sz);

	/* face -Y */
	glNormal3f(0, -1, 0);
	glColor3f(1, 0, 0);
	glTexCoord2f(1, 0);
	glVertex3f(-sz, -sz, -sz);
	glTexCoord2f(1, 1);
	glVertex3f(-sz, -sz, sz);
	glTexCoord2f(0, 1);
	glVertex3f(sz, -sz, sz);
	glTexCoord2f(0, 0);
	glVertex3f(sz, -sz, -sz);

	glEnd();
}

static void draw_plane(float xsz, float ysz, int subdiv, float r, float g, float b)
{
	static const float tile_u = 3.0;
	static const float tile_v = 6.0;
	int i, j;
	int quads_row = subdiv + 1;		/* quads per row */

	float du = 1.0 / (float)quads_row;
	float dv = 1.0 / (float)quads_row;

	float color[4];
	float black[] = {0, 0, 0, 0};

	color[0] = r;
	color[1] = g;
	color[2] = b;
	color[3] = 1;

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, black);

	glBegin(GL_QUADS);
	glColor3f(r, g, b);

	for(j=0; j<quads_row; j++) {
		for(i=0; i<quads_row; i++) {
			float x, y;
			float u = (float)i / (float)quads_row;
			float v = (float)j / (float)quads_row;

			x = (u - 0.5) * xsz;
			y = (1.0 - v - 0.5) * ysz;
			glTexCoord2f(u * tile_u, (1.0 - v) * tile_v);
			glVertex3f(x, 0, y);
			
			x = (u - 0.5) * xsz;
			y = (1.0 - (v + dv) - 0.5) * ysz;
			glTexCoord2f(u * tile_u, (1.0 - (v + dv)) * tile_v);
			glVertex3f(x, 0, y);

			x = (u + du - 0.5) * xsz;
			y = (1.0 - (v + dv) - 0.5) * ysz;
			glTexCoord2f((u + du) * tile_u, (1.0 - (v + dv)) * tile_v);
			glVertex3f(x, 0, y);

			x = (u + du - 0.5) * xsz;
			y = (1.0 - v - 0.5) * ysz;
			glTexCoord2f((u + du) * tile_u, (1.0 - v) * tile_v);
			glVertex3f(x, 0, y);
		}
	}

	glEnd();
}

#define SQ(x)	((x) * (x))
static void draw_starfield(float t)
{
	int i;
	float x, y, z, col, dist_sq;
	float cx, cy, cz;

	cx = cos(t * 1.5) * 5.0;
	cy = 0.0;
	cz = sin(t * 3.0) * 5.0;

	/*glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glRotatef(t * 50.0, 1, 0, 0);
	glRotatef(t * 40.0, 0, 1, 0);
	glTranslatef(cx, cy, cz);*/

	glDisable(GL_LIGHTING);

	glPointSize(2);
	for(i=0; i<NSTARS; i++) {
		x = stars[i * 3];
		y = stars[i * 3 + 1];
		z = stars[i * 3 + 2];

		float len = sqrt(x * x + y * y + z * z);
		float col = (1.0 - len / 20.0) * 0.5 + 0.25;

		glPointSize(len / 20.0 + 1.0);
		glBegin(GL_POINTS);
		glColor3f(col, col, col);
		glVertex3f(x, y, z);
		glEnd();
	}

	glEnable(GL_LIGHTING);
}
