#ifndef _CMD_NAVIGATION_ORDERS_H_
#define _CMD_NAVIGATION_ORDERS_H_

#include "cmd_order.h"

class MoveOrder : public Order {
  bool moving;
public:
  MoveOrder(const Vector &target) : Order() {
    targetlocation = target;
    moving = false;
  }
  AI *Execute() {
    Vector dir = targetlocation - parent->Position();
    //if(dir.Magnitude()<0.001) { done = true; delete this; return new AI(); }
    if(!moving) parent->ApplyBalancedLocalTorque(dir.Normalize(), Vector(1,0,0));
    return this;
  }
};

#endif
