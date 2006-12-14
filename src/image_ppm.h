#ifndef _IMAGE_PPM_H_
#define _IMAGE_PPM_H_

#ifdef __cplusplus
extern "C" {
#endif

void *load_ppm(const char *fname, int *xsz, int *ysz);
void free_ppm(void *img);

#ifdef __cplusplus
}
#endif

#endif	/* _IMAGE_PPM_H_ */
