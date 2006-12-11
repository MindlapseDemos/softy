// tunnel.h

#ifndef _TUNNEL_H_
#define _TUNNEL_H_

#include "image.h"
#include "vmath.h"

void TunnelRot(const Matrix3x3 &rot);
void TunnelTex(const Image *tex);
void TunnelShift(float shift);
void Tunnel(Image &dst);

#endif // ndef _TUNNEL_H_
