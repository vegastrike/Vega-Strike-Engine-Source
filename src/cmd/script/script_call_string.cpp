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
  xml Mission Scripting written by Alexander Rawass <alexannika@users.sourceforge.net>
*/

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <ctype.h>
#include <assert.h>
#ifndef WIN32
// this file isn't available on my system (all win32 machines?) i dun even know what it has or if we need it as I can compile without it
#include <unistd.h>
#endif

#include <expat.h>
#include "xml_support.h"

#include "vegastrike.h"

#include "mission.h"
#include "easydom.h"

//#include "vs_globals.h"
//#include "vegastrike.h"



varInst *Mission::call_string(missionNode *node,int mode){

  //varInst *viret=new varInst;
  varInst *viret=NULL;

  string cmd=node->attr_value("name");

  if(cmd=="new"){
    viret=call_string_new(node,mode,"");

    return viret;
  }
  else{
    varInst *ovi=getObjectArg(node,mode);
    string *my_string=getStringObject(node,mode,ovi);
    
    if(cmd=="print"){
      
      if(mode==SCRIPT_RUN){
	call_string_print(node,mode,ovi);
      }

      viret=new varInst;
      viret->type=VAR_VOID;
      return viret;
    }
    else{
      fatalError(node,mode,"unknown command "+cmd+" for callback string");
      assert(0);
    }
    
    return NULL; // never reach
  }
  return NULL; // never reach
}

string Mission::call_string_getstring(missionNode *node,int mode,varInst *ovi){
   if(ovi->type!=VAR_OBJECT || (ovi->type==VAR_OBJECT && ovi->objectname!="string")){
      fatalError(node,mode,"call_string_getstring needs string object as arg");
      assert(0);
    }

  string *my_string=getStringObject(node,mode,ovi);

  string ret=*my_string;

  return ret;
}

void Mission::call_string_print(missionNode *node,int mode,varInst *ovi){

  string *my_string =getStringObject(node,mode,ovi);

  cout << *my_string ;
}

varInst * Mission::call_string_new(missionNode *node,int mode,string initstring){
  debug(10,node,mode,"call_string");

	varInst *viret=new varInst;

	string* my_string=new string(initstring);

	viret->type=VAR_OBJECT;
	viret->objectname="string";
	viret->object=(void *)my_string;

	return viret;

}


string *Mission::getStringObject(missionNode *node,int mode,varInst *ovi){
   string *my_object=NULL;

	if(mode==SCRIPT_RUN){
	  my_object=(string *)ovi->object;
	  if(my_object==NULL){
	    fatalError(node,mode,"string: no object");
	    assert(0);
	  }
	}

	return(my_object);
 
}
