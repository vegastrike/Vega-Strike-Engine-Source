#include "cmd_flybywire.h"

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
  bool dirty;//it wasn't updated...
  void UnDirty() {uppress=uprelease=downpress=downrelease=leftpress=leftrelease=rightpress=rightrelease=ABpress=ABrelease=accelpress=accelrelease=decelpress=decelrelease=0;
  dirty=false;}
  StarShipControlKeyboard() {UnDirty();}
} starshipcontrolkeys;

FlyByWire::FlyByWire (float max_ab_spd,float max_spd,float maxyaw,float maxpitch,float maxroll): Order(), max_speed(max_spd), max_ab_speed(max_ab_spd), max_yaw(maxyaw),max_pitch(maxpitch),max_roll(maxroll) {
  type = TARGET;
  done = false;
  
}

AI * FlyByWire::Execute () {
  
  return this;
} 
#define FBWABS(m) (m>=0?m:-m)

void FlyByWire::UpKey(int, KBSTATE k) {
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
