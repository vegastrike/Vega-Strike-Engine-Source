#include "in_joystick.h"

#include "flyjoystick.h"
#include "firekeyboard.h"
#include "flykeyboard.h"

FlyByJoystick::FlyByJoystick(int whichjoystick, const char * configfile): FlyByKeyboard (configfile), which_joystick(whichjoystick) {
  //remember keybindings from config file?  
  if (whichjoystick>=MAX_JOYSTICKS)
    whichjoystick=0;

#if 0
  // why does the compiler not allow this?//check out my queued events section in firekeyboard.cpp
  BindButton(0,FireKeyboard::FireKey);
  BindButton(1,FireKeyboard::MissileKey);
#endif
  BindJoyKey(whichjoystick,2,FlyByJoystick::JAccelKey);
  BindJoyKey(whichjoystick,3,FlyByJoystick::JDecelKey);

}
void FlyByJoystick::JAccelKey (KBSTATE k, float, float, int) {
  FlyByKeyboard::AccelKey (0,k);
}
void FlyByJoystick::JDecelKey (KBSTATE k, float, float, int) {
  FlyByKeyboard::DecelKey (0,k);
}

void FlyByJoystick::Execute() {
  desired_ang_velocity=Vector(0,0,0); 
  if (joystick[which_joystick]->isAvailable()) {
    JoyStick *joy=joystick[which_joystick];


    //    joy->GetJoyStick(x,y,z,buttons);//don't want state updates in ai function
    Up(-joy->joy_y);   // pretty easy
    Right(-joy->joy_x);
    RollRight (-joy->joy_z);

  }
  FlyByKeyboard::Execute(false);
}
FlyByJoystick::~FlyByJoystick() {

}


