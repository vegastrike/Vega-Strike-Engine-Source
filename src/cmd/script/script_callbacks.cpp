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

#include "msgcenter.h"

//#include "vs_globals.h"
//#include "vegastrike.h"


varInst *Mission::doCall(missionNode *node,int mode){
  if(mode==SCRIPT_PARSE){
    string name=node->attr_value("name");
    string module=node->attr_value("module");
    string object=node->attr_value("object");
    if(object.empty() && module.empty()){
      fatalError(node,mode,"you have to give a callback object or module");
      assert(0);
    }
    if(name.empty()){
      fatalError(node,mode,"you have to give a callback name");
      assert(0);
    }
    node->script.name=name;
  }

  // RUNTIME && PARSE
  string module=node->attr_value("module");
  if(module.empty()){
    string object=node->attr_value("object");
    assert(0);
    //varInst *ovi=lookupVariable(object);
    varInst *ovi=NULL;
    if(ovi==NULL){
      fatalError(node,mode,"no object found with name "+object);
      assert(0);
    }
    if(ovi->type!=VAR_OBJECT){
      fatalError(node,mode,"given variable"+object+" is not an object");
      assert(0);
    }
    module=ovi->objectname;
    if(module.empty()){
      fatalError(node,mode,"object "+object+" not yet initialized");
      assert(0);
    }
    module="_"+module;
  }

  varInst *vi=NULL;

  if(module=="_io"){
    if(node->script.name=="PrintFloats"){
      vi=callPrintFloats(node,mode);
    }
    else if(node->script.name=="printf"){
      vi=call_io_printf(node,mode);
    }
    else if(node->script.name=="message"){
      vi=call_io_message(node,mode);
    }
    else if(node->script.name=="printMsgList"){
      vi=call_io_printmsglist(node,mode);
    }
  }
  else if(module=="_std"){
    if(node->script.name=="Rnd"){
      vi=callRnd(node,mode);
    }
    else if(node->script.name=="getGameTime"){
      vi=callGetGameTime(node,mode);
    }
    else if(node->script.name=="isNull"){
      vi=call_isNull(node,mode);
    }
  }
  else if(module=="_olist"){
    vi=call_olist(node,mode);
  }
  else if(module=="_unit"){
    vi=call_unit(node,mode);
  }
  else if(module=="_string"){
    vi=call_string(node,mode);
  }

  if(vi==NULL){
    fatalError(node,mode,"no such callback named "+module+"."+node->script.name);
    assert(0);
  }

  return vi;

}

extern double gametime;

varInst *Mission::call_isNull(missionNode *node,int mode){
  varInst *ovi=getObjectArg(node,mode);

  varInst *viret=new varInst;
  
  viret->type=VAR_BOOL;
  viret->bool_val=(ovi->object==NULL);

  return viret;
}

varInst *Mission::callGetGameTime(missionNode *node,int mode){
  varInst *vi=new varInst;

  vi->type=VAR_FLOAT;
  if(mode==SCRIPT_RUN){
    vi->float_val=gametime;
  }
  return vi;
}

varInst *Mission::call_io_printmsglist(missionNode *node,int mode){

  int i=0;

  if(mode==SCRIPT_RUN){
    gameMessage *msg=msgcenter->last(i);
  
    while(msg!=NULL && i<7.0){
      cout << "MESSAGE" << msg->message << endl;
      i++;
      msg=msgcenter->last(i);
    }
  }

  varInst *viret=new varInst;
  viret->type=VAR_VOID;

  return viret;

}

varInst *Mission::call_io_message(missionNode *node,int mode){
  missionNode *args[3];
  varInst *args_vi[3];
  string args_str[3];

  for(int i=0;i<3;i++){
    args[i]=getArgument(node,mode,i);
    args_vi[i]=checkObjectExpr(args[i],mode);
    if(mode==SCRIPT_RUN){
      args_str[i]=call_string_getstring(node,mode,args_vi[i]);
    }
   }

  if(mode==SCRIPT_RUN){
    msgcenter->add(args_str[0],args_str[1],args_str[2]);
  }

  varInst *viret=new varInst;
  viret->type=VAR_VOID;

  return viret;

}

varInst *Mission::call_io_printf(missionNode *node,int mode){
  return NULL;
#if 0
  missionNode *stringnode=getArgument(node,mode,0);
  varInst *str_vi=checkObjectExpr(stringnode,mode);
  if(str_vi->type!=VAR_OBJECT || (str_vi->type==VAR_OBJECT && str_vi->objectname!="string")){
    fatalError(node,mode,"io.printf needs string object as first arg");
    assert(0);
  }

  int nr_of_args=node->subnodes.size();
  int current_arg=1;
  string * fullstringptr;
  string fullstring;

  if(mode==SCRIPT_RUN){
    fullstringptr=(string *)str_vi->object;
    fullstring=*fullstringptr;

  string endstring=fullstring;

  while(current_arg<nr_of_args){

  int breakpos=endstring.find("%",0);

  string beforestring=endstring.substr(0,breakpos);

  printf("-%s-",beforestring.c_str());

  string breakstring=endstring.substr(breakpos,breakpos+1);

  if(breakstring[1]=='f'){
    missionNode *anode=getArgument(node,mode,current_arg);
    float res=checkFloatExpr(anode,mode);

    printf("%f",res);
  }
  else if(breakstring[1]=='s'){
    missionNode *anode=getArgument(node,mode,current_arg);
    varInst *res_vi=doObjectVar(anode,mode);
    if(res_vi->type!=VAR_OBJECT || (res_vi->type==VAR_OBJECT && res_vi->objectname!="string")){
      fatalError(node,mode,"io.printf needs string object as some arg");
      assert(0);
    }
    
    string * strptr=(string *)res_vi->object;

    printf("%s",strptr->c_str());
  }

  endstring=endstring.substr(breakpos+2,endstring.size()-1);
  current_arg++;
  }
  }

  varInst *viret=new varInst;
  viret->type=VAR_VOID;
  return viret;
#endif
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

varInst *Mission::getObjectArg(missionNode *node,int mode){
      if(node->subnodes.size()<1){
	fatalError(node,mode,method_str(node)+" needs an object as first  argument");
	assert(0);
      }

      missionNode *snode=(missionNode *)node->subnodes[0];
      varInst *ovi=doObjectVar(snode,mode);

      debug(3,node,mode,node->attr_value("module")+"."+node->attr_value("name")+" object: ");
      printVarInst(3,ovi);

      return ovi;
}


string Mission::method_str(missionNode *node){
  return  node->attr_value("module")+"."+node->attr_value("name");
}

missionNode *Mission::getArgument(missionNode *node,int mode,int arg_nr){
      if(node->subnodes.size() < arg_nr+1){
	char buf[200];
	sprintf(buf," needs at least %d arguments",arg_nr+1);
	fatalError(node,mode,method_str(node)+buf);
	assert(0);
      }

      missionNode *snode=(missionNode *)node->subnodes[arg_nr];

      return snode;
}
