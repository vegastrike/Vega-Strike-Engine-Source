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

#define NUM_BUTTONS 3

/** Gets the button number of the function used to draw the mouse*/
int getMouseDrawFunc() {
  return NUM_BUTTONS;
}
KBSTATE MouseState [NUM_BUTTONS+1]= {RELEASE};
static MouseHandler mouseBindings [NUM_BUTTONS+1];
int mousex=0;
int mousey=0;

void mouseClick( int button, int state, int x, int y ) {
  mousex = x;
  mousey = y;
  int mod =glutGetModifiers();
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

void mouseMotion(int x, int y) {
  //  int mod =glutGetModifiers();
  for (int i=0;i<NUM_BUTTONS+1;i++) {
    mouseBindings[i](MouseState[i],x,y,x-mousex,y-mousey,0);
  }
  mousex = x;
  mousey = y;
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
void BindKey (int key, MouseHandler handler) {
  mouseBindings[key]=handler;
  handler (RESET,mousex,mousey,0,0,0);
}
void InitMouse(){
  for (int a=0;a<NUM_BUTTONS+1;a++) {
    UnbindMouse (a);
  }
  glutMouseFunc(mouseClick);
  glutMotionFunc(mouseDrag);
  glutPassiveMotionFunc(mouseMotion);
}				
void ProcessMouse () {
  for (int a=0;a<NUM_BUTTONS+1;a++) {
    mouseBindings[a](MouseState[a],mousex,mousey,0,0,0);
  }

}

