#include "mission.h"


varInst * Mission::call_briefing (missionNode * node, int mode) {
  varInst *viret=NULL;
  
  trace(node,mode);
  
  if(mode==SCRIPT_PARSE){
    string cmd=node->attr_value("name");
    node->script.method_id=module_unit_map[cmd];
  }
  viret=newVarInst(VI_TEMP);
  callback_module_briefing_type method_id=(callback_module_unit_type) node->script.method_id;
  if (briefing!=NULL) {
    if(method_id==CMT_BRIEFING_addShip){
      std::string name = getStringArgument(node,mode,0);
      std::string faction= getStringArgument (node,mode,1);
      float x = getFloatArg(node,mode,2);
      float y = getFloatArg(node,mode,3);
      float z = getFloatArg(node,mode,4);
      if (mode==SCRIPT_RUN) {
	viret->int_val=briefing->AddStarship(name.c_str(),
					     _Universe->GetFaction(faction.c_str()),
					     Vector(x,y,z));
      }
      viret->type = VAR_INT;
    }else if (method_id==CMT_BRIEFING_removeShip) {
      
    }else if (method_id==CMT_BRIEFING_enqueueOrder){
      
    }else if (method_id==CMT_BRIEFING_replaceOrder) {
      
    }else if (method_id==CMT_BRIEFING_getShipPosition) {
      
    }else if (method_id==CMT_BRIEFING_setShipPosition) {
      
    }else if (method_id==CMT_BRIEFING_terminate) {
      
    }
  }

}
