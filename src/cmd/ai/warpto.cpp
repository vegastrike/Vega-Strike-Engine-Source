#include "cmd/unit_generic.h"
#include "cmd/unit_util.h"
#include "universe_util.h"
#include "config_xml.h"
float max_allowable_travel_time () {
  static float mat = XMLSupport::parse_float (vs_config->getVariable ("AI","max_allowable_travel_time","1000"));
  return mat;
}
bool DistanceWarrantsWarpTo (Unit * parent, float dist){
  //first let us decide whether the target is far enough to warrant using warp
  //  double dist =UnitUtil::getSignificantDistance(parent,target);
  float timetolive = dist/parent->GetComputerData().max_combat_ab_speed;
  if (timetolive>max_allowable_travel_time()) {
    return true;
  }
  return false;
}
bool DistanceWarrantsTravelTo (Unit * parent, float dist){
  //first let us decide whether the target is far enough to warrant using warp
  //  double dist =UnitUtil::getSignificantDistance(parent,target);
  float timetolive = dist/parent->GetComputerData().max_combat_speed;
  if (timetolive>max_allowable_travel_time()) {
    return true;
  }
  return false;
}
bool TargetWorthPursuing (Unit * parent, Unit * target) {
  Unit* un;
  for (un_iter i=_Universe->activeStarSystem()->getUnitList().createIterator();(un=*i)!=NULL;++i) {
    /// DO SOMETHING
  }
  return true;
}
void WarpToP(Unit * parent, Unit * target) {
  float dist =UnitUtil::getSignificantDistance(parent,target);
  if (DistanceWarrantsWarpTo (parent,dist)) {
    static float insys_jump_cost = XMLSupport::parse_float (vs_config->getVariable ("physics","insystem_jump_cost",".1"));
	if(parent->GetWarpEnergy()>parent->GetJumpStatus().insysenergy) {
		if (TargetWorthPursuing(parent,target)){
			parent->AutoPilotTo(target,false);
		}
    }
  }
}
