#include "flybywire.h"
#include "vegastrike.h"
#include <math.h>
#include <stdio.h>
#include "cmd/unit.h"
#include "lin_time.h"

#include "gfx/cockpit.h"
#include "force_feedback.h"

#define VELTHRESHOLD .1
using Orders::MatchLinearVelocity;
using Orders::MatchVelocity;
using Orders::MatchAngularVelocity;
//#define MATCHLINVELSETUP()   Vector desired (desired_velocity);  /* cout << "desired= " << desired << endl; */ if (!(desired.i==-2 && desired.j==0 && desired.k==0) && !LocalVelocity) {     desired = parent->ToLocalCoordinates (desired);   }   Vector velocity (parent->UpCoordinateLevel(parent->GetVelocity()));
//#define MATCHLINVELEXECUTE()  if(!(desired.i==-2 && desired.j==0 && desired.k==0)){ parent->Thrust ( (parent->GetMass()*(desired-velocity)/SIMULATION_ATOM), afterburn); }

#define MATCHLINVELSETUP()   Unit *match=parent->VelocityReference(); Vector desired (desired_velocity);  Vector FrameOfRef(0,0,0); if (match!=NULL) {float dif1,dif2; match->GetVelocityDifficultyMult(dif1);parent->GetVelocityDifficultyMult(dif2);FrameOfRef=parent->ToLocalCoordinates(match->GetVelocity()*dif1/dif2);};  if (!LocalVelocity) {desired = parent->ToLocalCoordinates (desired);}   Vector velocity (parent->UpCoordinateLevel(parent->GetVelocity()));
#define MATCHLINVELEXECUTE()  { parent->Thrust ( (parent->GetMass()*(parent->ClampVelocity(desired,afterburn)+FrameOfRef-velocity)/SIMULATION_ATOM), afterburn); }
/**
 * don't need to clamp thrust since the Thrust does it for you
 * caution might change 
 */
void MatchLinearVelocity::Execute () {
  MATCHLINVELSETUP()
  if (willfinish) {
    if ((done = fabs(desired.i+FrameOfRef.i-velocity.i)<VELTHRESHOLD&&fabs(desired.j+FrameOfRef.j-velocity.j)<VELTHRESHOLD&&fabs(desired.k+FrameOfRef.k-velocity.k)<VELTHRESHOLD))
      return;
  }
  MATCHLINVELEXECUTE();
}
MatchLinearVelocity::~MatchLinearVelocity () {
#ifdef ORDERDEBUG
  fprintf (stderr,"mlv%x",this);
  fflush (stderr);
#endif
}


/*  //deprecated: now inherits from MatchAngVelocity and uses LinVel macros
#define MATCHANGVELOCITYSETUP() \
  Vector desired (desired_ang_velocity); \
  Vector angvel(parent->UpCoordinateLevel(parent->GetAngularVelocity())); \
  if (!LocalAng)\
    desired = parent->ToLocalCoordinates (desired);
#define MATCHANGVELOCITYEXECUTE() \
  parent->ApplyLocalTorque (parent->GetMoment()*(desired-parent->UpCoordinateLevel(parent->GetAngularVelocity()))/SIMULATION_ATOM); 
*/

void MatchAngularVelocity::Execute () {
  Vector desired (desired_ang_velocity); 
  Vector angvel(parent->UpCoordinateLevel(parent->GetAngularVelocity())); 
  if (!LocalAng)
    desired = parent->ToLocalCoordinates (desired);
  if (willfinish) {
    if ((done = fabs(desired.i-angvel.i)<VELTHRESHOLD&&fabs(desired.j-angvel.j)<VELTHRESHOLD&&fabs(desired.k-angvel.k)<VELTHRESHOLD ))
      return;
  }
  parent->ApplyLocalTorque (parent->GetMoment()*(desired-parent->UpCoordinateLevel(parent->GetAngularVelocity()))/SIMULATION_ATOM); 
}

MatchAngularVelocity::~MatchAngularVelocity () {
#ifdef ORDERDEBUG
  fprintf (stderr,"mav%x",this);
  fflush (stderr);
#endif
}

void MatchVelocity::Execute () {
  
  MatchAngularVelocity::Execute();
  MATCHLINVELSETUP()
  if (willfinish) {
    if ((done = done&&fabs(desired.i-velocity.i)<VELTHRESHOLD&&fabs(desired.j-velocity.j)<VELTHRESHOLD&&fabs(desired.k-velocity.k)<VELTHRESHOLD))
      return;
  }
  MATCHLINVELEXECUTE();
}
MatchVelocity::~MatchVelocity () {
#ifdef ORDERDEBUG
  fprintf (stderr,"mv%x",this);
  fflush (stderr);
#endif
}









FlyByWire::FlyByWire (): MatchVelocity(Vector(0,0,0),Vector(0,0,0),true,false,false), sheltonslide(false){

}
void FlyByWire::Stop (float per) {
  SetDesiredVelocity (Vector (0,0,per*parent->GetComputerData().max_speed),true);
  parent->GetComputerData().set_speed =per*parent->GetComputerData().max_speed;
}
void FlyByWire::Right (float per) {
  desired_ang_velocity += (-per*parent->GetComputerData().max_yaw/getTimeCompression())*Vector (0,1,0);
}

void FlyByWire::Up (float per) {
  desired_ang_velocity += (-per*parent->GetComputerData().max_pitch/getTimeCompression())*Vector (1,0,0);
}

void FlyByWire::RollRight (float per) {
  desired_ang_velocity += (-per*parent->GetComputerData().max_roll/getTimeCompression())*Vector (0,0,1);
}

void FlyByWire::Afterburn (float per){
  Unit::Computer * cpu = &parent->GetComputerData();
  afterburn=(per>.1);
  desired_velocity=Vector (0,0,cpu->set_speed+per*(cpu->max_ab_speed-cpu->set_speed));

  if(parent==_Universe->AccessCockpit()->GetParent()){
    //printf("afterburn is %d\n",afterburn);
    forcefeedback->playAfterburner(afterburn);
  }
}
void FlyByWire::SheltonSlide (bool onoff) {
  sheltonslide=onoff;
}

void FlyByWire::MatchSpeed (const Vector & vec) {
  Unit::Computer *cpu = &parent->GetComputerData(); 
  cpu->set_speed = (vec).Magnitude();
  if (cpu->set_speed>cpu->max_speed)
    cpu->set_speed=cpu->max_speed;
}

void FlyByWire::Accel (float per) {
  Unit::Computer *cpu = &parent->GetComputerData(); 
  cpu->set_speed+=per*100*SIMULATION_ATOM;
  if (cpu->set_speed>cpu->max_speed)
    cpu->set_speed=cpu->max_speed;
  if (cpu->set_speed<-cpu->max_speed*parent->Limits().retro/parent->Limits().forward)
    cpu->set_speed = -cpu->max_speed*parent->Limits().retro/parent->Limits().forward;
  afterburn =false;
  desired_velocity= Vector (0,0,cpu->set_speed);
}

#define FBWABS(m) (m>=0?m:-m)
void FlyByWire::Execute () {
  if (sheltonslide) {
    MatchAngularVelocity::Execute();//only match turning, keep velocity same
  }else {
    MatchVelocity::Execute();
  }
} 

FlyByWire::~FlyByWire () {
#ifdef ORDERDEBUG
  fprintf (stderr,"fbw%x",this);
  fflush (stderr);
#endif
}
