#ifndef _CMD_KEYBOARD_AI_H_
#define _CMD_KEYBOARD_AI_H_
#include "in.h"
#include "order.h"
#include "event_xml.h"
//all unified AI's should inherit from FireAt, so they can choose targets together.


class FireKeyboard: public Order {
  bool itts;
  float rxntime;
  float delay;
  float agg;
  bool ShouldFire(Unit * targ);
  static void FireKey(int, KBSTATE);
  static void TargetKey(int, KBSTATE);

protected:
  float distance;
  void ChooseTargets();//chooses n targets and puts the best to attack in unit's target container
public:
  FireKeyboard ();//weapon prefs?
  virtual void Execute();
};




#endif


