#ifndef _P_SLIMY_H_
#define _P_SLIMY_H_

#ifdef __cplusplus
extern "C" {
#endif

int slimy_init(void);
void slimy_run(unsigned int msec, int param);
void slimy_free(void);

#ifdef __cplusplus
}
#endif

#endif
