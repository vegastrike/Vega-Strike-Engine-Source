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
    string module=node->attr_value("module");
    if(name.empty() || module.empty()){
      fatalError(node,mode,"you have to give a callback module and name");
      assert(0);
    }
    node->script.name=name;
  }

  string module=node->attr_value("module");
  varInst *vi=NULL;

  if(module=="_io"){
    if(node->script.name=="PrintFloats"){
      vi=callPrintFloats(node,mode);
    }
  }
  else if(module=="_std"){
    if(node->script.name=="Rnd"){
      vi=callRnd(node,mode);
    }
  }
  else if(module=="_olist"){
    vi=call_olist(node,mode);
  }

  if(vi==NULL){
    fatalError(node,mode,"no such callback named "+module+"."+node->script.name);
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

  varInst *viret=new varInst;

  string cmd=node->attr_value("name");

  if(cmd=="new"){
    olist_t *my_object=new olist_t;

    viret->type=VAR_OBJECT;
    viret->objectname="olist";
    
    viret->object=(void *)my_object;

    debug(3,node,mode,"olist new object: ");
    printVarInst(3,viret);

    return viret;
  }
  else{

      if(node->subnodes.size()<1){
	fatalError(node,mode,"olist methods need arguments");
	assert(0);
      }

      missionNode *snode=(missionNode *)node->subnodes[0];
      varInst *ovi=doObjectVar(snode,mode);

      debug(3,node,mode,"olist object: ");
      printVarInst(3,ovi);


  	olist_t *my_object=(olist_t *)ovi->object;
	if(mode==SCRIPT_RUN){
	  if(my_object==NULL){
	    fatalError(node,mode,"olist.pushback: no object");
	    assert(0);
	  }
	}
    
    if(cmd=="push_back"){
      if(node->subnodes.size()!=2){
	fatalError(node,mode,"olist.pushback needs two arguments");
	assert(0);
      }

      snode=(missionNode *)node->subnodes[1];
      varInst *vi=doVariable(snode,mode);
      
      debug(3,node,mode,"olist.push_back pushing variable");
      printf("vi is 0x%x\n",vi);
      printVarInst(2,vi);

      if(mode==SCRIPT_RUN){
	olist_t *my_object=(olist_t *)ovi->object;

	if(my_object==NULL){
	  fatalError(node,mode,"olist.pushback: no object");
	  assert(0);
	}
	varInst *push_vi=new varInst;
	push_vi->type=vi->type;
	assignVariable(push_vi,vi);
	my_object->push_back(push_vi);
      }

      viret->type=VAR_VOID;
      return viret;
    }
    else if(cmd=="pop_back"){
      if(node->subnodes.size()!=1){
	fatalError(node,mode,"olist.pop_back needs one arguments");
	assert(0);
      }

      debug(3,node,mode,"olist.pop");

      if(mode==SCRIPT_RUN){
	my_object->pop_back();
      }

      viret->type=VAR_VOID;
      return viret;
    }
    else if(cmd=="back"){
      if(node->subnodes.size()!=1){
	fatalError(node,mode,"olist.back needs one arguments");
	assert(0);
      }

      debug(3,node,mode,"olist.back");

      viret->type=VAR_OBJECT;

      if(mode==SCRIPT_RUN){
	varInst *back_vi=my_object->back();
	assignVariable(viret,back_vi);
      }

      return viret;
    }
    else if(cmd=="at"){
      if(node->subnodes.size()!=2){
	fatalError(node,mode,"olist.at needs two arguments");
	assert(0);
      }

      debug(3,node,mode,"olist.at");

      snode=(missionNode *)node->subnodes[1];
      float findex=doFloatVar(snode,mode);
      debug(3,snode,mode,"index is in that node");


      viret->type=VAR_FLOAT;

      if(mode==SCRIPT_RUN){
	int index=(int)findex;
	if(index>=my_object->size()){
	  char buffer[200];
	  sprintf(buffer,"olist.at: index out of range size=%d, index=%d\n",my_object->size(),index);
	  fatalError(node,mode,buffer);
	  assert(0);
	}
	varInst *back_vi=(*my_object)[index];
	assignVariable(viret,back_vi);
      }

      return viret;
    }
    else if(cmd=="toxml"){
      if(node->subnodes.size()!=1){
	fatalError(node,mode,"olist.toxml needs no arguments");
	assert(0);
      }

      debug(3,node,mode,"olist.toxml");

      if(mode==SCRIPT_RUN){
	int len=my_object->size();
	
	for(int i=0;i<len;i++){
	  varInst *vi=(*my_object)[i];
	  char buffer[200];
	  sprintf(buffer,"index=\"%d\"",i);
	  debug(3,node,mode,buffer);
	  cout << buffer << " " ;
	  saveVarInst(vi,cout);
	}
      }

      viret->type=VAR_VOID;
      return viret;
    }
    else if(cmd=="size"){
      if(node->subnodes.size()!=1){
	fatalError(node,mode,"olist.size needs one arguments");
	assert(0);
      }

      debug(3,node,mode,"olist.size");

      if(mode==SCRIPT_RUN){
	int len=my_object->size();
	viret->float_val=(float)len;
      }

      viret->type=VAR_FLOAT;
      return viret;
    }

    else{
      fatalError(node,mode,"unknown command "+cmd+" for callback olist");
      assert(0);
    }
    
    return NULL; // never reach
  }
  return NULL; // never reach
}
