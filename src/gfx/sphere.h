#ifndef _GFX_SPHERE_H_
#define _GFX_SPHERE_H_

#include "mesh.h"
#include "quaternion.h"
#ifndef M_PI
#define M_PI 3.1415926536F
#endif

class SphereMesh : public Mesh {
 public:
  SphereMesh () :Mesh(){}
  SphereMesh(float radius, int stacks, int slices, char *texture, char *alpha=NULL, bool insideout=false, const BLENDFUNC a=ONE, const BLENDFUNC b=ZERO, bool envMap=false, float rho_min=0.0, float rho_max=M_PI, float theta_min=0.0, float theta_max=2*M_PI);
  void Draw(float lod, bool centered =false, const Matrix m=identity_matrix);
  void ProcessDrawQueue(int which);
};

#endif

