#ifndef _CMD_NAVIGATION_ORDERS_H_
#define _CMD_NAVIGATION_ORDERS_H_


#include "cmd_order.h"
#include "physics.h"

namespace Orders {
  // This moveto always attempts to move in a straight line (unaware of strafing)
  const float bleed_threshold = 0.0001;
  const float THRESHOLD = 0.001;

class MoveTo : public Order {
  bool afterburn;
  bool OptimizeSpeed (float v, float &a);
public:
  MoveTo(const Vector &target, bool aft) : Order(), afterburn(aft)  {
    type = LOCATION;
    targetlocation = target;
    done=false;
  }
  void SetDest (const Vector&);
  AI *Execute();
};

class ChangeHeading : public Order {
  Vector final_heading;
  bool OptimizeAngSpeed(float limit, float v, float &a);
  bool Done (const Vector &, const Vector &);
  void TurnToward (float angle, float ang_vel, float &torque);
 public:
   ChangeHeading(const Vector &final_heading) : Order(), final_heading(final_heading) { type = 1;}
  void SetDest (const Vector&);
  AI *Execute();
};
}
#endif

