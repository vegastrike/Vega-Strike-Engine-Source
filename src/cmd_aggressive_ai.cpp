#include "cmd_aggressive_ai.h"
#include "cmd_ai_event_xml.h"
#include "cmd_aiscript.h"
#include "physics.h"
#include <list>
#include <vector>
using namespace Orders;

const EnumMap::Pair element_names[] = {
  EnumMap::Pair ("AggressiveAI" , AggressiveAI::AGGAI),
  EnumMap::Pair ("UNKNOWN", AggressiveAI::UNKNOWN),
  EnumMap::Pair ("Distance", AggressiveAI::DISTANCE),
  EnumMap::Pair ("Threat", AggressiveAI::THREAT),
  EnumMap::Pair ("FShield", AggressiveAI::FSHIELD),
  EnumMap::Pair ("LShield",AggressiveAI:: LSHIELD),
  EnumMap::Pair ("RShield", AggressiveAI::RSHIELD),
  EnumMap::Pair ("BShield", AggressiveAI::BSHIELD),
  EnumMap::Pair ("Hull", AggressiveAI::HULL),
  EnumMap::Pair ("Facing", AggressiveAI::FACING),
  EnumMap::Pair ("Movement", AggressiveAI::MOVEMENT)
};
const EnumMap AggressiveAIel_map(element_names, 11);

AggressiveAI::AggressiveAI (const char * filename, Unit * target=NULL):FireAt(.2,6,false), logic (AggressiveAIel_map), count (rand()%10) {
  
  if (target !=NULL) {
    UnitCollection tmp;
    tmp.prepend (target);
    AttachOrder (&tmp);
  }
  AIEvents::LoadAI (filename,logic);
}

void AggressiveAI::ExecuteLogicItem (const AIEvents::AIEvresult &item, bool force) {
  Order * tmp = new AIScript (item.script.c_str());	
  if (force)
    parent->SetAI (tmp);
  else
    parent->EnqueueAI (tmp);
}

bool AggressiveAI::ProcessLogicItem (const AIEvents::AIEvresult &item) {
  float value;
  switch (abs(item.type)) {
  case DISTANCE:
    value = distance;
    break;
  case THREAT:
    value = parent->GetComputerData().threatlevel;
    break;
  case FSHIELD:
    value = parent->FShieldData();
    break;
  case BSHIELD:
    value = parent->BShieldData();
    break;
  case HULL:
    value = parent->GetHull();
    break;
  case LSHIELD:
    value = parent->LShieldData();
    break;
  case RSHIELD:
    value = parent->RShieldData();
    break;
  case FACING:
    return parent->getAIState()->queryType (FACING)==NULL;
  case MOVEMENT:
    return parent->getAIState()->queryType (MOVEMENT)==NULL;
  case AGGAI:
  case UNKNOWN:
    return false;
  }
  return item.Eval(value);
}

void AggressiveAI::ProcessLogic (bool force) {
  //go through the logic. 
  Unit * tmp = parent->Target();
  distance = tmp? (tmp->Position()-parent->Position()).Magnitude() : FLT_MAX;
  std::vector <std::list <AIEvents::AIEvresult> >::iterator i = logic.result.begin();
  for (;i!=logic.result.end();i++) {
    std::list <AIEvents::AIEvresult>::iterator j;
    for (j= i->begin();j!=i->end();j++) {
      if (!ProcessLogicItem(*j)) {
	break;
      }
    }
    if (j==i->end()&&(!i->empty())) {
      //do it
      ExecuteLogicItem (i->front(),force);
      AIEvents::AIEvresult tmp = i->front();
      i->erase(i->begin());
      i->push_back (tmp);
    }
  }
}


void AggressiveAI::Execute () {  
  const int maxcount=10;//num sec before it rechecks AI	
  FireAt::Execute();
  if (parent->getAIState()->queryType (FACING)==NULL||parent->getAIState()->queryType (MOVEMENT)==NULL) {
    ProcessLogic(false);
  } else {
    if ( (--count)==0) {
      ProcessLogic (true);
      count +=(short)(maxcount/SIMULATION_ATOM);      
    }
  }
}  
  //if (parent->getAIState()->queryType (FACING)==NULL) {
    //  parent->EnqueueAI (new Orders::FaceTarget (false));
  //}
  //if (parent->getAIState()->queryType (MOVEMENT)==NULL) {
  //  parent->EnqueueAI (new Orders::MatchLinearVelocity (Vector (0,0,420),true,false));
  //}
  


