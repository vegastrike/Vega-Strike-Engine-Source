#include "cmd_flybywire.h"
#include "physics.h"
AI * MatchVelocity::Execute () {
  Vector desired (desired_velocity);
  if (!LocalVelocity) {
    desired = parent->ToLocalCoordinates (desired);
  }
  Vector velocity (parent->ToLocalCoordinates (parent->GetVelocity()));
  parent->Thrust ( /*parent->ClampThrust*/(parent->GetMass()*(desired-velocity)/SIMULATION_ATOM));//don't need to clamp thrust since the Thrust does it for you
                 //caution might change 
  return this;
}

AI * MatchAngularVelocity::Execute () {
  Vector desired (desired_ang_velocity);
  if (!LocalVelocity)
    desired = parent->ToLocalCoordinates (desired);
  //parent->GetAngularVelocity();//local coords
  parent->ApplyLocalTorque (parent->GetMoment()*(desired-parent->GetAngularVelocity())/SIMULATION_ATOM);
  
  return this;
}







struct StarShipControlKeyboard {
  int uppress;//negative if not pressed last
  int uprelease;
  int downpress;
  int downrelease;
  int leftpress;
  int leftrelease;
  int rightpress;
  int rightrelease;
  int ABpress;
  int ABrelease;
  int accelpress;
  int accelrelease;
  int decelpress;
  int decelrelease;
  int rollrightpress;
  int rollrightrelease;
  int rollleftpress;
  int rollleftrelease;
  bool dirty;//it wasn't updated...
  int refcount;
  void UnDirty() {uppress=uprelease=downpress=downrelease=leftpress=leftrelease=rightpress=rightrelease=ABpress=ABrelease=accelpress=accelrelease=decelpress=decelrelease=rollrightpress=rollrightrelease=rollleftpress=rollleftrelease=0;
  dirty=false;}
  StarShipControlKeyboard() {UnDirty();refcount=0;}
} starshipcontrolkeys;

FlyByWire::FlyByWire (float max_ab_spd,float max_spd,float maxyaw,float maxpitch,float maxroll): Order(), max_speed(max_spd), max_ab_speed(max_ab_spd), max_yaw(maxyaw),max_pitch(maxpitch),max_roll(maxroll) {
  type = TARGET;
  done = false;
  //FIXME:: change hard coded keybindings
  if (starshipcontrolkeys.refcount==0) {
    BindKey(GLUT_KEY_UP,FlyByWire::UpKey);
    BindKey(GLUT_KEY_DOWN,FlyByWire::DownKey);
    BindKey(GLUT_KEY_LEFT,FlyByWire::RightKey);
    BindKey(GLUT_KEY_RIGHT,FlyByWire::LeftKey);
    BindKey('\t',FlyByWire::ABKey);
    BindKey('+',FlyByWire::AccelKey);
    BindKey('-',FlyByWire::DecelKey);   
    BindKey('/',FlyByWire::RollLeftKey);
    BindKey('*',FlyByWire::RollRightKey);
    
  }
  starshipcontrolkeys.refcount++;
}
FlyByWire::~FlyByWire() {
  starshipcontrolkeys.refcount--;
  if (starshipcontrolkeys.refcount==0) {
    UnbindKey (GLUT_KEY_UP);
    UnbindKey (GLUT_KEY_DOWN);
    UnbindKey (GLUT_KEY_LEFT);
    UnbindKey (GLUT_KEY_RIGHT);
    UnbindKey ('/');
    UnbindKey ('*');
    UnbindKey ('+');
    UnbindKey ('-');
    UnbindKey ('\t');
  }
}

void FlyByWire::Right (float per) {
  fprintf (stderr,"r %f\n",per);
}

void FlyByWire::Up (float per) {
  fprintf (stderr,"u %f\n",per);
}

void FlyByWire::RollRight (float per) {
  fprintf (stderr,"rr %f\n",per);
}

void FlyByWire::Afterburn (float per){
  fprintf (stderr,"ab %f\n",per);
}

void FlyByWire::Accel (float per) {
  fprintf (stderr,"ac %f\n",per);
}









#define FBWABS(m) (m>=0?m:-m)
AI * FlyByWire::Execute () {

#define SSCK starshipcontrolkeys
  if (SSCK.dirty) {
    //go with what's last there: no frames since last physics frame
    if (SSCK.uppress<=0&&SSCK.downpress<=0)
      Up(0);
    else {
      if (SSCK.uppress>0)
	Up(1);
      if (SSCK.downpress>0)
	Up(-1);
    }
    if (SSCK.leftpress<=0&&SSCK.rightpress<=0)
      Up(0);
    else {
      if (SSCK.rightpress>0)
	Right(1);
      if (SSCK.leftpress>0)
	Right(-1);
    }
    if (SSCK.rollrightpress<=0&&SSCK.rollleftpress<=0)
      RollRight(0);
    else {
      if (SSCK.rollrightpress>0)
	RollRight(1);
      if (SSCK.rollleftpress>0)
	RollRight(-1);
    }
    if (SSCK.ABpress>0)
      Afterburn(1);
    else
      Afterburn (0);
    if (SSCK.accelpress>0)
      Accel(1);
    if (SSCK.decelpress>0)
      Accel(-1);
  }else {
    if (SSCK.uppress==0&&SSCK.downpress==0)
      Up(0);
    else {
      if (SSCK.uppress!=0&&SSCK.downpress==0)
	Up(((float)FBWABS(SSCK.uppress))/(FBWABS(SSCK.uppress)+SSCK.uprelease));
      else {
	if (SSCK.downpress!=0&&SSCK.uppress==0)
	  Up(-((float)FBWABS(SSCK.downpress))/(FBWABS(SSCK.downpress)+SSCK.downrelease));
        else {
	  Up(((float)FBWABS(SSCK.uppress)-(float)FBWABS(SSCK.downpress))/(FBWABS(SSCK.downpress)+SSCK.downrelease+FBWABS(SSCK.uppress)+SSCK.uprelease));
	}
      }
    }
    if (SSCK.rightpress==0&&SSCK.leftpress==0)
      Right(0);
    else {
      if (SSCK.rightpress!=0&&SSCK.leftpress==0)
	Right(((float)FBWABS(SSCK.rightpress))/(FBWABS(SSCK.rightpress)+SSCK.rightrelease));
      else {
	if (SSCK.leftpress!=0&&SSCK.rightpress==0)
	  Right(-((float)FBWABS(SSCK.leftpress))/(FBWABS(SSCK.leftpress)+SSCK.leftrelease));
        else {
	  Right(((float)FBWABS(SSCK.rightpress)-(float)FBWABS(SSCK.leftpress))/(FBWABS(SSCK.leftpress)+SSCK.leftrelease+FBWABS(SSCK.rightpress)+SSCK.rightrelease));
	}
      }
    }
    if (SSCK.rollrightpress==0&&SSCK.rollleftpress==0)
      RollRight(0);
    else {
      if (SSCK.rollrightpress!=0&&SSCK.rollleftpress==0)
	RollRight(((float)FBWABS(SSCK.rollrightpress))/(FBWABS(SSCK.rollrightpress)+SSCK.rollrightrelease));
      else {
	if (SSCK.rollleftpress!=0&&SSCK.rollrightpress==0)
	  RollRight(-((float)FBWABS(SSCK.rollleftpress))/(FBWABS(SSCK.rollleftpress)+SSCK.rollleftrelease));
        else {
	  RollRight(((float)FBWABS(SSCK.rollrightpress)-(float)FBWABS(SSCK.rollleftpress))/(FBWABS(SSCK.rollleftpress)+SSCK.rollleftrelease+FBWABS(SSCK.rollrightpress)+SSCK.rollrightrelease));
	}
      }
    }
    if (SSCK.accelpress!=0) {
      Accel (((float)FBWABS(SSCK.accelpress))/(FBWABS(SSCK.accelpress)+SSCK.accelrelease));
    }
    if (SSCK.decelpress!=0) {
      Accel (-((float)FBWABS(SSCK.decelpress))/(FBWABS(SSCK.decelpress)+SSCK.decelrelease));
    }
    if (SSCK.ABpress||SSCK.ABrelease) {
      Afterburn (((float)FBWABS(SSCK.ABpress))/(FBWABS(SSCK.ABpress)+SSCK.ABrelease));
    }
  }
  SSCK.dirty=true;
#undef SSCK

  return this;
} 


void FlyByWire::UpKey(int, KBSTATE k) {
  if (starshipcontrolkeys.dirty) starshipcontrolkeys.UnDirty();
  switch (k) {
  case UP: starshipcontrolkeys.uprelease++;
    break;
  case DOWN:starshipcontrolkeys.uppress=FBWABS(starshipcontrolkeys.uppress)+1;
    break;
  case PRESS: starshipcontrolkeys.uppress=FBWABS(starshipcontrolkeys.uppress);
    break;
  case RELEASE: starshipcontrolkeys.uppress=-FBWABS(starshipcontrolkeys.uppress);
    break;
  default:break;
  }
}
void FlyByWire::DownKey (int,KBSTATE k) {
  if (starshipcontrolkeys.dirty)starshipcontrolkeys.UnDirty();
  switch (k) {
  case UP: starshipcontrolkeys.downrelease++;
    break;
  case DOWN:starshipcontrolkeys.downpress=FBWABS(starshipcontrolkeys.downpress)+1;
    break;
  case PRESS: starshipcontrolkeys.downpress=FBWABS(starshipcontrolkeys.downpress);
    break;
  case RELEASE: starshipcontrolkeys.downpress=-FBWABS(starshipcontrolkeys.downpress);
    break;
  default:break;
  }
}
void FlyByWire::LeftKey (int, KBSTATE k) {
  if (starshipcontrolkeys.dirty) starshipcontrolkeys.UnDirty();
  switch (k) {
  case UP: starshipcontrolkeys.leftrelease++;
    break;
  case DOWN:starshipcontrolkeys.leftpress=FBWABS(starshipcontrolkeys.leftpress)+1;
    break;
  case PRESS: starshipcontrolkeys.leftpress=FBWABS(starshipcontrolkeys.leftpress);
    break;
  case RELEASE: starshipcontrolkeys.leftpress=-FBWABS(starshipcontrolkeys.leftpress);
    break;
  default:break;
  }
}
void FlyByWire::RightKey (int,KBSTATE k) {
  if (starshipcontrolkeys.dirty)  starshipcontrolkeys.UnDirty();
  switch (k) {
  case UP: starshipcontrolkeys.rightrelease++;
    break;
  case DOWN:starshipcontrolkeys.rightpress=FBWABS(starshipcontrolkeys.rightpress)+1;
    break;
  case PRESS: starshipcontrolkeys.rightpress=FBWABS(starshipcontrolkeys.rightpress);
    break;
  case RELEASE: starshipcontrolkeys.rightpress=-FBWABS(starshipcontrolkeys.rightpress);
    break;
  default:break;
  }
}
void FlyByWire::ABKey (int, KBSTATE k) {
  if (starshipcontrolkeys.dirty)  starshipcontrolkeys.UnDirty();
  switch (k) {
  case UP: starshipcontrolkeys.ABrelease++;
    break;
  case DOWN:starshipcontrolkeys.ABpress=FBWABS(starshipcontrolkeys.ABpress)+1;
    break;
  case PRESS: starshipcontrolkeys.ABpress=FBWABS(starshipcontrolkeys.ABpress);
    break;
  case RELEASE: starshipcontrolkeys.ABpress=-FBWABS(starshipcontrolkeys.ABpress);
    break;
  default:break;
  }
}
void FlyByWire::AccelKey (int,KBSTATE k) {
  if (starshipcontrolkeys.dirty)  starshipcontrolkeys.UnDirty();
  switch (k) {
  case UP: starshipcontrolkeys.accelrelease++;
    break;
  case DOWN:starshipcontrolkeys.accelpress=FBWABS(starshipcontrolkeys.accelpress)+1;
    break;
  case PRESS: starshipcontrolkeys.accelpress=FBWABS(starshipcontrolkeys.accelpress);
    break;
  case RELEASE: starshipcontrolkeys.accelpress=-FBWABS(starshipcontrolkeys.accelpress);
    break;
  default:break;
  }
}
void FlyByWire::DecelKey (int,KBSTATE k) {
  if (starshipcontrolkeys.dirty)  starshipcontrolkeys.UnDirty();
  switch (k) {
  case UP: starshipcontrolkeys.decelrelease++;
    break;
  case DOWN:starshipcontrolkeys.decelpress=FBWABS(starshipcontrolkeys.decelpress)+1;
    break;
  case PRESS: starshipcontrolkeys.decelpress=FBWABS(starshipcontrolkeys.decelpress);
    break;
  case RELEASE: starshipcontrolkeys.decelpress=-FBWABS(starshipcontrolkeys.decelpress);
    break;
  default:break;
  }
}


void FlyByWire::RollRightKey (int,KBSTATE k) {
  if (starshipcontrolkeys.dirty)  starshipcontrolkeys.UnDirty();
  switch (k) {
  case UP: starshipcontrolkeys.rollrightrelease++;
    break;
  case DOWN:starshipcontrolkeys.rollrightpress=FBWABS(starshipcontrolkeys.rollrightpress)+1;
    break;
  case PRESS: starshipcontrolkeys.rollrightpress=FBWABS(starshipcontrolkeys.rollrightpress);
    break;
  case RELEASE: starshipcontrolkeys.rollrightpress=-FBWABS(starshipcontrolkeys.rollrightpress);
    break;
  default:break;
  }
}
void FlyByWire::RollLeftKey (int,KBSTATE k) {
  if (starshipcontrolkeys.dirty)  starshipcontrolkeys.UnDirty();
  switch (k) {
  case UP: starshipcontrolkeys.rollleftrelease++;
    break;
  case DOWN:starshipcontrolkeys.rollleftpress=FBWABS(starshipcontrolkeys.rollleftpress)+1;
    break;
  case PRESS: starshipcontrolkeys.rollleftpress=FBWABS(starshipcontrolkeys.rollleftpress);
    break;
  case RELEASE: starshipcontrolkeys.rollleftpress=-FBWABS(starshipcontrolkeys.rollleftpress);
    break;
  default:break;
  }
}
