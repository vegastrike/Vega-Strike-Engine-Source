#ifndef _RING_H_
#define _RING_H_
#include "mesh.h"
class RingMesh:public Mesh {
  virtual Mesh * AllocNewMeshesEachInSizeofMeshSpace (int num) {return new RingMesh[num];}
  virtual void InitRing (float iradius,float oradius, int slices,const char *texture, const QVector & r, const QVector & s, int texwrapx=1, int texwrapy=1,const BLENDFUNC a=ONE, const BLENDFUNC b=ZERO, bool envMap=false, float theta_min=0.0, float theta_max=2*M_PI, FILTER mipmap=MIPMAP);

 public:
  RingMesh () :Mesh(){}
  RingMesh(float iradius, float oradius, int slices, const char *texture, const QVector & r, const QVector & s,int texwrapx=1, int texwrapy=1,const BLENDFUNC a=SRCALPHA,  const BLENDFUNC b=INVSRCALPHA,   bool envMap=false, float theta_min=0.0, float theta_max=2*M_PI, FILTER mipmap=TRILINEAR){
    InitRing (iradius,oradius,slices,texture,r,s,texwrapx, texwrapy, a,b,envMap,theta_min,theta_max,mipmap);
  }
  virtual float clipRadialSize();
};
#endif
