#ifndef _GFX_SPHERE_H_
#define _GFX_SPHERE_H_

#include "gfx_mesh.h"

class SphereMesh : public Mesh {
 public:
  SphereMesh(float radius, int stacks, int slices, char *texture);
};

#endif

