#ifndef _GFX_SPHERE_H_
#define _GFX_SPHERE_H_

#include "mesh.h"
#include "quaternion.h"

class SphereMesh : public Mesh {
  bool insideout;
 public:
  SphereMesh () :Mesh(), insideout(false){}
  SphereMesh(float radius, int stacks, int slices, char *texture, char *alpha=NULL, bool insideout=false, const BLENDFUNC a=ONE, const BLENDFUNC b=ZERO, bool envMap=false);
  void Draw(float lod, bool centered =false, const Transformation &transform = identity_transformation, const Matrix m=identity_matrix);
  //  void ProcessDrawQueue();
};

#endif

