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
#include <stdio.h>
#define NUM_BUTTONS 3
KBSTATE MouseState [NUM_BUTTONS]= {RELEASE};
static MouseHandler mouseBindings [NUM_BUTTONS];
int mousex=0;
int mousey=0;

void mouseClick( int button, int state, int x, int y ) {
  mousex = x;
  mousey = y;
  mouseBindings[button](state==GLUT_DOWN?PRESS:RELEASE,x,y,0,0);
  MouseState[button]=(state==GLUT_DOWN)?DOWN:UP;
}

void  mouseDrag( int x, int y ) {
  for (int i=0;i<NUM_BUTTONS;i++) {
    mouseBindings[i](MouseState[i],x,y,x-mousex,y-mousey);
  }

  mousex = x;
  mousey = y;
  
}	

void mouseMotion(int x, int y) {
  //draw cursor?  or update position??
  for (int i=0;i<NUM_BUTTONS;i++) {
    mouseBindings[i](MouseState[i],x,y,x-mousex,y-mousey);
  }
  mousex = x;
  mousey = y;
}


static void DefaultMouseHandler (KBSTATE, int x, int y, int delx, int dely) {

  fprintf (stderr,"good");
  return;
}

void UnbindMouse (int key) {
  mouseBindings[key]=DefaultMouseHandler;

}
void BindKey (int key, MouseHandler handler) {
  mouseBindings[key]=handler;
  handler (RESET,mousex,mousey,0,0);
}
void InitMouse(){
  for (int a=0;a<NUM_BUTTONS;a++) {
    UnbindMouse (a);
  }
  glutMouseFunc(mouseClick);
  glutMotionFunc(mouseDrag);
  glutPassiveMotionFunc(mouseMotion);
}				
void ProcessMouse () {
  for (int a=0;a<NUM_BUTTONS;a++) {
    mouseBindings[a](MouseState[a],mousex,mousey,0,0);
  }

}

