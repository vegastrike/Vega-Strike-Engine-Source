#include "in_joystick.h"
#include "flykeyboard.h"
#include "cmd/unit.h"
#include "navigation.h"
struct StarShipControlKeyboard {
  bool setunvel;
  bool setnulvel;
  bool matchspeed;
  bool jumpkey;
  int sheltonpress;
  int sheltonrelease;
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
  bool stoppress;
  bool startpress;
  bool dirty;//it wasn't updated...
  bool autopilot;
  bool terminateauto;
  void UnDirty() {sheltonpress=sheltonrelease=uppress=uprelease=downpress=downrelease=leftpress=leftrelease=rightpress=rightrelease=ABpress=ABrelease=accelpress=accelrelease=decelpress=decelrelease=rollrightpress=rollrightrelease=rollleftpress=rollleftrelease=0;jumpkey=startpress=stoppress=autopilot=dirty=terminateauto=setunvel=setnulvel=false;}
  StarShipControlKeyboard() {UnDirty();}
};
static vector <StarShipControlKeyboard> starshipcontrolkeys;
static StarShipControlKeyboard &g() {
  while (starshipcontrolkeys.size()<=(unsigned int)_Universe->CurrentCockpit()) {
    starshipcontrolkeys.push_back(StarShipControlKeyboard());
  }
  return starshipcontrolkeys [_Universe->CurrentCockpit()];
}


FlyByKeyboard::FlyByKeyboard (unsigned int whichplayer): FlyByWire () {
  this->whichplayer=whichplayer;
  while(starshipcontrolkeys.size()<=whichplayer) {
    starshipcontrolkeys.push_back (StarShipControlKeyboard());
  }
  autopilot=NULL;
}
FlyByKeyboard::~FlyByKeyboard() {
  if (autopilot)
    delete autopilot;
}

#define FBWABS(m) (m>=0?m:-m)
void FlyByKeyboard::Execute () {
  FlyByKeyboard::Execute (true);
}
void FlyByKeyboard::Execute (bool resetangvelocity) {
#define SSCK starshipcontrolkeys[whichplayer]
  if (SSCK.setunvel) {
    SSCK.setunvel=false;
    parent->VelocityReference (parent->Target());
  }
  if (SSCK.setnulvel) {
    SSCK.setnulvel=false;
    parent->VelocityReference( NULL);
  }
  if (autopilot) {
    autopilot->Execute();
  }
  if (resetangvelocity)
    desired_ang_velocity=Vector(0,0,0);


  //  printf("flybykey::exec\n");

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

    //    fprintf(stderr,"AB: press %d rel %d\n",SSCK.ABpress,SSCK.ABrelease);
    if (SSCK.ABpress>=0)
      Afterburn(0);
    else
      Afterburn (1);

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
      //fprintf(stderr,"AB: press %d rel %d\n",SSCK.ABpress,SSCK.ABrelease);
      Afterburn ((SSCK.ABpress>=SSCK.ABrelease)?1:0);
    }
  }
  if (SSCK.stoppress) {
    Stop(0);
  }
  if (SSCK.startpress) {
    Stop(1);
  }
  if (SSCK.sheltonpress>SSCK.sheltonrelease) {
    SheltonSlide(true);
  } else {
    SheltonSlide(false);
  }
  if (SSCK.autopilot&&!autopilot) {
    autopilot = new Orders::FaceTarget (false,1);
    autopilot->SetParent (parent);
    SSCK.autopilot=false;
  }
  if (SSCK.autopilot||SSCK.terminateauto) {
    if (autopilot) {
      delete autopilot;
      autopilot=NULL;
    }
    SSCK.autopilot=false;
    SSCK.terminateauto=false;
  }
  if (SSCK.matchspeed) {
    SSCK.matchspeed=false;
    Unit * targ = parent->Target();
    if (targ)
      MatchSpeed (targ->GetVelocity());
  }
  if (SSCK.jumpkey) {
    parent->ActivateJumpDrive();
    SSCK.jumpkey=false;
  }else {
    parent->DeactivateJumpDrive();
  }
  SSCK.dirty=true;
#undef SSCK

  FlyByWire::Execute();

}



void FlyByKeyboard::SetVelocityRefKey(int, KBSTATE k) {
  if (g().dirty) g().UnDirty();
  switch (k) {
  case UP:
    break;
  case DOWN:g().setunvel=true;
    break;
  default:break;
  }
}
void FlyByKeyboard::SetNullVelocityRefKey(int, KBSTATE k) {
  if (g().dirty) g().UnDirty();
  switch (k) {
  case UP:
    break;
  case DOWN:g().setnulvel=true;
    break;
  default:break;
  }
}


void FlyByKeyboard::SheltonKey(int, KBSTATE k) {
  if (g().dirty) g().UnDirty();
  switch (k) {
  case UP:
    g().sheltonrelease=FBWABS(g().sheltonrelease)+1;
    break;
  case DOWN:g().sheltonpress=FBWABS(g().sheltonpress)+1;
    break;
  default:break;
  }
}
void FlyByKeyboard::JumpKey(int, KBSTATE k) {
  switch (k) {
  case PRESS:
    g().jumpkey=true;
    break;
  case UP:
  case RELEASE:
    g().jumpkey = false;
    break;
  default:
    break;
  }
}
void FlyByKeyboard::UpKey(int, KBSTATE k) {
  if (g().dirty) g().UnDirty();
  switch (k) {
  case UP: g().uprelease++;
    break;
  case DOWN:g().uppress=FBWABS(g().uppress)+1;
    break;
  case PRESS: g().uppress=FBWABS(g().uppress);
    break;
  case RELEASE: g().uppress=-FBWABS(g().uppress);
    break;
  default:break;
  }
}
void FlyByKeyboard::DownKey (int,KBSTATE k) {
  if (g().dirty)g().UnDirty();
  switch (k) {
  case UP: g().downrelease++;
    break;
  case DOWN:g().downpress=FBWABS(g().downpress)+1;
    break;
  case PRESS: g().downpress=FBWABS(g().downpress);
    break;
  case RELEASE: g().downpress=-FBWABS(g().downpress);
    break;
  default:break;
  }
}
void FlyByKeyboard::LeftKey (int, KBSTATE k) {
  if (g().dirty) g().UnDirty();
  switch (k) {
  case UP: g().leftrelease++;
    break;
  case DOWN:g().leftpress=FBWABS(g().leftpress)+1;
    break;
  case PRESS: g().leftpress=FBWABS(g().leftpress);
    break;
  case RELEASE: g().leftpress=-FBWABS(g().leftpress);
    break;
  default:break;
  }
}
void FlyByKeyboard::RightKey (int,KBSTATE k) {
  if (g().dirty)  g().UnDirty();
  switch (k) {
  case UP: g().rightrelease++;
    break;
  case DOWN:g().rightpress=FBWABS(g().rightpress)+1;
    break;
  case PRESS: g().rightpress=FBWABS(g().rightpress);
    break;
  case RELEASE: g().rightpress=-FBWABS(g().rightpress);
    break;
  default:break;
  }
}
void FlyByKeyboard::ABKey (int, KBSTATE k) {
  if (g().dirty)  g().UnDirty();
  switch (k) {
  case UP: g().ABrelease++;
    break;
  case DOWN:g().ABpress=FBWABS(g().ABpress)+1;
    break;
  case PRESS: g().ABpress=FBWABS(g().ABpress);
    break;
  case RELEASE: g().ABpress=-FBWABS(g().ABpress);
    break;
  default:break;
  }
}

void FlyByKeyboard::AutoKey (int, KBSTATE k) {
  if (g().dirty)  g().UnDirty();
  if (k==PRESS) {
    g().autopilot=true;
  }
}
void FlyByKeyboard::StopAutoKey (int, KBSTATE k) {
  if (g().dirty)  g().UnDirty();
  if (k==PRESS) {
    g().terminateauto=true;
  }
}

void FlyByKeyboard::StopKey (int,KBSTATE k)  {
  if (g().dirty)  g().UnDirty();
  switch (k) {
  case UP: g().stoppress=false;
    break;
  case DOWN:g().stoppress=true;
    break;
  default:break;
  }
}
void FlyByKeyboard::AccelKey (int,KBSTATE k) {
  if (g().dirty)  g().UnDirty();
  switch (k) {
  case UP: g().accelrelease++;
    break;
  case DOWN:g().accelpress=FBWABS(g().accelpress)+1;
    break;
  case PRESS: g().accelpress=FBWABS(g().accelpress);
    break;
  case RELEASE: g().accelpress=-FBWABS(g().accelpress);
    break;
  default:break;
  }
}
void FlyByKeyboard::DecelKey (int,KBSTATE k) {
  if (g().dirty)  g().UnDirty();
  switch (k) {
  case UP: g().decelrelease++;
    break;
  case DOWN:g().decelpress=FBWABS(g().decelpress)+1;
    break;
  case PRESS: g().decelpress=FBWABS(g().decelpress);
    break;
  case RELEASE: g().decelpress=-FBWABS(g().decelpress);
    break;
  default:break;
  }
}


void FlyByKeyboard::StartKey (int,KBSTATE k)  {
  if (g().dirty)  g().UnDirty();
  switch (k) {
  case UP: g().startpress=false;
    break;
  case DOWN:g().startpress=true;
    break;
  default:break;
  }
}




void FlyByKeyboard::RollRightKey (int,KBSTATE k) {
  if (g().dirty)  g().UnDirty();
  switch (k) {
  case UP: g().rollrightrelease++;
    break;
  case DOWN:g().rollrightpress=FBWABS(g().rollrightpress)+1;
    break;
  case PRESS: g().rollrightpress=FBWABS(g().rollrightpress);
    break;
  case RELEASE: g().rollrightpress=-FBWABS(g().rollrightpress);
    break;
  default:break;
  }
}
void FlyByKeyboard::MatchSpeedKey (int, KBSTATE k) {
  if (k==PRESS) {
    if (g().dirty)  g().UnDirty();
    g().matchspeed=true;
  }
}
void FlyByKeyboard::RollLeftKey (int,KBSTATE k) {
  if (g().dirty)  g().UnDirty();
  switch (k) {
  case UP: g().rollleftrelease++;
    break;
  case DOWN:g().rollleftpress=FBWABS(g().rollleftpress)+1;
    break;
  case PRESS: g().rollleftpress=FBWABS(g().rollleftpress);
    break;
  case RELEASE: g().rollleftpress=-FBWABS(g().rollleftpress);
    break;
  default:break;
  }
}

