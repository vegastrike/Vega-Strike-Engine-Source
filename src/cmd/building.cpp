#include "building.h"
#include "cont_terrain.h"

Building::Building (ContinuousTerrain * parent, const char * filename, bool xml, bool SubUnit, int faction, Flightgroup * fg):Unit (filename,xml,SubUnit,faction,fg) {
  continuous=true;
  this->parent.plane = parent;
}

Building::Building (Terrain * parent, const char *filename, bool xml, bool SubUnit, int faction, Flightgroup * fg):Unit (filename,xml,SubUnit,faction,fg) {
  continuous=false;
  this->parent.terrain = parent;
}



void Building::UpdatePhysics (const Transformation &trans, const Matrix transmat, const Vector & cum_vel,  bool lastframe, UnitCollection *uc){
  Unit::UpdatePhysics (trans,transmat,cum_vel,lastframe,uc);
  Vector tmp (LocalPosition());
  Vector p,q,r;
  GetOrientation (p,q,r);
  if (continuous) {
    tmp = parent.plane->GetGroundPos (tmp,q);
  } else {
    parent.terrain->GetGroundPos (tmp,q,0,0);

  }

  SetCurPosition (tmp);
  /*
  r = r- (q*(r.Dot (q)));
  Normalize(r);
  p = q.Cross (r);
  ;
  curr_physical_state.orientation = Quaternion::from_vectors (-p,q,r);
  */

}
