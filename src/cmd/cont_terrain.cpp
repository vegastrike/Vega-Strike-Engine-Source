#include "cont_terrain.h"
#include "universe.h"
#include "star_system.h"
#include "gfx/matrix.h"
#include "vegastrike.h"
ContinuousTerrain::ContinuousTerrain (char ** filenames, const int numwidth, const Vector & Scales, const float mass): Scales(Scales), width(numwidth) {
  numcontterr= numwidth*numwidth;
  int i;
  data = new Terrain *[numcontterr];
  for (i=0;i<numcontterr;i++) {
    data[i] = new Terrain (filenames[i],Scales,mass,0);
  }
  location = new Vector [numcontterr];
  dirty = new bool [numcontterr];

  
  sizeX = data[0]->getSizeX();  sizeZ = data[0]->getSizeZ();
  for (i=0;i<numcontterr;i++) {
    if (sizeX!=data[i]->getSizeX()||sizeZ!=data[i]->getSizeZ()) {
      fprintf (stderr,"Warning: Sizes of terrain do not match...expect gaps in continuous terrain\n");
    }
  }
  for (i=0;i<width;i++) {
    for (int j=0;j<width;j++) {
      location[j+width*i].Set (0+sizeX*j,0,0-sizeZ*i);

    }
  }
  Matrix tmpmat;
  Identity (tmpmat);
  SetTransform (tmpmat);
}

ContinuousTerrain::~ContinuousTerrain() {
  for (int i=0;i<numcontterr;i++) {
    delete data[i];
  }
  delete []dirty;
  delete []location;
  delete []data;
}
void ContinuousTerrain::SetTransform(Matrix transformation) {
  CopyMatrix (this->transformation,transformation);
  ScaleMatrix (this->transformation, Scales);
  for (int i=0;i<numcontterr;i++) {
    dirty[i]=true;
  }
  AdjustTerrain();
}

bool ContinuousTerrain::checkInvScale (float &pos, float campos, float size) {
  bool retval=false;
  size*=width;
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
