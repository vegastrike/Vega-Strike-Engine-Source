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
inline bool insideDock (const DockingPorts &dock, const Vector & pos, float radius) {
  if (dock.used)
    return false;
  double rad=dock.radius+radius;
  return (pos-dock.pos).MagnitudeSquared()<rad*rad;
  if (dock.internal) {
    if ((pos.i+radius<dock.max.i)&&
	(pos.j+radius<dock.max.j)&&
	(pos.k+radius<dock.max.k)&&
	(pos.i-radius>dock.min.i)&&
	(pos.j-radius>dock.min.j)&&
	(pos.k-radius>dock.min.k)) {
      return true;
    }    
  }else {
    if ((pos-dock.pos).Magnitude()<dock.radius+radius&&
	(pos.i-radius<dock.max.i)&&
	(pos.j-radius<dock.max.j)&&
	(pos.k-radius<dock.max.k)&&
	(pos.i+radius>dock.min.i)&&
	(pos.j+radius>dock.min.j)&&
	(pos.k+radius>dock.min.k)) {
      return true;
    }
  }
  return false;
}
bool Unit::RequestClearance (Unit * dockingunit) {
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

int Unit::CanDockWithMe(Unit * un) {
  //  if (_Universe->GetRelation(faction,un->faction)>=0) {//already clearneed
    for (unsigned int i=0;i<image->dockingports.size();i++) {
      if (un->image->dockingports.size()) {
	for (unsigned int j=0;j<un->image->dockingports.size();j++) {
	  if (insideDock (image->dockingports[i],InvTransform (cumulative_transformation_matrix,Transform (un->cumulative_transformation_matrix,un->image->dockingports[j].pos)),un->image->dockingports[j].radius)) {
	    if (((un->docked&(DOCKED_INSIDE|DOCKED))==0)&&(!(docked&DOCKED_INSIDE))) {
	      return i;
	    }
	  }
	}  
      }else {
	if (insideDock (image->dockingports[i],InvTransform (cumulative_transformation_matrix,un->Position().Cast()),un->rSize())) {
	  return i;
	}
      }
    }
    //  }
  return -1;
}
void Unit::FreeDockingPort (unsigned int i) {
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
  holder.position=holder.position-changeold.position;

  Quaternion invandrest =changeold.orientation.Conjugate();
  invandrest*=  changenew.orientation;
  holder.orientation*=invandrest;
  Matrix m;

  invandrest.to_matrix(m);
  holder.position = TransformNormal (m,holder.position);

  holder.position=holder.position+changenew.position;
  return holder;
}
const vector <DockingPorts>& Unit::DockingPortLocations() {
  return image->dockingports;
}

bool Unit::IsCleared (Unit * DockingUnit) {
  return (std::find (image->clearedunits.begin(),image->clearedunits.end(),DockingUnit)!=image->clearedunits.end());
}
void Unit::PerformDockingOperations () {
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
    if (un==_Universe->AccessCockpit()->GetParent()) {
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
bool Unit::isDocked (Unit* d) {
  if (!(d->docked&DOCKED_INSIDE|DOCKED)) {
    return false;
  }
  for (unsigned int i=0;i<image->dockedunits.size();i++) {
    Unit * un;
    if ((un=image->dockedunits[i]->uc.GetUnit())!=NULL) {
      if (un==d) {
	return true;
      }
    }
  }
  return false;
}
bool Unit::Dock (Unit * utdw) {
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
      if (this==_Universe->AccessCockpit()->GetParent()) {
	_Universe->AccessCockpit()->RestoreGodliness();
      }
      
      return true;
    }
  }
  return false;
}
bool Unit::UnDock (Unit * utdw) {
  unsigned int i=0;

  for (i=0;i<utdw->image->dockedunits.size();i++) {
    if (utdw->image->dockedunits[i]->uc.GetUnit()==this) {
      utdw->FreeDockingPort (i);
      i--;
      invisible=false;
      docked&=(~(DOCKED_INSIDE|DOCKED));
      image->DockedTo.SetUnit (NULL);
      Velocity=utdw->Velocity;
      return true;
    }
  }
  return false;
}
