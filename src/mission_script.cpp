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

  cout << "DIRECTOR START" << endl;

  missionThread *main_thread=new missionThread;
  runtime.thread_nr=0;
  runtime.threads.push_back(main_thread);
  runtime.cur_thread=main_thread;

  doModule(node,SCRIPT_PARSE);

  while(true){
    DirectorLoop();
#ifndef _WIN32
    sleep(1);
#endif
  }
}

void Mission::DirectorLoop(){
  cout << "DIRECTOR LOOP" << endl;
    doModule(director,SCRIPT_RUN);
}


/* *********************************************************** */

void Mission::fatalError(missionNode *node,int mode,string message){
  cout << "fatalError: " << message << " : ";
  printNode(node,mode);
}

void Mission::runtimeFatal(string message){
  cout << "runtime fatalError: " << message << endl;
}

void Mission::warning(string message){
  cout << "warning: " << message << endl;
}

#define DEBUG_LEVEL 0

void Mission::debug(int level,missionNode *node,int mode,string message){
  if(level<=DEBUG_LEVEL){
    debug(node,mode,message);
  }
}

void Mission::debug(missionNode *node,int mode,string message){

  cout << "debug: " << message << " : " ;
  printNode(node,mode);
  //  cout << endl;
}

void Mission::printNode(missionNode *node,int mode){
  node->printNode(cout,0,0);
}

/* *********************************************************** */

void Mission::doModule(missionNode *node,int mode){
  if(mode==SCRIPT_PARSE){
    string name=node->attr_value("name");
  
    if(name.empty()){
      fatalError(node,mode,"you have to give a module name");
      assert(0);
    }

    if(runtime.modules[name]!=NULL){
      fatalError(node,mode,"there can only be one module with name "+name);
      assert(0);
    }

    if(name=="director"){
      director=node;
    }
 
    node->script.name=name;

    runtime.modules[name]=node; // add this module to the list of known modules

    scope_stack.push_back(node);

    debug(5,node,mode,"added module "+name+" to list of known modules");
  }
  else{
    // SCRIPT_RUN
    runtime.cur_thread->module_stack.push_back(node);
  }


  vector<easyDomNode *>::const_iterator siter;
  
  for(siter= node->subnodes.begin() ; siter!=node->subnodes.end() ; siter++){
    missionNode *snode=(missionNode *)*siter;
    if(snode->tag==DTAG_SCRIPT){
      doScript(snode,mode);
    }
    else if(snode->tag==DTAG_DEFVAR){
      doDefVar(snode,mode);
    }
    else{
      fatalError(node,mode,"unkown node type");
    }

  }

  if(mode==SCRIPT_PARSE){
    scope_stack.pop_back();
  }
  else{
    runtime.cur_thread->module_stack.pop_back();
  }
}

/* *********************************************************** */

scriptContext *Mission::makeContext(missionNode *node){
  scriptContext *context=new scriptContext;

  context->varinsts=new varInstMap;

  return context;
}

/* *********************************************************** */

void Mission::removeContextStack(){
  runtime.cur_thread->exec_stack.pop_back();
}

void Mission::addContextStack(missionNode *node){
  contextStack *cstack=new contextStack;

  runtime.cur_thread->exec_stack.push_back(cstack);
}

scriptContext *Mission::addContext(missionNode *node)
{

  scriptContext *context=makeContext(node);
  contextStack *stack=runtime.cur_thread->exec_stack.back();
  stack->contexts.push_back(context);

  debug(5,node,0,"added context for this node");
  printRuntime();

  return context;
}

/* *********************************************************** */

void Mission::removeContext()
{
  contextStack *stack=runtime.cur_thread->exec_stack.back();

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
      fatalError(node,mode,"you have to give a script name");
    }
    scope_stack.push_back(node);
  }

  debug(5,node,mode,"executing script name="+node->script.name);

  if(mode==SCRIPT_RUN){
    addContextStack(node);
    addContext(node);
  }

  vector<easyDomNode *>::const_iterator siter;
  
  for(siter= node->subnodes.begin() ; siter!=node->subnodes.end() ; siter++){
    missionNode *snode=(missionNode *)*siter;
    checkStatement(snode,mode);
  }

  if(mode==SCRIPT_RUN){
    removeContext();
    removeContextStack();
  }
  else{
    scope_stack.pop_back();
  }
}

/* *********************************************************** */

void Mission::doBlock(missionNode *node,int mode){
  if(mode==SCRIPT_PARSE){
    scope_stack.push_back(node);
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
    if(mode==SCRIPT_PARSE){
      scope_stack.pop_back();
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

float Mission::doFloatVar(missionNode *node,int mode){
  varInst *var=doVariable(node,mode);

  bool ok=checkVarType(var,VAR_FLOAT);

  if(!ok){
    runtimeFatal("expected a float variable - got a different type");
    assert(0);
  }

  return var->float_val;
}

/* *********************************************************** */

varInst *Mission::lookupLocalVariable(missionNode *asknode){
  contextStack *cstack=runtime.cur_thread->exec_stack.back();
  varInst *defnode=NULL;

  for(unsigned int i=0;i<cstack->contexts.size() && defnode==NULL;i++){
    scriptContext *context=cstack->contexts[i];
    varInstMap *map=context->varinsts;
    defnode=(*map)[asknode->script.name];
    if(defnode!=NULL){
      debug(5,defnode->defvar_node,0,"FOUND local variable defined in that node");
    }
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
    fatalError(asknode,0,"no such module named "+mname);
    assert(0);
    return NULL;
  }

  vector<easyDomNode *>::const_iterator siter;
  
  for(siter= module_node->subnodes.begin() ; siter!=module_node->subnodes.end() ; siter++){
    missionNode *varnode=(missionNode *)*siter;
    if(varnode->script.name==asknode->script.name){
      char buffer[200];
      sprintf(buffer,"FOUND module variable %s in that node",varnode->script.name.c_str());
      debug(4,varnode,0,buffer);
      printVarInst(varnode->script.varinst);

      return varnode->script.varinst;
    }
  }

  return NULL;

}

/* *********************************************************** */

varInst *Mission::lookupModuleVariable(missionNode *asknode){
  // only when runtime
  missionNode *module=runtime.cur_thread->module_stack.back();

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
      fatalError(node,mode,"you have to give a variable name");
      assert(0);
    }

    varInst *vi=searchScopestack(node->script.name);

    if(vi==NULL){
      fatalError(node,mode,"no variable "+node->script.name+" found on the scopestack");
      assert(0);
    }

    return vi;


  }
}

/* *********************************************************** */

void Mission::doDefVar(missionNode *node,int mode){
  if(mode==SCRIPT_RUN){
    missionNode *scope=node->script.context_block_node;
    if(scope->tag==DTAG_MODULE){
      // this is a module variable - it has been initialized at parse time
      debug(4,node,mode,"defined module variable "+node->script.name);
      return;
    }

    debug(5,node,mode,"defining context variable "+node->script.name);

    contextStack *stack=runtime.cur_thread->exec_stack.back();
    scriptContext *context=stack->contexts.back();


    varInstMap *vmap=context->varinsts;

    varInst *vi=new varInst;
    vi->defvar_node=node;
    vi->block_node=scope;
    vi->type=node->script.vartype;
    vi->name=node->script.name;

    (*vmap)[node->script.name]=vi;

    printRuntime();

    return;
  }


  node->script.name=node->attr_value("name");
    if(node->script.name.empty()){
      fatalError(node,mode,"you have to give a variable name");
      assert(0);
    }

    string value=node->attr_value("init");

    debug(5,node,mode,"defining variable "+node->script.name);

    string type=node->attr_value("type");
    //    node->initval=node->attr_value("init");

    if(type=="float"){
      node->script.vartype=VAR_FLOAT;
    }
    else if(type=="bool"){
      node->script.vartype=VAR_BOOL;
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
      fatalError(node,mode,"unknown variable type");
      assert(0);
    }

    missionNode *scope=scope_stack.back();
    
  varInst *vi=new varInst;
  vi->defvar_node=node;
  vi->block_node=scope;
  vi->type=node->script.vartype;
  vi->name=node->script.name;

  if(scope->tag==DTAG_MODULE){
    if(!value.empty()){
      debug(4,node,mode,"setting init for "+node->script.name);
      if(vi->type==VAR_FLOAT){
	vi->float_val=atof(value.c_str());
      }
      else if(vi->type==VAR_BOOL){
	if(value=="true"){
	vi->bool_val=true;
	}
	else if(value=="false"){
	  vi->bool_val=false;
	}
	else{
	  fatalError(node,mode,"wrong bool value");
	  assert(0);
	}
      }
      printVarInst(vi);
    }
  }

  scope->script.variables[node->script.name]=vi;
  node->script.varinst=vi;//FIXME
  node->script.context_block_node=scope;
    //}
    //else
  //    scope->script.variables.push_back(node);

    debug(5,scope,mode,"defined variable in that scope");
}


void Mission::doSetVar(missionNode *node,int mode){

  if(mode==SCRIPT_PARSE){
    node->script.name=node->attr_value("name");
    if(node->script.name.empty()){
      fatalError(node,mode,"you have to give a variable name");
    }
  }

  debug(5,node,mode,"trying to set variable "+node->script.name);

  //    varInst *var_expr=checkExpression((missionNode *)node->subnodes[0],mode);
  missionNode *expr=(missionNode *)node->subnodes[0];

  if(mode==SCRIPT_PARSE){
    varInst *vi=searchScopestack(node->script.name);

    if(vi==NULL){
      fatalError(node,mode,"no variable "+node->script.name+" found on the scopestack");
      assert(0);
    }

    if(vi->type==VAR_BOOL){
      bool res=checkBoolExpr(expr,mode);
    }
    else if(vi->type==VAR_FLOAT){
      float res=checkFloatExpr(expr,mode);
    }

  }

    if(mode==SCRIPT_RUN){
      varInst *var_inst=doVariable(node,mode); // lookup variable instance

      if(var_inst==NULL){
	fatalError(node,mode,"variable lookup failed for "+node->script.name);
	printRuntime();
	assert(0);
      }
    if(var_inst->type==VAR_BOOL){
      bool res=checkBoolExpr(expr,mode);
      var_inst->bool_val=res;
    }
    else if(var_inst->type==VAR_FLOAT){
      float res=checkFloatExpr(expr,mode);
      var_inst->float_val=res;
    }
#if 0
      if(var_expr->type != var_inst->type){
	runtimeFatal("variable "+node->script.name+" is not of the correct type\n");
	assert(0);
      }

      assignVariable(var_inst,var_expr);
      
      delete var_expr; // only temporary
#endif
    }
  
}

varInst *Mission::doConst(missionNode *node,int mode){
  if(mode==SCRIPT_PARSE){
    //string name=node->attr_value("name");
    string typestr=node->attr_value("type");
    string valuestr=node->attr_value("value");

    if(typestr.empty() || valuestr.empty()){
      fatalError(node,mode,"no valid const declaration");
      assert(0);
    }
    
    debug(5,node,mode,"parsed const value "+valuestr);

    varInst *vi=new varInst;
    if(typestr=="float"){
      node->script.vartype=VAR_FLOAT;
      vi->float_val=atof(valuestr.c_str());
    }
    else if(typestr=="bool"){
      node->script.vartype=VAR_BOOL;
      if(valuestr=="true"){
	vi->bool_val=true;
      }
      else if(valuestr=="false"){
	vi->bool_val=false;
      }
      else{
	fatalError(node,mode,"wrong bool value");
	assert(0);
      }
    }
    else if(typestr=="string"){
      node->script.vartype=VAR_STRING;
    }
    else if(typestr=="vector"){
      node->script.vartype=VAR_VECTOR;
    }
    else if(typestr=="object"){
      node->script.vartype=VAR_OBJECT;
    }
    else{
      fatalError(node,mode,"unknown variable type");
      assert(0);
    }

    vi->type=node->script.vartype;

    node->script.varinst=vi;
    
  }

  return node->script.varinst;
    

}

void Mission::doExec(missionNode *node,int mode){
}

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

  if(vi==NULL){
    fatalError(node,mode,"no such callback routine named "+node->script.name);
    assert(0);
  }

  return vi;

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
  tagmap["const"]=DTAG_CONST;

}

void Mission::assignVariable(varInst *v1,varInst *v2){
  v1->float_val=v2->float_val;
  v1->bool_val=v2->bool_val;
}

varInst *Mission::checkExpression(missionNode *node,int mode){

  varInst *ret=NULL;
  debug(5,node,mode,"checking expression");
  printRuntime();

  switch(node->tag){
  case DTAG_AND_EXPR:
  case DTAG_OR_EXPR:
  case DTAG_NOT_EXPR:
  case DTAG_TEST_EXPR:
    checkBoolExpr(node,mode);
    break;

  case DTAG_VAR_EXPR:
    ret=doVariable(node,mode);
    break;
  case DTAG_FMATH:
    break;
  default:
    fatalError(node,mode,"no such expression");
    assert(0);
    break;
  }
  return ret;
}
void Mission::printVarInst(varInst *vi){

  return;

    char buffer[100];
    if(vi==NULL){
      sprintf(buffer," NULL");
    }
    else{
      if(vi->type==VAR_BOOL){
	sprintf(buffer," bool  %d",vi->bool_val);
      }
      else if(vi->type==VAR_FLOAT){
	sprintf(buffer," float %f",vi->float_val);
      }
    }
    cout << vi->name << buffer << endl;
}

void Mission::printVarmap(const varInstMap & vmap){
  map<string,varInst *>::const_iterator iter;

  for(iter=vmap.begin();iter!=vmap.end();iter++){
    cout << "variable " << (*iter).first ;
    varInst *vi=(*iter).second;

    printVarInst(vi);
  }
}

void Mission::printRuntime(){
  return;
  cout << "RUNTIME" << endl;
  cout << "MODULES:" << endl;

  map<string,missionNode *>::iterator iter;
  //=runtime.modules.begin()

  for(iter=runtime.modules.begin();iter!=runtime.modules.end();iter++){
    cout << "  module " << (*iter).first ;
    printNode((*iter).second,0);
  }


  cout << "CURRENT THREAD:" << endl;

  printThread(runtime.cur_thread);
}

void Mission::printThread(missionThread *thread){
  return;
      vector<contextStack *>::const_iterator siter;

    for(siter= thread->exec_stack.begin() ; siter!=thread->exec_stack.end() ; siter++){
      contextStack *stack= *siter;
      
      vector<scriptContext *>::const_iterator iter2;

      cout << "SCRIPT CONTEXTS" << endl;

      for(iter2=stack->contexts.begin(); iter2!=stack->contexts.end() ; iter2++){
	scriptContext *context= *iter2;

	cout << "VARMAP " << endl;
	printVarmap(*(context->varinsts));
      }
    }
}

varInst *Mission::searchScopestack(string name){

  int elem=scope_stack.size()-1;
  varInst *vi=NULL;

  while(vi==NULL && elem>=0){
    missionNode *scope=scope_stack[elem];

    vi=scope->script.variables[name];

    if(vi==NULL){
      debug(5,scope,0,"variable "+name+" not found in that scope");
      //printVarmap(scope->script.variables);
    }
    else{
      debug(5,scope,0,"variable "+name+" FOUND in that scope");
      //printVarmap(scope->script.variables);
    }
    elem--;
  };
  
  return vi;
}
