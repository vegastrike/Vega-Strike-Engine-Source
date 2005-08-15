#include "navigation.h"
#include "macosx_math.h"
#include <math.h>
#ifndef _WIN32
#include <assert.h>
#endif
#include "cmd/unit_generic.h"
using namespace Orders;
#include "lin_time.h"
#include "cmd/script/flightgroup.h"
#include "config_xml.h"
#include "vs_globals.h"
#include "warpto.h"
#include "flybywire.h"
/**
 * the time we need to start slowing down from now calculation (if it's in this frame we'll only accelerate for partial
 * vslowdown - decel * t = 0               t = vslowdown/decel
 * finalx = -.5 decel ( v/decel)^2 + v^2 / decel + slowdownx = .5 * v^2 / decel + slowdownx 
 * slowdownx =  .5 accel * t^2 + v0 * t + initx
 * finalx = (.5*(accel * t + v0)^2)/decel + .5 accel * t^2 + v0*t + initx      ;       Length = finalx-initx
 * Length = (.5*accel^2*t^2+accel*t*v0+ .5 *v0^2)/decel + .5 accel * t^2 + v0*t

 * balanced thrust equation
 * Length = accel * t^2 +  2*t*v0 + .5*v0^2/accel
 * t = ( -2v0 (+/-) sqrtf (4*v0^2 - 4*(.5*v0^2 - accel*Length) ) / (2*accel)) 
 * t = -v0/accel (+/-) sqrtf (.5*v0^2 + Length*accel)/accel;
 *
 * 8/15/05 Patched Calulate BalancedDecel time: our previous quantization factor ignored the quantization during ACCEL phase and also ignored the fact that we overestimated the integral rather than underestimated
 *         new quantization factor is .5*accel*SIMULATION_ATOM*SIMULATION_ATOM-.5*initialVelocity*SIMULATION_ATOM
 *            also this threshold idea is silly--accelerate if t>SIM_ATOM decel if t<0  still havent fixed t between 0 and SIM_ATOM...have decent approx for now.
 * 3/2/02  Patched CalculateBalancedDecel time with the fact that length should be more by a
 * quantity of .5*initialVelocity*SIMULATION_ATOM
 *  
 */

static float CalculateBalancedDecelTime (float l, float v, float &F, float mass) {

  float accel = F/mass;
  if( accel<=0)
	return 0;
  if (l<0) {
    l=-l;
    v=-v;
    F=-F;
  }
  double temp = .5*v*v+(l-v*SIMULATION_ATOM*(.5)+.5*SIMULATION_ATOM*SIMULATION_ATOM*accel)*accel;
  if( temp < 0)
	temp = 0;
  return (-v+sqrtf(temp))/accel;
} 
/**
 * the time we need to start slowing down from now calculation (if it's in this frame we'll only accelerate for partial
 * vslowdown - decel * t = 0               t = vslowdown/decel
 * finalx = -.5 decel ( v/decel)^2 + v^2 / decel + slowdownx = .5 * v^2 / decel + slowdownx 
 * slowdownx =  .5 accel * t^2 + v0 * t + initx
 * finalx = (.5*(accel * t + v0)^2)/decel + .5 accel * t^2 + v0*t + initx      ;       Length = finalx-initx
 * Length = (.5*accel^2*t^2+accel*t*v0+ .5 *v0^2)/decel + .5 accel * t^2 + v0*t

 * imbalanced thrust equation
 * Length = .5*(accel+accel*accel/decel) * t^2 +  t*v0(1+accel/decel) + .5*v0^2/decel
 * t = ( -v0*(1+accel/decel) (+/-) sqrtf (v0^2*(1+accel/decel)^2 - 2*(accel+accel*accel/decel)*(.5*v0^2/decel-Length)))/2*.5*(accel+accel*accel/decel);
 * t = (-v0 (+/-) sqrtf (v0^2 - 2*(accel/(1+accel/decel))*(.5*v0^2/decel-Length)))/accel
 */

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
  return (-v + sqrtf (v*v - 2*accel*((.5*v*v/decel) - v*SIMULATION_ATOM*.5- l)/(1+accel/decel)))/accel;
} 

//failed attempt below
/**
 * The Time We need to start slowing down from the now calculation.
 * k = num simulation atoms before we slow down. m = num sim atoms till we stop
 * R = P = 1    We know  v + S*k*a-S*m*d=0  (stopped at end)  m = v/(d*S)+k*a/d
 * L = a*S^2 (k*(k+R)*.5) -  d*S^2 (m*(m+P)*.5) + v*(k+m)*S
 * L = .5*a*k^2*S^2+.5*a*k*R*s^2 - .5*d*S^2 (k*a/d + v/(d*S))*(k*a/d+v/(d*S)+P) + S*v*(v/(d*S) + k*a/d + k)
 * L = k^2 (.5*a*S^2) +.5*a*k*R*S^2 - .5*d*S^2 (k^2 a^2 / d^2 + 2*(k*a/d)*v/(dS) + v^2/(d^2 s^2) + Pk*a/d + P * v/(d*S)) + v^2/d + S*v*k (a/d + 1)
 * 0 = k^2 (.5*a*S*S*(1-a/d)) + k*S*( .5*R*a*S - .5*P*a*S- .5*2*a*v/d+v(1+a/d)) + v*v/d-.5*v*v/d- P*v*S*.5 - L
 * 0 = (k*S)^2 *(.5*a*(1-a/d)) + k*S*v + v^2/(2d) - vS/2 -L
 * kS = -v (+/-) sqrtf (v*v-a*(1-a/d)*(v*v/d-vS-L))/(a*(1-a/d))
 */
//end failed attempt

void MoveTo::SetDest (const QVector &target) {
    targetlocation = target;
    done = false;
}

bool MoveToParent::OptimizeSpeed (Unit * parent, float v, float &a, float max_speed) {
  v += (a/parent->GetMass())*SIMULATION_ATOM;
  //  float max_speed = ((afterburnAndSwitchbacks&ABURN)?parent->GetComputerData().max_ab_speed():parent->GetComputerData().max_speed());
  if ((!max_speed)||fabs(v)<=max_speed) {
    return true;
  }
  float deltaa = parent->GetMass()*(fabs(v)-max_speed)/SIMULATION_ATOM;//clamping should take care of it
  a += (v>0) ? -deltaa : deltaa;
  return false;
}

float MOVETHRESHOLD=SIMULATION_ATOM/1.9;
bool MoveToParent::Done(const Vector & ang_vel) {
  if (fabs(ang_vel.i) < THRESHOLD&&
      fabs(ang_vel.j) < THRESHOLD&&
      fabs(ang_vel.k) < THRESHOLD) { //if velocity is lower than threshold
      return true;
  }
  return false;
}



void MoveTo::Execute(){
	done = done||m.Execute(parent,targetlocation);
}
  //  cout << "MOVETO target =" << targetlocation << endl;
bool MoveToParent::Execute(Unit * parent, const QVector& targetlocation) {
  bool done=false;
  Vector local_vel (parent->UpCoordinateLevel(parent->GetVelocity()));
  //local location is ued for storing the last velocity;
  terminatingX += ((local_vel.i>0)!=(last_velocity.i>0)||(!local_vel.i));
  terminatingY += ((local_vel.j>0)!=(last_velocity.j>0)||(!local_vel.j));
  terminatingZ += ((local_vel.k>0)!=(last_velocity.k>0)||(!local_vel.k));

  last_velocity = local_vel;
  Vector heading = parent->ToLocalCoordinates((targetlocation-parent->Position()).Cast());
  Vector thrust (parent->Limits().lateral, parent->Limits().vertical,(afterburnAndSwitchbacks&ABURN)?parent->Limits().afterburn:parent->Limits().forward);
  float max_speed = ((afterburnAndSwitchbacks&ABURN)?parent->GetComputerData().max_ab_speed():parent->GetComputerData().max_speed());
  Vector normheading=heading;
  normheading.Normalize();
  Vector max_velocity=max_speed*normheading;
  max_velocity.Set(fabs(max_velocity.i),
                   fabs(max_velocity.j),
                   fabs(max_velocity.k));
                   
  if (done) return done;//unreachable
  unsigned char numswitchbacks = afterburnAndSwitchbacks>>1;
  if (terminatingX>numswitchbacks&&
      terminatingY>numswitchbacks&&
      terminatingZ>numswitchbacks) {
    if (Done(last_velocity)) {
      if (selfterminating) {
	done = true;
      }else {
	terminatingX=0;
	terminatingY=0;
	terminatingZ=0;
      }	
      return done;
    }
    thrust = (-parent->GetMass()/SIMULATION_ATOM)*last_velocity;
  }else {
    //start with Forward/Reverse:
    float t = CalculateDecelTime(heading.k, last_velocity.k, thrust.k, parent->Limits().retro, parent->GetMass());
    if (t<THRESHOLD) {
      thrust.k = (thrust.k>0?-parent->Limits().retro:((afterburnAndSwitchbacks&ABURN)?parent->Limits().afterburn:parent->Limits().forward));
    }else {
      if (t<SIMULATION_ATOM) {
	thrust.k*=t/SIMULATION_ATOM;
	thrust.k+= (SIMULATION_ATOM-t)*(thrust.k>0?-parent->Limits().retro:((afterburnAndSwitchbacks&ABURN)?parent->Limits().afterburn:parent->Limits().forward))/SIMULATION_ATOM;
      }
    }
    OptimizeSpeed (parent,last_velocity.k,thrust.k,max_velocity.k);
    t = CalculateBalancedDecelTime(heading.i, last_velocity.i, thrust.i,parent->GetMass());
    if (t<THRESHOLD) {
      thrust.i = -thrust.i;
    }else {
      if (t<SIMULATION_ATOM) {
	thrust.i *= (t-(SIMULATION_ATOM-t))/SIMULATION_ATOM;
      }
    }
    OptimizeSpeed (parent,last_velocity.i,thrust.i,max_velocity.i);
    t = CalculateBalancedDecelTime(heading.j, last_velocity.j, thrust.j,parent->GetMass());
    if (t<THRESHOLD) {
      thrust.j = -thrust.j;
    }else {
      if (t<SIMULATION_ATOM) {
	thrust.j *= (t-(SIMULATION_ATOM-t))/SIMULATION_ATOM;
      }
    }
    OptimizeSpeed (parent,last_velocity.j,thrust.j,max_velocity.j);
  }
  parent->ApplyLocalForce (thrust);

  /*moved to subclass
  if (_Universe->AccessCockpit()->autoInProgress()) {
    WarpToP(parent,dest);
  }else {
    WarpToP(parent,targetlocation,0);
  }
*/
  return done;
}
MoveTo::~MoveTo () {
#ifdef ORDERDEBUG
  VSFileSystem::vs_fprintf (stderr,"mt%x",this);
  fflush (stderr);
#endif
}

bool ChangeHeading::OptimizeAngSpeed (float optimal_speed_pos, float optimal_speed_neg,float v, float &a) {
  v += (a/parent->GetMoment())*SIMULATION_ATOM;
  if ((optimal_speed_pos==0&&optimal_speed_neg==0)||(v>=-optimal_speed_neg&&v<=optimal_speed_pos)) {
    return true;
  }
  if (v>0) {
    float deltaa = parent->GetMoment()*(v-optimal_speed_pos)/SIMULATION_ATOM;//clamping should take care of it
    a-=deltaa;
  }else {
    float deltaa = parent->GetMoment()*(-v-optimal_speed_neg)/SIMULATION_ATOM;//clamping should take care of it
    a+=deltaa;
  }
  return false;
}

/**
 * uses CalculateBalancedDecelTime to figure out which way (left or righT) is best to aim for.
 * works for both pitch and yaw axis if you pass in the -ang_vel.j for the y
 */
void ChangeHeading::TurnToward (float atancalc, float ang_veli, float &torquei) {
  float t = CalculateBalancedDecelTime (atancalc, ang_veli, torquei, parent->GetMoment());//calculate when we should decel
  if (t<0) {//if it can't make it: try the other way
    torquei = fabs(torquei);//copy sign again
    t = CalculateBalancedDecelTime (atancalc>0?atancalc-2*PI:atancalc+2*PI,ang_veli, torquei, parent->GetMoment());
  }
  if (t>0) {
    if (t<SIMULATION_ATOM) {
      torquei *= ((t/SIMULATION_ATOM)-((SIMULATION_ATOM-t)/SIMULATION_ATOM));
      //torquei=0;//this is just a test --hellcatv
    }
  } else {
    torquei = -parent->GetMoment()*ang_veli/SIMULATION_ATOM;//clamping should take care of it
  }
/* // Print out the variables for debugging.
  char msg[200];
  sprintf(msg, " t=%f, torquei=%f ",t,torquei);
  mission->msgcenter->add("game","all",msg, 0); */
}
void ChangeHeading::SetDest (const QVector &target) {
  final_heading = target;
  ResetDone();
}
float TURNTHRESHOLD=SIMULATION_ATOM/1.9;
///if velocity is lower than threshold
bool ChangeHeading::Done(const Vector & ang_vel) {
    if (fabs(ang_vel.i) < THRESHOLD&&
	fabs(ang_vel.j) < THRESHOLD&&
	fabs(ang_vel.k) < THRESHOLD) { 
      return true;
    }
  return false;
}

void ChangeHeading::Execute() {
  bool temp=done;
  Order::Execute();
  done=temp;
  Vector ang_vel=parent->GetAngularVelocity();
  Vector local_velocity (parent->UpCoordinateLevel(ang_vel));
  Vector local_heading (parent->ToLocalCoordinates ((final_heading-parent->Position()).Cast()));
  //Vector local_heading (parent->UpCoordinateLevel(ang_vel));
  char xswitch = ((local_heading.i>0)!=(last_velocity.i>0)||(!local_heading.i))&&last_velocity.i!=0?1:0;
  char yswitch = ((local_heading.j>0)!=(last_velocity.j>0)||(!local_heading.j))&&last_velocity.j!=0?1:0;
  static bool AICheat = XMLSupport::parse_bool(vs_config->getVariable ("AI","turn_cheat","true"));
  bool cheater=false;
  static float min_for_no_oversteer=XMLSupport::parse_float(vs_config->getVariable("AI","min_angular_accel_cheat","50"));
  if (AICheat&&((parent->Limits().yaw+parent->Limits().pitch)*180/(PI*parent->GetMass())>min_for_no_oversteer)&&!parent->isSubUnit()) {
    if (xswitch||yswitch) {   
      Vector P,Q,R;
      parent->GetOrientation(P,Q,R);
      Vector desiredR=(final_heading-parent->Position()).Cast();
      desiredR.Normalize();
      static float cheatpercent=XMLSupport::parse_float(vs_config->getVariable("AI","ai_cheat_dot",".99"));
      if (desiredR.Dot(R)>cheatpercent) {
        P=Q.Cross(desiredR);
        Q = desiredR.Cross(P);
        parent->SetOrientation(Q,desiredR);
        xswitch=yswitch=1;
      
      if (xswitch) {
	if (yswitch) {
	  local_velocity.j=.0f;
          local_velocity.i=.0f;
          ang_vel.i=.0f;
          ang_vel.j=.0f;
	}else {
	  local_velocity.i=.0f;
          ang_vel.i=.0f;
	}
      }else if (yswitch) {
	  local_velocity.j=.0f;
          ang_vel.j=.0f;
      }
      cheater=true;
	  ang_vel.k=local_velocity.k=0;
      parent->SetAngularVelocity(ang_vel);
      }
    }
  }
  terminatingX += xswitch;
  terminatingY += yswitch;
  last_velocity = local_velocity;

  if (done/*||(xswitch&&yswitch)*/) {
    
    return ;
  }
  Vector torque (parent->Limits().pitch, parent->Limits().yaw,0);//set torque to max accel in any direction
  if (terminatingX>switchbacks&&terminatingY>switchbacks) {
    if (Done (local_velocity)) {
      if (this->terminating) {
	done = true;
      }else {
	terminatingX=0;
	terminatingY=0;
      }
      return;
    }
    torque= (-parent->GetMoment()/SIMULATION_ATOM)*local_velocity;
  } else {
    TurnToward (atan2(local_heading.j, local_heading.k),local_velocity.i,torque.i);// find angle away from axis 0,0,1 in yz plane
    OptimizeAngSpeed(turningspeed*parent->GetComputerData().max_pitch_down,
                     turningspeed*parent->GetComputerData().max_pitch_up,
                     local_velocity.i,
                     torque.i);
    
    TurnToward (atan2 (local_heading.i, local_heading.k), -local_velocity.j, torque.j);
    torque.j=-torque.j;
    OptimizeAngSpeed(turningspeed*parent->GetComputerData().max_yaw_left,
                     turningspeed*parent->GetComputerData().max_yaw_right,
                     local_velocity.j,
                     torque.j);
    torque.k  =-parent->GetMoment()*local_velocity.k/SIMULATION_ATOM;//try to counteract roll;
  }
  if (!cheater)
    parent->ApplyLocalTorque (torque);
}
ChangeHeading::~ChangeHeading() {
#ifdef ORDERDEBUG
  VSFileSystem::vs_fprintf (stderr,"ch%x",this);
  fflush (stderr);
#endif

}
FaceTargetITTS::FaceTargetITTS (bool fini, int accuracy):ChangeHeading(QVector(0,0,1),accuracy),finish(fini) {
  type=FACING;
  subtype = STARGET;
  speed=float(.00001);
  useitts=true;
  static bool alwaysuseitts = XMLSupport::parse_bool(vs_config->getVariable ("AI","always_use_itts","false"));
  if (!alwaysuseitts) {
	  if (rand()>=g_game.difficulty*RAND_MAX)
		  useitts=false;
  }
}
FaceTargetITTS::~FaceTargetITTS () {
#ifdef ORDERDEBUG
  VSFileSystem::vs_fprintf (stderr,"fti%x",this);
  fflush (stderr);
#endif
}

void FaceTargetITTS::Execute() {
  Unit * target = parent->Target();
  if (target==NULL){
    done = finish;
    return;
  }
  if (speed == float(.00001)) {
	  float mrange;
	  float range;
	  parent->getAverageGunSpeed(speed,range,mrange);
	  if (speed ==float (.00001)) {
		  speed = FLT_MAX;
	  }
  }
  SetDest(useitts?target->PositionITTS(parent->Position(),parent->cumulative_velocity,speed,false):target->Position());
  ChangeHeading::Execute();
  if (!finish) {
    ResetDone();
  } 
}


FaceTarget::FaceTarget (bool fini, int accuracy):ChangeHeading(QVector(0,0,1),accuracy),finish(fini) {
  type=FACING;
  subtype =STARGET;
  
}

void FaceTarget::Execute() {
  Unit * target = parent->Target();
  if (target==NULL){
    done = finish;
    return;
  }
  SetDest(target->isSubUnit()?target->Position():target->LocalPosition());
  ChangeHeading::Execute();
  if (!finish) {
    ResetDone();
  } 
}


FaceTarget::~FaceTarget() {
#ifdef ORDERDEBUG
  VSFileSystem::vs_fprintf (stderr,"ft%x",this);
  fflush (stderr);
#endif
}

AutoLongHaul::AutoLongHaul (bool fini, int accuracy):ChangeHeading(QVector(0,0,1),accuracy),finish(fini) {
  type=FACING|MOVEMENT;
  subtype =STARGET;
  
}

void AutoLongHaul::SetParent(Unit *parent1){
	ChangeHeading::SetParent(parent1);
	MatchLinearVelocity *temp = new MatchLinearVelocity(Vector(0,0,parent1->GetComputerData().max_ab_speed()),true,true,false);
	temp->SetParent(parent1);
	Order::EnqueueOrder(temp);
}

void AutoLongHaul::Execute() {
  Unit * target = parent->Target();
  if (target==NULL){
    done = finish;
    return;
  }
  SetDest(target->isSubUnit()?target->Position():target->LocalPosition());
  ChangeHeading::Execute();
  if (!finish) {
    ResetDone();
  } 
}


AutoLongHaul::~AutoLongHaul() {
#ifdef ORDERDEBUG
  VSFileSystem::vs_fprintf (stderr,"ft%x",this);
  fflush (stderr);
#endif
}

void FaceDirection::SetParent(Unit * un) {
  if (un->getFlightgroup()) {
    AttachSelfOrder (un->getFlightgroup()->leader.GetUnit());
  }
  ChangeHeading::SetParent(un);
}
FaceDirection::FaceDirection (float dist, bool fini, int accuracy):ChangeHeading(QVector(0,0,1),accuracy),finish(fini) {
  type=FACING;
  subtype|=SSELF;
  this->dist=dist;
}

void FaceDirection::Execute() {
  Unit * target = group.GetUnit();
  if (target==NULL){
    done = finish;
    return;
  }
  Vector face (target->GetTransformation().getR());

  if ((parent->Position()-target->Position()).Magnitude()-parent->rSize()-target->rSize()>dist) {
    SetDest (target->Position());
  }else {
    SetDest(parent->Position()+face.Cast());
    //VSFileSystem::vs_fprintf (stderr,"facing...cool");
  }
  ChangeHeading::Execute();
  if (!finish) {
    ResetDone();
  } 
}


FaceDirection::~FaceDirection() {
#ifdef ORDERDEBUG
  VSFileSystem::vs_fprintf (stderr,"ft%x",this);
  fflush (stderr);
#endif
}


void FormUp::SetParent(Unit * un) {
  if (un->getFlightgroup()) {
    AttachSelfOrder (un->getFlightgroup()->leader.GetUnit());
  }
  MoveTo::SetParent(un);
}

FormUp::FormUp (const QVector & pos):MoveTo (QVector(0,0,0),false,1000,false), Pos(pos) { 
  subtype |= SSELF;
}
void FormUp::SetPos (const QVector &v) {
  Pos=v;
}
void FormUp::Execute() {
  Unit * targ = group.GetUnit();
  if (targ) {
    MoveTo::SetDest (Transform (targ->GetTransformation(),Pos));
    static bool can_warp_to=XMLSupport::parse_bool(vs_config->getVariable("AI","warp_to_wingmen","true"));   
    if (rand()%64==0&&(can_warp_to||_Universe->AccessCockpit()->autoInProgress()))
      WarpToP(parent,targ,true);
  }
  MoveTo::Execute();
}

FormUp::~FormUp() {}
