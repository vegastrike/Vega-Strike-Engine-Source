#include "cmd_flybywire.h"
#include "physics.h"
AI * MatchLinearVelocity::Execute () {
  Vector desired (desired_velocity);
  if (!LocalVelocity) {
    desired = parent->ToLocalCoordinates (desired);
  }
  Vector velocity (parent->ToLocalCoordinates (parent->GetVelocity()));
  parent->Thrust ( /*parent->ClampThrust*/(parent->GetMass()*(desired-velocity)/SIMULATION_ATOM));//don't need to clamp thrust since the Thrust does it for you
                 //caution might change 
  return this;
}

#define MATCHANGVELOCITYEXECUTE() Vector desired (desired_ang_velocity); \
  if (!LocalAng)\
    desired = parent->ToLocalCoordinates (desired);\
  parent->ApplyLocalTorque (parent->GetMoment()*(desired-parent->ToLocalCoordinates(parent->GetAngularVelocity()))/SIMULATION_ATOM); 


AI * MatchAngularVelocity::Execute () {
  MATCHANGVELOCITYEXECUTE();
  //  Vector desired (desired_ang_velocity);
  //  if (!LocalAng)
  //    desired = parent->ToLocalCoordinates (desired);
  //  //parent->GetAngularVelocity();//local coords
  //  parent->ApplyLocalTorque (parent->GetMoment()*(desired-parent->GetAngularVelocity())/SIMULATION_ATOM);
  
  return this;
}

AI * MatchVelocity::Execute () {
  MATCHANGVELOCITYEXECUTE();
  MatchLinearVelocity::Execute();
  //  Vector desired (desired_ang_velocity);
  //  if (!LocalAng)
  //    desired = parent->ToLocalCoordinates (desired);
  //  //parent->GetAngularVelocity();//local coords
  //  parent->ApplyLocalTorque (parent->GetMoment()*(desired-parent->GetAngularVelocity())/SIMULATION_ATOM);
  
   return this;
}







FlyByWire::FlyByWire (float max_ab_spd,float max_spd,float maxyaw,float maxpitch,float maxroll): MatchVelocity(Vector(0,0,0),Vector(0,0,0),true), max_speed(max_spd), max_ab_speed(max_ab_spd), max_yaw(maxyaw),max_pitch(maxpitch),max_roll(maxroll) {
  SetDesiredVelocity (Vector (0,0,0),true);
}
void FlyByWire::Stop (float per) {
  SetDesiredVelocity (Vector (0,0,per*max_speed),true);
}
void FlyByWire::Right (float per) {
  desired_ang_velocity += (per*max_yaw)*Vector (0,1,0);
  fprintf (stderr,"r %f\n",per);
}

void FlyByWire::Up (float per) {
  desired_ang_velocity += (per*max_pitch)*Vector (1,0,0);
  fprintf (stderr,"u %f\n",per);
}

void FlyByWire::RollRight (float per) {
  desired_ang_velocity += (per*max_roll)*Vector (0,0,1);
  fprintf (stderr,"rr %f\n",per);
}

void FlyByWire::Afterburn (float per){
  desired_velocity=Vector (0,0,set_speed+per*(max_ab_speed-set_speed));
  fprintf (stderr,"ab %f STRENGTH: %f MAX_AB_SPEED %f\n",per,set_speed+per*(max_ab_speed-set_speed),max_ab_speed);
}

void FlyByWire::Accel (float per) {
  set_speed+=per*100*SIMULATION_ATOM;
  if (set_speed>max_speed)
    set_speed=max_speed;
  if (set_speed<-max_speed*parent->Limits().retro/parent->Limits().forward)
    set_speed = -max_speed*parent->Limits().retro/parent->Limits().forward;
  desired_velocity= Vector (0,0,set_speed);
  fprintf (stderr,"ac %f STRENGTH: %f\n",per, set_speed);
}

#define FBWABS(m) (m>=0?m:-m)
AI * FlyByWire::Execute () {
  MatchVelocity::Execute();
  return this;
} 

