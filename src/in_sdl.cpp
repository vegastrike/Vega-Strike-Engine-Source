#include "SDL.h"
#include "in_joystick.h"
#ifdef HAVE_SDL

//static KBHandler keyBindings [SDLK_LAST];
//KBSTATE keyState [SDLK_LAST];
JoyHandler JoystickBindings [MAX_JOYSTICKS][NUMJBUTTONS];
KBSTATE JoystickState [MAX_JOYSTICKS][NUMJBUTTONS];
void DefaultJoyHandler (KBSTATE st, float x, float y, int mod) {
  //  fprintf (stderr,"STATE: %d", st);
}

void UnbindJoyKey (int joystick, int key) {
  assert (key<NUMJBUTTONS&&joystick<MAX_JOYSTICKS);
  JoystickBindings[joystick][key]=DefaultJoyHandler;
  JoystickState[joystick][key]=UP;
}
void BindJoyKey (int key, int joystick, JoyHandler handler) {
  assert (key<NUMJBUTTONS&&joystick<MAX_JOYSTICKS);
  JoystickBindings[joystick][key]=handler;
  handler (RESET,0,0,0);
}

void ProcessJoystick () {
  SDL_JoystickUpdate();//FIXME isn't this supposed to be called already by SDL?
  SDL_Event event;
  float x,y,z;
  int buttons;


#ifdef SDLEVENTSNOW
  while(SDL_PollEvent(&event)){
    switch(event.type){
    case SDL_JOYBUTTONDOWN:
      if (event.jbutton.which<NUMJBUTTONS) {
	if (joystick[event.jbutton.which]->isAvailable()) {
	  joystick[event.jbutton.which]->GetJoyStick (x,y,z,buttons);
	}
	JoystickBindings[event.jbutton.which][event.jbutton.button](PRESS,x,y,buttons);
	JoystickState[event.jbutton.which][event.jbutton.button]=DOWN;
      }
      break;
    case SDL_JOYBUTTONUP:
      if (event.jbutton.which<NUMJBUTTONS) {
	if (joystick[event.jbutton.which]->isAvailable()) {
	  joystick[event.jbutton.which]->GetJoyStick (x,y,z,buttons);
	}
	JoystickBindings[event.jbutton.which][event.jbutton.button](RELEASE,x,y,buttons);
	JoystickState[event.jbutton.which][event.jbutton.button]=UP;
      }
      break;
    case SDL_KEYDOWN:
      /*soon...soooooonn
      keyState[event.key.keysym.sym]=PRESS;
      keyBindings[event.key.keysym.sym](event.key.keysym.sym,PRESS);
      keyState[event.key.keysym.sym]=DOWN;
      */
      break;
    case SDL_KEYUP:
      /*
      keyState[event.key.keysym.sym]=RELEASE;
      keyBindings[event.key.keysym.sym](event.key.keysym.sym,RELEASE);
      keyState[event.key.keysym.sym]=UP;
      */
      break;
    }
  }
#endif
  for (int i=0;i<MAX_JOYSTICKS;i++) {
    buttons=0;
    if (joystick[i]->isAvailable()) {
      joystick[i]->GetJoyStick (x,y,z,buttons);
    }
    for (int j=0;j<NUMJBUTTONS;j++) {
      if (buttons&(1<<j)) {
	if (JoystickState[i][j]==UP) {
	  (*JoystickBindings [i][j])(PRESS,x,y,buttons);		  
	  JoystickState[i][j]=DOWN;
	}else {
	  if (JoystickState[i][j]==DOWN) {
	    (*JoystickBindings [i][j])(RELEASE,x,y,buttons);		    
	  }
	  JoystickState[i][j]=UP;
	}
      }
      (*JoystickBindings [i][j])(JoystickState[i][j],x,y,buttons);	
    }
  }
}
#endif
