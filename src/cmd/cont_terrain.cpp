#include "cont_terrain.h"
#include "universe.h"
#include "star_system.h"
#include "gfx/matrix.h"
#include "vegastrike.h"
ContinuousTerrain::ContinuousTerrain (const char * filename, const Vector & Scales, const float mass) {
  float tmass;
  FILE *fp = fopen (filename,"r");
  if (fp) {
    fscanf (fp,"%d %f\n<%f %f %f>",&width,&tmass,&this->Scales.i,&this->Scales.j,&this->Scales.k);
    if (mass)
      tmass = mass;
    if (Scales.i&&Scales.j&&Scales.k) {
      this->Scales.i *= Scales.i;
      this->Scales.j *= Scales.j;
      this->Scales.k *= Scales.k;
    }
    numcontterr= width*width;
    data = new Terrain *[numcontterr];
    std::string *filenames = new std::string[numcontterr];
    for (int i=0;i<numcontterr;i++) {
      char tmp[512];
      fscanf (fp,"%511s",tmp);
      tmp[511]='\0';
      filenames[i]=tmp;
    }
    fclose (fp);
    int i;
    
    for (i=0;i<width;i++) {
      for (int j=0;j<width;j++) {
	updateparity * up = &identityparity;
	if (i%2&&j%2) {
	  up = &sideupparityodd;
	} else if (j%2) {
	  up = &sideparityodd;
	} else if (i%2) {
	  up = &upparityodd;
	}
	
	data[i*width+j] = new Terrain (filenames[i*width+j].c_str(),this->Scales,tmass,0,up);
      }
    }
    location = new Vector [numcontterr];
    dirty = new bool [numcontterr];
    delete [] filenames;
  
    sizeX = data[0]->getSizeX();  sizeZ = data[0]->getSizeZ();
    for (i=0;i<numcontterr;i++) {
      if (sizeX!=data[i]->getSizeX()||sizeZ!=data[i]->getSizeZ()) {
	fprintf (stderr,"Warning: Sizes of terrain do not match...expect gaps in continuous terrain\n");
      }
      data[i]->SetTotalSize (sizeX*width,sizeZ*width);
    }
    for (i=0;i<width;i++) {
      for (int j=0;j<width;j++) {
	int nj = j-1<0?width-1:j-1;
	int ni = i-1<0?width-1:i-1;
	data[j+width*i]->SetNeighbors (data[(j+1)%width+width*i],
				       data[j+width*((i+1)%width)],
				       data[nj+width*i],
				       data[j+width*ni]);
	location[j+width*i].Set (0+sizeX*j,0,0-sizeZ*i);
	data[j+width*i]->StaticCullData (25);
      }
    }
    Matrix tmpmat;
    Identity (tmpmat);
    SetTransformation (tmpmat);
    
  } else {
    numcontterr=0;
    width=0;
    dirty= NULL;
    location=NULL;
    data=NULL;
  }

}

ContinuousTerrain::~ContinuousTerrain() {
  for (int i=0;i<numcontterr;i++) {
    delete data[i];
  }
  if (dirty)
    delete []dirty;
  if (location)
    delete []location;
  if (data)
    delete []data;
}
void ContinuousTerrain::SetTransformation(Matrix transformation) {
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
