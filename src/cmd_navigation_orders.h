#ifndef _CMD_NAVIGATION_ORDERS_H_
#define _CMD_NAVIGATION_ORDERS_H_

#include "cmd_order.h"

class MoveOrder : public Order {
  bool moving;
  int count;
public:
  MoveOrder(const Vector &target) : Order() {
    targetlocation = target;
    moving = false;
    count = 0;
  }
  AI *Execute() {
    Vector dir = targetlocation - parent->Position();
    //if(dir.Magnitude()<0.001) { done = true; delete this; return new AI(); }
    if(!moving) {
      moving = true;
      parent->ApplyBalancedLocalTorque(Vector(0,5,0), Vector(1,0,0));
    }
    if(count==120) {
      count = 0;
      parent->ApplyBalancedLocalTorque(Vector(0,5,0), Vector(1,0,0));
      parent->ApplyBalancedLocalTorque(Vector(0,-5,0), Vector(0,0,1));
    }
    if(count++==60) {
      parent->ApplyBalancedLocalTorque(Vector(0,-5,0), Vector(1,0,0));
      parent->ApplyBalancedLocalTorque(Vector(0,5,0), Vector(0,0,1));
    }
    return this;
  }
};

#endif
