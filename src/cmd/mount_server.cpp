#include "unit_generic.h"

Mount::Mount(const string& filename, short am,short vol): size(weapon_info::NOWEAP),ammo(am),sound(-1){
  static weapon_info wi(weapon_info::BEAM);
  type = &wi;
  this->volume=vol;
  ref.gun = NULL;
  status=(UNCHOSEN);
  processed=Mount::PROCESSED;
  /*
  weapon_info * temp = getTemplate (filename);  
  if (temp==NULL) {
    status=UNCHOSEN;
    time_to_lock=0;
  }else {
    type = temp;
    status=ACTIVE;
    time_to_lock = temp->LockTime;
  }
  */
}

void Mount::ReplaceSound () {
}

void Mount::PhysicsAlignedUnfire() {}
bool Mount::PhysicsAlignedFire (const Transformation &Cumulative, const Matrix & mat, const Vector & Velocity, Unit *owner,  Unit *target, signed char autotrack, float trackingcone) { return false;}
bool Mount::Fire (Unit *owner, bool Missile) {return false;}
void Mount::UnFire() {}
