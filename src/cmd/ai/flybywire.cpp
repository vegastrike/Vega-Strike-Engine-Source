#include "flybywire.h"

#include "vegastrike.h"

#include <math.h>
#include <stdio.h>
#include "cmd/unit_generic.h"
#include "lin_time.h"

#include "gfx/cockpit_generic.h"
//#include "force_feedback.h"
#include "vs_globals.h"
#include "config_xml.h"
#define VELTHRESHOLD .1

using Orders::MatchLinearVelocity;

using Orders::MatchVelocity;

using Orders::MatchAngularVelocity;

//#define MATCHLINVELSETUP()   Vector desired (desired_velocity);  /* cout << "desired= " << desired << endl; */ if (!(desired.i==-2 && desired.j==0 && desired.k==0) && !LocalVelocity) {     desired = parent->ToLocalCoordinates (desired);   }   Vector velocity (parent->UpCoordinateLevel(parent->GetVelocity()));

//#define MATCHLINVELEXECUTE()  if(!(desired.i==-2 && desired.j==0 && desired.k==0)){ parent->Thrust ( (parent->GetMass()*(desired-velocity)/SIMULATION_ATOM), afterburn); }


#define MATCHLINVELSETUP()   Unit *match=parent->VelocityReference(); Vector desired (desired_velocity);  Vector FrameOfRef(0,0,0); if (match!=NULL) {float dif1,dif2; match->GetVelocityDifficultyMult(dif1);dif1*=match->graphicOptions.WarpFieldStrength;parent->GetVelocityDifficultyMult(dif2);dif2*=parent->graphicOptions.WarpFieldStrength;FrameOfRef=parent->ToLocalCoordinates(match->GetWarpVelocity()*dif1/dif2);};  if (!LocalVelocity) {desired = parent->ToLocalCoordinates (desired);}   Vector velocity (parent->UpCoordinateLevel(parent->GetVelocity()));

#define MATCHLINVELEXECUTE()  { parent->Thrust ( (parent->GetMass()*(parent->ClampVelocity(desired,afterburn)+FrameOfRef-velocity)/SIMULATION_ATOM), afterburn); }

/**

 * don't need to clamp thrust since the Thrust does it for you

 * caution might change 

 */

void MatchLinearVelocity::Execute () {
	if (!suborders.empty()) {
		static int i=0;
		if (i++%1000==0) {
			VSFileSystem::vs_fprintf (stderr,"cannot execute suborders as Linear Velocity Matcher");//error printout just in case
		}
	}
  MATCHLINVELSETUP()

  if (willfinish) {

    if ((done = fabs(desired.i+FrameOfRef.i-velocity.i)<VELTHRESHOLD&&fabs(desired.j+FrameOfRef.j-velocity.j)<VELTHRESHOLD&&fabs(desired.k+FrameOfRef.k-velocity.k)<VELTHRESHOLD))

      return;

  }

  MATCHLINVELEXECUTE();

}

MatchLinearVelocity::~MatchLinearVelocity () {
#ifdef ORDERDEBUG
  VSFileSystem::vs_fprintf (stderr,"mlv%x",this);
  fflush (stderr);
#endif
}

void Orders::MatchRoll::Execute() {
  bool temp=done;
  Order::Execute();
  done=temp;
  Vector angvel(parent->UpCoordinateLevel(parent->GetAngularVelocity())); 
  if (willfinish) {
    if (fabs(desired_roll-angvel.k)<VELTHRESHOLD ) {
      return;
    }
  }
  parent->ApplyLocalTorque (parent->GetMoment()*Vector(0,0,desired_roll-angvel.k)/SIMULATION_ATOM);
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
	bool temp = done;
	Order::Execute();
	done=temp;
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
  VSFileSystem::vs_fprintf (stderr,"mav%x",this);
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
  VSFileSystem::vs_fprintf (stderr,"mv%x",this);
  fflush (stderr);
#endif
}
static bool getControlType() {
  static bool control=XMLSupport::parse_bool (vs_config->getVariable ("physics","CarControl",
#ifdef CAR_SIM
                                                                 "true"
#else
                                                                 "false"
#endif
                                                ));
  return control;
}
FlyByWire::FlyByWire (): MatchVelocity(Vector(0,0,0),Vector(0,0,0),true,false,false), sheltonslide(false),controltype(!getControlType()){
  DesiredThrust= Vector(0,0,0);
  stolen_setspeed=false;
  stolen_setspeed_value=0;
}

void FlyByWire::Stop (float per) {

  SetDesiredVelocity (Vector (0,0,per*parent->GetComputerData().max_speed()),true);

  parent->GetComputerData().set_speed =per*parent->GetComputerData().max_speed();

}

void FlyByWire::Right (float per) {

  desired_ang_velocity += (-per*(per>0?parent->GetComputerData().max_yaw_right:parent->GetComputerData().max_yaw_left)/getTimeCompression())*Vector (0,1,0);

}



void FlyByWire::Up (float per) {

  desired_ang_velocity += (-per*(per>0?parent->GetComputerData().max_pitch_up:parent->GetComputerData().max_pitch_down)/getTimeCompression())*Vector (1,0,0);

}



void FlyByWire::RollRight (float per) {

  desired_ang_velocity += (-per*(per>0?parent->GetComputerData().max_roll_right:parent->GetComputerData().max_roll_left)/getTimeCompression())*Vector (0,0,1);

}



void FlyByWire::Afterburn (float per){

  Unit::Computer * cpu = &parent->GetComputerData();

  afterburn=(per>.1);

  desired_velocity=Vector (0,0,cpu->set_speed+per*(cpu->max_ab_speed()-cpu->set_speed));


  if(parent==_Universe->AccessCockpit()->GetParent()){
    //printf("afterburn is %d\n",afterburn);
	// COMMENTED BECAUSE OF SERVER -- NEED TO REINTEGRATE IT IN ANOTHER WAY
    //forcefeedback->playAfterburner(afterburn);
  }
}

void FlyByWire::SheltonSlide (bool onoff) {

  sheltonslide=onoff;

}



void FlyByWire::MatchSpeed (const Vector & vec) {

  Unit::Computer *cpu = &parent->GetComputerData(); 

  cpu->set_speed = (vec).Magnitude();

  if (cpu->set_speed>cpu->max_speed())

    cpu->set_speed=cpu->max_speed();

}



void FlyByWire::Accel (float per) {

  Unit::Computer *cpu = &parent->GetComputerData(); 

  cpu->set_speed+=per*100*SIMULATION_ATOM;

  if (cpu->set_speed>cpu->max_speed())

    cpu->set_speed=cpu->max_speed();

  if (cpu->set_speed<-cpu->max_speed()*parent->Limits().retro/parent->Limits().forward)

    cpu->set_speed = -cpu->max_speed()*parent->Limits().retro/parent->Limits().forward;

  afterburn =false;

  desired_velocity= Vector (0,0,cpu->set_speed);

}



#define FBWABS(m) (m>=0?m:-m)
void FlyByWire::ThrustRight (float percent) {
  DesiredThrust.i = parent->Limits().lateral * percent;
}
void FlyByWire::ThrustUp (float percent) {
  DesiredThrust.j = parent->Limits().vertical * percent;
}
void FlyByWire::ThrustFront (float percent) {
  if (percent>0) {
    DesiredThrust.k = parent->Limits().forward * percent;
  }else {
    DesiredThrust.k = parent->Limits().retro *percent;
  }
}
void FlyByWire::Execute () {
  bool desireThrust=false;
  Vector des_vel_bak (desired_velocity);
  if (DesiredThrust.i||DesiredThrust.j||DesiredThrust.k) {
    desired_velocity = parent->UpCoordinateLevel(parent->GetVelocity())+(SIMULATION_ATOM*DesiredThrust);
    if (!stolen_setspeed) {
      stolen_setspeed=true;
      stolen_setspeed_value= parent->GetComputerData().set_speed;
    }
    parent->GetComputerData().set_speed = desired_velocity.Magnitude();

    desireThrust=true;
    if (!controltype) {
      if (desired_velocity.k<0) {
	desired_velocity.k=0;
	parent->GetComputerData().set_speed = 0;
      }
    }
  }else {
    if (stolen_setspeed) {
      parent->GetComputerData().set_speed = stolen_setspeed_value;
      stolen_setspeed=false;
    }
  }
  static double collidepanic = XMLSupport::parse_float (vs_config->getVariable("physics","collision_inertial_time","1.25"));
  Cockpit * tempcp = _Universe->isPlayerStarship (parent);
  if (((sheltonslide||!controltype)&&(!desireThrust))||(tempcp&&((getNewTime()-tempcp->TimeOfLastCollision)<collidepanic))) {
    MatchAngularVelocity::Execute();//only match turning, keep velocity same
  }else {
    MatchVelocity::Execute();
  }
  DesiredThrust.Set(0,0,0);
  desired_velocity=des_vel_bak;
} 



FlyByWire::~FlyByWire () {
#ifdef ORDERDEBUG
  VSFileSystem::vs_fprintf (stderr,"fbw%x",this);
  fflush (stderr);
#endif
}
