#ifndef _CMD_NAVIGATION_ORDERS_H_
#define _CMD_NAVIGATION_ORDERS_H_


#include "cmd_order.h"
#include "physics.h"

namespace Orders {
  // This moveto always attempts to move in a straight line (unaware of strafing)
  const float bleed_threshold = 0.0001;
  const float THRESHOLD = 0.001;

class MoveTo : public Order {
  int state; // 0 = accel, 1 = glide, 2 = brake; there should not be a braking phase if this is an active intercept
  float brake_time;
  float max_speed;
public:
  MoveTo(const Vector &target, float max_velocity) : Order(), state(0), max_speed(max_velocity) {
    type = LOCATION;
    targetlocation = target;
    done=false;
  }
  AI *Execute();
};

class ChangeHeading : public Order {
  int method;
  Vector final_heading;
  int state; // 0 = acceleration phase, 1 = deceleration phase
  float optimal_speed;
  bool braking;
 public:
  ChangeHeading(const Vector &final_heading, float limit) : state(0), final_heading(final_heading), optimal_speed(limit), braking(false) { type = 1;}
  AI *Execute();
};
}
#endif

