#include "unit.h"
#include "images.h"
#include <vector>
#include <algorithm>
#include <vegastrike.h>
#include <universe.h>
#include "config_xml.h"
#include "vs_globals.h"
#include "cmd/ai/flybywire.h"
inline bool insideDock (const DockingPorts &dock, const Vector & pos, float radius) {
  if (dock.used)
    return false;
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
  if (_Universe->GetRelation(faction,dockingunit->faction)>.5) {
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
  return false;
}

int Unit::CanDockWithMe(Unit * un) {
  if (_Universe->GetRelation(faction,un->faction)>.5) {
    for (unsigned int i=0;i<image->dockingports.size();i++) {
      if (un->image->dockingports.size()) {
	for (unsigned int j=0;j<un->image->dockingports.size();j++) {
	  if (insideDock (image->dockingports[i],InvTransform (cumulative_transformation_matrix,Transform (un->cumulative_transformation_matrix,un->image->dockingports[j].pos)),un->image->dockingports[j].radius)) {
	    if ((un->docked==NOT_DOCKED)&&docked!=DOCKED_INSIDE) {
	      return i;
	    }
	  }
	}  
      }else {
	if (insideDock (image->dockingports[i],InvTransform (cumulative_transformation_matrix,un->Position()),un->rSize())) {
	  return i;
	}
      }
    }
  }
  return -1;
}
void Unit::PerformDockingOptions (Unit * dockedunit) {
  for (unsigned int i=0;i<image->dockedunits.size();i++) {
    if (image->dockedunits[i]->uc.GetUnit()==NULL) {
      image->dockedunits.erase (image->dockedunits.begin()+i);
      i--;
      continue;
    }
    //now we know the unit's still alive... what do we do to him *G*
  }
}

bool Unit::Dock (Unit * utdw) {
  if (docked!=NOT_DOCKED)
    return false;
  std::vector <Unit *>::iterator lookcleared;
  if ((lookcleared = std::find (utdw->image->clearedunits.begin(),
				utdw->image->clearedunits.end(),this))!=utdw->image->clearedunits.end()) {
    int whichdockport;
    if ((whichdockport=utdw->CanDockWithMe(this))!=-1) {
      utdw->image->dockingports[whichdockport].used=true;
      utdw->image->clearedunits.erase (lookcleared);
      utdw->image->dockedunits.push_back (new DockedUnits (this,whichdockport));
      if (utdw->image->dockingports[whichdockport].internal) {
	RemoveFromSystem();	
	invisible=true;
	docked=DOCKED_INSIDE;
      }else {
	docked = DOCKED;
      }
      image->DockedTo.SetUnit (utdw);
      return true;
    }
  }
  return false;
}
bool Unit::UnDock (Unit * utdw) {
  unsigned int i=0;
  for (i=0;utdw->image->dockedunits.size();i++) {
    if (utdw->image->dockedunits[i]->uc.GetUnit()==this) {
      unsigned int whichdock =utdw->image->dockedunits[i]->whichdock;
      utdw->image->dockingports[whichdock].used=false;
      utdw->image->dockedunits[i]->uc.SetUnit(NULL);
      delete utdw->image->dockedunits[i];
      utdw->image->dockedunits.erase (utdw->image->dockedunits.begin()+i);
      i--;
      invisible=false;
      docked=NOT_DOCKED;
      image->DockedTo.SetUnit (NULL);
      return true;
    }
  }
  return false;
}
