#include "unit.h"
#include "images.h"
#include <vector>
#include <algorithm>
#include <vegastrike.h>
#include <universe.h>
#include "config_xml.h"
#include "vs_globals.h"
#include "cmd/ai/flybywire.h"
#include "gfx/cockpit.h"
bool GameUnit::RequestClearance (Unit * dockingunit) {
    static float clearencetime=(XMLSupport::parse_float (vs_config->getVariable ("general","dockingtime","20")));
    EnqueueAIFirst (new ExecuteFor (new Orders::MatchVelocity (Vector(0,0,0),
							       Vector(0,0,0),
							       true,
							       false,
							       true),clearencetime));
    if (std::find (image->clearedunits.begin(),image->clearedunits.end(),dockingunit)==image->clearedunits.end())
      image->clearedunits.push_back (dockingunit);
    return true;
}

void GameUnit::FreeDockingPort (unsigned int i) {
      if (image->dockedunits.size()==1) {
	docked&= (~DOCKING_UNITS);
      }
      unsigned int whichdock =image->dockedunits[i]->whichdock;
      image->dockingports[whichdock].used=false;      
      image->dockedunits[i]->uc.SetUnit(NULL);
      delete image->dockedunits[i];
      image->dockedunits.erase (image->dockedunits.begin()+i);

}
static Transformation HoldPositionWithRespectTo (Transformation holder, const Transformation &changeold, const Transformation &changenew) {
  Quaternion bak = holder.orientation;
  holder.position=holder.position-changeold.position;

  Quaternion invandrest =changeold.orientation.Conjugate();
  invandrest*=  changenew.orientation;
  holder.orientation*=invandrest;
  Matrix m;

  invandrest.to_matrix(m);
  holder.position = TransformNormal (m,holder.position);

  holder.position=holder.position+changenew.position;
  static bool changeddockedorient=(XMLSupport::parse_bool (vs_config->getVariable ("physics","change_docking_orientation","false")));
  if (!changeddockedorient) {
    holder.orientation = bak;
  }
  return holder;
}
void GameUnit::PerformDockingOperations () {
  for (unsigned int i=0;i<image->dockedunits.size();i++) {
    Unit * un;
    if ((un=image->dockedunits[i]->uc.GetUnit())==NULL) {
      FreeDockingPort (i);
      i--;
      continue;
    }
    Transformation t = un->prev_physical_state;
    un->prev_physical_state=un->curr_physical_state;
    un->curr_physical_state =HoldPositionWithRespectTo (un->curr_physical_state,prev_physical_state,curr_physical_state);
    un->NetForce=Vector(0,0,0);
    un->NetLocalForce=Vector(0,0,0);
    un->NetTorque=Vector(0,0,0);
    un->NetLocalTorque=Vector (0,0,0);
    un->AngularVelocity=Vector (0,0,0);
    un->Velocity=Vector (0,0,0);
    if (un==_Universe.AccessCockpit()->GetParent()) {
      ///CHOOSE NEW MISSION
      for (unsigned int i=0;i<image->clearedunits.size();i++) {
	if (image->clearedunits[i]==un) {//this is a hack because we don't have an interface to say "I want to buy a ship"  this does it if you press shift-c in the base
	  image->clearedunits.erase(image->clearedunits.begin()+i);
	  un->UpgradeInterface(this);
	}
      }
    }
    //now we know the unit's still alive... what do we do to him *G*
    ///force him in a box...err where he is
  }
}
bool GameUnit::Dock (Unit * utdw) {
  if (docked&(DOCKED_INSIDE|DOCKED))
    return false;
  std::vector <Unit *>::iterator lookcleared;
  if ((lookcleared = std::find (utdw->image->clearedunits.begin(),
				utdw->image->clearedunits.end(),this))!=utdw->image->clearedunits.end()) {
    int whichdockport;
    if ((whichdockport=utdw->CanDockWithMe(this))!=-1) {
      utdw->image->dockingports[whichdockport].used=true;
      utdw->docked|=DOCKING_UNITS;
      utdw->image->clearedunits.erase (lookcleared);
      utdw->image->dockedunits.push_back (new DockedUnits (this,whichdockport));
      if (utdw->image->dockingports[whichdockport].internal) {
	RemoveFromSystem();	
       	invisible=true;
	docked|=DOCKED_INSIDE;
      }else {
	docked|= DOCKED;
      }
      image->DockedTo.SetUnit (utdw);
      computer.set_speed=0;
      if (this==_Universe.AccessCockpit()->GetParent()) {
	_Universe.AccessCockpit()->RestoreGodliness();
      }
      
      return true;
    }
  }
  return false;
}
