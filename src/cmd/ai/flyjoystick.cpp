#include "in_joystick.h"

#include "flyjoystick.h"

FlyByJoystick::FlyByJoystick(int whichjoystick, bool kb): which_joystick(whichjoystick), keyboard(kb) {
  

}

void FlyByJoystick::Execute() {
  if (keyboard)
    FlyByKeyboard::Execute();
  //see largely the flybykeyboard execute...except use "joystick[which_joystick]->x
  //and stuff

}
FlyByJoystick::~FlyByJoystick() {

}


