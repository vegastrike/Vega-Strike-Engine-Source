#ifndef __AGGRESSIVE_AI_H
#define __AGGRESSIVE_AI_H

#include "fire.h"
class Flightgroup;
namespace Orders {
class AggressiveAI: public FireAt {
  enum INTERCUR {INTERR,INTNORMAL,INTRECOVER} curinter;
  bool obedient;//am I currently obedient
  bool last_time_insys;
protected:
  void SignalChosenTarget();
  AIEvents::ElemAttrMap *logic;
  AIEvents::ElemAttrMap *interrupts;
  float logiccurtime;
  bool ProcessLogicItem (const AIEvents::AIEvresult & item);
  bool ExecuteLogicItem (const AIEvents::AIEvresult & item);
  bool ProcessLogic(AIEvents::ElemAttrMap &logic, bool inter=false);//returns if found anything
  string last_directive;

  void ReCommandWing(Flightgroup* fg);
  bool ProcessCurrentFgDirective(Flightgroup *fg);
public:
  virtual void SetParent(Unit *parent1);
  enum types {AGGAI, MOVEMENT, FACING, UNKNOWN, DISTANCE, METERDISTANCE, THREAT, FSHIELD, LSHIELD, RSHIELD, BSHIELD, FARMOR, BARMOR, LARMOR, RARMOR, HULL, RANDOMIZ};
  AggressiveAI (const char *file, const char * interruptename, Unit * target=NULL);
  void Execute ();
  virtual string getOrderDescription() { return "aggressive"; };
};
}

#endif
