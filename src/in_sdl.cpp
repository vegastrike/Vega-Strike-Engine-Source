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
  JoystickBindings[joystick][key]=DefaultJoyHandler;
  JoystickState[joystick][key]=UP;
}
void BindJoyKey (int key, int joystick, JoyHandler handler) {
  JoystickBindings[joystick][key]=handler;
  handler (RESET,0,0,0);
}

void ProcessJoystick () {
  SDL_Event event;
  float x,y;
  int buttons;
  while(SDL_PollEvent(&event)){
    switch(event.type){
    case SDL_JOYBUTTONDOWN:
      if (event.jbutton.which<NUMJBUTTONS) {
	if (joystick[event.jbutton.which]->isAvailable()) {
	  joystick[event.jbutton.which]->GetJoyStick (x,y,buttons);
	}
	JoystickBindings[event.jbutton.which][event.jbutton.button](PRESS,x,y,buttons);
	JoystickState[event.jbutton.which][event.jbutton.button]=DOWN;
      }
      break;
    case SDL_JOYBUTTONUP:
      if (event.jbutton.which<NUMJBUTTONS) {
	if (joystick[event.jbutton.which]->isAvailable()) {
	  joystick[event.jbutton.which]->GetJoyStick (x,y,buttons);
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
  for (int i=0;i<MAX_JOYSTICKS;i++) {
    if (joystick[event.jbutton.which]->isAvailable()) {
      joystick[event.jbutton.which]->GetJoyStick (x,y,buttons);
    }
    for (int j=0;j<NUMJBUTTONS;j++) {
      (*JoystickBindings [i][j])(JoystickState[i][j],x,y,buttons);
    }
  }
}
#endif
