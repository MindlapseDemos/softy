// tunnel.h

#ifndef _TUNNEL_H_
#define _TUNNEL_H_

#include "image.h"
#include "vmath.h"
#include "color.h"

Color * GetTunnelBuffer();
void TunnelReplica(unsigned int n);
void TunnelFogColor(const Color &clr);
void TunnelRot(const Vector2 &rot);
void TunnelTex(const Image *tex);
void TunnelShift(float shift);
void TunnelFogStart(float start);
void TunnelFogEnd(float end);
void TunnelFogAmp(float amp);
void Tunnel(Image &dst);

#endif // ndef _TUNNEL_H_
