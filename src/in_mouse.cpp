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

#include "vegastrike.h"
//#include "glob.h"
//#include "dbg.h"
#include "in_handler.h"
#include "in_mouse.h"
#include <deque>
#include "vs_globals.h"
#include "config_xml.h"
using std::deque;
#define NUM_BUTTONS 15

/** Gets the button number of the function used to draw the mouse*/
int getMouseDrawFunc() {
  return NUM_BUTTONS;
}
KBSTATE MouseState [NUM_BUTTONS+1]= {RELEASE};
static MouseHandler mouseBindings [NUM_BUTTONS+1];

int mousex=0;
int mousey=0;
int getMouseButtonStatus() {
  int ret=0;
  for (int i=0;i<NUM_BUTTONS;i++) {
    ret |= (MouseState[i]==PRESS||MouseState[i]==DOWN)?(1<<i):0;
  }
  return ret;
}

struct MouseEvent {
  enum EventType { CLICK, DRAG, MOTION } type;
  int button;
  int state;
  int mod;
  int x;
  int y;
  MouseEvent(EventType type, int button, int state, int mod, int x, int y) : type(type), button(button), state(state), mod(mod), x(x), y(y) { }
};

static deque<MouseEvent> eventQueue;
void mouseClickQueue(int button, int state, int x, int y) {
  int mod = glutGetModifiers();
  eventQueue.push_back(MouseEvent(MouseEvent::CLICK, button, state, mod, x, y));
}
void mouseDragQueue(int x, int y) {
  eventQueue.push_back(MouseEvent(MouseEvent::DRAG, -1, -1, -1, x, y));
}
void mouseMotionQueue(int x, int y) {
  eventQueue.push_back(MouseEvent(MouseEvent::MOTION, -1, -1, -1, x, y));
}

void mouseClick( int button, int state, int x, int y ) {
  int mod = glutGetModifiers();
  if(button>=NUM_BUTTONS) return;
  mousex = x;
  mousey = y;
  mouseBindings[button](state==GLUT_DOWN?PRESS:RELEASE,x,y,0,0,mod);
  MouseState[button]=(state==GLUT_DOWN)?DOWN:UP;
}

void mouseClick0( int button, int state, int mod, int x, int y ) {
  if(button>=NUM_BUTTONS) return;
  mousex = x;
  mousey = y;
  mouseBindings[button](state==GLUT_DOWN?PRESS:RELEASE,x,y,0,0,mod);
  MouseState[button]=(state==GLUT_DOWN)?DOWN:UP;
}

void  mouseDrag( int x, int y ) {
  //  int mod =glutGetModifiers();
  for (int i=0;i<NUM_BUTTONS+1;i++) {
    mouseBindings[i](MouseState[i],x,y,x-mousex,y-mousey,0);
  }

  mousex = x;
  mousey = y;
  
}	
bool warpedmouse=true;
void mouseMotion(int x, int y) {
  static bool warp_pointer = XMLSupport::parse_bool(vs_config->getVariable ("joystick","warp_mouse","false"));
  static int mouse_warp_zone = XMLSupport::parse_int(vs_config->getVariable ("joystick","warp_mouse_zone","100"));
  //  int mod =glutGetModifiers();
  for (int i=0;i<NUM_BUTTONS+1;i++) {
    mouseBindings[i](MouseState[i],x,y,x-mousex,y-mousey,0);
  }
 if (warp_pointer) {
    if (x<mouse_warp_zone||y<mouse_warp_zone||x>g_game.x_resolution-mouse_warp_zone||y>g_game.y_resolution-mouse_warp_zone) {
    warpMousePointer(g_game.x_resolution/2,g_game.y_resolution/2);
    warpedmouse=true;
    x= x-mousex+g_game.x_resolution/2;
    y= y-mousey+g_game.y_resolution/2;
  }
  mousex = x;
  mousey = y;

  }
}




/**
GLUT_ACTIVE_SHIFT 
    Set if the Shift modifier or Caps Lock is active. 
GLUT_ACTIVE_CTRL 
    Set if the Ctrl modifier is active. 
GLUT_ACTIVE_ALT 
    Set if the Alt modifier is active. 
*/
static void DefaultMouseHandler (KBSTATE, int x, int y, int delx, int dely,int mod) {
  return;
}

void UnbindMouse (int key) {
  mouseBindings[key]=DefaultMouseHandler;

}
void warpMousePointer(int x, int y) {
  glutWarpPointer(x,y);
}
void BindKey (int key, MouseHandler handler) {
  mouseBindings[key]=handler;
  handler (RESET,mousex,mousey,0,0,0);
}
void RestoreMouse() {
  glutMouseFunc(mouseClickQueue);
  glutMotionFunc(mouseDragQueue);
  glutPassiveMotionFunc(mouseMotionQueue);

}

void InitMouse(){
  for (int a=0;a<NUM_BUTTONS+1;a++) {
    UnbindMouse (a);
  }
  /*
  glutMouseFunc(mouseClick);
  glutMotionFunc(mouseDrag);
  glutPassiveMotionFunc(mouseMotion);
  */
  RestoreMouse();
}
				
void ProcessMouse () {
  while(eventQueue.size()) {
    MouseEvent e = eventQueue.front();
    switch(e.type) {
    case MouseEvent::CLICK:
      mouseClick0(e.button, e.state, e.mod, e.x, e.y);
      break;
    case MouseEvent::DRAG:
      mouseDrag(e.x, e.y);
      break;
    case MouseEvent::MOTION:
      mouseMotion(e.x, e.y);
      break;
    }
    eventQueue.pop_front();
  }
  /*
  for (int a=0;a<NUM_BUTTONS+1;a++) {
    mouseBindings[a](MouseState[a],mousex,mousey,0,0,0);
    }*/
}

