#ifndef __AGGRESSIVE_AI_H
#define __AGGRESSIVE_AI_H

#include "fire.h"
class Flightgroup;
namespace Orders {
class AggressiveAI: public FireAt {
  bool obedient;//am I currently obedient
  bool last_time_insys;
  char jump_time_check;
  float last_jump_distance;
  float currentpriority;
protected:
  void SignalChosenTarget();
  AIEvents::ElemAttrMap *logic;
  AIEvents::ElemAttrMap *interrupts;
  float logiccurtime;
  float interruptcurtime;
  bool ProcessLogicItem (const AIEvents::AIEvresult & item);
  bool ExecuteLogicItem (const AIEvents::AIEvresult & item);
  bool ProcessLogic(AIEvents::ElemAttrMap &logic, bool inter);//returns if found anything
  string last_directive;

  void ReCommandWing(Flightgroup* fg);
  bool ProcessCurrentFgDirective(Flightgroup *fg);
public:
  virtual void SetParent(Unit *parent1);
  enum types {AGGAI, MOVEMENT, FACING, UNKNOWN, DISTANCE, METERDISTANCE, THREAT, FSHIELD, LSHIELD, RSHIELD, BSHIELD, FARMOR, BARMOR, LARMOR, RARMOR, HULL, RANDOMIZ, FSHIELD_HEAL_RATE, BSHIELD_HEAL_RATE, LSHIELD_HEAL_RATE, RSHIELD_HEAL_RATE, FARMOR_HEAL_RATE, BARMOR_HEAL_RATE, LARMOR_HEAL_RATE, RARMOR_HEAL_RATE, HULL_HEAL_RATE, TARGET_FACES_YOU, TARGET_IN_FRONT_OF_YOU, TARGET_GOING_YOUR_DIRECTION};
  AggressiveAI (const char *file, const char * interruptename, Unit * target=NULL);
  void Execute ();
  virtual string getOrderDescription() { return "aggressive"; };
  void AfterburnerJumpTurnTowards(Unit * target);
  float Fshield_prev;
  float Fshield_rate_old;
  double Fshield_prev_time;
  float Bshield_prev;
  float Bshield_rate_old;
  double Bshield_prev_time;
  float Lshield_prev;
  float Lshield_rate_old;
  double Lshield_prev_time;
  float Rshield_prev;
  float Rshield_rate_old;
  double Rshield_prev_time;
  float Farmour_prev;
  float Farmour_rate_old;
  double Farmour_prev_time;
  float Barmour_prev;
  float Barmour_rate_old;
  double Barmour_prev_time;
  float Larmour_prev;
  float Larmour_rate_old;
  double Larmour_prev_time;
  float Rarmour_prev;
  float Rarmour_rate_old;
  double Rarmour_prev_time;
  float Hull_prev;
  float Hull_rate_old;
  double Hull_prev_time;
};
}

#endif
