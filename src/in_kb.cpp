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

#include <queue>
#include <list> 
//#include "glob.h"
#include "vegastrike.h"
#include "in_kb.h"
#include "in_handler.h"
//#include "cmd_unit.h"

extern queue<InputListener*> activationreqqueue;
extern list<InputListener*> listeners;

extern InputListener* activelistener;


static KBHandler keyBindings [KEYMAP_SIZE];
KBSTATE keyState [KEYMAP_SIZE];

static void kbGetInput(int key, int special, int release, int x, int y){
  if ((keyState[key]==RESET||keyState[key]==UP)&&!release)
    keyBindings[key](key,PRESS);
  if ((keyState[key]==DOWN||keyState[key]==RESET)&&release)
    keyBindings[key](key,RELEASE);
  keyState[key] = release?UP:DOWN;
}

 void glut_keyboard_cb( unsigned char ch, int x, int y ) 
{
    kbGetInput( ch, 0, 0, x, y );
}

static void glut_special_cb( int key, int x, int y ) 
{
    kbGetInput( key, 1, 0, x, y );
}

static void glut_keyboard_up_cb( unsigned char ch, int x, int y ) 
{
    kbGetInput( ch, 0, 1, x, y );
}

static void glut_special_up_cb( int key, int x, int y ) 
{
    kbGetInput( key, 1, 1, x, y );
}


void InitKB()
{
  for(int a=0; a<KEYMAP_SIZE; a++) {
    keyState[a] = UP;
    UnbindKey(a);
  }

  glutKeyboardFunc( glut_keyboard_cb );

  glutKeyboardUpFunc( glut_keyboard_up_cb );
  glutSpecialFunc( glut_special_cb );
  glutSpecialUpFunc( glut_special_up_cb );
}


void ProcessKB()
{
  /*  if(!activationreqqueue.empty()) {
    InputListener *newactive = NULL;
    list<InputListener*>::const_iterator li_it = listeners.begin();
    float min = FLT_MAX;
    
    while(li_it!=listeners.end()) { //pick the one with lowest z
      float curr_z = (*li_it)->parent->Position().k;
      if(curr_z<min ) {
	min = curr_z;
	newactive = *li_it;
      }
    }
    if(newactive!=NULL)
      activelistener = newactive;
    //empty & analyze to see which one deserves to be activated
    }*/
  for(int a=0; a<KEYMAP_SIZE; a++) {
    keyBindings[a](a,keyState[a]);
  }
}	

void BindKey(int key, KBHandler handler) {
	keyBindings[key] = handler;
	handler(-1,RESET); // key is not used in handler
}

static void DefaultKBHandler(int key, KBSTATE newState) {
	// do nothing
	return;
}

void UnbindKey(int key) {
  keyBindings[key] = DefaultKBHandler;
}

