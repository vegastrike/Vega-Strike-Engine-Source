#include "cont_terrain.h"
#include "universe.h"
#include "star_system.h"
#include "gfx/matrix.h"
#include "vegastrike.h"
ContinuousTerrain::ContinuousTerrain (const char* filenameUL, const char * filenameUR, const char * filenameLL, const char *filenameLR,  const Vector & Scales, const float mass): Scales(Scales) {
  for (int i=0;i<numcontterr;i++) {
    data[i] = new Terrain (*(&filenameUL+i),Scales,mass,0);
  }

  sizeX = data[0]->getSizeX();  sizeZ = data[0]->getSizeZ();
  for (int i=0;i<numcontterr;i++) {
    if (sizeX!=data[i]->getSizeX()||sizeZ!=data[i]->getSizeZ()) {
      fprintf (stderr,"Warning: Sizes of terrain do not match...expect gaps in continuous terrain\n");
    }
  }
  location[0].Set (0,0,0);
  location[1].Set (sizeX,0,0);
  location[2].Set (0,0,-sizeZ);
  location[3].Set (sizeX,0,-sizeZ);
  Matrix tmpmat;
  Identity (tmpmat);
  SetTransform (tmpmat);
}

ContinuousTerrain::~ContinuousTerrain() {
  for (int i=0;i<numcontterr;i++) {
    delete data[i];
  }
}
void ContinuousTerrain::SetTransform(Matrix transformation) {
  CopyMatrix (this->transformation,transformation);
  ScaleMatrix (this->transformation, Scales);
  for (int i=0;i<numcontterr;i++) {
    dirty[i]=true;
  }
  AdjustTerrain();
}

inline bool checkInvScale (float &pos, float campos, float size) {
  bool retval=false;
  size*=2;
  float tmp = pos-campos;
  while (fabs (tmp-size)<fabs (tmp)) {
    tmp -=size;
    retval=true;
  }
  while (fabs (tmp+size) <fabs (tmp)) {
    tmp +=size;
    retval=true;
  }
  if (retval) {
    pos=tmp+campos;
  }
  return retval;
}
void ContinuousTerrain::AdjustTerrain() {
  Matrix transform;

  Vector campos =InvScaleTransform (transformation, _Universe->activeStarSystem()->AccessCamera()->GetPosition());
  for (int i=0;i<numcontterr;i++) {
    dirty[i]|=checkInvScale (location[i].i,campos.i,sizeX);
    dirty[i]|=checkInvScale (location[i].k,campos.k,sizeZ);
    if (dirty[i]) {
      CopyMatrix (transform,transformation);
      Vector tmp  (Transform (transformation,location[i]-Vector ((data[i])->getminX()+.5*(data[i])->getSizeX(),0,(data[i])->getminZ()+.5*(data[i])->getSizeZ())));
      transform[12]=tmp.i;
      transform[13]=tmp.j;
      transform[14]=tmp.k;
      (data[i])->SetTransformation (transform);
      dirty[i]=false;
    }
  }
}
