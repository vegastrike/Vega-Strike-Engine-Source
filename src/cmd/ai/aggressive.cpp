#include "aggressive.h"
#include "event_xml.h"
#include "script.h"
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
  EnumMap::Pair ("Movement", AggressiveAI::MOVEMENT),
  EnumMap::Pair ("Rand", AggressiveAI::RANDOMIZ)
};
const EnumMap AggressiveAIel_map(element_names, 12);

AggressiveAI::AggressiveAI (const char * filename, const char * interruptname, Unit * target):FireAt(.2,15), logic (AggressiveAIel_map), interrupts (AggressiveAIel_map) {
  if (target !=NULL) {
    UnitCollection tmp;
    tmp.prepend (target);
    AttachOrder (&tmp);
  }
  AIEvents::LoadAI (filename,logic);
  AIEvents::LoadAI (interruptname,interrupts);
}

bool AggressiveAI::ExecuteLogicItem (const AIEvents::AIEvresult &item) {
  
  if (item.script.length()!=0) {
    Order * tmp = new AIScript (item.script.c_str());	
    parent->EnqueueAI (tmp);
    return true;
  }else {
    return false;
  }
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
    return parent->getAIState()->queryType (Order::FACING)==NULL;
  case MOVEMENT:
    return parent->getAIState()->queryType (Order::MOVEMENT)==NULL;
  case RANDOMIZ:
    value= ((float)rand())/RAND_MAX;
  default:
    return false;
  }
  return item.Eval(value);
}

bool AggressiveAI::ProcessLogic (AIEvents::ElemAttrMap & logi, bool inter) {
  //go through the logic. 
  bool retval=false;
  //  Unit * tmp = parent->Target();
  //  distance = tmp? (tmp->Position()-parent->Position()).Magnitude()-parent->rSize()-tmp->rSize() : FLT_MAX;
  std::vector <std::list <AIEvents::AIEvresult> >::iterator i = logi.result.begin();
  for (;i!=logi.result.end();i++) {
    std::list <AIEvents::AIEvresult>::iterator j;
    bool trueit=true;
    for (j= i->begin();j!=i->end();j++) {
      if (!ProcessLogicItem(*j)) {
	trueit=false;
	break;
      }
    }
    if (trueit&&j==i->end()) {
      //do it
      if (inter) {
	parent->getAIState()->eraseType (Order::FACING);
	parent->getAIState()->eraseType (Order::MOVEMENT);
      }
      j = i->begin();
      while (j!=i->end()) {
	if (ExecuteLogicItem (*j)) {
	  AIEvents::AIEvresult tmp = *j;
	  i->erase(j);
	  retval=true;
	  i->push_back (tmp);
	  break; 
	}else {
	  j++;
	}
      }
    }
  }
  return retval;
}


void AggressiveAI::Execute () {  
  FireAt::Execute();
  if (
#if 1
      curinter==INTRECOVER||//this makes it so only interrupts may not be interrupted
#endif
      curinter==INTNORMAL) {


    if ((curinter = (ProcessLogic (interrupts, true)?INTERR:curinter))==INTERR) {
      logic.curtime=interrupts.maxtime;//set it to the time allotted
    }
  }
  if (parent->getAIState()->queryType (Order::FACING)==NULL&&parent->getAIState()->queryType (Order::MOVEMENT)==NULL) { 
     ProcessLogic(logic);
     curinter=(curinter==INTERR)?INTRECOVER:INTNORMAL;
  } else {
    if ((--logic.curtime)==0) {
      curinter=(curinter==INTERR)?INTRECOVER:INTNORMAL;
      parent->getAIState()->eraseType (Order::FACING);
      parent->getAIState()->eraseType (Order::MOVEMENT);
      
      ProcessLogic (logic);
      logic.curtime = logic.maxtime;      
    }
  }

}  


