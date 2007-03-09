#include "in_joystick.h"

#include "flyjoystick.h"
#include "firekeyboard.h"
#include "flykeyboard.h"
#include "vs_globals.h"
#include "config_xml.h"
#include  "in_kb_data.h"
//static bool ab;
//static bool shelt;

FlyByJoystick::FlyByJoystick(unsigned int configfile): FlyByKeyboard (configfile){
  for (int i=0;i<MAX_JOYSTICKS;i++) {
    if ((unsigned int)joystick[i]->player==configfile)
      whichjoystick.push_back(i);
  }
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
    FlyByKeyboard::SheltonKey (std::string(),DOWN);
    FlyByKeyboard::SheltonKey (std::string(),DOWN);
    FlyByKeyboard::SheltonKey (std::string(),DOWN);
  }
  if (k==UP) {

  }


}
void FlyByJoystick::JAB (KBSTATE k, float, float, int) {
  if (k==PRESS) {
    FlyByKeyboard::ABKey (std::string(),PRESS);
    FlyByKeyboard::ABKey (std::string(),DOWN);

  }
  if (k==DOWN) {
    FlyByKeyboard::ABKey (std::string(),DOWN);
  }
}

void FlyByJoystick::JAccelKey (KBSTATE k, float, float, int) {
  FlyByKeyboard::AccelKey (std::string(),k);
}
void FlyByJoystick::JDecelKey (KBSTATE k, float, float, int) {
  FlyByKeyboard::DecelKey (std::string(),k);
}

#endif

void FlyByJoystick::Execute() {
  static bool clamp_joystick_axes = XMLSupport::parse_bool (vs_config->getVariable("joystick","clamp_axes","true"));
  static bool nonlinear_throttle_nav    = XMLSupport::parse_bool (vs_config->getVariable("joystick","nonlinear_throttle_nav","true"));
  static bool nonlinear_throttle_combat = XMLSupport::parse_bool (vs_config->getVariable("joystick","nonlinear_throttle_combat","false"));
  static float expfactorn = XMLSupport::parse_float (vs_config->getVariable("joystick","nonlinear_expfactor_nav","6.0"));
  static float pfactorn = XMLSupport::parse_float (vs_config->getVariable("joystick","nonlinear_pfactor_nav","2.0"));
  static float expamountn = XMLSupport::parse_float (vs_config->getVariable("joystick","nonlinear_expamount_nav","1.0"));
  static float pamountn = XMLSupport::parse_float (vs_config->getVariable("joystick","nonlinear_pamount_nav","0.0"));
  static float expfactorc = XMLSupport::parse_float (vs_config->getVariable("joystick","nonlinear_expfactor_combat","6.0"));
  static float pfactorc = XMLSupport::parse_float (vs_config->getVariable("joystick","nonlinear_pfactor_combat","2.0"));
  static float expamountc = XMLSupport::parse_float (vs_config->getVariable("joystick","nonlinear_expamount_combat","1.0"));
  static float pamountc = XMLSupport::parse_float (vs_config->getVariable("joystick","nonlinear_pamount_combat","0.0"));
  desired_ang_velocity=Vector(0,0,0); 
  for (unsigned int i=0;i<this->whichjoystick.size();i++) {
  int which_joystick = this->whichjoystick[i];
  if (which_joystick<MAX_JOYSTICKS) {

    int joy_nr;

    joy_nr=vs_config->axis_joy[AXIS_Y];

    if (joy_nr!=-1) {
      joy_nr = which_joystick;
    }
    if( joy_nr!= -1 && joystick[joy_nr]->isAvailable()){
      int config_axis=joystick[joy_nr]->axis_axis[AXIS_Y];
      if (config_axis!=-1) {
      bool inverse=joystick[joy_nr]->axis_inverse[AXIS_Y];
      float axis_value=- joystick[joy_nr]->joy_axis[config_axis];
      if(inverse) { axis_value= - axis_value; }
	  if (clamp_joystick_axes) {
		  if (axis_value<-1)
			  axis_value=-1;
		  if (axis_value>1)
			  axis_value=1;
	  }
      if ((joy_mode==joyModeNormal)||(joy_mode&(joyModeRoll|joyModeBank)))
        Up(axis_value); else if (joy_mode&joyModeInertialXY)
        DirectThrustUp(axis_value); else if (joy_mode&joyModeInertialXZ)
        DirectThrustFront(-axis_value); 
      }
    }

    joy_nr=vs_config->axis_joy[AXIS_X];
    if (joy_nr!=-1) {
      joy_nr = which_joystick;
    }

    if( joy_nr != -1 && joystick[joy_nr]->isAvailable() ){
      int config_axis=joystick[joy_nr]->axis_axis[AXIS_X];
      if (config_axis!=-1) {
      bool inverse=joystick[joy_nr]->axis_inverse[AXIS_X];
      float axis_value=- joystick[joy_nr]->joy_axis[config_axis];
      if(inverse) { axis_value= - axis_value; }
	  if (clamp_joystick_axes) {
		  if (axis_value<-1)
			  axis_value=-1;
		  if (axis_value>1)
			  axis_value=1;
	  }
      if (joy_mode==joyModeNormal)
        Right(axis_value); else if (joy_mode&joyModeRoll)
        RollRight(-axis_value); else if (joy_mode&(joyModeInertialXY|joyModeInertialXZ))
        DirectThrustRight(axis_value); else if (joy_mode&joyModeBank)
        Right(axis_value),RollRight(-axis_value);
      }
    }

    joy_nr=vs_config->axis_joy[AXIS_Z];
    if (joy_nr!=-1) {
      joy_nr = which_joystick;
    }

    if( joy_nr!= -1 && joystick[joy_nr]->isAvailable()){
      int config_axis=joystick[joy_nr]->axis_axis[AXIS_Z];
      if (config_axis!=-1) {
      bool inverse=joystick[joy_nr]->axis_inverse[AXIS_Z];
      float axis_value=- joystick[joy_nr]->joy_axis[config_axis];
      if(inverse) { axis_value= - axis_value; }
	  if (clamp_joystick_axes) {
		  if (axis_value<-1)
			  axis_value=-1;
		  if (axis_value>1)
			  axis_value=1;
	  }
      if ((joy_mode==joyModeNormal)||(joy_mode&joyModeBank))
        RollRight(axis_value); else if (joy_mode&joyModeRoll)
        Right(axis_value); else if (joy_mode&joyModeInertialXY)
        DirectThrustFront(axis_value); else if (joy_mode&joyModeInertialXZ)
        DirectThrustUp(axis_value);
      }
    }

    joy_nr=vs_config->axis_joy[AXIS_THROTTLE];
    if (joy_nr!=-1) {
      joy_nr = which_joystick;
    }

    if( joy_nr != -1 &&  joystick[joy_nr]->isAvailable()){
      int config_axis=joystick[joy_nr]->axis_axis[AXIS_THROTTLE];
      if (config_axis!=-1) {
      bool inverse=joystick[joy_nr]->axis_inverse[AXIS_THROTTLE];
      float axis_value=- joystick[joy_nr]->joy_axis[config_axis];
      if(inverse) { axis_value= - axis_value; }
	  Unit::Computer *cpu = &parent->GetComputerData(); 

	  if (axis_value>1)
		  axis_value=1;
          /*
	  float minspeed = parent->Limits().retro/parent->Limits().forward;
	  static bool minzero=XMLSupport::parse_bool(vs_config->getVariable("joystick","zero_min_throttle","false"));
	  if (minzero)
		  minspeed = 0;
	  if (axis_value<minspeed)
		  axis_value=minspeed;
          */
          if (axis_value<-1) 
            axis_value = -1; //this code due to paolodinca
          
          // put axis from 0 to 2 
          axis_value++; 
          // put axis from 0 to 1 
          axis_value= axis_value / 2; //thanks!
          if(nonlinear_throttle_nav && !cpu->combat_mode) {
            static float norm = float(exp(expfactorn)-1);
            axis_value = float(expamountn * (exp(expfactorn*axis_value)-1) / norm + pamountn * pow(axis_value,pfactorn));
          } else if (nonlinear_throttle_combat && cpu->combat_mode) {
            static float norm = float(exp(expfactorc)-1);
            axis_value = float(expamountc * (exp(expfactorc*axis_value)-1) / norm + pamountc * pow(axis_value,pfactorc));
          }
	  cpu->set_speed=axis_value*cpu->max_speed();
	  desired_velocity= Vector (0,0,cpu->set_speed);
      }
    }

  }
  }
  FlyByKeyboard::Execute(false);
}
FlyByJoystick::~FlyByJoystick() {

}


