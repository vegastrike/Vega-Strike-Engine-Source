#include "in_joystick.h"

#include "flyjoystick.h"

FlyByJoystick::FlyByJoystick(int whichjoystick, const char * configfile): FlyByKeyboard (configfile), which_joystick(whichjoystick) {
  //remember keybindings from config file?  
  if (whichjoystick>=MAX_JOYSTICKS)
    whichjoystick=0;

}

void FlyByJoystick::Execute() {
  FlyByKeyboard::Execute();
  if (joystick[which_joystick]->isAvailable()) {
    //see largely the flybykeyboard execute...except use "joystick[which_joystick]->x
    //and stuff
  }
}
FlyByJoystick::~FlyByJoystick() {

}


