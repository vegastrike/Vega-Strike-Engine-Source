#include "building_generic.h"

Building::Building (ContinuousTerrain * parent, bool vehicle, const char * filename, bool SubUnit, int faction, const string &modifications, Flightgroup * fg):Unit (filename,SubUnit,faction,modifications,fg) {
  this->vehicle = vehicle;
  continuous=true;
  this->parent.plane = NULL;
}

Building::Building (Terrain * parent, bool vehicle, const char *filename, bool SubUnit, int faction, const string &modifications, Flightgroup * fg):Unit (filename,SubUnit,faction,modifications,fg) {
  this->vehicle = vehicle;
  continuous=false;
  this->parent.terrain = NULL;
}

/********** IGNORE BUILDINGS ON SERVER SIDE FOR NOW *********/
  /*
  Vector p,q,r;
  GetOrientation (p,q,r);
  if (vehicle) {
    Normalize(p);
    //    float k = p.Dot (q);
    Vector tmp1;
#if 0
    if (k<=0) {
      tmp1 = Vector (0,0,1);
      if (k=tmp1.Magnitude()){
	tmp1*=800./k;
      }
    } else 
#endif
      {
      tmp1 = 200*q.Cross (p);
    }
    NetLocalTorque+=((tmp1-tmp1*(tmp1.Dot (GetAngularVelocity())/tmp1.Dot(tmp1))))*1./GetMass();
  }
  */
  // tmp is unknow since I can't include planetary_transform yet
  //SetCurPosition (tmp);
//}
