// p_tunnel.h

#ifndef _P_TUNNEL_H_
#define _P_TUNNEL_H_

#include "tunnel.h"

bool tunnel_init();
void tunnel_render(float secs);
void tunnel_run(unsigned int msec, int param);
void tunnel_cleanup();

#endif // ndef _P_TUNNEL_H_
