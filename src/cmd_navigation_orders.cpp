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






AI* MoveTo::Execute(){

  Vector local_location = targetlocation - parent->GetPosition();
  Vector heading = parent->ToLocalCoordinates(local_location);
  Vector velocity = parent->UpCoordinateLevel(parent->GetVelocity());
  Vector thrust (parent->Limits().lateral, parent->Limits().vertical,afterburn?parent->Limits().afterburn:parent->Limits().forward);
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
  t = CalculateBalancedDecelTime(heading.i, velocity.i, thrust.i,parent->GetMass());
  if (t<THRESHOLD) {
    thrust.i = -thrust.i;
  }else {
    if (t<SIMULATION_ATOM) {
      thrust.i *= (t-(SIMULATION_ATOM-t))/SIMULATION_ATOM;
    }
  }
  t = CalculateBalancedDecelTime(heading.j, velocity.j, thrust.j,parent->GetMass());
  if (t<THRESHOLD) {
    thrust.j = -thrust.j;
  }else {
    if (t<SIMULATION_ATOM) {
      thrust.j *= (t-(SIMULATION_ATOM-t))/SIMULATION_ATOM;
    }
  }
  parent->ApplyLocalForce (thrust);
  return this;
}




static float CalculateAngDecelTime (float x, float y, float w, float &F, float mass) { //(x=1,y=0 is base)
  return CalculateBalancedDecelTime (atan2(y,x), w, F, mass);
}

AI * ChangeHeading::Execute() {
  Vector local_heading = parent->ToLocalCoordinates (final_heading);
  Vector ang_vel = parent->UpCoordinateLevel(parent->GetAngularVelocity());
  if(fabs(ang_vel.i) < THRESHOLD&&
     fabs(ang_vel.j) < THRESHOLD&&
     ang_vel.k>0&& //if velocity is lower than threshold
     fabs(local_heading.i) < THRESHOLD&&//and local heading is close to the front
     fabs(local_heading.j) < THRESHOLD) {
    done = true;
    fprintf (stderr, "Done\n");
  }
  if (done) return NULL;
  Vector torque (parent->Limits().pitch, parent->Limits().yaw,0);//set torque to max accel in any direction
  float atancalc = atan2(local_heading.j, local_heading.k);// find angle away from axis 0,0,1 in yz plane
  float t = CalculateBalancedDecelTime (atancalc, ang_vel.i, torque.i, parent->GetMoment());//calculate when we should decel
  if (t<0) {//if it can't make it: try the other way
    torque.i = fabs(torque.i);//copy sign again
    t = CalculateBalancedDecelTime (atancalc>0?atancalc-2*PI:atancalc+2*PI,ang_vel.i, torque.i, parent->GetMoment());
  }
  if (t>THRESHOLD) {
    if (t<SIMULATION_ATOM) {
      torque.i *= ((t/SIMULATION_ATOM)-((SIMULATION_ATOM-t)/SIMULATION_ATOM));
    }
  } else {
    torque.i = -parent->GetMoment()*ang_vel.i/SIMULATION_ATOM;//clamping should take care of it
  }
  fprintf (stderr,"pitch angle: %f", atancalc);


  atancalc = atan2 (local_heading.i, local_heading.k);//find angle away from axis 0,0,1 in xz plane
  t = CalculateBalancedDecelTime (atancalc,-ang_vel.j,torque.j,parent->GetMoment());
  if (t<0) {//if it can't make it: try the other way;
    torque.j = fabs(torque.j);//copy sign again
    t = CalculateBalancedDecelTime (atancalc>0?atancalc-2*PI:atancalc+2*PI,-ang_vel.j,torque.j, parent->GetMoment());
  }
  if (t>THRESHOLD) {
    if (t<SIMULATION_ATOM) {
      torque.j *= ((t/SIMULATION_ATOM)-((SIMULATION_ATOM-t)/SIMULATION_ATOM));
    }
  } else {
    torque.j = -parent->GetMoment()*ang_vel.j/SIMULATION_ATOM;//clamping should take care of it
  }
  //torque.i=-torque.i;
  torque.j=-torque.j;
  torque.k  =-parent->GetMoment()*ang_vel.k/SIMULATION_ATOM;//try to counteract roll;
  fprintf (stderr,"yaw angle: %f\n", atancalc);
  parent->ApplyLocalTorque (torque);
  return this;
}

