#include "cmd_navigation_orders.h"
#include "vegastrike.h"
using namespace Orders;
AI* MoveTo::Execute(){
  Vector local_location = targetlocation - parent->GetPosition();
  float distance = local_location.Magnitude();
  Vector heading = parent->ToLocalCoordinates(local_location);
  
  heading.Normalize();
  //062201 FIXME do I need to be in local coords? i thinks o    Vector velocity = parent->GetVelocity();
  
  
  
  Vector p,q,r;
  parent->GetOrientation(p,q,r);

  Vector velocity = parent->UpCoordinateLevel(parent->GetVelocity());
  Vector vel_normal = velocity;

  //clog << "r: " << r << " target : " << local_location << " heading: " << heading << "\n";
  


  float speed = vel_normal.Magnitude();
  //  float direction = vel_normal * heading;??
  
  vel_normal = vel_normal/speed;
  Vector orthogonal_velocity = velocity - vel_normal * (velocity * heading);
  float orthogonal_magnitude = orthogonal_velocity.Magnitude();
  
  //  float max_accel = parent->MaxThrust(heading).Magnitude() / parent->GetMass();??
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


//the time we need to start slowing down from now calculation (if it's in this frame we'll only accelerate for partial
// vslowdown - decel * t = 0               t = vslowdown/decel
// finalx = -.5 decel ( v/decel)^2 + v^2 / decel + slowdownx = 1.5 * v^2 / decel + slowdownx 
// slowdownx =  .5 accel * t^2 + v0 * t + initx
// finalx = (.5*(accel * t + v0)^2)/decel + .5 accel * t^2 + v0*t + initx      ;       Length = finalx-initx

// Length = (.5*accel^2*t^2+accel*t*v0+ .5 *v0^2)/decel + .5 accel * t^2 + v0*t
// Now we assume accel = decel.... for purposes of stupidity :-)
// Length = accel * t^2 +  2*t*v0 + .5*v0^2/accel

// t = ( -2v0 (+/-) sqrtf (4*v0^2 - 4*(.5*v0^2 - accel*Length) ) / (2*accel)) 
// t = -v0/accel (+/-) sqrtf (.5*v0^2 + Length*accel)/accel;

AI * ChangeHeading::Execute() {
  if (done) return NULL;
  Vector local_heading = parent->ToLocalCoordinates (final_heading);
  Vector turning (local_heading.j,-local_heading.i,0);//Vector(0,0,1) X local_heading
  Vector ang_vel = parent->UpCoordinateLevel(parent->GetAngularVelocity());
  float angle = asin (sqrtf(local_heading.j*local_heading.j+local_heading.i*local_heading.i));//asin(turning.Magnitude());
  if(local_heading.k < 0)  // sin alone is not capable of determining the spin
    angle = angle<0?-PI-angle : PI-angle;
  
  return this;
}
/*
  // Algorithm: at each step do a greedy course correction to:
  // 1. convert angular momentum into the r axis. This will cause
  // the spin to precess
  
  // 2. apply impulse that will reorient r towards the destination
  // if this does not contradict #1, ie must thrust at less than 90
  // degrees off of the original direction

AI *ChangeHeading::Execute(){

  if(done) return NULL;
  Vector local_heading = parent->ToLocalCoordinates(final_heading);
  //float ang_speed = ang_vel.Magnitude();
  Vector torque(0,0,0);
  Vector turning (local_heading.j,-local_heading.i,0);//  CrossProduct(local_heading, Vector(0,0,1), turning);

  fprintf(stderr,"turning: (%f, %f, %f)\n", turning.i, turning.j, turning.k);
  float angle = asin (sqrtf(local_heading.j*local_heading.j+local_heading.i*local_heading.i));//asin(turning.Magnitude());

  if(fabs(parent->GetAngularVelocity().i) < THRESHOLD&&fabs(parent->GetAngularVelocity().j) < THRESHOLD&&fabs(parent->GetAngularVelocity().k) < THRESHOLD&& angle < THRESHOLD) {
    done = true;
    fprintf (stderr, "Done\n");
    return NULL;
  }

//**FAULTY**the time we need to start slowing down from now calculation (if it's in this frame we'll only accelerate for partial
// **FAULTY**finalx = .5 decel ( 2 v/decel)^2 + 2 v * v / decel + slowdownx = 4 v^2 / decel + slowdown x 
// **FAULTY**slowdownx =  .5 accel * t^2 + v0 * t + initx
// **FAULTY**finalx = (4*(accel * t + v0)^2)/decel + .5 accel * t^2 + v0*t + initx      ;       Length = finalx-initx

// **FAULTY**Length = (4*accel^2*t^2+8*accel*t*v0+ 4 *v0^2)/decel + .5 accel * t^2 + v0*t
// **FAULTY**Now we assume accel = decel.... for purposes of stupidity :-)
// **FAULTY**Length = 4.5 accel * t^2 + 9 * t*v0 + 4*v0^2/accel

// **FAULTY**t = ( -9*v0 (+/-) sqrtf (81*v0^2 - 18*(4*v0^2 - accel*Length) ) / (9*accel)) 

  if(fabs(angle) < THRESHOLD) { // handle case where we're really close to the target (or 180 degrees away)
    // for now, pick fastest pure axis
    if (local_heading.k<0) {
      fprintf (stderr,"thresholding\n");
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
    } else {
      //slow down;
      //      braking=true;
    }
  }
  
  if(local_heading.k < 0) { // sin alone is not capable of determining the spin
    if(angle >= 0) {
      angle = PI - angle;
    }
    else if(angle < 0) {
      angle = -PI - angle;
    }
  }
  fprintf (stderr, "angle: %f", angle );
  Vector angular_velocity = parent->UpCoordinateLevel(parent->GetAngularVelocity());
  if(fabs(angle) > THRESHOLD) {
    Vector turning_norm (turning);
    turning_norm.Normalize();
    

    
    //    angular_velocity = angular_velocity - (torque / parent->GetMoment()) * SIMULATION_ATOM;?????070901
    
    float angular_speed = angular_velocity * turning_norm;
    fprintf (stderr,"Current angular speed: %f\n", angular_speed );
    angular_velocity = turning_norm * (angular_speed);
    angle = angle - angular_speed * SIMULATION_ATOM; // how much we want to try to conver in the next frame
    fprintf (stderr, "angle after adjustment: %f\n", angle) ;
    
    float max_accel = parent->MaxTorque(turning_norm).Magnitude()*SIMULATION_ATOM/parent->GetMoment();
    Vector max_retro_torque = parent->MaxTorque(-turning_norm);
    Vector max_positive_torque = parent->MaxTorque(turning_norm);
    float max_accel_distance = max_positive_torque.Magnitude()*SIMULATION_ATOM/(parent->GetMoment());
    float retro_torque = max_retro_torque.Magnitude();
    
    if(angle > 0) {
      // Figure out maximum torque in deceleration direction
      fprintf (stderr,"max accel distance: %f\n", max_accel_distance );
      int n = fabs(ceil(angular_speed / (SIMULATION_ATOM*retro_torque/parent->GetMoment()))); // n = # of turns needed to kill all speed
      float braking_distance = .5* n*angular_speed;//fabs(max_accel_distance) * (((n+1)*(n)*.5) );
      fprintf (stderr,"Need %d turns to kill all speed, distance of %f\n",n ,braking_distance) ;
      // n*(n+1)/2
      if(!braking && braking_distance > fabs(angle)) {
	// start braking
	braking = true;
	fprintf (stderr, "Starting to brake\n");
      }
      else if(!braking && braking_distance > fabs(angle - max_accel_distance)) { // prevent this turn from doing the full acceleration
	angle -= braking_distance;
	fprintf (stderr, "P\nP\nP\nP\nPartial brake\n");
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

    Vector more_torque (-angular_velocity.i, -angular_velocity.j, 0);
    more_torque = more_torque * parent->GetMoment() / SIMULATION_ATOM;
    
    fprintf (stderr, "Killing angular momentum (%f,%f,%f)\n",more_torque.i,more_torque.j,more_torque.k);
    torque += more_torque;
    if(torque.Magnitude() < THRESHOLD) {
      done = true;
    }
  }

  parent->ApplyLocalTorque(torque);
  
  return this;
}
*/
