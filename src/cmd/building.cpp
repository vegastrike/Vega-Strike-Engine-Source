#include "building.h"
#include "cont_terrain.h"

Building::Building (ContinuousTerrain * parent, bool vehicle, const char * filename, bool xml, bool SubUnit, int faction, Flightgroup * fg):Unit (filename,xml,SubUnit,faction,fg) {
  this->vehicle = vehicle;
  continuous=true;
  this->parent.plane = parent;
}

Building::Building (Terrain * parent, bool vehicle, const char *filename, bool xml, bool SubUnit, int faction, Flightgroup * fg):Unit (filename,xml,SubUnit,faction,fg) {
  this->vehicle = vehicle;
  continuous=false;
  this->parent.terrain = parent;
}



void Building::UpdatePhysics (const Transformation &trans, const Matrix transmat, const Vector & cum_vel,  bool lastframe, UnitCollection *uc){
  Unit::UpdatePhysics (trans,transmat,cum_vel,lastframe,uc);
  Vector tmp (LocalPosition());
  Vector p,q,r;
  GetOrientation (p,q,r);
  if (continuous) {
    tmp = parent.plane->GetGroundPos (tmp,p);
  } else {
    parent.terrain->GetGroundPos (tmp,p,0,0);

  }
  if (vehicle) {
    Normalize(p);
    Vector tmp1 = 200*p.Cross (q);
    
    NetLocalTorque+=((tmp1-tmp1*(tmp1.Dot (GetAngularVelocity())/tmp1.Dot(tmp1))))*1./GetMass();
  }
  SetCurPosition (tmp);

}
