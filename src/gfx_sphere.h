#ifndef _GFX_SPHERE_H_
#define _GFX_SPHERE_H_

#include "gfx_mesh.h"
#include "quaternion.h"

class SphereMesh : public Mesh {
  bool centered;
  bool insideout;
 public:
  SphereMesh () :Mesh(),centered(false), insideout(false){}
  SphereMesh(float radius, int stacks, int slices, char *texture, char *alpha=NULL, bool insideout=false,bool centeredOnShip=false, const BLENDFUNC a=ONE, const BLENDFUNC b=ZERO);
  void Draw(const Transformation &transform = identity_transformation, const Matrix m=identity_matrix);
  void ProcessDrawQueue();
};

#endif

