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
    type = 1;
    targetlocation = target;
  }
  AI *Execute() {
    Vector local_location = targetlocation - parent->GetPosition();
    float distance = local_location.Magnitude();
    Vector heading = parent->ToLocalCoordinates(local_location);

    heading.Normalize();
    Vector velocity = parent->GetVelocity();
    Vector vel_normal = velocity;



    Vector p,q,r;
    parent->GetOrientation(p,q,r);
    //clog << "r: " << r << " target : " << local_location << " heading: " << heading << "\n";



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
	    clog << parent->GetPosition() << "switching to brake state\n";
	    state = 3;
	    goto case3;
	    /*
	    det = sqrt(det);
	    float first_arrival_time = (-speed + det)/max_retro_accel;
	    float second_arrival_time = (-speed - det)/max_retro_accel;
	    clog.form("Arrival times: %f %f\n", first_arrival_time, second_arrival_time);
	    if(second_arrival_time - first_arrival_time < SIMULATION_ATOM) {
	      clog << parent->GetPosition() << "switching to brake state\n";
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
	      clog << "Killing velocity: " << parent->GetPosition() << ", " << speed << "\n";
	    }
	    clog << parent->GetPosition() << "switching to glide state\n";
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
	clog.form("Arrival times: %f %f\n", first_arrival_time, second_arrival_time);
	if(second_arrival_time - first_arrival_time < SIMULATION_ATOM) {
	  clog << parent->GetPosition() << "switching to brake state\n";
	  state = 3;
	  goto case3;
	  } */
      } else if(distance + speed * SIMULATION_ATOM + 0.5 * max_retro_accel * SIMULATION_ATOM*SIMULATION_ATOM < 0) { // enough retro deltav to land on the target in the next tick
	state = 3;
	clog << parent->GetPosition() << "Switching to brake state\n";
	goto case3;
      }
      break;
    case2:
    case 2:
      // Decelerate until we hit the target that we want (could also decelerate until stop on the precise position)
      clog << "braking, distance remaining: " << distance << "\n";
      // distance = v*t + 0.5a*t^2
      if(distance-SIMULATION_ATOM > 0 && vel_normal * heading > 0) {
	clog << "full retro\n";
	parent->Thrust(parent->MaxThrust(-vel_normal));
      } else {
	clog << "partial retro\n";
	// (v+at)*t + 0.5a*t^2
	accel = 1/1.5 * (distance-speed*SIMULATION_ATOM) / (SIMULATION_ATOM * SIMULATION_ATOM);
	parent->Thrust(heading * accel * parent->GetMass());	
	state = 3;
	clog << parent->GetPosition() << "Switching to brake phase2\n";
	break;
      }
      break;
    case3:
    case 3:
      if(speed < bleed_threshold) { 
	clog << "velocity killed; done\n";
	done = true;
	return NULL;
      }
      accel = (0 - speed)/(SIMULATION_ATOM);
      parent->Thrust(vel_normal * accel * parent->GetMass());
      clog << "Killing velocity: " << parent->GetPosition() << ", " << speed << "\n";
    }
    // Always bleed off lateral speed
    if(orthogonal_magnitude > bleed_threshold) {
      // Bleed off momentum in other directions
      // v = at ==> a = v/t 
      parent->Thrust(-orthogonal_velocity / (SIMULATION_ATOM * orthogonal_magnitude) * parent->GetMass());
    }
    return this;
  }
};

class ChangeHeading : public Order {
  int method;
  Vector final_heading;
  int state; // 0 = acceleration phase, 1 = deceleration phase
  float optimal_speed;
  bool braking;
 public:
  ChangeHeading(const Vector &final_heading, float limit) : state(0), final_heading(final_heading), optimal_speed(limit), braking(false) { type = 1;}
  AI *Execute() {
    Vector local_heading = parent->ToLocalCoordinates(final_heading);
    float angle = acos(local_heading * Vector(0,0,1));
    
    Vector ang_vel = parent->GetAngularVelocity();
    Vector ang_vel_norm = ang_vel;
    float ang_speed = ang_vel.Magnitude();
    ang_vel_norm = ang_vel_norm / ang_speed;

    Vector torque_norm;
    float torque_start;
    float torque_maxaccel;
    float torque_end;
    float a,b,c;

    // Calculate the 2 candidate arcs
    // 1. The cross product of the current heading and the final heading
    // 2. The average of the current heading and the final heading
    // The angle of 1 will be the arcsin of the length
    // Angle of 2 is 180 degrees

    Vector cand1 = Vector(0,0,1).Cross(local_heading);
    float cand1_angle = asin(cand1.Magnitude());
    cand1 = cand1 / cand1.Magnitude();
    float max_cand1_accel = parent->MaxTorque(cand1).Magnitude()/parent->GetMoment();
    Vector cand2 = ( Vector(0,0,1) + local_heading )/ 2.0;
    cand2.Normalize();
    float max_cand2_accel = parent->MaxTorque(cand2).Magnitude()/parent->GetMoment();

    float a1 = 0.5 * max_cand1_accel;
    float b1 = ang_vel * cand1;
    float c1 = cand1_angle;

    float a2 = 0.5 * max_cand2_accel;
    float b2 = ang_vel * cand2;
    float c2 = PI;

    // figure out which one is better.  

    // 1. how long it takes to stop; what position to aim for (first
    // repetition of the target vector)

    // 2. figure out starting position, assuming max constant
    // acceleration up to this point

    // 3. find turnaround point of journey, calculate time to reach
    // from current state, add to braking time. this is the total
    // time.

    // for now, do something simple
    /*
      if(fabs(b1/max_cand1_accel) < fabs(b2/max_cand2_accel)) { */
      torque_norm = cand1;
      torque_start = 0;
      torque_maxaccel = max_cand1_accel;
      torque_end = cand1_angle;
      a = a1;
      b = b1;
      c = c1;
      /*    } else {
      torque_norm = cand2;
      torque_start = 0;
      torque_maxaccel = max_cand2_accel;
      torque_end = PI;
      a = a1;
      b = b1;
      c = c1;
      }*/
      clog << "target: " << local_heading << "\n";
      clog << "angle: " << angle << "\n";
    if(angle < bleed_threshold*30) {
      clog << "almost there, bleeding off speed\n";
      Vector orth_vel = ang_vel - ang_vel_norm * (ang_vel * local_heading);
      clog << "orth_vel: " << orth_vel << "\n";
      if(orth_vel.Magnitude() < bleed_threshold*30 || ang_speed < bleed_threshold*30) {
	clog << "done\n";
	state = 2;
	done = true;
	return NULL;
      } else {
	// bleed off angular velocity
	parent->ApplyLocalTorque(-orth_vel / (SIMULATION_ATOM) * parent->GetMoment());
      }
    } else {
      switch(state) {
      case 0: 
	{
	  /*
	    float soln_1 = (-b + sqrt (b*b - 4 * a * c)) / (2 * a);
	    float soln_2 = (-b - sqrt (b*b - 4 * a * c)) / (2 * a);
	  */
	  // Find out where we'll be when all speed is bled off
	  float vel = ang_vel * torque_norm;
	  float end_time = vel / torque_maxaccel;
	  float end_position = vel * end_time + 0.5 * copysign(torque_maxaccel,-vel) * end_time * end_time;
	  end_position = copysign(fabs(end_position) - fabs(floor(end_position/(2*PI))) * 2 * PI, end_position);
	  if(end_position < 0) end_position += 2 * PI;
	  if(end_position > PI/2) end_position -= PI/2;
	  clog << "end_position: " << end_position << "\ntorque_end: " << torque_end << "\nprojected torque_end: " << torque_end + vel * SIMULATION_ATOM << "\n";
	  if((end_position - torque_end) * (end_position - (torque_end + vel * SIMULATION_ATOM)) <= 0||braking) { // not quite right, should inculde contribution from current decision; possible issue if angular velocity is exactly 2pi
	    vel = 0 - vel;
	    if(true) { // apply burst to make sure that we land on the right position
	      braking = true;
	      if((end_position - torque_end) * vel < 0) { // stopped before, apply a positive adjustment
		float d = torque_end - end_position;
		float a = -(((vel/SIMULATION_ATOM)>torque_maxaccel)?torque_maxaccel:vel/SIMULATION_ATOM);
		//float accel = -(vel * SIMULATION_ATOM + 0.5 * a * SIMULATION_ATOM * SIMULATION_ATOM - d)/(SIMULATION_ATOM * SIMULATION_ATOM); // not exact; 6am approximation
		//float accel = 2.0 * (d + 0.5 * a * SIMULATION_ATOM * SIMULATION_ATOM) / (SIMULATION_ATOM * SIMULATION_ATOM);
		float fraction = fabs(d/vel)/SIMULATION_ATOM;
		clog << "fraction: " << fraction<< "\n";
		parent->ApplyLocalTorque(torque_norm * vel * parent->GetMoment() * fraction / SIMULATION_ATOM); 
		clog << "applying momentum (braking, not full): " << torque_norm * vel * fraction / SIMULATION_ATOM << "\n";
	      } else {
		clog << "warning: will overshoot\n";
	      }
	    } else {
	      //parent->ApplyLocalTorque(torque_norm * vel * parent->GetMoment() / SIMULATION_ATOM);
	      //clog << "applying momentum (braking): " << torque_norm * vel / SIMULATION_ATOM << "\n";
	    }
	    if((torque_norm * vel * parent->GetMoment() / SIMULATION_ATOM).Magnitude() < parent->MaxTorque(torque_norm).Magnitude()) {
	      state = 3; // stopped
	    }
	  } else {
	    vel = optimal_speed - vel;
	    parent->ApplyLocalTorque(torque_norm * vel * parent->GetMoment() / SIMULATION_ATOM );
	    clog << "applying momentum: " << torque_norm * vel / SIMULATION_ATOM << "\n";
	  }
	}
	break;
      case 2:
	break;
      case 3:
	break;
      }
      if(state!=2) {
	// Bleed off momentum in other directions to stabilize ship
	if(!isnan(ang_speed) && ang_speed > bleed_threshold) {
	  Vector orth_vel = ang_vel - ang_vel_norm * (ang_vel * torque_norm);
	  parent->ApplyLocalTorque(-orth_vel / (SIMULATION_ATOM) * parent->GetMoment());
	  clog << "Momentum bled off: " << -orth_vel / (SIMULATION_ATOM) << "\n";
	}
      }
      return this;
    }
  }
};
}
#endif
