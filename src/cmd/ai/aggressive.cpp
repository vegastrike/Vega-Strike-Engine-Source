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
  EnumMap::Pair ("Movement", AggressiveAI::MOVEMENT)
};
const EnumMap AggressiveAIel_map(element_names, 11);

AggressiveAI::AggressiveAI (const char * filename, Unit * target):FireAt(.2,6,false), logic (AggressiveAIel_map) {
  if (target !=NULL) {
    UnitCollection tmp;
    tmp.prepend (target);
    AttachOrder (&tmp);
  }
  AIEvents::LoadAI (filename,logic);
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

bool AggressiveAI::ItemDistChange (const float newdist, const AIEvents::AIEvresult &item) {
  if (abs(item.type)==DISTANCE) {
    return item.Eval(distance)!=item.Eval (newdist);    
  } else {
    return false;
  }
}
/*deprecated
bool AggressiveAI::DistChange () {
  std::vector <std::list <AIEvents::AIEvresult> >::iterator i = logic.result.begin();  
  for (;i!=logic.result.end();i++) {
    std::list <AIEvents::AIEvresult>::iterator j;
    for (j= i->begin();j!=i->end();j++) {
      if (ItemDistChange(olddist,*j)) {
	return true;
      }
    }  
  }
  return false;
}
*/
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
  default:
    return false;
  }
  return item.Eval(value);
}

void AggressiveAI::ProcessLogic () {
  //go through the logic. 
  Unit * tmp = parent->Target();
  //  distance = tmp? (tmp->Position()-parent->Position()).Magnitude()-parent->rSize()-tmp->rSize() : FLT_MAX;
  std::vector <std::list <AIEvents::AIEvresult> >::iterator i = logic.result.begin();
  for (;i!=logic.result.end();i++) {
    std::list <AIEvents::AIEvresult>::iterator j;
    for (j= i->begin();j!=i->end();j++) {
      if (!ProcessLogicItem(*j)) {
	break;
      }
    }
    if (j==i->end()) {
      //do it
      j = i->begin();
      while (j!=i->end()) {
	if (ExecuteLogicItem (*j)) {
	  AIEvents::AIEvresult tmp = *j;
	  i->erase(j);
	  i->push_back (tmp);
	  break; 
	}else {
	  j++;
	}
      }
    }
  }
}


void AggressiveAI::Execute () {  
  FireAt::Execute();
  if (parent->getAIState()->queryType (Order::FACING)==NULL&&parent->getAIState()->queryType (Order::MOVEMENT)==NULL) { 
     ProcessLogic();
  } else {
    if ( (distance<logic.distclose&&close!=(distance<logic.distclose))||(--logic.curtime)==0) {
      
      parent->getAIState()->eraseType (Order::FACING);
      parent->getAIState()->eraseType (Order::MOVEMENT);
      
      ProcessLogic ();
      logic.curtime = logic.maxtime;      
    }
  }
  close= distance<logic.distclose;
}  
  //if (parent->getAIState()->queryType (FACING)==NULL) {
    //  parent->EnqueueAI (new Orders::FaceTarget (false));
  //}
  //if (parent->getAIState()->queryType (MOVEMENT)==NULL) {
  //  parent->EnqueueAI (new Orders::MatchLinearVelocity (Vector (0,0,420),true,false));
  //}
  


