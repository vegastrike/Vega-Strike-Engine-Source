/* 
 * Vega Strike
 * Copyright (C) 2001-2002 Daniel Horn
 * 
 * http://vegastrike.sourceforge.net/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

/*
  Joystick support written by Alexander Rawass <alexannika@users.sourceforge.net>
*/

#include "vegastrike.h"
//#include "glob.h"
//#include "dbg.h"
#include "in_handler.h"
#include "in_joystick.h"

JoyStick *joystick[MAX_JOYSTICKS]; // until I know where I place it

void InitJoystick(){
  //  SDL_EventState (SDL_KEYDOWN,SDL_ENABLE);
  //  SDL_EventState (SDL_KEYUP,SDL_ENABLE);
  for (int i=0;i<NUMJBUTTONS;i++) {
    for (int j=0;j<MAX_JOYSTICKS;j++) {
      UnbindJoyKey (j,i);
    }
  }
#ifdef HAVE_SDL
  int num_joysticks=SDL_NumJoysticks() ;
  printf("%i joysticks were found.\n\n", num_joysticks);
  printf("The names of the joysticks are:\n");
  for(int i=0; i < MAX_JOYSTICKS; i++ )  {
    if (i<num_joysticks){
      //      SDL_EventState (SDL_JOYBUTTONDOWN,SDL_ENABLE);
      //      SDL_EventState (SDL_JOYBUTTONUP,SDL_ENABLE);
      printf("    %s\n", SDL_JoystickName(i));
    }
    joystick[i]=new JoyStick(i); // SDL_Init is done in main.cpp
  }
  
#endif
}

void DeInitJoystick() {
#ifdef HAVE_SDL
  int num_joysticks = SDL_NumJoysticks();
  for (int i=0;i<MAX_JOYSTICKS;i++) {
    delete joystick[i];
  }
#endif
}
/*
void BindButton(int button,KBHandler handler){
// have to check if button>allowed
joyBindings[button]=handler;
handler(button,RESET);
}

static void DefaultJoyHandler(int button,KBSTATE state){
// do nothing
return;
}

void UnbindButton(int button) {
joyBindings[button] = DefaultJoyHandler;
}
*/

// we don't need code here cause we don't queue events
/*
// not needed, this is done in FlyByJoystick
  int num_joysticks=SDL_NumJoysticks() ;
  for(int i=0; i < num_joysticks; i++ )  {
    if(joystick[i]->isAvailable()){
      float x,y;
      int buttons;
      joystick[i]->GetJoyStick(x,y,buttons);
    }
  }

}

#endif
*/
JoyStick::JoyStick(int which) {
  deadzone=0.01;

  joy_available = 0;

#if !defined(HAVE_SDL)
  return;
#else
  int num_joysticks=SDL_NumJoysticks() ;
  if (which>=num_joysticks)
    return;
  
  //    SDL_JoystickEventState(SDL_ENABLE);
  joy=SDL_JoystickOpen(which);  // joystick nr should be configurable
  
  if(joy==NULL)
    {
      printf("warning: no joystick nr %d\n",which);
      joy_available = false;
      return;
    }
  
    joy_available=true;

    nr_of_axes=SDL_JoystickNumAxes(joy);
    nr_of_buttons=SDL_JoystickNumButtons(joy);

    printf("axes: %d buttons %d\n",nr_of_axes,nr_of_buttons);
    joy_xmin = (float) -1;
    joy_xmax = (float) 1;
    joy_ymin = (float) -1;
    joy_ymax = (float) 1;
    joy_zmin = (float) -1;
    joy_zmax = (float) 1;

#endif // we have SDL
}

bool JoyStick::isAvailable(){
return joy_available;
}

void JoyStick::GetJoyStick(float &x,float &y, float &z, int &buttons)
{

    int status;
    
    if(joy_available==false){
        joy_x= x=0;
        joy_y=y=0;
	joy_z=z=0;
        joy_buttons=buttons=0;
        return;
    }
#if defined(HAVE_SDL)

    int numaxes = SDL_JoystickNumAxes (joy);
    Sint16 xi =  SDL_JoystickGetAxis(joy,0);
    Sint16 yi =  SDL_JoystickGetAxis(joy,1);
    Sint16 zi=0;
if (numaxes>2) {
      zi = SDL_JoystickGetAxis(joy,2);
    }
    buttons=0;
    nr_of_buttons=SDL_JoystickNumButtons(joy);
   for(int i=0;i<nr_of_buttons;i++){
     int  butt=SDL_JoystickGetButton(joy,i);
     fprintf (stderr,"button %d  %d\n",i,butt);
     if(butt==1){
       buttons|=(1<<i);
      }
   }
    joy_buttons = buttons;

    joy_x=x=((float)xi/32768.0);
    joy_y=y=((float)yi/32768.0);
    joy_z=z=((float)zi/32768.0);
    //    printf("x=%f   y=%f buttons=%d\n",x,y,buttons);

    if(fabs(x)<=deadzone){
        joy_x =x=0;
    }
    if(fabs(y)<=deadzone){
        joy_y = y=0;
    }
    if(fabs(z)<=deadzone){
        joy_z = z=0;
    }
#endif // we have SDL
    
    return;
}

int JoyStick::NumButtons(){
  return nr_of_buttons;
}
