#include "xml_support.h"
#include "cmd/unit_generic.h"
#include "mission.h"

void Mission::call_unit_toxml(missionNode *node,int mode,varInst *ovi){
  Unit *my_object=getUnitObject(node,mode,ovi);

  if (my_object) {
   string fgid=my_object->getFgID();
   Flightgroup *fg=my_object->getFlightgroup();

  

  }
}

Unit *Mission::getUnitObject(missionNode *node,int mode,varInst *ovi){
  Unit *my_object=NULL;

	if(mode==SCRIPT_RUN){
	  my_object=(Unit *)ovi->object;
#if 0
	  if(my_object==NULL){
	    fatalError(node,mode,"unit: no object");
	    assert(0);
	  }
#endif
	}

	return(my_object);
}

