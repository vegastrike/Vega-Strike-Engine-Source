#include "cmd_navigation_orders.h"
#include "vegastrike.h"
#ifndef _WIN32
#include <assert.h>
#endif
using namespace Orders;
//the time we need to start slowing down from now calculation (if it's in this frame we'll only accelerate for partial
// vslowdown - decel * t = 0               t = vslowdown/decel
// finalx = -.5 decel ( v/decel)^2 + v^2 / decel + slowdownx = 1.5 * v^2 / decel + slowdownx 
// slowdownx =  .5 accel * t^2 + v0 * t + initx
// finalx = (.5*(accel * t + v0)^2)/decel + .5 accel * t^2 + v0*t + initx      ;       Length = finalx-initx
// Length = (.5*accel^2*t^2+accel*t*v0+ .5 *v0^2)/decel + .5 accel * t^2 + v0*t

//balanced thrust equation
// Length = accel * t^2 +  2*t*v0 + .5*v0^2/accel
// t = ( -2v0 (+/-) sqrtf (4*v0^2 - 4*(.5*v0^2 - accel*Length) ) / (2*accel)) 
// t = -v0/accel (+/-) sqrtf (.5*v0^2 + Length*accel)/accel;

static float CalculateBalancedDecelTime (float l, float v, float &F, float mass) {

  float accel = F/mass;
  if (l<0) {
    l=-l;
    v=-v;
    F=-F;
  }
  return (-v+sqrtf(.5*v*v+l*accel))/accel;
} 

//the time we need to start slowing down from now calculation (if it's in this frame we'll only accelerate for partial
// vslowdown - decel * t = 0               t = vslowdown/decel
// finalx = -.5 decel ( v/decel)^2 + v^2 / decel + slowdownx = .5 * v^2 / decel + slowdownx 
// slowdownx =  .5 accel * t^2 + v0 * t + initx
// finalx = (.5*(accel * t + v0)^2)/decel + .5 accel * t^2 + v0*t + initx      ;       Length = finalx-initx
// Length = (.5*accel^2*t^2+accel*t*v0+ .5 *v0^2)/decel + .5 accel * t^2 + v0*t

//imbalanced thrust equation
// Length = .5*(accel+accel*accel/decel) * t^2 +  t*v0(1+accel/decel) + .5*v0^2/decel
// t = ( -v0*(1+accel/decel) (+/-) sqrtf (v0^2*(1+accel/decel)^2 - 2*(accel+accel*accel/decel)*(.5*v0^2/decel-Length)))/2*.5*(accel+accel*accel/decel);
// t = (-v0 (+/-) sqrtf (v0^2 - 2*(accel/(1+accel/decel))*(.5*v0^2/decel-Length)))/accel

static float CalculateDecelTime (float l, float v, float &F, float D,  float mass) {
  float accel = F/mass;
  float decel = D/mass;
  if (l<0) {
    l=-l;
    v=-v;
    accel = decel;
    decel = F/mass;
    F=-D;
  }
  return (-v + sqrtf (v*v - 2*accel*((.5*v*v/decel) - l)/(1+accel/decel)))/accel;
} 

void MoveTo::SetDest (const Vector &target) {
    type = LOCATION;
    targetlocation = target;
    done = false;
  }

bool MoveTo::OptimizeSpeed (float v, float &a) {
  v += (a/parent->GetMass())*SIMULATION_ATOM;
  float max_speed = (afterburn?parent->GetComputerData().max_ab_speed:parent->GetComputerData().max_speed);
  if ((!max_speed)||fabs(v)<=max_speed) {
    return true;
  }
  float deltaa = parent->GetMass()*(fabs(v)-max_speed)/SIMULATION_ATOM;//clamping should take care of it
  a += (v>0) ? -deltaa : deltaa;
  return false;
}

float MOVETHRESHOLD=.05;
bool MoveTo::Done(const Vector & local_heading, const Vector & ang_vel) {
  if (fabs(local_heading.i) < MOVETHRESHOLD&&//and local heading is close to the front
      fabs(local_heading.j) < MOVETHRESHOLD&&
      fabs(local_heading.k) < MOVETHRESHOLD) {
    terminating = true;
    if (fabs(ang_vel.i) < THRESHOLD&&
	fabs(ang_vel.j) < THRESHOLD&&
	fabs(ang_vel.k) < THRESHOLD) { //if velocity is lower than threshold
      return true;
    }
  }else {
    if (terminating&& (
	fabs(local_heading.k) > 2*MOVETHRESHOLD||
	fabs(local_heading.i) > 2*MOVETHRESHOLD||//and local heading is close to the front
	fabs(local_heading.j) > 2*MOVETHRESHOLD)) 
      terminating=false;
  }
  return false;
}


AI* MoveTo::Execute(){
  Vector local_location = targetlocation - parent->Position();
  Vector heading = parent->ToLocalCoordinates(local_location);
  Vector velocity = parent->UpCoordinateLevel(parent->GetVelocity());
  Vector thrust (parent->Limits().lateral, parent->Limits().vertical,afterburn?parent->Limits().afterburn:parent->Limits().forward);
  if (Done(heading,velocity)) {
    done = true;
  }
  if (done) return NULL;
  if (terminating) {
    thrust = (-parent->GetMass()/SIMULATION_ATOM)*velocity;
  }else {
    //start with Forward/Reverse:
    float t = CalculateDecelTime(heading.k, velocity.k, thrust.k, parent->Limits().retro, parent->GetMass());
    if (t<THRESHOLD) {
      thrust.k = (thrust.k>0?-parent->Limits().retro:(afterburn?parent->Limits().afterburn:parent->Limits().forward));
    }else {
      if (t<SIMULATION_ATOM) {
	thrust.k*=t/SIMULATION_ATOM;
	thrust.k+= (SIMULATION_ATOM-t)*(thrust.k>0?-parent->Limits().retro:(afterburn?parent->Limits().afterburn:parent->Limits().forward))/SIMULATION_ATOM;
      }
    }
    OptimizeSpeed (velocity.k,thrust.k);
    t = CalculateBalancedDecelTime(heading.i, velocity.i, thrust.i,parent->GetMass());
    if (t<THRESHOLD) {
      thrust.i = -thrust.i;
    }else {
      if (t<SIMULATION_ATOM) {
	thrust.i *= (t-(SIMULATION_ATOM-t))/SIMULATION_ATOM;
      }
    }
    OptimizeSpeed (velocity.i,thrust.i);
    t = CalculateBalancedDecelTime(heading.j, velocity.j, thrust.j,parent->GetMass());
    if (t<THRESHOLD) {
      thrust.j = -thrust.j;
    }else {
      if (t<SIMULATION_ATOM) {
	thrust.j *= (t-(SIMULATION_ATOM-t))/SIMULATION_ATOM;
      }
    }
    OptimizeSpeed (velocity.j,thrust.j);
  }
  parent->ApplyLocalForce (thrust);
  return this;
}
bool ChangeHeading::OptimizeAngSpeed (float optimal_speed,float v, float &a) {
  v += (a/parent->GetMoment())*SIMULATION_ATOM;
  if (!optimal_speed||fabs(v)<=optimal_speed) {
    return true;
  }
  float deltaa = parent->GetMoment()*(fabs(v)-optimal_speed)/SIMULATION_ATOM;//clamping should take care of it
  a += (v>0) ? -deltaa : deltaa;
  return false;
}

//uses CalculateBalancedDecelTime to figure out which way (left or righT) is best to aim for.
//works for both pitch and yaw axis if you pass in the -ang_vel.j for the y
void ChangeHeading::TurnToward (float atancalc, float ang_veli, float &torquei) {
  float t = CalculateBalancedDecelTime (atancalc, ang_veli, torquei, parent->GetMoment());//calculate when we should decel
  if (t<0) {//if it can't make it: try the other way
    torquei = fabs(torquei);//copy sign again
    t = CalculateBalancedDecelTime (atancalc>0?atancalc-2*PI:atancalc+2*PI,ang_veli, torquei, parent->GetMoment());
  }
  if (t>THRESHOLD) {
    if (t<SIMULATION_ATOM) {
      torquei *= ((t/SIMULATION_ATOM)-((SIMULATION_ATOM-t)/SIMULATION_ATOM));
    }
  } else {
    torquei = -parent->GetMoment()*ang_veli/SIMULATION_ATOM;//clamping should take care of it
  }
  fprintf (stderr," angle: %f\n", atancalc);
}
void ChangeHeading::SetDest (const Vector &target) {
  final_heading = target;
  done = false;
}
float TURNTHRESHOLD=.05;
bool ChangeHeading::Done(const Vector & local_heading, const Vector & ang_vel) {
  if (fabs(local_heading.i) < TURNTHRESHOLD&&//and local heading is close to the front
      fabs(local_heading.j) < TURNTHRESHOLD&&
      local_heading.k>0) {
    terminating = true;
    if (fabs(ang_vel.i) < THRESHOLD&&
	fabs(ang_vel.j) < THRESHOLD&&
	fabs(ang_vel.k) < THRESHOLD) { //if velocity is lower than threshold
      return true;
    }
  }else {
    if (terminating&&(
	fabs(local_heading.i) > 2*TURNTHRESHOLD||//and local heading is close to the front
	fabs(local_heading.j) > 2*TURNTHRESHOLD)) 
      terminating=false;
  }
  return false;
}
AI * ChangeHeading::Execute() {
  Vector local_heading = parent->ToLocalCoordinates (final_heading);
  Vector ang_vel = parent->UpCoordinateLevel(parent->GetAngularVelocity());
  done =  Done (local_heading,ang_vel);
  if (done) return NULL;
  Vector torque (parent->Limits().pitch, parent->Limits().yaw,0);//set torque to max accel in any direction
  if (terminating) {
    torque= (-parent->GetMoment()/SIMULATION_ATOM)*ang_vel;
  } else {
    TurnToward (atan2(local_heading.j, local_heading.k),ang_vel.i,torque.i);// find angle away from axis 0,0,1 in yz plane
    OptimizeAngSpeed(parent->GetComputerData().max_pitch,ang_vel.i,torque.i);
    
    TurnToward (atan2 (local_heading.i, local_heading.k), -ang_vel.j, torque.j);
    torque.j=-torque.j;
    OptimizeAngSpeed(parent->GetComputerData().max_yaw,ang_vel.j,torque.j);
    torque.k  =-parent->GetMoment()*ang_vel.k/SIMULATION_ATOM;//try to counteract roll;
  }
  parent->ApplyLocalTorque (torque);
  return this;
}

AI * FaceTarget::Execute() {
  Unit * target = parent->Target();
  if (target==NULL){
    done = finish;
    return NULL;
  }
  SetDest(target->Position());
  ChangeHeading::Execute();
  if (!finish) {
    done=FALSE;
    return this;
  } else{ 
    return done?NULL:this;
  }
}
