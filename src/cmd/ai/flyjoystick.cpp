#include "in_joystick.h"

#include "flyjoystick.h"
#include "firekeyboard.h"
#include "flykeyboard.h"
#include "vs_globals.h"
#include "config_xml.h"

static bool ab;
static bool shelt;

FlyByJoystick::FlyByJoystick(int whichjoystick, unsigned int configfile): FlyByKeyboard (configfile), which_joystick(whichjoystick) {
  //remember keybindings from config file?  

  // this below is outdated
#if 0
  // why does the compiler not allow this?//check out my queued events section in firekeyboard.cpp
  BindButton(0,FireKeyboard::FireKey);
  BindButton(1,FireKeyboard::MissileKey);
#endif
  //  BindJoyKey(whichjoystick,2,FlyByJoystick::JAB);
  //BindJoyKey(whichjoystick,5,FlyByJoystick::JShelt);
  
}


#if 0
void FlyByJoystick::JShelt (KBSTATE k, float, float, int) {
  if (k==DOWN) {
    FlyByKeyboard::SheltonKey (0,DOWN);
    FlyByKeyboard::SheltonKey (0,DOWN);
    FlyByKeyboard::SheltonKey (0,DOWN);
  }
  if (k==UP) {

  }


}
void FlyByJoystick::JAB (KBSTATE k, float, float, int) {
  if (k==PRESS) {
    FlyByKeyboard::ABKey (0,PRESS);
    FlyByKeyboard::ABKey (0,DOWN);

  }
  if (k==DOWN) {
    FlyByKeyboard::ABKey (0,DOWN);
  }
}

void FlyByJoystick::JAccelKey (KBSTATE k, float, float, int) {
  FlyByKeyboard::AccelKey (0,k);
}
void FlyByJoystick::JDecelKey (KBSTATE k, float, float, int) {
  FlyByKeyboard::DecelKey (0,k);
}

#endif

void FlyByJoystick::Execute() {
  desired_ang_velocity=Vector(0,0,0); 

  if (which_joystick<MAX_JOYSTICKS) {

  if (1 ){ //joystick[which_joystick]->isAvailable()) {
    JoyStick *joy=joystick[which_joystick];


    //    joy->GetJoyStick(x,y,z,buttons);//don't want state updates in ai function
#if 0
    Up(-joy->joy_y);   // pretty easy
    Right(-joy->joy_x);
    Accel (-joy->joy_z);
#endif

    int joy_nr;

    joy_nr=vs_config->axis_joy[AXIS_Y];

    if (joy_nr!=-1&&_Universe->numPlayers()>1) {
      joy_nr = which_joystick;
    }
    if( joy_nr!= -1 && joystick[joy_nr]->isAvailable()){
      int config_axis=vs_config->axis_axis[AXIS_Y];
      bool inverse=vs_config->axis_inverse[AXIS_Y];
      float axis_value=- joystick[joy_nr]->joy_axis[config_axis];
      if(inverse) { axis_value= - axis_value; }

      Up( axis_value);
    }

    joy_nr=vs_config->axis_joy[AXIS_X];
    if (joy_nr!=-1&&_Universe->numPlayers()>1) {
      joy_nr = which_joystick;
    }

    if( joy_nr != -1 && joystick[joy_nr]->isAvailable() ){
      int config_axis=vs_config->axis_axis[AXIS_X];
      bool inverse=vs_config->axis_inverse[AXIS_X];
      float axis_value=- joystick[joy_nr]->joy_axis[config_axis];
      if(inverse) { axis_value= - axis_value; }

      Right( axis_value);
    }

    joy_nr=vs_config->axis_joy[AXIS_Z];
    if (joy_nr!=-1&&_Universe->numPlayers()>1) {
      joy_nr = which_joystick;
    }

    if( joy_nr!= -1 && joystick[joy_nr]->isAvailable()){
      int config_axis=vs_config->axis_axis[AXIS_Z];
      bool inverse=vs_config->axis_inverse[AXIS_Z];
      float axis_value=- joystick[joy_nr]->joy_axis[config_axis];
      if(inverse) { axis_value= - axis_value; }

      RollRight( axis_value );
    }

    joy_nr=vs_config->axis_joy[AXIS_THROTTLE];
    if (joy_nr!=-1&&_Universe->numPlayers()>1) {
      joy_nr = which_joystick;
    }

    if( joy_nr != -1 &&  joystick[joy_nr]->isAvailable()){
      int config_axis=vs_config->axis_axis[AXIS_THROTTLE];
      bool inverse=vs_config->axis_inverse[AXIS_THROTTLE];
      float axis_value=- joystick[joy_nr]->joy_axis[config_axis];
      if(inverse) { axis_value= - axis_value; }

      Accel( axis_value );
    }
  }
  }
  FlyByKeyboard::Execute(false);
}
FlyByJoystick::~FlyByJoystick() {

}


