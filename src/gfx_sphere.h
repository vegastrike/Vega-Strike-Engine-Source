#ifndef _GFX_SPHERE_H_
#define _GFX_SPHERE_H_

#include "gfx_mesh.h"
#include "quaternion.h"

class SphereMesh : public Mesh {
  bool centered;
  bool insideout;
 public:
  SphereMesh(float radius, int stacks, int slices, char *texture, char *alpha=NULL, bool insideout=false,bool centeredOnShip=false);
  void Draw(const Transformation &transform = identity_transformation);
  void ProcessDrawQueue();
};

#endif

