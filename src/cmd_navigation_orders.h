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
#ifndef WIN32
	    clog << parent->GetPosition() << "switching to brake state\n";
#endif
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
#ifndef WIN32
		  clog << "Killing velocity: " << parent->GetPosition() << ", " << speed << "\n";
#endif
		}
#ifndef WIN32
	    clog << parent->GetPosition() << "switching to glide state\n";
#endif
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
#ifndef WIN32
	clog << parent->GetPosition() << "Switching to brake state\n";
#endif
	goto case3;
      }
      break;
    case2:
    case 2:
      // Decelerate until we hit the target that we want (could also decelerate until stop on the precise position)
#ifndef WIN32
		clog << "braking, distance remaining: " << distance << "\n";
#endif
		// distance = v*t + 0.5a*t^2
      if(distance-SIMULATION_ATOM > 0 && vel_normal * heading > 0) {
#ifndef WIN32
		  clog << "full retro\n";
#endif
		  parent->Thrust(parent->MaxThrust(-vel_normal));
      } else {
#ifndef WIN32
		  clog << "partial retro\n";
#endif
		  // (v+at)*t + 0.5a*t^2
	accel = 1/1.5 * (distance-speed*SIMULATION_ATOM) / (SIMULATION_ATOM * SIMULATION_ATOM);
	parent->Thrust(heading * accel * parent->GetMass());	
	state = 3;
#ifndef WIN32
	clog << parent->GetPosition() << "Switching to brake phase2\n";
#endif
	break;
      }
      break;
    case3:
    case 3:
      if(speed < bleed_threshold) { 
#ifndef WIN32
	clog << "velocity killed; done\n";
#endif
	done = true;
	return NULL;
      }
      accel = (0 - speed)/(SIMULATION_ATOM);
      parent->Thrust(vel_normal * accel * parent->GetMass());
#ifndef WIN32
      clog << "Killing velocity: " << parent->GetPosition() << ", " << speed << "\n";
#endif
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
    if(done) return NULL;
    Vector local_heading = parent->ToLocalCoordinates(final_heading);
    Vector ang_vel = parent->ToLocalCoordinates(parent->GetAngularVelocity());
    Vector torque(0,0,0);

    Vector p,q,r;
    parent->GetOrientation(p,q,r);
    
    fprintf (stderr,"local heading:  (%f %f %f)\n",local_heading.i,local_heading.j,local_heading.k );

    Vector ang_vel_norm = ang_vel;
    float ang_speed = ang_vel.Magnitude();
    // Algorithm: at each step do a greedy course correction to:
    // 1. convert angular momentum into the r axis. This will cause
    // the spin to precess

    // 2. apply impulse that will reorient r towards the destination
    // if this does not contradict #1, ie must thrust at less than 90
    // degrees off of the original direction
    /*
    if(ang_speed > THRESHOLD) {
      ang_vel_norm = ang_vel_norm / ang_speed;
      ang_vel = ang_vel - local_heading * (ang_vel * local_heading);
      torque = parent->ClampTorque(-ang_vel * parent->GetMoment()/SIMULATION_ATOM);
      
      cerr << "Torque for converting to pure roll " << torque << endl;
    }
    */
    Vector turning;
    CrossProduct(local_heading, Vector(0,0,1), turning);
    fprintf(stderr,"turning: (%f, %f, %f)\n", turning.i, turning.j, turning.k);
    float angle = asin(turning.Magnitude());

    if(fabs(angle) < THRESHOLD) { // handle case where we're really close to the target (or 180 degrees away)
      // for now, pick fastest pure axis
      turning = Vector(parent->Limits().pitch,0,0);
      float max = parent->Limits().pitch;
      if(parent->Limits().yaw > max) {
	turning = Vector(0,parent->Limits().yaw,0);
	max = parent->Limits().yaw;
      }
      if(parent->Limits().roll > max) {
	turning = Vector(0,0,parent->Limits().roll);
	max = parent->Limits().roll;
      }
    }

    if(local_heading * Vector(0,0,1) < 0) { // sin alone is not capable of determining the spin
      if(angle >= 0) {
	angle = PI - angle;
      }
      else if(angle < 0) {
	angle = -PI - angle;
      }
    }
    fprintf (stderr, "angle: %f", angle );
    if(fabs(angle) > THRESHOLD) {
      Vector turning_norm = turning;
      turning_norm.Normalize();
      /*
	Vector temp = NetTorque *SIMULATION_ATOM*(1.0/MomentOfInertia);
	AngularVelocity += temp;
      */

      // should write some routines to factor out calculation of how
      // much impulse is actually needed

      Vector angular_velocity = parent->GetAngularVelocity();
      angular_velocity = angular_velocity - torque / parent->GetMoment() * SIMULATION_ATOM;

      float angular_speed = angular_velocity * turning_norm;
      fprintf (stderr,"Current angular speed: %f\n", angular_speed );
      angular_velocity = turning_norm * (angular_speed);
      angle = angle - angular_speed * SIMULATION_ATOM; // how much we want to try to conver in the next frame
      fprintf (stderr, "angle after adjustment: %f\n", angle) ;

      float max_accel = parent->MaxTorque(turning_norm).Magnitude()/parent->GetMoment();
      Vector max_retro_torque = parent->MaxTorque(-turning_norm);
      Vector max_positive_torque = parent->MaxTorque(turning_norm);
      float max_accel_distance = max_positive_torque.Magnitude()/(parent->GetMoment());
      float retro_torque = max_retro_torque.Magnitude();
      
      if(angle > 0) {
     	// Figure out maximum torque in deceleration direction
	fprintf (stderr,"max accel distance: %f\n", max_accel_distance );
	int n = fabs(floor(angular_speed / (retro_torque/parent->GetMoment()))); // n = # of turns needed to kill all speed
	float braking_distance = fabs(max_accel_distance) * (((n+1)*(n)/2) );
	fprintf (stderr,"Need %d turns to kill all speed, distance of %f\n",n ,braking_distance) ;
	// n*(n+1)/2
      // figure out how far we can get while braking (discrete
	// summation since everything is done with impulses
	if(n<0) {
	  fprintf (stderr,"overbraked\n");
	  braking = false;
	} else if(!braking && braking_distance > fabs(angle)) {
	// start braking
	    braking = true;
	    fprintf (stderr, "Starting to brake\n");
	  }
	 else if(!braking && braking_distance > fabs(angle - max_accel_distance)) { // prevent this turn from doing the full acceleration
	  angle -= braking_distance;
	}
      }
      if(braking && (angular_speed > THRESHOLD && fabs(angle) > max_accel_distance)) { // max brake if positive angle and braking flag is set and we are still moving
	fprintf (stderr,"Braking\n");
	// apply maximum braking power
	angle = - angular_speed / SIMULATION_ATOM;
	angular_speed = 0;
      }
      float delta_v = angle / SIMULATION_ATOM; // get the angular velocity needed to
                                // cover this distance
      delta_v -= angular_speed; // adjust speed downward
      Vector more_torque = turning_norm * (delta_v / SIMULATION_ATOM * parent->GetMoment());
      fprintf (stderr,"Torque for turning towards target (%f,%f,%f)\n",more_torque.i,more_torque.j,more_torque.k);
      fprintf (stderr,"clamped torque (%f,%f,%f)\n", parent->ClampTorque(more_torque).i, parent->ClampTorque(more_torque).j, parent->ClampTorque(more_torque).k);
      torque += more_torque;
    } else {
      // Kill angular momentum
      Vector angular_velocity = parent->GetAngularVelocity();
      Vector more_torque = Vector(-angular_velocity.i, -angular_velocity.j, 0);
      more_torque = more_torque * parent->GetMoment() / SIMULATION_ATOM;

      fprintf (stderr, "Killing angular momentum (%f,%f,%f)\n",more_torque.i,more_torque.j,more_torque.k);
      torque += more_torque;
      if(torque.Magnitude() < THRESHOLD) {
	done = true;
      }
    }

    parent->ApplyLocalTorque(torque);

    if(ang_vel.Magnitude() < THRESHOLD && angle < THRESHOLD) {
      done = true;
      fprintf (stderr, "Done\n");
    }
  }
};
}
#endif

