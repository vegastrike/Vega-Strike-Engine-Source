#ifndef _CMD_NAVIGATION_ORDERS_H_
#define _CMD_NAVIGATION_ORDERS_H_

#include "cmd_order.h"
#include "physics.h"

namespace Orders {
  // This moveto always attempts to move in a straight line (unaware of strafing)
  const float bleed_threshold = 0.0001;

class MoveTo : public Order {
  int state; // 0 = accel, 1 = glide, 2 = brake; there should not be a braking phase if this is an active intercept
  float brake_time;
  float max_speed;
public:
  MoveTo(const Vector &target, float max_velocity) : Order(), state(0), max_speed(max_velocity) {
    targetlocation = target;
  }
  AI *Execute() {
    Vector local_location = targetlocation - parent->GetPosition();
    float distance = local_location.Magnitude();
    Vector heading = parent->ToLocalCoordinates(local_location);
    heading.Normalize();
    Vector velocity = parent->GetVelocity();
    Vector vel_normal = velocity;

    float speed = vel_normal.Magnitude();
    float direction = vel_normal * heading;

    vel_normal = vel_normal/speed;
    Vector orthogonal_velocity = velocity - vel_normal * (velocity * heading);
    float orthogonal_magnitude = orthogonal_velocity.Magnitude();

    float max_accel = parent->MaxThrust(heading).Magnitude() / parent->GetMass();
    float max_retro_accel = -parent->MaxThrust(-heading).Magnitude() / parent->GetMass();

    float accel, det;
    switch(state) {
    case 0: 
      // Acceleration phase
      {
	if (vel_normal * heading > 0) {
	  // Check to see if we should start braking
	  /* distance = vt + 1/2 at^2 */
	  /*distance = v*t + 0.5a*t^2
	    accel = 2.0 * (distance - speed*SIMULATION_ATOM) / (SIMULATION_ATOM * SIMULATION_ATOM);*/
	  /* ==> start braking just before real zeros exist */
	  det = speed * speed + 2.0 * max_retro_accel * distance;
	  if(det > 0) { 
	    cerr << parent->GetPosition() << "switching to brake state\n";
	    state = 3;
	    goto case3;
	    /*
	    det = sqrt(det);
	    float first_arrival_time = (-speed + det)/max_retro_accel;
	    float second_arrival_time = (-speed - det)/max_retro_accel;
	    cerr.form("Arrival times: %f %f\n", first_arrival_time, second_arrival_time);
	    if(second_arrival_time - first_arrival_time < SIMULATION_ATOM) {
	      cerr << parent->GetPosition() << "switching to brake state\n";
	      state = 3;
	      goto case3;
	    }
	    */
	  }
	  // check if we should start gliding (exceeded max_speed)
	  if(max_speed - speed <= 0) {
	    if(fabs(max_speed - speed) > bleed_threshold) {
	      // bleed off the excess
	      accel = (max_speed - speed)/(SIMULATION_ATOM);
	      parent->Thrust(vel_normal * accel * parent->GetMass());
	      cerr << "Killing velocity: " << parent->GetPosition() << ", " << speed << endl;
	    }
	    cerr << parent->GetPosition() << "switching to glide state\n";
	    state = 1;
	    break;
	  }
	}
	// Boost up to max velocity; handles overshoot
	/* v = at ==> a = v/t */
	float accel = (max_speed - speed)/(SIMULATION_ATOM);
	parent->Thrust(heading * accel * parent->GetMass());
      }
      break;
    case 1:
      // Glide phase: Don't thrust in the direction of movement, but stabilize the ship in the other directions. In other words, just fall through
      det = speed * speed + 2.0 * max_retro_accel * distance;
      if(det > 0) { // if det < 0, then we can brake before reaching the target
	  state = 3;
	  goto case3;
	  /*	
	det = sqrt(det);
	float first_arrival_time = (-speed + det)/max_retro_accel;
	float second_arrival_time = (-speed - det)/max_retro_accel;
	cerr.form("Arrival times: %f %f\n", first_arrival_time, second_arrival_time);
	if(second_arrival_time - first_arrival_time < SIMULATION_ATOM) {
	  cerr << parent->GetPosition() << "switching to brake state\n";
	  state = 3;
	  goto case3;
	  } */
      } else if(distance + speed * SIMULATION_ATOM + 0.5 * max_retro_accel * SIMULATION_ATOM*SIMULATION_ATOM < 0) { // enough retro deltav to land on the target in the next tick
	state = 3;
	cerr << parent->GetPosition() << "Switching to brake state\n";
	goto case3;
      }
      break;
    case2:
    case 2:
      // Decelerate until we hit the target that we want (could also decelerate until stop on the precise position)
      cerr << "braking, distance remaining: " << distance << endl;
      // distance = v*t + 0.5a*t^2
      if(distance-SIMULATION_ATOM > 0 && vel_normal * heading > 0) {
	cerr << "full retro\n";
	parent->Thrust(parent->MaxThrust(-vel_normal));
      } else {
	cerr << "partial retro\n";
	// (v+at)*t + 0.5a*t^2
	accel = 1/1.5 * (distance-speed*SIMULATION_ATOM) / (SIMULATION_ATOM * SIMULATION_ATOM);
	parent->Thrust(heading * accel * parent->GetMass());	
	state = 3;
	cerr << parent->GetPosition() << "Switching to brake phase2\n";
	break;
      }
      break;
    case3:
    case 3:
      if(speed < bleed_threshold) { 
	cerr << "velocity killed; done\n";
	return NULL;
      }
      accel = (0 - speed)/(SIMULATION_ATOM);
      parent->Thrust(vel_normal * accel * parent->GetMass());
      cerr << "Killing velocity: " << parent->GetPosition() << ", " << speed << endl;
    }
    // Always bleed off lateral speed
    /* disabled for purposes of debug
    if(orthogonal_magnitude > bleed_threshold) {
      // Bleed off momentum in other directions
      // v = at ==> a = v/t 
      parent->Thrust(-orthogonal_velocity / (SIMULATION_ATOM * orthogonal_magnitude) * parent->GetMass());
    }
    */
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
