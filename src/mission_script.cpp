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


void Mission::DirectorStart(missionNode *node){

  missionThread *main_thread=new missionThread;
  runtime.thread_nr=0;
  runtime.threads.push_back(main_thread);

  doModule(node,SCRIPT_PARSE);
}

void Mission::DirectorLoop(){
}


/* *********************************************************** */

void Mission::fatalError(string message){
  cout << "fatalError: " << message << endl;
}

void Mission::runtimeFatal(string message){
  cout << "runtime fatalError: " << message << endl;
}

void Mission::warning(string message){
  cout << "warning: " << message << endl;
}

/* *********************************************************** */

void Mission::doModule(missionNode *node,int mode){
  if(mode==SCRIPT_PARSE){
    string name=node->attr_value("name");
  
    if(name.empty()){
      fatalError("you have to give a module name");
      assert(0);
    }

    if(runtime.modules[name]!=NULL){
      fatalError("there can only be one module with name "+name);
      assert(0);
    }

    if(name=="director"){
      director=node;
    }
 
    node->script.name=name;

    runtime.modules[name]=node; // add this module to the list of known modules

    scope_stack.push(node);
  }

  vector<easyDomNode *>::const_iterator siter;
  
  for(siter= node->subnodes.begin() ; siter!=node->subnodes.end() ; siter++){
    missionNode *snode=(missionNode *)*siter;
    if(snode->tag==DTAG_SCRIPT){
      doScript(snode,mode);
    }
  }
}

/* *********************************************************** */

scriptContext *Mission::makeContext(missionNode *node){
  return NULL;
}

/* *********************************************************** */

scriptContext *Mission::addContext(missionNode *node)
{
  scriptContext *context=makeContext(node);
  contextStack *stack=runtime.cur_thread->exec_stack.top();
  stack->contexts.push_back(context);

  return context;
}

/* *********************************************************** */

void Mission::removeContext()
{
  contextStack *stack=runtime.cur_thread->exec_stack.top();

  int lastelem=stack->contexts.size()-1;

  scriptContext *old=stack->contexts[lastelem];
  stack->contexts.pop_back();

  delete old;
}

/* *********************************************************** */

void Mission::doScript(missionNode *node,int mode){
  if(mode==SCRIPT_PARSE){
    node->script.name=node->attr_value("name");

    if(node->script.name.empty()){
      fatalError("you have to give a script name");
    }
    scope_stack.push(node);
  }
  if(mode==SCRIPT_RUN){
    addContext(node);
  }

  vector<easyDomNode *>::const_iterator siter;
  
  for(siter= node->subnodes.begin() ; siter!=node->subnodes.end() ; siter++){
    missionNode *snode=(missionNode *)*siter;
    checkStatement(snode,mode);
  }

  if(mode==SCRIPT_RUN){
    removeContext();
  }
  else{
    scope_stack.pop();
  }
}

/* *********************************************************** */

void Mission::doBlock(missionNode *node,int mode){
  if(mode==SCRIPT_PARSE){
    scope_stack.push(node);
  }
  if(mode==SCRIPT_RUN){
    addContext(node);
  }

    vector<easyDomNode *>::const_iterator siter;
  
  for(siter= node->subnodes.begin() ; siter!=node->subnodes.end() ; siter++){
    missionNode *snode=(missionNode *)*siter;
    checkStatement(snode,mode);
  }
  if(mode==SCRIPT_RUN){
    removeContext();
  }
}

/* *********************************************************** */

bool Mission::checkVarType(varInst *var,enum var_type check_type){
  if(var->type==check_type){
    return true;
  }
  return false;
}

/* *********************************************************** */

bool Mission::doBooleanVar(missionNode *node,int mode){
  varInst *var=doVariable(node,mode);

  bool ok=checkVarType(var,VAR_BOOL);

  if(!ok){
    runtimeFatal("expected a bool variable - got a different type");
    assert(0);
  }

  return var->bool_val;
}

/* *********************************************************** */

varInst *Mission::lookupLocalVariable(missionNode *asknode){
  contextStack *cstack=runtime.cur_thread->exec_stack.top();
  varInst *defnode=NULL;

  for(unsigned int i=0;i<cstack->contexts.size() && defnode==NULL;i++){
    scriptContext *context=cstack->contexts[i];
    varInstMap *map=context->varinsts;
    defnode=(*map)[asknode->script.name];
  }
  if(defnode==NULL){
    return NULL;
  }

  return defnode;
}

/* *********************************************************** */

varInst *Mission::lookupModuleVariable(string mname,missionNode *asknode){
  // only when runtime
  missionNode *module_node=runtime.modules[mname];

  if(module_node==NULL){
    return NULL;
  }

  vector<easyDomNode *>::const_iterator siter;
  
  for(siter= module_node->subnodes.begin() ; siter!=module_node->subnodes.end() ; siter++){
    missionNode *varnode=(missionNode *)*siter;
    if(varnode->script.name==asknode->script.name){
      return varnode->script.varinst;
    }
  }

  return NULL;

}

/* *********************************************************** */

varInst *Mission::lookupModuleVariable(missionNode *asknode){
  // only when runtime
  missionNode *module=runtime.cur_thread->module_stack.top();

  string mname=module->script.name;

  varInst *var=lookupModuleVariable(mname,asknode);


  return var;

}

/* *********************************************************** */

varInst *Mission::lookupGlobalVariable(missionNode *asknode){
  missionNode *varnode=runtime.global_variables[asknode->script.name];

  if(varnode==NULL){
    return NULL;
  }

  return varnode->script.varinst;
}

/* *********************************************************** */

varInst *Mission::doVariable(missionNode *node,int mode){
  if(mode==SCRIPT_RUN){
    varInst *var=lookupLocalVariable(node);
    if(var==NULL){
      // search in module namespace
      var=lookupModuleVariable(node);
      if(var==NULL){
	// search in global namespace
	var=lookupGlobalVariable(node);
	if(var==NULL){
	  runtimeFatal("did not find variable");
	  assert(0);
	}
      }
    }
    return var;
  }
  else{
    // SCRIPT_PARSE
    node->script.name=node->attr_value("name");
    if(node->script.name.empty()){
      fatalError("you have to give a variable name");
      assert(0);
    }
    return NULL;
  }
}

/* *********************************************************** */

void Mission::doDefVar(missionNode *node,int mode){
  if(SCRIPT_RUN){
    return;
  }

  node->script.name=node->attr_value("name");
    if(node->script.name.empty()){
      fatalError("you have to give a variable name");
    }

    string type=node->attr_value("type");
    //    node->initval=node->attr_value("init");

    if(type=="float"){
      node->script.vartype=VAR_FLOAT;
    }
    else if(type=="string"){
      node->script.vartype=VAR_STRING;
    }
    else if(type=="vector"){
      node->script.vartype=VAR_VECTOR;
    }
    else if(type=="object"){
      node->script.vartype=VAR_OBJECT;
    }
    else{
      fatalError("unknown variable type");
      assert(0);
    }

    missionNode *scope=scope_stack.top();
    
    scope->script.variables.push_back(node);
}


void Mission::doSetVar(missionNode *node,int mode){

  if(SCRIPT_PARSE){
    node->script.name=node->attr_value("name");
    if(node->script.name.empty()){
      fatalError("you have to give a variable name");
    }
  }

    varInst *var_expr=checkExpression((missionNode *)node->subnodes[0],mode);

    if(SCRIPT_RUN){
      varInst *var_inst=doVariable(node,mode); // lookup variable instance

      if(var_expr->type != var_inst->type){
	runtimeFatal("variable "+node->script.name+" is not of the correct type
");
	assert(0);
      }

      assignVariable(var_inst,var_expr);
      
      delete var_expr; // only temporary
    }
  
}

void Mission::doExec(missionNode *node,int mode){
}

void Mission::doCall(missionNode *node,int mode){
}

void Mission::initTagMap(){
  tagmap["module"]=DTAG_MODULE;
  tagmap["script"]=DTAG_SCRIPT;
  tagmap["if"]=DTAG_IF;
  tagmap["block"]=DTAG_BLOCK;
  tagmap["setvar"]=DTAG_SETVAR;
  tagmap["exec"]=DTAG_EXEC;
  tagmap["call"]=DTAG_CALL;
  tagmap["while"]=DTAG_WHILE;
  tagmap["and"]=DTAG_AND_EXPR;
  tagmap["or"]=DTAG_OR_EXPR;
  tagmap["not"]=DTAG_NOT_EXPR;
  tagmap["test"]=DTAG_TEST_EXPR;
  tagmap["fmath"]=DTAG_FMATH;
  tagmap["vmath"]=DTAG_VMATH;
  tagmap["var"]=DTAG_VAR_EXPR;
  tagmap["defvar"]=DTAG_DEFVAR;

}

void Mission::assignVariable(varInst *v1,varInst *v2){
  v1->float_val=v2->float_val;
  v1->bool_val=v2->bool_val;
}

varInst *Mission::checkExpression(missionNode *node,int mode){

  varInst *ret=NULL;

  switch(node->tag){
  case DTAG_AND_EXPR:
  case DTAG_OR_EXPR:
  case DTAG_NOT_EXPR:
  case DTAG_TEST_EXPR:
    checkBoolExpr(node,mode);
    break;

  case DTAG_VAR_EXPR:
    ret=doVariable(node,mode);
  case DTAG_FMATH:
    
  default:
    break;
  }

}
