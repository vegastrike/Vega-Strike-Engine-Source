//#include "unit.h"
#include "unit_factory.h"
#include "beam.h"
#include "bolt.h"
#include "gfx/lerp.h"
#include "vs_globals.h"
#include "audiolib.h"
#include "vegastrike.h"
#include <string>
#include "ai/script.h"
#include "ai/navigation.h"
#include "ai/flybywire.h"
#include "images.h"
#include "missile.h"
#include "cmd/ai/fire.h"
#include "gfx/cockpit.h"
#include "config_xml.h"
#include "force_feedback.h"
#include "role_bitmask.h"
extern unsigned short apply_float_to_short (float tmp);

extern void AdjustMatrix (Matrix &mat, Unit * target, float speed, bool lead, float cone);

template <class UnitType>
void GameUnit<UnitType>::Fire (unsigned int weapon_type_bitmask,bool follow_target) {//FIXME FIRE
  UnitType::Fire(weapon_type_bitmask,follow_target);
}

template <class UnitType>
void GameUnit<UnitType>::TargetTurret (Unit * targ) {
	if (!SubUnits.empty()) {
		un_iter iter = getSubUnits();
		Unit * su;
		bool inrange = (targ!=NULL)?InRange(targ):true;
                if (inrange) {
                while ((su=iter.current())) {
			su->Target (targ);
			su->TargetTurret(targ);
			iter.advance();
		}
                }
                

	}

}
template <class UnitType>
void GameUnit<UnitType>::Target (Unit *targ) {
  if (targ==this) {
    return;
  }
  if (!(activeStarSystem==NULL||activeStarSystem==_Universe->activeStarSystem())) {
    computer.target.SetUnit(NULL);
    return;
    fprintf (stderr,"bad target system");
    const int BADTARGETSYSTEM=0;
    assert (BADTARGETSYSTEM);
  }
  if (targ) {
    if (targ->activeStarSystem==_Universe->activeStarSystem()||targ->activeStarSystem==NULL) {
		if (targ!=Unit::Target()) {
        for (int i=0;i<GetNumMounts();i++){ 
  	  mounts[i]->time_to_lock = mounts[i]->type->LockTime;
        }
        computer.target.SetUnit(targ);
	LockTarget(false);
      }
    }else {
      if (jump.drive!=-1) {
	un_iter i= _Universe->activeStarSystem()->getUnitList().createIterator();
	Unit * u;
	for (;(u=*i)!=NULL;i++) {
	  if (!u->GetDestinations().empty()) {
	    if (std::find (u->GetDestinations().begin(),u->GetDestinations().end(),targ->activeStarSystem->getFileName())!=u->GetDestinations().end()) {
	      Target (u);
	      ActivateJumpDrive(0);
	    }
	  }
	}
      }else {
	computer.target.SetUnit(NULL);
      }
    }
  }else {
    computer.target.SetUnit(NULL);
  }
}
