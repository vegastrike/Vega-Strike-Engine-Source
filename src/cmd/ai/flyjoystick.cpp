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
  if (joystick[which_joystick]->isAvailable()) {
    JoyStick *joy=joystick[which_joystick];

    float x,y;
    int buttons;
    joy->GetJoyStick(x,y,buttons);
    //printf("flybyjoy x=%f y=%f\n",x,y);

    Up(-y);   // pretty easy
    Right(-x);


#if 0
    for(int i=0;i<joystick[0]->NumButtons();i++){
      if( ( (1<<i) & buttons )==(1<<i)){
	printf("button %d pressed\n",i);
	joyBindings[i](-1,DOWN);
      }
    }
#endif

    /*
#if 1
    for(int i=0;i<joystick[0]->NumButtons();i++){
      if( ( (1<<i) & buttons )==(1<<i)){
	printf("button %d pressed\n",i);
	buttonState[i]=DOWN;
      }
    }
#endif
    */
    FlyByKeyboard::Execute();
  }
}
FlyByJoystick::~FlyByJoystick() {

}


