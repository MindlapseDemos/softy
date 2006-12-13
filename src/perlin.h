#ifndef _PERLIN_H_
#define _PERLIN_H_

#ifdef __cplusplus
extern "C" {
#endif

double noise1(double);
double noise2(double *);
double noise3(double *);
void normalize3(double *);
void normalize2(double *);

double perlin_noise_1d(double x, double alpha, double beta, int n);
double perlin_noise_2d(double x, double y, double alpha, double beta, int n);
double perlin_noise_3d(double x, double y, double z, double alpha, double beta, int n);

#ifdef __cplusplus
}
#endif

#endif
