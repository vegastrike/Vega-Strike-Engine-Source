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
    JoyStick *joy=joystick[which_joystick];

    float x,y;
    int buttons;
    joy->GetJoyStick(x,y,buttons);
    //printf("flybyjoy x=%f y=%f\n",x,y);
    Up(-y);   // pretty easy
    Right(-x);

    FlyByWire::Execute(); // has to be done here instead of FlyByKeyboard to avoid strange wobbling effects
  }
}
FlyByJoystick::~FlyByJoystick() {

}


