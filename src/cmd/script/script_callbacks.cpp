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
#include "lin_time.h"

#include "mission.h"
#include "easydom.h"

#include "msgcenter.h"

//#include "vs_globals.h"
//#include "vegastrike.h"

void Mission::doCall_toxml(string module,varInst *ovi){
  if(module=="_olist"){
    call_olist_toxml(NULL,SCRIPT_RUN,ovi);
  }
  else if(module=="_unit"){
    call_unit_toxml(NULL,SCRIPT_RUN,ovi);
  }
}

varInst *Mission::doCall(missionNode *node,int mode,string module,string method){
  varInst *vi=NULL;

  if(module=="_io"){
    if(method=="PrintFloats"){
      vi=callPrintFloats(node,mode);
    }
    else if(method=="printf"){
      vi=call_io_printf(node,mode);
    }
    else if(method=="sprintf"){
      vi=call_io_sprintf(node,mode);
    }
    else if(method=="message"){
      vi=call_io_message(node,mode);
    }
    else if(method=="printMsgList"){
      vi=call_io_printmsglist(node,mode);
    }
  }
  else if(module=="_std"){
    if(method=="Rnd"){
      vi=callRnd(node,mode);
    }
    else if(method=="getGameTime"){
      vi=callGetGameTime(node,mode);
    }
    else if(method=="ResetTimeCompression"){
      vi=callResetTimeCompression(node,mode);
    }
    else if(method=="getCurrentAIUnit"){
      vi=callGetCurrentAIUnit(node,mode);
    }
    else if(method=="getCurrentAIOrder"){
      vi=callGetCurrentAIOrder(node,mode);
    }
    else if(method=="isNull"){
      vi=call_isNull(node,mode);
    }
    else if(method=="setNull"){
      vi=call_setNull(node,mode);
    }
    else if(method=="equal"){
      vi=call_isequal(node,mode);
    }
    else if(method=="Float"){
      vi=call_float_cast(node,mode);
    }
    else if(method=="Int"){
      vi=call_int_cast(node,mode);
    }
  }
  else if(module=="_olist"){
    vi=call_olist(node,mode);
  }
  else if(module=="_omap"){
    vi=call_omap(node,mode);
  }
  else if(module=="_order"){
    vi=call_order(node,mode);
  }
  else if(module=="_unit"){
    vi=call_unit(node,mode);
  }
  else if(module=="_string"){
    vi=call_string(node,mode);
  }

  return vi;
}


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
    // does not work yet
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

  string method=node->script.name;

  varInst *vi=NULL;

  vi=doCall(node,mode,module,method);


  if(vi==NULL){
    fatalError(node,mode,"no such callback named "+module+"."+node->script.name);
    assert(0);
  }

  return vi;

}

//extern double gametime;

varInst *Mission::call_isNull(missionNode *node,int mode){
  varInst *ovi=getObjectArg(node,mode);

  varInst *viret=newVarInst(VI_TEMP);
  
  viret->type=VAR_BOOL;
  viret->bool_val=(ovi->object==NULL);

  deleteVarInst(ovi);
  return viret;
}

varInst *Mission::call_setNull(missionNode *node,int mode){
  varInst *ovi=getObjectArg(node,mode);

  ovi->object=NULL;

  varInst *viret=newVarInst(VI_TEMP);
  viret->type=VAR_VOID;

  deleteVarInst(ovi);
  return viret;
}

varInst *Mission::call_float_cast(missionNode *node,int mode){
  missionNode *snode=getArgument(node,mode,0);
  int intval=checkIntExpr(snode,mode);

  varInst *viret=newVarInst(VI_TEMP);
  
  viret->type=VAR_FLOAT;
  viret->float_val=(float)intval;

  return viret;
}
varInst *Mission::call_int_cast(missionNode *node,int mode){
  missionNode *snode=getArgument(node,mode,0);
  float floatval=checkFloatExpr(snode,mode);

  varInst *viret=newVarInst(VI_TEMP);
  
  viret->type=VAR_INT;
  viret->int_val=(int)floatval;

  return viret;
}

varInst *Mission::call_isequal(missionNode *node,int mode){
  varInst *ovi=getObjectArg(node,mode);
  missionNode *other_node=getArgument(node,mode,1);
  varInst *other_vi=checkObjectExpr(other_node,mode);

  varInst *viret=newVarInst(VI_TEMP);
  
  viret->type=VAR_BOOL;
  bool res=false;
  
  if(mode==SCRIPT_RUN){
    if(other_vi->objectname == ovi->objectname){
      if(other_vi->object == ovi->object){
	res=true;
      }
    }
  }

  deleteVarInst(ovi);
  deleteVarInst(other_vi);

  viret->bool_val=res;
  return viret;
}

varInst *Mission::callGetGameTime(missionNode *node,int mode){
  varInst *vi=newVarInst(VI_TEMP);

  vi->type=VAR_FLOAT;
  if(mode==SCRIPT_RUN){
    vi->float_val=gametime;
  }
  return vi;
}

varInst *Mission::callResetTimeCompression(missionNode *node,int mode){
  varInst *vi=newVarInst(VI_TEMP);

  vi->type=VAR_VOID;
  if(mode==SCRIPT_RUN){
    setTimeCompression(1.0);
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

  varInst *viret=newVarInst(VI_TEMP);
  viret->type=VAR_VOID;

  return viret;
}

varInst *Mission::call_io_message(missionNode *node,int mode){
  missionNode *args[3];
  varInst *args_vi[3];
  string args_str[3];

  int delay=getIntArg(node,mode,0);

  for(int i=0;i<3;i++){
    args[i]=getArgument(node,mode,i+1);
    args_vi[i]=checkObjectExpr(args[i],mode);
    if(mode==SCRIPT_RUN){
      args_str[i]=call_string_getstring(node,mode,args_vi[i]);
    }
    deleteVarInst(args_vi[i]);
   }

  if(mode==SCRIPT_RUN){
    msgcenter->add(args_str[0],args_str[1],args_str[2],delay);
  }

  varInst *viret=newVarInst(VI_TEMP);
  viret->type=VAR_VOID;

  return viret;

}
#if 1
string Mission::replaceNewline(string origstr){
  string ostr=origstr;
  
  int breakpos=ostr.find("\\n",0);
  
  if(breakpos>=0){
    //printf("breakpos=%d\n",breakpos);

    string newstr=ostr.replace(breakpos,2,"\n");

    return replaceNewline(newstr);
  }
  else{
    return ostr;
  }
}
#endif


varInst *Mission::call_io_sprintf(missionNode *node,int mode){
  missionNode *outstr_node=getArgument(node,mode,0);
  varInst *outstr_vi=checkObjectExpr(outstr_node,mode);
  string *outstrptr=getStringObject(outstr_node,mode,outstr_vi);

  char outbuffer[1024];
  string outstring;

  missionNode *stringnode=getArgument(node,mode,1);
  if(stringnode->tag!=DTAG_CONST){
    fatalError(node,mode,"only const string allowed for second arg of sprintf");
    assert(0);
  }
  varInst *str_vi=checkObjectExpr(stringnode,mode);
  if(str_vi->type!=VAR_OBJECT || (str_vi->type==VAR_OBJECT && str_vi->objectname!="string")){
    fatalError(node,mode,"io.sprintf needs string object as second arg");
    assert(0);
  }

  int nr_of_args=node->subnodes.size();
  int current_arg=2;
  string * fullstringptr;
  string fullstring;

  fullstringptr=(string *)str_vi->object;
  fullstring=*fullstringptr;

    fullstring=replaceNewline(fullstring);

  //cout << "printf string-" << fullstring << "-" << endl;

  //  if(mode==SCRIPT_RUN){

  string endstring=fullstring;

  while(current_arg<nr_of_args){

    int breakpos=endstring.find("%",0);

    string beforestring=endstring.substr(0,breakpos);

    //printf("beforestr=-%s-",beforestring.c_str());

    string breakstring=endstring.substr(breakpos,2);

    //printf("breakstr=-%s-\n",breakstring.c_str());

    //    printf("**");
    if(breakstring[1]=='f'){
      missionNode *anode=getArgument(node,mode,current_arg);
      float res=checkFloatExpr(anode,mode);
      
      if(mode==SCRIPT_RUN){
	sprintf(outbuffer,beforestring.c_str());
	outstring+=outbuffer;
	sprintf(outbuffer,"%f",res);
	outstring+=outbuffer;
      }
    }
    else if(breakstring[1]=='d'){
      missionNode *anode=getArgument(node,mode,current_arg);
      int res=checkIntExpr(anode,mode);

      if(mode==SCRIPT_RUN){
	sprintf(outbuffer,beforestring.c_str());
	outstring+=outbuffer;
	sprintf(outbuffer,"%d",res);
	outstring+=outbuffer;
      }
    }
    else if(breakstring[1]=='s'){
      missionNode *anode=getArgument(node,mode,current_arg);
      varInst *res_vi=doObjectVar(anode,mode);

      if(mode==SCRIPT_RUN){
	if(res_vi->type!=VAR_OBJECT || (res_vi->type==VAR_OBJECT && res_vi->objectname!="string")){
	  fatalError(node,mode,"io.printf needs string object as some arg");
	  assert(0);
	}

	string * strptr=(string *)res_vi->object;

	sprintf(outbuffer,beforestring.c_str());
	outstring+=outbuffer;
	sprintf(outbuffer,"%s",strptr->c_str());
	outstring+=outbuffer;
      }
      deleteVarInst(res_vi);
    }
    //printf("++");

    endstring=endstring.substr(breakpos+2,endstring.size()-(breakpos+2));
    //        printf("endstr=-%s-\n",endstring.c_str());


    current_arg++;
  }//while

  if(mode==SCRIPT_RUN){
    sprintf(outbuffer,endstring.c_str());
    outstring+=outbuffer;
    (*outstrptr)=outstring;
  }

  //  printf("--end==\n");
  varInst *viret=newVarInst(VI_TEMP);
  viret->type=VAR_VOID;
  deleteVarInst(str_vi);
  deleteVarInst(outstr_vi);

  return viret;
}

varInst *Mission::call_io_printf(missionNode *node,int mode){
  missionNode *stringnode=getArgument(node,mode,0);
  if(stringnode->tag!=DTAG_CONST){
    fatalError(node,mode,"only const string allowed for first arg of printf");
    assert(0);
  }
  varInst *str_vi=checkObjectExpr(stringnode,mode);
  if(str_vi->type!=VAR_OBJECT || (str_vi->type==VAR_OBJECT && str_vi->objectname!="string")){
    fatalError(node,mode,"io.printf needs string object as first arg");
    assert(0);
  }

  int nr_of_args=node->subnodes.size();
  int current_arg=1;
  string * fullstringptr;
  string fullstring;

  fullstringptr=(string *)str_vi->object;
  fullstring=*fullstringptr;

    fullstring=replaceNewline(fullstring);

  //cout << "printf string-" << fullstring << "-" << endl;

  //  if(mode==SCRIPT_RUN){

  string endstring=fullstring;

  while(current_arg<nr_of_args){

    int breakpos=endstring.find("%",0);

    string beforestring=endstring.substr(0,breakpos);

    //printf("beforestr=-%s-",beforestring.c_str());

    string breakstring=endstring.substr(breakpos,2);

    //printf("breakstr=-%s-\n",breakstring.c_str());

    //    printf("**");
    if(breakstring[1]=='f'){
      missionNode *anode=getArgument(node,mode,current_arg);
      float res=checkFloatExpr(anode,mode);
      
      if(mode==SCRIPT_RUN){
	printf(beforestring.c_str());
	printf("%f",res);
      }
    }
    else if(breakstring[1]=='d'){
      missionNode *anode=getArgument(node,mode,current_arg);
      int res=checkIntExpr(anode,mode);

      if(mode==SCRIPT_RUN){
	printf(beforestring.c_str());
	printf("%d",res);
      }
    }
    else if(breakstring[1]=='s'){
      missionNode *anode=getArgument(node,mode,current_arg);
      varInst *res_vi=doObjectVar(anode,mode);

      if(mode==SCRIPT_RUN){
	if(res_vi->type!=VAR_OBJECT || (res_vi->type==VAR_OBJECT && res_vi->objectname!="string")){
	  fatalError(node,mode,"io.printf needs string object as some arg");
	  assert(0);
	}

	string * strptr=(string *)res_vi->object;

	printf(beforestring.c_str());
	printf("%s",strptr->c_str());
      }
      deleteVarInst(res_vi);
    }
    //printf("++");

    endstring=endstring.substr(breakpos+2,endstring.size()-(breakpos+2));
    //        printf("endstr=-%s-\n",endstring.c_str());


    current_arg++;
  }//while

  if(mode==SCRIPT_RUN){
    printf(endstring.c_str());
  }

  //  printf("--end==\n");
  varInst *viret=newVarInst(VI_TEMP);
  viret->type=VAR_VOID;
  deleteVarInst(str_vi);

  return viret;
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

  varInst *vi=newVarInst(VI_TEMP);
  vi->type=VAR_VOID;

  return vi;
}

varInst *Mission::callGetCurrentAIUnit(missionNode *node,int mode){
  varInst *vi=newVarInst(VI_TEMP);
  vi->type=VAR_OBJECT;
  vi->objectname="unit";
  vi->object=(void *)current_ai_unit;

  return vi;
}

varInst *Mission::callGetCurrentAIOrder(missionNode *node,int mode){
  varInst *vi=newVarInst(VI_TEMP);
  vi->type=VAR_OBJECT;
  vi->objectname="order";
  vi->object=(void *)current_ai_order;

  return vi;
}

varInst *Mission::callRnd(missionNode *node,int mode){
  varInst *vi=newVarInst(VI_TEMP);
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

bool Mission::getBoolArg(missionNode *node,int mode,int arg_nr){
  missionNode *val_node=getArgument(node,mode,arg_nr);
  bool res=checkBoolExpr(val_node,mode);
  return res;
}
float Mission::getFloatArg(missionNode *node,int mode,int arg_nr){
  missionNode *val_node=getArgument(node,mode,arg_nr);
  float res=checkFloatExpr(val_node,mode);
  return res;
}
float Mission::getIntArg(missionNode *node,int mode,int arg_nr){
  missionNode *val_node=getArgument(node,mode,arg_nr);
  int res=checkIntExpr(val_node,mode);
  return res;
}

Unit* Mission::getUnitArg(missionNode *node,int mode,int arg_nr){
  Unit *ret=NULL;

  missionNode *unit_node=getArgument(node,mode,arg_nr);
  varInst *unit_vi=checkObjectExpr(unit_node,mode);
  if(mode==SCRIPT_RUN){
    if(unit_vi->type==VAR_OBJECT && unit_vi->objectname=="unit"){
      ret=getUnitObject(unit_node,mode,unit_vi);
    }
    else{
      fatalError(node,mode,"getUnitArg: expected unit arg - got else");
      assert(0);
      return NULL; // never reach
    }
  }
  deleteVarInst(unit_vi);
  return ret;
}
Vector Mission::getVec3Arg(missionNode *node,int mode,int arg_nr){
  missionNode *pos_node=getArgument(node,mode,arg_nr);
  varInst *pos_vi=checkObjectExpr(pos_node,mode);
  olist_t *pos_olist=getOListObject(pos_node,mode,pos_vi);

  Vector vec3;
  if(mode==SCRIPT_RUN){
    vec3=call_olist_tovector(pos_node,mode,pos_vi);
  }

  deleteVarInst(pos_vi);
  return vec3;
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
