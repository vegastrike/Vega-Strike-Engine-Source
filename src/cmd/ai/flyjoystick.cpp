#include "in_joystick.h"

#include "flyjoystick.h"

FlyByJoystick::FlyByJoystick(int whichjoystick): which_joystick(whichjoystick) {
  

}

void FlyByJoystick::Execute() {
  FlyByKeyboard::Execute();
  //see largely the flybykeyboard execute...except use "joystick[which_joystick]->x
  //and stuff

}
FlyByJoystick::~FlyByJoystick() {

}


