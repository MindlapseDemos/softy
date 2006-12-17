#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <gl.h>
#include <vmath.h>
#include "image_ppm.h"

static void draw_plane(void);
static void draw_object(unsigned int msec);

#define ASPECT	(16.0 / 9.0)
static int xres = 640;
static int yres = 480;
static int vp_x, vp_y;

static unsigned int *over;
static unsigned int norm_tex, ref_tex;

extern unsigned int *cfb;

static float white[] = {1, 1, 1, 1};
static float black[] = {0, 0, 0, 0};
static float amb[] = {0.03, 0.03, 0.03, 0};


int glow_init(void)
{
	void *img;
	int tex_x, tex_y;

	vp_x = xres;
	vp_y = (float)xres / ASPECT;

	if(!(img = load_ppm("data/norm.ppm", &tex_x, &tex_y))) {
		return -1;
	}
	glGenTextures(1, &norm_tex);
	glBindTexture(GL_TEXTURE_2D, norm_tex);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, tex_x, tex_y, 0, GL_BGRA, GL_UNSIGNED_BYTE, img);
	free_ppm(img);

	if(!(img = load_ppm("data/refgold.ppm", &tex_x, &tex_y))) {
		return -1;
	}
	glGenTextures(1, &ref_tex);
	glBindTexture(GL_TEXTURE_2D, ref_tex);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, tex_x, tex_y, 0, GL_BGRA, GL_UNSIGNED_BYTE, img);
	free_ppm(img);


	if(!(over = load_ppm("data/norm_over.ppm", 0, 0))) {
		return -1;
	}


	return 0;
}

void glow_run(unsigned int msec, int param)
{
	int i, j;
	unsigned int *sptr, *dptr;
	float lpos[] = {0, 0, 0, 1};

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, amb);

	glClearColor(amb[0], amb[1], amb[2], 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0, 3.5, 6, 0, 1, 0, 0, 1, 0);

	lpos[0] = cos(msec / 1000.0) * 5.0;
	lpos[1] = 2.5 + sin(msec / 500.0);
	lpos[2] = sin(msec / 1000.0) * 5.0 - 1.0;
	glLightfv(GL_LIGHT0, GL_POSITION, lpos);

	draw_plane();

	draw_object(msec);

	memcpy(cfb + ((yres - vp_y) / 2) * xres, fglGetFrameBuffer(), vp_x * vp_y * 4);

	dptr = cfb;
	sptr = over;
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

static void draw_plane(void)
{
	float dif[] = {0.76, 0.36, 0.16, 1.0};
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, dif);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, white);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 40.0);

	glEnable(GL_PHONG);
	glEnable(GL_NORMAL_MAP);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, norm_tex);

	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);
	glTexCoord2f(1, 0); glVertex3f(3.5, 0, -2);
	glTexCoord2f(1, 1); glVertex3f(3.5, 0, 2.7);
	glTexCoord2f(0, 1); glVertex3f(-3.5, 0, 2.7);
	glTexCoord2f(0, 0); glVertex3f(-3.5, 0, -2);
	glEnd();

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_NORMAL_MAP);
	glDisable(GL_PHONG);
	glEnable(GL_SMOOTH);
}

#include "sgi_mesh.h"

static void draw_object(unsigned int msec)
{
	int i, j;
	float t = (float)msec / 20.0f;

	glPushMatrix();
	glTranslatef(0, 1.5, 0);
	glRotatef(t, 1, 0, 0);
	glRotatef(t, 0, 1, 0);

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, white);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, white);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 50.0);

	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, ref_tex);

	glDisable(GL_LIGHTING);

	glBegin(GL_TRIANGLES);
	glColor3f(1, 1, 1);
	for(i=0; i<MESH_NFACE; i++) {
		for(j=0; j<3; j++) {
			int nidx = i * 3 + j;
			int vidx = triangles[i][j];
			
			float x = vertices[vidx][0] / 50.0 + 1.321617;
			float y = vertices[vidx][1] / 50.0 - 0.888939;
			float z = vertices[vidx][2] / 50.0 + 0.668628;

			float nx = normals[nidx][0];
			float ny = normals[nidx][1];
			float nz = normals[nidx][2];

			glNormal3f(nx, ny, nz);
			glVertex3f(x, y, z);
		}
	}
	glEnd();

	glEnable(GL_LIGHTING);

	glDisable(GL_TEXTURE_2D);

	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);

	glPopMatrix();
}
