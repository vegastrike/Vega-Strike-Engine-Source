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
#include "vs_globals.h"
#include "in_kb.h"
#include "in_handler.h"
#include "gldrv/winsys.h"
//#include "cmd_unit.h"
/*
extern queue<InputListener*> activationreqqueue;
extern list<InputListener*> listeners;

extern InputListener* activelistener;
*/
static void DefaultKBHandler(const std::string&,KBSTATE newState) {
	// do nothing
	return;
}
struct HandlerCall{
  KBHandler function;
  std::string data;
  HandlerCall() {
    function=DefaultKBHandler;
  }
};
static HandlerCall keyBindings [LAST_MODIFIER][WSK_LAST];
static unsigned int playerBindings [LAST_MODIFIER][WSK_LAST];
KBSTATE keyState [LAST_MODIFIER][WSK_LAST];

static void kbGetInput(int key, int modifiers, bool release, int x, int y){
  int i=_Universe->CurrentCockpit();
  _Universe->SetActiveCockpit(playerBindings[modifiers][key]);


  if ((keyState[modifiers][key]==RESET||keyState[modifiers][key]==UP)&&!release)
    keyBindings[modifiers][key].function(keyBindings[modifiers][key].data,PRESS);
  if ((keyState[modifiers][key]==DOWN||keyState[modifiers][key]==RESET)&&release)
    keyBindings[modifiers][key].function(keyBindings[modifiers][key].data,RELEASE);
  keyState[modifiers][key] = release?UP:DOWN;
  _Universe->SetActiveCockpit(i);
}
int shiftup (int ch) {
  switch (ch) {
  case '1':
    return '!';
  case '2':
    return '@';
  case '3':
    return '#';
  case '4':
    return '$';
  case '5':
    return '%';
  case '6':
    return '^';
  case '7':
    return '&';
  case '8':
    return '*';
  case '9':
    return '(';
  case '0':
    return ')';
  case '-':
    return '_';
  case '=':
    return '+';
  case '\\':
    return '|';
  case '\'':
    return '"';
  case ';':
    return ':';
  case '/':
    return '?';
  case '.':
    return '>';
  case ',':
    return '<';
  case '`':
    return '~';
  }
  return toupper(ch);
}

int shiftdown (int ch) {
  switch (ch){
  case '!':
    return '1';
  case '@':
    return '2';
  case '#':
    return '3';
  case '$':
    return '4'; 
  case '%':
    return '5';
  case '^':
    return '6';
  case '&':
    return '7';
  case '*':
    return '8';
  case '(':
    return '9';
  case ')':
    return '0';
  case '_':
    return '-';
  case '+':
    return '=';
  case '|':
    return '\\';
  case '"':
    return '\'';
  case ';':
    return ':';
  case '?':
    return '/';
  case '>':
    return '.';
  case '<':
    return ',';
  case '~':
    return '`';
  }
  return tolower(ch);
}

unsigned int getModifier(const char* mod_name){
  if (mod_name[0]=='\0')
    return 0;
  if (strstr(mod_name,"ctrl")||strstr(mod_name,"cntrl")||strstr(mod_name,"control"))
    return 2;
  if (strstr(mod_name,"alt")||strstr(mod_name,"alternate"))
    return 1;
  return 0;
}
int getModifier(bool alton, bool cntrlon) {
  return cntrlon?2:(alton?1:0);
}
 void glut_keyboard_cb( unsigned int  ch,unsigned int mod, bool release, int x, int y ) 
{
  bool shifton=false;
  int alton=false;
  int ctrlon=false;
  
  //  VSFileSystem::Fprintf (stderr,"keyboard  %d",ch);
  if ((WSK_MOD_LSHIFT==(mod&WSK_MOD_LSHIFT))||(WSK_MOD_RSHIFT==(mod&WSK_MOD_RSHIFT))) {
    ch = shiftup(ch);
    shifton=true;
  }
  if ((WSK_MOD_LALT==(mod&WSK_MOD_LALT))||(WSK_MOD_RALT==(mod&WSK_MOD_RALT))) {
    alton=true;
  }
  if ((WSK_MOD_LCTRL==(mod&WSK_MOD_LCTRL))||(WSK_MOD_RCTRL==(mod&WSK_MOD_RCTRL))) {
    ctrlon=true;
  }
  int curmod=getModifier(alton,ctrlon);
  kbGetInput( ch, curmod,release, x, y );
  if (release) {
    for (int i=0;i<LAST_MODIFIER;++i) {
      if (i!=curmod){
        if(keyState[i][shiftdown(ch)]==DOWN)
          kbGetInput (shiftdown(ch),i,release,x,y);
        if(keyState[i][shiftup(ch)]==DOWN)
          kbGetInput (shiftup(ch),i,release,x,y);
      }else{
        if (shifton) {
          if (((unsigned int)shiftdown (ch))!=ch&&keyState[i][shiftdown(ch)]==DOWN) {
            kbGetInput (shiftdown(ch),i,release,x,y);
          }
        }else {
          if (((unsigned int)shiftup (ch))!=ch&&keyState[i][shiftup(ch)]==DOWN) {
            kbGetInput (shiftup(ch),i,release,x,y);
          }
        }
      }
    }
  }
}
/*
static void glut_special_cb( int key, int x, int y ) 
{
  //  VSFileSystem::Fprintf (stderr,"keyboard s %d",key);
    kbGetInput( 128+key, 1, 0, x, y );
}

static void glut_keyboard_up_cb( unsigned char ch, int x, int y ) 
{
  //  VSFileSystem::Fprintf (stderr,"keyboard up %d",ch);
    kbGetInput( ch, 0, 1, x, y );
}

static void glut_special_up_cb( int key, int x, int y ) 
{
  //  VSFileSystem::Fprintf (stderr,"keyboard s up %d",key);
    kbGetInput( 128+key, 1, 1, x, y );
}
*/
void RestoreKB() {
  for (int i=0;i<LAST_MODIFIER;++i) {
    for(int a=0; a<KEYMAP_SIZE; a++) {
      if (keyState[i][a]==DOWN) {
        keyBindings[i][a].function(keyBindings[i][a].data,RELEASE);      
        keyState[i][a] = UP;
      }
    }
  }
  winsys_set_keyboard_func( glut_keyboard_cb );
}

void InitKB()
{
  for (int i=0;i<LAST_MODIFIER;++i) {
    for(int a=0; a<KEYMAP_SIZE; a++) {
      keyState[i][a] = UP;
      UnbindKey(a,i);
    }
  }
  RestoreKB();
}


void ProcessKB(unsigned int player)
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
  for(int mod=0; mod<LAST_MODIFIER; mod++) {
    for(int a=0; a<KEYMAP_SIZE; a++) {
      if (playerBindings[mod][a]==player)
        keyBindings[mod][a].function(keyBindings[mod][a].data,keyState[mod][a]);
    }
  }
}	

void BindKey(int key,unsigned int mod, unsigned int player, KBHandler handler, const std::string&data) {
	keyBindings[mod][key].function = handler;
	keyBindings[mod][key].data = data;
	playerBindings[mod][key]=player;
	handler(std::string(),RESET); // key is not used in handler
}


void UnbindKey(int key,unsigned int mod) {
  keyBindings[mod][key] = HandlerCall();
}

