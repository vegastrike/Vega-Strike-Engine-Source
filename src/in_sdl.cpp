#include "in_joystick.h"
#include "vs_globals.h"
#include "config_xml.h"

#include <assert.h>	/// needed for assert() calls.

KBHandler JoystickBindings [MAX_JOYSTICKS][NUMJBUTTONS];
KBSTATE JoystickState [MAX_JOYSTICKS][NUMJBUTTONS];

KBHandler HatswitchBindings [MAX_HATSWITCHES][MAX_VALUES];
KBSTATE HatswitchState [MAX_HATSWITCHES][MAX_VALUES];


KBHandler DigHatswitchBindings [MAX_JOYSTICKS][MAX_DIGITAL_HATSWITCHES][MAX_DIGITAL_VALUES];
KBSTATE DigHatswitchState [MAX_JOYSTICKS][MAX_DIGITAL_HATSWITCHES][MAX_DIGITAL_VALUES];

void DefaultJoyHandler (int key, KBSTATE newState) {
  //  VSFileSystem::Fprintf (stderr,"STATE: %d", st);
}

void UnbindJoyKey (int joystick, int key) {
  assert (key<NUMJBUTTONS&&joystick<MAX_JOYSTICKS);
  JoystickBindings[joystick][key]=DefaultJoyHandler;
  JoystickState[joystick][key]=UP;
}

void BindJoyKey (int joystick, int key, KBHandler handler) {
  assert (key<NUMJBUTTONS&&joystick<MAX_JOYSTICKS);
  JoystickBindings[joystick][key]=handler;
  handler (0,RESET);
}

void UnbindHatswitchKey (int hatswitch, int val_index) {
  assert (hatswitch<MAX_HATSWITCHES && val_index<MAX_VALUES);
  HatswitchBindings[hatswitch][val_index]=DefaultJoyHandler;
  //  JoystickState[joystick][key]=UP;
}

void BindHatswitchKey (int hatswitch, int val_index, KBHandler handler) {
  assert (hatswitch<MAX_HATSWITCHES && val_index<MAX_VALUES);
  HatswitchBindings[hatswitch][val_index]=handler;
  handler (0,RESET);
}

void BindDigitalHatswitchKey (int joy_nr,int hatswitch, int dir_index, KBHandler handler) {
  assert (hatswitch<MAX_DIGITAL_HATSWITCHES && dir_index<MAX_DIGITAL_VALUES);
  DigHatswitchBindings[joy_nr][hatswitch][dir_index]=handler;
  handler (0,RESET);
}

void UnbindDigitalHatswitchKey (int joy_nr,int hatswitch, int dir_index) {
  assert (hatswitch<MAX_DIGITAL_HATSWITCHES && dir_index<MAX_DIGITAL_VALUES);
 DigHatswitchBindings[joy_nr][hatswitch][dir_index]=DefaultJoyHandler;
  //  JoystickState[joystick][key]=UP;
}


void ProcessJoystick (int whichplayer) {
  float x,y,z;
  int buttons;
#ifdef HAVE_SDL
#ifndef NO_SDL_JOYSTICK
  SDL_JoystickUpdate();//FIXME isn't this supposed to be called already by SDL?
#endif
#endif
  for (int i=whichplayer;i<whichplayer+1&&i<MAX_JOYSTICKS;i++) {
    buttons=0;
    if(joystick[i]->isAvailable()){
      joystick[i]->GetJoyStick (x,y,z,buttons);

      if(joystick[i]->debug_digital_hatswitch){
      for(int h=0;h<joystick[i]->nr_of_hats;h++){
#ifdef HAVE_SDL
	Uint8 
#else
	  unsigned char
#endif
	  hsw=joystick[i]->digital_hat[h];
	char buf[100];
	sprintf(buf,"hsw: %d",hsw);
	cout << buf << endl;

	for(int dir_index=0;dir_index<MAX_DIGITAL_VALUES;dir_index++){
	  bool press=false;
#ifdef HAVE_SDL
#ifndef NO_SDL_JOYSTICK
	  if(dir_index==VS_HAT_CENTERED && (hsw & SDL_HAT_CENTERED)){
	    cout << "center" << endl;
	    press=true;
	  }
	  if(dir_index==VS_HAT_LEFT && (hsw & SDL_HAT_LEFT)){
	    press=true;
	  }
	  if(dir_index==VS_HAT_RIGHT && (hsw & SDL_HAT_RIGHT)){
	    press=true;
	  }
	  if(dir_index==VS_HAT_DOWN && (hsw & SDL_HAT_DOWN)){
	    press=true;
	  }
	  if(dir_index==VS_HAT_UP && (hsw & SDL_HAT_UP)){
	    press=true;
	  }
	  if(dir_index==VS_HAT_RIGHTUP && (hsw & SDL_HAT_RIGHTUP)){
	    press=true;
	  }
	  if(dir_index==VS_HAT_RIGHTDOWN && (hsw & SDL_HAT_RIGHTDOWN)){
	    press=true;
	  }
	  if(dir_index==VS_HAT_LEFTUP && (hsw & SDL_HAT_LEFTUP)){
	    press=true;
	  }
	  if(dir_index==VS_HAT_LEFTDOWN && (hsw & SDL_HAT_LEFTDOWN)){
	    press=true;
	  }
#endif
#endif
	  if(press==true){
	    if(DigHatswitchState[i][h][dir_index]==UP){
	      (*DigHatswitchBindings[i][h][dir_index])(0,PRESS);
	      DigHatswitchState[i][h][dir_index]=DOWN;
	    }
	  }
	  else{
	    if(DigHatswitchState[i][h][dir_index]==DOWN){
	      (*DigHatswitchBindings[i][h][dir_index])(0,RELEASE);
	    }
	    DigHatswitchState[i][h][dir_index]=UP;
	  }
	  (*DigHatswitchBindings[i][h][dir_index])(0,DigHatswitchState[i][h][dir_index]);
	}
      }
      } // digital_hatswitch

      for (int j=0;j<NUMJBUTTONS;j++) {
	if ((buttons&(1<<j))) {
	  if (JoystickState[i][j]==UP) {
	    (*JoystickBindings [i][j])(0,PRESS);
	    JoystickState[i][j]=DOWN;
	  }
	}else {
	  if (JoystickState[i][j]==DOWN) {
	    (*JoystickBindings [i][j])(0,RELEASE);
	  }
	  JoystickState[i][j]=UP;
	}
	(*JoystickBindings [i][j])(0,JoystickState[i][j]);
      }
    } // is available
  } // for nr joysticks


  for(int h=0;h<MAX_HATSWITCHES;h++){
    float margin=fabs(vs_config->hatswitch_margin[h]);
      if(margin<1.0){
	// we have hatswitch nr. h
	int hs_axis=vs_config->hatswitch_axis[h];
	int hs_joy=vs_config->hatswitch_joystick[h];

	if(joystick[hs_joy]->isAvailable()){
	  float axevalue=joystick[hs_joy]->joy_axis[hs_axis];
	  
	  for(int v=0;v<MAX_VALUES;v++){
	    float hs_val=vs_config->hatswitch[h][v];
	    if(fabs(hs_val)<=1.0){
	      // this is set
	      KBHandler handler=(*HatswitchBindings[h][v]);
	      if(hs_val-margin<=axevalue && axevalue<=hs_val+margin){
		// hatswitch pressed
		
		if(HatswitchState[h][v]==UP){
		  handler(0,PRESS);
		  HatswitchState[h][v]=DOWN;
		}
	      }
	      else{
		// not pressed
		if(HatswitchState[h][v]==DOWN){
		  handler(0,RELEASE);
		}
		HatswitchState[h][v]=UP;
	      }
	      handler(0,HatswitchState[h][v]);
	    }
	  } // for all values
	} // is available
      }
  }


}

