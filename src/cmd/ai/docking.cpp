#include "docking.h"
#include "xml_support.h"
#include "config_xml.h"
#include "cmd/unit_generic.h"
#include "warpto.h"
namespace Orders {
  DockingOps::DockingOps (Unit * unitToDockWith, Order * ai): MoveTo (QVector (0,0,1),
								      false,
								      10,false),
							      
							      docking(unitToDockWith),
							      state(GETCLEARENCE),
							      oldstate(ai){
    formerOwnerDoNotDereference = NULL;
    port=-1;
    timer = XMLSupport::parse_float (vs_config->getVariable ("physics","docking_time","10"));
    
  }
  void DockingOps::SetParent (Unit * par) {
    MoveTo::SetParent (par);
    if (parent) {
      formerOwnerDoNotDereference = parent->owner;
      parent->SetOwner (docking.GetUnit());
    }
  }
  void DockingOps::Execute () {
    Unit * utdw = docking.GetUnit();
    if (!utdw) {
      RestoreOldAI();
      Destroy();
      return;
    }
    switch (state) {
    case GETCLEARENCE:
      if (!RequestClearence(utdw)) {
	RestoreOldAI();
	Destroy();
	return;
      }
      state = DOCKING;
      //no break
    case DOCKING:
      if (DockToTarget(utdw))
	state= DOCKED;
      break;
    case DOCKED:
      if (PerformDockingOperations(utdw))
	state = UNDOCKING;
      break;
    case UNDOCKING:
      if (Undock(utdw)) {
	RestoreOldAI();
	Destroy();
	return;
      }
      break;
    }
    parent->SetAngularVelocity(Vector(0,0,0));//FIXME if you want it to turn to dock point
    done= false;
  }
  void DockingOps::Destroy() {
    if (parent) {
      if (oldstate)
	oldstate->Destroy();
      oldstate=NULL;
      if (formerOwnerDoNotDereference) {
	parent->SetOwner(formerOwnerDoNotDereference);
	formerOwnerDoNotDereference=NULL;
      }

    }
    docking.SetUnit(NULL);    
  }
  void DockingOps::RestoreOldAI() {
    if (parent) {
      parent->aistate = oldstate;//that's me!
      if (formerOwnerDoNotDereference) {
	parent->SetOwner(formerOwnerDoNotDereference);
	formerOwnerDoNotDereference=NULL;
      }
      oldstate=NULL;
    }
  }
  int SelectDockPort (Unit * utdw, Unit * parent) {
    const vector <DockingPorts> * dp = &utdw->DockingPortLocations();
    float dist = FLT_MAX;
    int num=-1;
    for (unsigned int i=0;i<dp->size();++i) {
      if (!(*dp)[i].used) {
	Vector rez = Transform (utdw->GetTransformation(),(*dp)[i].pos);
	float wdist = (rez-parent->Position()).MagnitudeSquared();
	if (wdist <dist) {
	  num=i;
	  dist =wdist;
	}
      }

    }
    return num;
  }
  bool DockingOps::RequestClearence (Unit * utdw) {
    if (!utdw->RequestClearance(parent))
      return false;
    port =SelectDockPort (utdw, parent);
    if (port == -1) {
      return false;
    }
    return true;
  }
  bool DockingOps::DockToTarget(Unit * utdw) {
    if (utdw->DockingPortLocations()[port].used) {
      state = GETCLEARENCE;
      return false; 
    }
    const QVector loc (Transform (utdw->GetTransformation(),utdw->DockingPortLocations()[port].pos.Cast()));
    SetDest (loc);
    SetAfterburn (DistanceWarrantsTravelTo(parent,(loc-parent->Position()).Magnitude()));
    
    MoveTo::Execute();
    if (rand()%256==0){
      WarpToP(parent,utdw);
      
    }
    float rad = utdw->DockingPortLocations()[port].radius+parent->rSize();
    float diss =(parent->Position()-loc).MagnitudeSquared()-.1;
    if (diss<=parent->rSize()*parent->rSize()) {
      return parent->Dock(utdw);
    }else {
      if (diss <=1.2*rad*rad) {
	timer+=SIMULATION_ATOM;
	static float tmp=XMLSupport::parse_float (vs_config->getVariable ("physics","docking_time","10"));
	if (timer>=1.5*tmp) {
	  return parent->Dock(utdw);
	}
      }
    }
    return false;
  }
  bool DockingOps::PerformDockingOperations(Unit * utdw) {
    timer-=SIMULATION_ATOM;
    if (timer<0)
      return parent->UnDock (utdw);
    return false;
  }
  bool DockingOps::Undock(Unit * utdw) {//this is a good heuristic... find the location where you are.compare with center...then fly the fuck away
    QVector awaydir = parent->Position()-utdw->Position();
    float len = ((utdw->rSize()+parent->rSize()*2)/awaydir.Magnitude());
    awaydir *= len;
    SetDest(awaydir+utdw->Position());
    MoveTo::Execute();
    return (len<1)||done;
  }
}
