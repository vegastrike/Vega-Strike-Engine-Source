#include "cmd/unit_generic.h"
#include "cmd/unit_util.h"
#include "universe_util.h"
#include "config_xml.h"
float max_allowable_travel_time () {
  static float mat = XMLSupport::parse_float (vs_config->getVariable ("AI","max_allowable_travel_time","15"));
  return mat;
}
bool DistanceWarrantsWarpTo (Unit * parent, float dist){
  //first let us decide whether the target is far enough to warrant using warp
  //  double dist =UnitUtil::getSignificantDistance(parent,target);
  float diff = g_game.difficulty;
  if (diff>1)diff=1;
  float timetolive = dist/(diff*parent->GetComputerData().max_combat_speed);
  if (timetolive>max_allowable_travel_time()) {
    return true;
  }
  return false;
}
bool DistanceWarrantsTravelTo (Unit * parent, float dist){
  //first let us decide whether the target is far enough to warrant using warp
  //  double dist =UnitUtil::getSignificantDistance(parent,target);
  float diff = g_game.difficulty;
  if (diff>1)diff=1;
  float timetolive = dist/(diff*parent->GetComputerData().max_combat_speed);
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
	 if (TargetWorthPursuing(parent,target)){
		    static bool auto_valid = XMLSupport::parse_bool (vs_config->getVariable ("physics","insystem_jump_or_timeless_auto-pilot","false"));	
			if(auto_valid){
			  parent->AutoPilotTo(target,false);
			} else {
				Vector vel = parent->GetVelocity();
				Vector tarvel=target->GetVelocity();
				tarvel.Normalize();
				vel.Normalize();
				Vector dir = target->Position()-parent->Position();
				dir.Normalize();
				float dirveldot=dir.Dot(vel);
				dir*=-1;
				float chasedot=dir.Dot(tarvel);
				if(dirveldot>.80){
					parent->graphicOptions.InWarp=1;
				} else {
					parent->graphicOptions.InWarp=0;
				}
				if(chasedot>.80){
					parent->computer.velocity_ref.SetUnit(NULL);
				} else {
					parent->computer.velocity_ref.SetUnit(target);
				}
			}
		}
  } else {
	parent->graphicOptions.InWarp=0;
  }
}
