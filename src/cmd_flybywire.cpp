#include "cmd_flybywire.h"
#include "physics.h"
#define VELTHRESHOLD .05
AI * MatchLinearVelocity::Execute () {
  Vector desired (desired_velocity);
  if (!LocalVelocity) {
    desired = parent->UpCoordinateLevel (desired);
  }
  Vector velocity (parent->UpCoordinateLevel(parent->GetVelocity()));
  if (willfinish) {
    done = fabs(desired.i-velocity.i)<VELTHRESHOLD&&fabs(desired.j-velocity.j)<VELTHRESHOLD&&fabs(desired.k-velocity.k)<VELTHRESHOLD;
    if (done) 
      return NULL;
  }
  parent->Thrust ( /*parent->ClampThrust*/(parent->GetMass()*(desired-velocity)/SIMULATION_ATOM), desired.i>parent->GetComputerData().max_speed);//don't need to clamp thrust since the Thrust does it for you
                 //caution might change 
  return this;
}

#define MATCHANGVELOCITYSETUP() \
  Vector desired (desired_ang_velocity); \
  Vector angvel(parent->UpCoordinateLevel(parent->GetAngularVelocity())); \
  if (!LocalAng)\
    desired = parent->ToLocalCoordinates (desired);
#define MATCHANGVELOCITYEXECUTE() \
  parent->ApplyLocalTorque (parent->GetMoment()*(desired-parent->UpCoordinateLevel(parent->GetAngularVelocity()))/SIMULATION_ATOM); 


AI * MatchAngularVelocity::Execute () {
  MATCHANGVELOCITYSETUP();
  if (willfinish) {
    done = fabs(desired.i-angvel.i)<VELTHRESHOLD&&fabs(desired.j-angvel.j)<VELTHRESHOLD&&fabs(desired.k-angvel.k)<VELTHRESHOLD;
    if (done) return NULL;
  }
  MATCHANGVELOCITYEXECUTE();
  return this;
}

AI * MatchVelocity::Execute () {
  
  MatchLinearVelocity::Execute();
  MATCHANGVELOCITYSETUP()
  if (willfinish) {
    done = done&&fabs(desired.i-angvel.i)<VELTHRESHOLD&&fabs(desired.j-angvel.j)<VELTHRESHOLD&&fabs(desired.k-angvel.k)<VELTHRESHOLD;
    if (done) return NULL;
  }
  MATCHANGVELOCITYEXECUTE();
  //  Vector desired (desired_ang_velocity);
  //  if (!LocalAng)
  //    desired = parent->ToLocalCoordinates (desired);
  //  //parent->GetAngularVelocity();//local coords
  //  parent->ApplyLocalTorque (parent->GetMoment()*(desired-parent->GetAngularVelocity())/SIMULATION_ATOM);
  
   return this;
}







FlyByWire::FlyByWire (): MatchVelocity(Vector(0,0,0),Vector(0,0,0),true,false){

}
void FlyByWire::Stop (float per) {
  SetDesiredVelocity (Vector (0,0,per*parent->GetComputerData().max_speed),true);
}
void FlyByWire::Right (float per) {
  desired_ang_velocity += (per*parent->GetComputerData().max_yaw)*Vector (0,1,0);
}

void FlyByWire::Up (float per) {
  desired_ang_velocity += (per*parent->GetComputerData().max_pitch)*Vector (1,0,0);
}

void FlyByWire::RollRight (float per) {
  desired_ang_velocity += (per*parent->GetComputerData().max_roll)*Vector (0,0,1);
}

void FlyByWire::Afterburn (float per){
  Unit::Computer * cpu = &parent->GetComputerData();
  desired_velocity=Vector (0,0,cpu->set_speed+per*(cpu->max_ab_speed-cpu->set_speed));
}

void FlyByWire::Accel (float per) {
  Unit::Computer *cpu = &parent->GetComputerData(); 
  cpu->set_speed+=per*100*SIMULATION_ATOM;
  if (cpu->set_speed>cpu->max_speed)
    cpu->set_speed=cpu->max_speed;
  if (cpu->set_speed<-cpu->max_speed*parent->Limits().retro/parent->Limits().forward)
    cpu->set_speed = -cpu->max_speed*parent->Limits().retro/parent->Limits().forward;
  desired_velocity= Vector (0,0,cpu->set_speed);
}

#define FBWABS(m) (m>=0?m:-m)
AI * FlyByWire::Execute () {
  MatchVelocity::Execute();
  return this;
} 

