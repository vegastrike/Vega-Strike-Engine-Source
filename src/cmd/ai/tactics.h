#ifndef TACTICS_H_
#define TACTICS_H_
#include "order.h"

class CloakFor:  public Order {
  float time;
  float maxtime;
  bool enable;
 public:
  CloakFor (bool enable,float seconds=0): Order(0x0),time(0),maxtime(seconds),enable(enable) {}
  void Execute ();
  ~CloakFor ();
};


#endif

