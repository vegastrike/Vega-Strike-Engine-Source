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


varInst *Mission::doCall(missionNode *node,int mode){
  if(mode==SCRIPT_PARSE){
    string name=node->attr_value("name");
    if(name.empty()){
      fatalError(node,mode,"you have to give a callback name");
      assert(0);
    }
    node->script.name=name;
  }

  varInst *vi=NULL;

  if(node->script.name=="PrintFloats"){
    vi=callPrintFloats(node,mode);
  }
  else if(node->script.name=="Rnd"){
    vi=callRnd(node,mode);
  }
  else if(node->script.name=="olist"){
    vi=call_olist(node,mode);
  }

  if(vi==NULL){
    fatalError(node,mode,"no such callback routine named "+node->script.name);
    assert(0);
  }

  return vi;

}






varInst *Mission::callPrintFloats(missionNode *node,int mode){
  string s1=node->attr_value("s1");
  string s2=node->attr_value("s2");

  if(mode==SCRIPT_RUN){
    cout << "print: " << s1 ;
  }

  int len=node->subnodes.size();

  for(int i=0; i<len;i++){
    float res=checkFloatExpr((missionNode *)node->subnodes[i],mode);
    if(mode==SCRIPT_RUN){
      cout << " " << res << " ," ;
    }
  }

  if(mode==SCRIPT_RUN){
    cout << " " << s2 << endl;
  }

  varInst *vi=new varInst;
  vi->type=VAR_VOID;

  return vi;
}

varInst *Mission::callRnd(missionNode *node,int mode){
  varInst *vi=new varInst;
  vi->type=VAR_FLOAT;
  vi->float_val=((float)rand())/(float)RAND_MAX;

  char buffer[100];
  sprintf(buffer,"rnd returning %f", (vi->float_val));
  debug(7,node,mode,buffer);

  return vi;
}

typedef vector<varInst *> olist_t;

varInst *Mission::call_olist(missionNode *node,int mode){

  varInst *vi=NULL;

  string cmd=node->attr_value("cmd");

  if(cmd=="new"){
    olist_t *my_object=new olist_t;

    vi=new varInst;
    vi->type=VAR_OBJECT;
    vi->objectname="olist";
    
    vi->object=(void *)my_object;
    return vi;
  }
  else{


    if(cmd=="push_back"){
      if(node->subnodes.size()!=2){
	fatalError(node,mode,"olist.pushback needs two arguments");
	assert(0);
      }
      missionNode *snode=(missionNode *)node->subnodes[0];
      varInst *ovi=doObjectVar(snode,mode);

      snode=(missionNode *)node->subnodes[1];
      varInst *vi=doVariable(snode,mode);

      olist_t *my_object=(olist_t *)ovi->object;

      if(my_object==NULL){
	fatalError(node,mode,"olist.pushback: no object");
	assert(0);
      }
      my_object->push_back(vi);

      return NULL;
      
    }
    
    return NULL;
  }
  return NULL;
}
