#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <gl.h>
#include <vmath.h>
#include "image_ppm.h"

#define ASPECT	(16.0 / 9.0)
static int xres = 640;
static int yres = 480;
static int vp_x, vp_y;

static void gsphere_triangle(vec3_t a, vec3_t b, vec3_t c, int iter);
void gsphere(float sz, int iter, int hemisphere);

static unsigned int amiga_tex;

extern unsigned int *cfb;

int amiga_init(void)
{
	void *img;
	int tex_x, tex_y;

	vp_x = xres;
	vp_y = (float)xres / ASPECT;

	if(!(img = load_ppm("data/amiga.ppm", &tex_x, &tex_y))) {
		return -1;
	}

	glGenTextures(1, &amiga_tex);
	glBindTexture(GL_TEXTURE_2D, amiga_tex);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, tex_x, tex_y, 0, GL_BGRA, GL_UNSIGNED_BYTE, img);
	free_ppm(img);

	return 0;
}

void amiga_run(unsigned int msec)
{
	float lpos[] = {-100, 100, 100, 1.0};
	float amb[] = {0, 0, 0, 0};

	float white[] = {1, 1, 1, 0};

	glEnable(GL_PHONG);
	glDisable(GL_DEPTH_TEST);
	
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0, GL_POSITION, lpos);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, amb);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0, -1, -4);
	glRotatef((float)msec / 10.0, 0, 1, 0);

	glClearColor(0.1, 0.1, 0.1, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, white);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 60.0);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, amiga_tex);
	gsphere(1.0, 3, 0);
	glDisable(GL_TEXTURE_2D);


	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);
	glVertex3f(-10, 0, -10);
	glVertex3f(-10, 0, 1);
	glVertex3f(10, 0, 1);
	glVertex3f(10, 0, -10);
	glEnd();

	/*glDisable(GL_LIGHTING);
	glBegin(GL_QUADS);
	glColor3f(1, 0, 0);
	glVertex3f(-10, -1, 0);
	glVertex3f(-10, 1, 0);
	glVertex3f(10, 1, 0);
	glVertex3f(10, -1, 0);

	glColor3f(0, 1, 0);
	glVertex3f(10, -1, 0);
	glVertex3f(10, 1, 0);
	glVertex3f(-10, 1, 0);
	glVertex3f(-10, -1, 0);
	glEnd();*/
	glEnable(GL_LIGHTING);


	memcpy(cfb + ((yres - vp_y) / 2) * xres, fglGetFrameBuffer(), vp_x * vp_y * 4);
}

#define PT0		{0, 1, 0}
#define PT1		{0, -1, 0}
#define PT2		{0.707, 0, 0.707}
#define PT3		{-0.707, 0, 0.707}
#define PT4		{-0.707, 0, -0.707}
#define PT5		{0.707, 0, -0.707}

static vec3_t sphv[] = {
	PT0, PT5, PT2,
	PT0, PT2, PT3,
	PT0, PT3, PT4,
	PT0, PT4, PT5,
	PT1, PT2, PT5,
	PT1, PT3, PT2,
	PT1, PT4, PT3,
	PT1, PT5, PT4,
};


void gsphere(float sz, int iter, int hemisphere)
{
	int i, vcount = hemisphere ? 4 : 8;

	/*glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glScalef(sz, sz, sz);*/

	/*glEnable(GL_NORMALIZE);*/

	glBegin(GL_TRIANGLES);
	glColor3f(0, 1, 0);

	for(i=0; i<vcount; i++) {
		gsphere_triangle(sphv[i * 3], sphv[i * 3 + 1], sphv[i * 3 + 2], iter);
	}

	glEnd();

	/*glDisable(GL_NORMALIZE);*/

	/*glPopMatrix();*/
}

static void gsphere_vertex(float x, float y, float z)
{
	float u, v;
	
	u = atan2(z, x) / TWO_PI;
	v = acos(y) / PI;	/* y / r, consider r == 1 */
	glTexCoord2f(u, v);
	
	glNormal3f(x, y, z);
	glVertex3f(x, y, z);
}

static void gsphere_triangle(vec3_t a, vec3_t b, vec3_t c, int iter)
{
	if(iter-- > 0) {
		vec3_t d, e, f;
		float dlen, elen, flen;

		d.x = a.x + c.x;
		d.y = a.y + c.y;
		d.z = a.z + c.z;
		dlen = sqrt(d.x * d.x + d.y * d.y + d.z * d.z);
		d.x /= dlen;
		d.y /= dlen;
		d.z /= dlen;

		e.x = a.x + b.x;
		e.y = a.y + b.y;
		e.z = a.z + b.z;
		elen = sqrt(e.x * e.x + e.y * e.y + e.z * e.z);
		e.x /= elen;
		e.y /= elen;
		e.z /= elen;

		f.x = b.x + c.x;
		f.y = b.y + c.y;
		f.z = b.z + c.z;
		flen = sqrt(f.x * f.x + f.y * f.y + f.z * f.z);
		f.x /= flen;
		f.y /= flen;
		f.z /= flen;

		gsphere_triangle(a, e, d, iter);
		gsphere_triangle(d, f, c, iter);
		gsphere_triangle(e, b, f, iter);
		gsphere_triangle(e, f, d, iter);
	} else {
		gsphere_vertex(a.x, a.y, a.z);
		gsphere_vertex(b.x, b.y, b.z);
		gsphere_vertex(c.x, c.y, c.z);
	}
}

