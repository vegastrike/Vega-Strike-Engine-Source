#ifndef _CMD_NAVIGATION_ORDERS_H_
#define _CMD_NAVIGATION_ORDERS_H_

#include "cmd_order.h"
#include "physics.h"

namespace Orders {
class MoveToPosition : public Order {
  bool moving;
  int count;
public:
  MoveToPosition(const Vector &target) : Order() {
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

class ChangeHeading : public Order {
  int method;
  Vector final_heading;
  /*
  Vector best_axis;
  Vector off_axis;
  */
 public:
  ChangeHeading(const Vector &final_heading) : method(0), final_heading(final_heading) { }
  AI *Execute() {
    Vector rel_heading = parent->ToLocalCoordinates(final_heading);
    float angle = acos(rel_heading.k); // cone
    float phi = asin(rel_heading.i); // yaw remaining
    float theta = asin(rel_heading.j); // pitch remaining
    Vector angular_velocity;
    float max_delta_yaw = SIMULATION_ATOM * parent->Limits().yaw/parent->GetMoment();
    if(phi<0) max_delta_yaw = -max_delta_yaw;
    float max_delta_pitch = SIMULATION_ATOM * parent->Limits().pitch/parent->GetMoment();
    if(theta<0) max_delta_pitch = -max_delta_pitch;
    float brake_time;

    if(angle < 0.01) method = -1;
    switch(method) {
    case 0: // method 0: use pitch and yaw exclusively
      //Time to start braking?
      // yaw
      brake_time = (-angular_velocity.j+sqrt(angular_velocity.j*angular_velocity.j - 2 * max_delta_yaw * phi))/max_delta_yaw;
      if(brake_time<SIMULATION_ATOM || (angular_velocity.j + max_delta_yaw * brake_time) * angular_velocity.j < 0 ) { // start braking if we can brake within one frame, or if waiting will cause the angular direction to flip (ie, we overshot)
	cerr << "ChangeHeading: Braking yaw\n";
	// refine this to do something less than yanking
	if(phi>0)
	  parent->YawThrust(-parent->Limits().yaw);
	else
	  parent->YawThrust(parent->Limits().yaw);
      } else { // keep accelerating
	cerr << "ChangeHeading: accelerating yaw\n";
	if(phi>0)
	  parent->YawThrust(parent->Limits().yaw);
	else 
	  parent->YawThrust(-parent->Limits().yaw);
      }
      /*
      // pitch
      brake_time = (-angular_velocity.j+sqrt(angular_velocity.j*angular_velocity.j - 2 * max_delta_pitch * theta))/max_delta_pitch;
      if(brake_time<SIMULATION_ATOM) { // start braking
	cerr << "ChangeHeading: Braking pitch\n";
	if(theta>0)
	  parent->PitchThrust(-parent->Limits().pitch);
	else
	  parent->PitchThrust(parent->Limits().pitch);
      } else { // keep accelerating
	cerr << "ChangeHeading: accelerating pitch\n";
	if(theta>0)
	  parent->YawThrust(parent->Limits().pitch);
	else 
	  parent->YawThrust(-parent->Limits().pitch);
      }
      */
      break;
    case -1: // stabilize craft
      cerr << "ChangeHeading: Stabilizing\n";
      angular_velocity = -parent->GetAngularVelocity();
      if(angular_velocity.i == 0 && angular_velocity.j == 0)
	return NULL;
      parent->YawThrust(parent->GetMoment() * angular_velocity.j/SIMULATION_ATOM);
      parent->PitchThrust(parent->GetMoment() * angular_velocity.i/SIMULATION_ATOM);
      break;
    default:
      assert(0);
    }
    return this;
  }
};
}
#endif
