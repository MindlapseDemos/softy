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

static void draw_amiga(unsigned int msec, int mirror);
static void draw_plane(void);
static void sphere(float rad, int vpoints, float umax, float vmax);

static unsigned int amiga_tex;

static float white[] = {1, 1, 1, 1};
static float black[] = {0, 0, 0, 0};
static float amb[] = {0.03, 0.03, 0.03, 0};
static float sph_diffuse[] = {0.8, 0.8, 0.8, 1};
static float mirror_color[] = {0.2, 0.4, 0.7, 1};
static float sph_mir_col[4];

extern unsigned int *cfb;
unsigned int *over1;

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

	if(!(over1 = load_ppm("data/phong.ppm", 0, 0))) {
		return -1;
	}

	sph_mir_col[0] = mirror_color[0] * sph_diffuse[0];
	sph_mir_col[1] = mirror_color[1] * sph_diffuse[1];
	sph_mir_col[2] = mirror_color[2] * sph_diffuse[2];
	sph_mir_col[3] = mirror_color[3] * sph_diffuse[3];

	return 0;
}

void amiga_run(unsigned int msec, int param)
{
	int i, j;
	unsigned int *sptr, *dptr;
	float lpos[][4] = {
		{-100, 100, 100, 1},
		{0, 0, 0, 1}
	};

	/*glEnable(GL_PHONG);*/
	glEnable(GL_DEPTH_TEST);
	
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	/*glEnable(GL_LIGHT1);*/
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, amb);

	glClearColor(amb[0], amb[1], amb[2], 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0, 3.5, 6, 0, 1, 0, 0, 1, 0);

	/*glLightfv(GL_LIGHT0, GL_POSITION, lpos[0]);*/


	/* ---- draw floor ---- */
	glPushMatrix();
	glTranslatef(cos(msec / 1000.0) * 4.0, 2 + sin(msec / 500.0), sin(msec / 1000.0) * 4.0);
	glLightfv(GL_LIGHT0, GL_POSITION, lpos[1]);
	glPopMatrix();
	glEnable(GL_PHONG);

	draw_plane();

	/* ---- draw reflected sphere ---- */
	glDisable(GL_PHONG);
	glEnable(GL_SMOOTH);
	
	glPushMatrix();
	glTranslatef(cos(msec / 1000.0) * 4.0, -(2 + sin(msec / 500.0)), sin(msec / 1000.0) * 4.0);
	glLightfv(GL_LIGHT0, GL_POSITION, lpos[1]);
	glPopMatrix();

	glDisable(GL_DEPTH_TEST);
	draw_amiga(msec, 1);
	glEnable(GL_DEPTH_TEST);

	/* ---- draw regular sphere ---- */
	glPushMatrix();
	glTranslatef(cos(msec / 1000.0) * 4.0, 2 + sin(msec / 500.0), sin(msec / 1000.0) * 4.0);
	glLightfv(GL_LIGHT0, GL_POSITION, lpos[1]);
	glPopMatrix();

	glEnable(GL_PHONG);
	draw_amiga(msec, 0);
	glDisable(GL_PHONG);
	glEnable(GL_SMOOTH);


	/*glDisable(GL_LIGHT1);*/

	memcpy(cfb + ((yres - vp_y) / 2) * xres, fglGetFrameBuffer(), vp_x * vp_y * 4);

	dptr = cfb;
	sptr = over1;
	for(j=0; j<480; j++) {
		for(i=0; i<640; i++) {
			unsigned int col = *sptr++;
			if(col != 0x00ff0000) {
				*dptr = col;
			}
			dptr++;
		}
	}
}

static float squish(float x)
{
	x = fmod(x, PI);
	float falloff;
	if(x > HALF_PI) {
		falloff = (x > PI * 0.9375 ? -(x - PI * 0.9375) * 2.0 : 0);
	} else {
		falloff = cos(2.0 * x) * 0.5 + 0.5;
	}
	return 1.0 - sin(3.0 * PI * x) * falloff * 0.25;
}

static void draw_amiga(unsigned int msec, int mirror)
{
	float sec = (float)msec / 1000.0;
	float x, y, z, sy;

	x = cos(sec * 0.4) * 2.0;
	y = 0.7 + fabs(sin(sec * 2.0)) * 1.6;
	z = sin(sec * 0.8);
	sy = squish(sec * 2.0);

	if(mirror) {
		y = -y;
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, sph_mir_col);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, black);
	} else {
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, sph_diffuse);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, white);
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 50.0);
	}

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	glTranslatef(x, y, z);
	/*glRotatef((float)msec / 10.0, 1, 0, 0);
	glRotatef((float)msec / 10.0, 0, 1, 0);*/
	glScalef(1, sy, 1);
	glRotatef((float)msec / 14.0, 0, 1, 0);
	glRotatef((float)msec / 15.0, 0, 0, 1);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, amiga_tex);
	sphere(1.0, 10, 1.0, 1.0);
	glDisable(GL_TEXTURE_2D);

	glPopMatrix();
}

static void draw_plane(void)
{
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, mirror_color);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, white);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 30.0);

	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);
	glVertex3f(5, 0, -3);
	glVertex3f(5, 0, 2.7);
	glVertex3f(-5, 0, 2.7);
	glVertex3f(-5, 0, -3);
	glEnd();
}


static void sphere_vertex(float x, float y, float z, float rad, float u, float v, float vmax)
{
	float nx, ny, nz;
	
	/* tex coords */
	glTexCoord2f(u, v);

	/* normal */
	nx = x / rad;
	ny = y / rad;
	nz = z / rad;
	glNormal3f(nx, ny, nz);

	glVertex3f(x, y, z);
}


#define SPH_X(rho, theta, phi)	((rho) * cos(theta) * sin(phi))
#define SPH_Y(rho, theta, phi)	((rho) * cos(phi))
#define SPH_Z(rho, theta, phi)	((rho) * sin(theta) * sin(phi))

static void sphere(float rad, int vpoints, float umax, float vmax)
{
	float u0, u1, v0, v1;
	float du, dv;
	int i, j, upoints = vpoints * 2;

	du = umax / upoints;
	dv = vmax / vpoints;

	v0 = 0.0;
	v1 = dv;

	glBegin(GL_QUADS);
	glColor3f(1, 1, 1);
	for(i=0; i<vpoints; i++) {
		float phi0 = v0 * PI;
		float phi1 = v1 * PI;
	
		u0 = 0.0;
		u1 = du;

		for(j=0; j<upoints; j++) {
			float theta0 = u0 * TWO_PI;
			float theta1 = u1 * TWO_PI;

			sphere_vertex(SPH_X(rad, theta0, phi1), SPH_Y(rad, theta0, phi1), SPH_Z(rad, theta0, phi1), rad, u0, v1, vmax);
			sphere_vertex(SPH_X(rad, theta1, phi1), SPH_Y(rad, theta1, phi1), SPH_Z(rad, theta1, phi1), rad, u1, v1, vmax);
			sphere_vertex(SPH_X(rad, theta1, phi0), SPH_Y(rad, theta1, phi0), SPH_Z(rad, theta1, phi0), rad, u1, v0, vmax);
			sphere_vertex(SPH_X(rad, theta0, phi0), SPH_Y(rad, theta0, phi0), SPH_Z(rad, theta0, phi0), rad, u0, v0, vmax);

			u0 += du;
			u1 += du;
		}
		v0 += dv;
		v1 += dv;
	}
	glEnd();
}
