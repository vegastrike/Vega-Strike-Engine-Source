#ifndef _GFX_SPHERE_H_
#define _GFX_SPHERE_H_

#include "gfx_mesh.h"

class SphereMesh : public Mesh {
  bool centered;
  bool insideout;
 public:
  SphereMesh(float radius, int stacks, int slices, char *texture, bool insideout=false,bool centeredOnShip=false);
  void Draw();
  void Draw(const Vector &pp, const Vector &pq, const Vector &pr, const Vector &ppos);
  void ProcessDrawQueue();
};

#endif

