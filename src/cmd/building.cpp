#include "building.h"
#include "cont_terrain.h"
#include "gfx/planetary_transform.h"
GameBuilding::GameBuilding (ContinuousTerrain * parent, bool vehicle, const char * filename, bool SubUnit, int faction, const string &modifications, Flightgroup * fg):GameUnit<Building> (filename,SubUnit,faction,modifications,fg) {
  this->vehicle = vehicle;
  continuous=true;
  this->parent.plane = parent;
}

GameBuilding::GameBuilding (Terrain * parent, bool vehicle, const char *filename, bool SubUnit, int faction, const string &modifications, Flightgroup * fg):GameUnit<Building> (filename,SubUnit,faction,modifications,fg) {
  this->vehicle = vehicle;
  continuous=false;
  this->parent.terrain = parent;
}

void GameBuilding::UpdatePhysics2 (const Transformation &trans, const Transformation & old_physical_state, const Vector & accel, float difficulty, const Matrix &transmat, const Vector & CumulativeVelocity, bool ResolveLast, UnitCollection *uc) {
  if (GetPlanetOrbit()) {
    SetPlanetOrbitData(GetPlanetOrbit());//makes it dirty
  }
  GameUnit<Building>::UpdatePhysics2 (trans,old_physical_state,accel,difficulty,transmat,CumulativeVelocity,ResolveLast,uc);
  QVector tmp (LocalPosition());
  Vector p,q,r;
  GetOrientation (p,q,r);
  if (continuous) {
    if (!GetPlanetOrbit()) {
      tmp = parent.plane->GetGroundPos (tmp,p);
    }else {
      tmp = GetPlanetOrbit()->Transform(parent.plane->GetGroundPosIdentTrans (GetPlanetOrbit()->InvTransform(tmp),p));
    }
  } else {
    parent.terrain->GetGroundPos (tmp,p,(float)0,(float)0);

  }
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
  SetCurPosition (tmp);

}
