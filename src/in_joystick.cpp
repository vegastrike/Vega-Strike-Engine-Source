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
#include "vs_globals.h"

//#include "glob.h"
//#include "dbg.h"
#include "in_handler.h"
#include "in_joystick.h"
#include "config_xml.h"
#include "in_mouse.h"
JoyStick *joystick[MAX_JOYSTICKS]; // until I know where I place it

void InitJoystick(){
  int i;

  for (i=0;i<NUMJBUTTONS;i++) {
    for (int j=0;j<MAX_JOYSTICKS;j++) {
      UnbindJoyKey (j,i);
    }
  }
  for(int h=0;h<MAX_HATSWITCHES;h++){
    for(int v=0;v<MAX_VALUES;v++){
      UnbindHatswitchKey(h,v);
    }
  }
  for(int j=0;j<MAX_JOYSTICKS;j++){
    for(int h=0;h<MAX_DIGITAL_HATSWITCHES;h++){
      for(int v=0;v<MAX_DIGITAL_VALUES;v++){
	UnbindDigitalHatswitchKey(j,h,v);
      }
    }
  }


#ifdef HAVE_SDL
  int num_joysticks=SDL_NumJoysticks() ;
  printf("%i joysticks were found.\n\n", num_joysticks);
  printf("The names of the joysticks are:\n");
#endif

  for(i=0; i < MAX_JOYSTICKS; i++ )  {
#ifdef HAVE_SDL
    if (i<num_joysticks){
      //      SDL_EventState (SDL_JOYBUTTONDOWN,SDL_ENABLE);
      //      SDL_EventState (SDL_JOYBUTTONUP,SDL_ENABLE);
      printf("    %s\n", SDL_JoystickName(i));
    }
#endif
    joystick[i]=new JoyStick(i); // SDL_Init is done in main.cpp
  }
}

void DeInitJoystick() {
  for (int i=0;i<MAX_JOYSTICKS;i++) {
    delete joystick[i];
  }
}
JoyStick::JoyStick(int which): mouse(which==MOUSE_JOYSTICK) {
  player=which;//by default bind players to whichever joystick it is
  debug_digital_hatswitch=XMLSupport::parse_bool(vs_config->getVariable("joystick","debug_digital_hatswitch","false"));
  if (which!=MOUSE_JOYSTICK)
    deadzone=atoi(vs_config->getVariable("joystick","deadband","0.05").c_str());
  else
    deadzone=0;
  joy_available = 0;
  joy_x=joy_y=joy_z=0;
  if (which==MOUSE_JOYSTICK) {
    InitMouse(which);
  }
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
    nr_of_hats=SDL_JoystickNumHats(joy);

    printf("axes: %d buttons: %d hats: %d\n",nr_of_axes,nr_of_buttons,nr_of_hats);

#endif // we have SDL
}
void JoyStick::InitMouse (int which) {
  player=0;//default to first player
  joy_available=true;
  nr_of_axes=2;//x and y for mouse
  nr_of_buttons=15;
  nr_of_hats=0;
}

bool JoyStick::isAvailable(){
  return joy_available;
}

void JoyStick::GetMouse (float &x, float &y, float &z, int &buttons) {
  static int savex = g_game.x_resolution/2;
  static int savey = g_game.x_resolution/2;
  static bool warp_pointer = XMLSupport::parse_bool(vs_config->getVariable ("joystick","warp_mouse","false"));
  static float mouse_sensitivity = XMLSupport::parse_float(vs_config->getVariable ("joystick","mouse_sensitivity","5"));
  static float mouse_exp = XMLSupport::parse_float(vs_config->getVariable ("joystick","mouse_exponent","3"));
  int dx, dy;
  GetMouseDelta (dx,dy);
  joy_axis[0]=x = (float(dx))/(g_game.x_resolution/mouse_sensitivity);
  joy_axis[1]=y = (float(dy))/(g_game.y_resolution/mouse_sensitivity);

  joy_axis[0]*=mouse_exp;
  joy_axis[1]*=mouse_exp;
  x*=mouse_exp;
  y*=mouse_exp;
  joy_axis[2]=z=0;
  buttons = getMouseButtonStatus();
}
void JoyStick::GetJoyStick(float &x,float &y, float &z, int &buttons)
{

    int status;
    
    if(joy_available==false){
      for(int a=0;a<MAX_AXES;a++){
        joy_axis[a]=0;
      }
        x=y=z=0;
        joy_buttons=buttons=0;
        return;
    } else if (mouse) {
      GetMouse (x,y,z,buttons);
      return;
    }
#if defined(HAVE_SDL)

    int numaxes = SDL_JoystickNumAxes (joy);

    Sint16 axi[8];
    int a;
    for(a=0;a<numaxes;a++){
      axi[a] = SDL_JoystickGetAxis(joy,a);
    }

    buttons=0;
    nr_of_buttons=SDL_JoystickNumButtons(joy);

   for(int i=0;i<nr_of_buttons;i++){
     int  butt=SDL_JoystickGetButton(joy,i);
     if(butt==1){
       buttons|=(1<<i);
      }
   }
    joy_buttons = buttons;

    if(debug_digital_hatswitch){
      for(int h=0;h<nr_of_hats;h++){
	digital_hat[h]=SDL_JoystickGetHat(joy,h);
      }
    }

    for(a=0;a<numaxes;a++){
      joy_axis[a]=((float)axi[a]/32768.0);
      if(fabs(joy_axis[a])<=deadzone){
	joy_axis[a]=0.0;
      }
    }
    x=joy_axis[0];
    y=joy_axis[1];
    z=joy_axis[2];

#endif // we have SDL
    
    return;
}

int JoyStick::NumButtons(){
  return nr_of_buttons;
}
