#include "docking.h"
#include "xml_support.h"
#include "config_xml.h"
#include "cmd/unit_generic.h"
#include "warpto.h"
#include "universe_util.h"
#include "python/python_compile.h"
static void DockedScript(Unit * docker, Unit * base) {
  static string script = vs_config->getVariable("AI","DockedToScript","");
  if (script.length()>0) {
    Unit * targ = docker->Target();
    docker->GetComputerData().target.SetUnit(base);
    UniverseUtil::setScratchUnit(docker);
    CompileRunPython(script);
    UniverseUtil::setScratchUnit(NULL);
    docker->GetComputerData().target.SetUnit(targ);//should be NULL;
  }
}
namespace Orders {
  DockingOps::DockingOps (Unit * unitToDockWith, Order * ai,bool physical_docking): MoveTo (QVector (0,0,1),
								      false,
								      10,false),
							      
							      docking(unitToDockWith),
							      state(GETCLEARENCE),
							      oldstate(ai){
    formerOwnerDoNotDereference = NULL;
    facedtarget=false;
    physicallyDock=true;//physical_docking;
    port=-1;
    static float temptimer=XMLSupport::parse_float (vs_config->getVariable ("physics","docking_time","10"));
    timer = temptimer;
    
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
    if (physicallyDock&&!utdw->RequestClearance(parent))
      return false;
    port =SelectDockPort (utdw, parent);
    if (port == -1) {
      return false;
    }
    return true;
  }
  QVector DockingOps::Movement(Unit * utdw) {
    const QVector loc (Transform (utdw->GetTransformation(),utdw->DockingPortLocations()[port].pos.Cast()));
    SetDest (loc);
    
    SetAfterburn (DistanceWarrantsTravelTo(parent,(loc-parent->Position()).Magnitude(),true));
    if (!facedtarget) {
      facedtarget=true;
      EnqueueOrder(new ChangeHeading(loc,4,1,true));
    }
    MoveTo::Execute();
    if (rand()%256==0){
      WarpToP(parent,utdw,true);     
    }
    return loc;
  }
  bool DockingOps::DockToTarget(Unit * utdw) {
    if (utdw->DockingPortLocations()[port].used) {
      state = GETCLEARENCE;
      return false; 
    }
    QVector loc=Movement(utdw);
    float rad = utdw->DockingPortLocations()[port].radius+parent->rSize();
    float diss =(parent->Position()-loc).MagnitudeSquared()-.1;
    bool isplanet=utdw->isUnit()==PLANETPTR;
    if (diss<=(isplanet?rad*rad:parent->rSize()*parent->rSize())) {
      DockedScript(parent,utdw);      
      if (physicallyDock)
        return parent->Dock(utdw);
      else{
        parent->RefillWarpEnergy();
        return true;
      }
    }else {
      if (diss <=1.2*rad*rad) {
	timer+=SIMULATION_ATOM;
	static float tmp=XMLSupport::parse_float (vs_config->getVariable ("physics","docking_time","10"));
	if (timer>=1.5*tmp) {
          if (physicallyDock)
            return parent->Dock(utdw);
          else {
            parent->RefillWarpEnergy();
            return true;
          }
	}
      }
    }
    return false;
  }
  bool DockingOps::PerformDockingOperations(Unit * utdw) {
    timer-=SIMULATION_ATOM;
    bool isplanet=utdw->isUnit()==PLANETPTR;
    if (timer<0){ 
      static float tmp=XMLSupport::parse_float (vs_config->getVariable ("physics","un_docking_time","180"));
      timer=tmp;
      EnqueueOrder(new ChangeHeading(parent->Position()*2-utdw->Position(),4,1,true));
      if (physicallyDock)
        return parent->UnDock (utdw);
      else 
        return true;
    }else {
      if (!physicallyDock) {
        if (isplanet) {
          //orbit;
          QVector cur=utdw->Position()-parent->Position();
          QVector up = QVector(0,1,0);
          if (up.i==cur.i&&up.j==cur.j&&up.k==cur.k) {
            up=QVector(0,0,1);
          }
          SetDest(cur.Cross(up)*10000);
          MoveTo::Execute();        
        }else {
          Movement(utdw);
        }
      }
    }
    return false;
  }
  bool DockingOps::Undock(Unit * utdw) {//this is a good heuristic... find the location where you are.compare with center...then fly the fuck away
    QVector awaydir = parent->Position()-utdw->Position();
    float len = ((utdw->rSize()+parent->rSize()*2)/awaydir.Magnitude());
    awaydir *= len;
    SetDest(awaydir+utdw->Position());
    MoveTo::Execute();
    timer-=SIMULATION_ATOM;
    return (len<1)||done||timer<0;
  }
}
