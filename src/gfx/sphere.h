#ifndef _GFX_SPHERE_H_
#define _GFX_SPHERE_H_

#include "mesh.h"
#include "quaternion.h"
#include <assert.h>
#ifndef M_PI
#define M_PI 3.1415926536F
#endif

class SphereMesh : public Mesh {
  //no local vars allowed
 protected:
  virtual float GetT ( float rho, float rho_min, float rho_max);
  virtual float GetS (float theta,  float theta_min, float theta_max);
  virtual Mesh * AllocNewMeshesEachInSizeofMeshSpace (int num) { assert(sizeof(Mesh)==sizeof(*this)); return new SphereMesh[num];}
  void InitSphere (float radius, int stacks, int slices, const char *texture, const char *alpha=NULL, bool insideout=false, const BLENDFUNC a=ONE, const BLENDFUNC b=ZERO, bool envMap=false, float rho_min=0.0, float rho_max=M_PI, float theta_min=0.0, float theta_max=2*M_PI, FILTER mipmap=MIPMAP, bool reverse_normals=false,bool subclass=false);
 public:
  SphereMesh () :Mesh(){setConvex(true);}
  virtual int MeshType() {return 1;}
  SphereMesh(float radius, int stacks, int slices, const char *texture, const char *alpha=NULL, bool insideout=false, const BLENDFUNC a=ONE, const BLENDFUNC b=ZERO, bool envMap=false, float rho_min=0.0, float rho_max=M_PI, float theta_min=0.0, float theta_max=2*M_PI, FILTER mipmap=MIPMAP,bool reverse_normals=false){
    InitSphere (radius,stacks,slices,texture,alpha,insideout,a,b,envMap,rho_min,rho_max,theta_min,theta_max,mipmap,reverse_normals);
  }
  void Draw(float lod, bool centered =false, const Matrix &m=identity_matrix);
#ifdef PARTITIONED_Z_BUFFER
  virtual void ProcessDrawQueue(int whichpass,int which,float zmin,float zmax);
#else
  virtual void ProcessDrawQueue(int whichpass,int which);
#endif
};
class CityLights : public SphereMesh {
  //no local vars allowed
  //these VARS BELOW ARE STATIC...change it and DIE
  static float wrapx;
  static float wrapy;
 protected:
  virtual float GetT ( float rho, float rho_min, float rho_max);
  virtual float GetS (float theta,  float theta_min, float theta_max);
  virtual Mesh * AllocNewMeshesEachInSizeofMeshSpace (int num ) { assert(sizeof(Mesh)==sizeof(*this)); return new CityLights[num];}
 public:
  virtual void SelectCullFace (int whichdrawqueue);
  virtual void RestoreCullFace (int whichdrawqueue);

  CityLights () : SphereMesh () {}
  CityLights (float radius, int stacks, int slices, const char *texture, int texturewrapx, int texturewrapy, bool insideout=false, const BLENDFUNC a=ONE, const BLENDFUNC b=ZERO, bool envMap=false, float rho_min=0.0, float rho_max=M_PI, float theta_min=0.0, float theta_max=2*M_PI, bool inside_out=true);
#ifdef PARTITIONED_Z_BUFFER
  virtual void ProcessDrawQueue(int whichpass,int which,float zmin,float zmax);
#else
  virtual void ProcessDrawQueue(int whichpass,int which);
#endif

};
#endif

