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

#include "vs_globals.h"
#include "config_xml.h"

//#include "vegastrike.h"


void Mission::DirectorStart(missionNode *node){

  cout << "DIRECTOR START" << endl;

  debuglevel=atoi(vs_config->getVariable("interpreter","debuglevel","0").c_str());

  missionThread *main_thread=new missionThread;
  runtime.thread_nr=0;
  runtime.threads.push_back(main_thread);
  runtime.cur_thread=main_thread;

  parsemode=PARSE_DECL;

  doModule(node,SCRIPT_PARSE);

  parsemode=PARSE_FULL;

  doModule(node,SCRIPT_PARSE);

  if(director==NULL){
    return;
  }


  missionNode *initgame=director->script.scripts["initgame"];

  if(initgame==NULL){
    warning("initgame not found");
  }
  else{
    runtime.cur_thread->module_stack.push_back(director);

    doScript(initgame,SCRIPT_RUN);
    runtime.cur_thread->module_stack.pop_back();
  }

  while(true){
    DirectorLoop();
#ifndef _WIN32
    sleep(1);
#endif
  }
}

void Mission::DirectorLoop(){

  if(director==NULL){
    return;
  }

  cout << "DIRECTOR LOOP" << endl;

  missionNode *gameloop=director->script.scripts["gameloop"];

  if(gameloop==NULL){
    warning("no gameloop");
    return;
  }
  else{
    runtime.cur_thread->module_stack.push_back(director);

    doScript(gameloop,SCRIPT_RUN);

    runtime.cur_thread->module_stack.pop_back();
  
    //    doModule(director,SCRIPT_RUN);
  }
}


/* *********************************************************** */

string Mission::modestring(int mode){
  if(mode==SCRIPT_PARSE){
    return "parse";
  }
  else{
    return "run";
  }
}
void Mission::fatalError(missionNode *node,int mode,string message){
  cout << "fatal (" << modestring(mode) << ") " << message << " : ";
  printNode(node,mode);
}

void Mission::runtimeFatal(string message){
  cout << "runtime fatalError: " << message << endl;
}

void Mission::warning(string message){
  cout << "warning: " << message << endl;
}


void Mission::debug(int level,missionNode *node,int mode,string message){
  if(level<=debuglevel){
    debug(node,mode,message);
  }
}

void Mission::debug(missionNode *node,int mode,string message){

  cout << "debug (" << modestring(mode) << ") " << message << " : " ;
  printNode(node,mode);
  //  cout << endl;
}

void Mission::printNode(missionNode *node,int mode){
  if(node){
    node->printNode(cout,0,0);
  }
}

/* *********************************************************** */

void Mission::doModule(missionNode *node,int mode){
  if(mode==SCRIPT_PARSE){
      string name=node->attr_value("name");
    if(parsemode==PARSE_DECL){

  
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
    }

    scope_stack.push_back(node);

    current_module=node;

    debug(5,node,mode,"added module "+name+" to list of known modules");
  }

  if(mode==SCRIPT_RUN){
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

  debug(5,node,SCRIPT_RUN,"added context for this node");
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

void Mission::doScript(missionNode *node,int mode, varInstMap *varmap){
  if(mode==SCRIPT_PARSE){
    if(parsemode==PARSE_DECL){
      node->script.name=node->attr_value("name");

      if(node->script.name.empty()){
	fatalError(node,mode,"you have to give a script name");
      }
      current_module->script.scripts[node->script.name]=node;
      node->script.nr_arguments=0;
    }
    scope_stack.push_back(node);

    
  }

  debug(5,node,mode,"executing script name="+node->script.name);

  if(mode==SCRIPT_RUN){
    addContextStack(node);
    addContext(node);
  }

  vector<easyDomNode *>::const_iterator siter;

  node->script.nr_arguments=0;

  for(siter= node->subnodes.begin() ; siter!=node->subnodes.end() ; siter++){
    missionNode *snode=(missionNode *)*siter;
    if(snode->tag==DTAG_ARGUMENTS){
      doArguments(snode,mode,varmap);
      if(mode==SCRIPT_PARSE && parsemode==PARSE_DECL){
	node->script.argument_node=snode;
      }
    }
    else{
      if(mode==SCRIPT_PARSE && parsemode==PARSE_DECL){
	// do nothing, break here
      }
      else{
	checkStatement(snode,mode);
      }
    }
  }

  if(mode==SCRIPT_RUN){
    removeContext();
    removeContextStack();
  }
  else{
    scope_stack.pop_back();
  }
}


void Mission::doArguments(missionNode *node,int mode,varInstMap *varmap){

  int nr_arguments=0;

  if(mode==SCRIPT_PARSE){
    if(parsemode==PARSE_DECL){
      vector<easyDomNode *>::const_iterator siter;
  
      for(siter= node->subnodes.begin() ; siter!=node->subnodes.end() ; siter++){
	missionNode *snode=(missionNode *)*siter;
	if(snode->tag==DTAG_DEFVAR){
	  doDefVar(snode,mode);
	  nr_arguments++;
	}
	else{
	  fatalError(node,mode,"only defvars allowed below argument node");
	  assert(0);
	}
      }
    
      node->script.nr_arguments=nr_arguments;
    }
  }

  nr_arguments=node->script.nr_arguments;

  if(mode==SCRIPT_RUN){
  if(varmap){
    int   nr_called=varmap->size();

    if(nr_arguments!=nr_called){
      fatalError(node,mode,"wrong number of args in doScript ");
      assert(0);
    }

    for(int i=0;i<nr_arguments;i++){
      missionNode *defnode=(missionNode *)node->subnodes[i];
      
      doDefVar(defnode,mode);
      varInst *vi=doVariable(defnode,mode);
      
      varInst *call_vi=(*varmap)[defnode->script.name];

      if(call_vi==NULL){
	fatalError(node,mode,"argument var "+node->script.name+" no found in varmap");
	assert(0);
      }
      assignVariable(vi,call_vi);
    }
  }
  else{
    // no varmap == 0 args
    if(nr_arguments!=0){
      fatalError(node,mode,"doScript expected to be called with arguments");
      assert(0);
    }
  }
  }

  if(mode==SCRIPT_PARSE){
    if(parsemode==PARSE_DECL){
      missionNode *exec_scope=scope_stack.back();
      exec_scope->script.nr_arguments=nr_arguments;
      node->script.nr_arguments=nr_arguments;
    }
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
    fatalError(node,mode,"expected a bool variable - got a different type");
    assert(0);
  }

  return var->bool_val;
}
/* *********************************************************** */

float Mission::doFloatVar(missionNode *node,int mode){
  varInst *var=doVariable(node,mode);

  bool ok=checkVarType(var,VAR_FLOAT);

  if(!ok){
    fatalError(node,mode,"expected a float variable - got a different type");
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
      debug(5,defnode->defvar_node,SCRIPT_RUN,"FOUND local variable defined in that node");
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
    fatalError(asknode,SCRIPT_RUN,"no such module named "+mname);
    assert(0);
    return NULL;
  }

  vector<easyDomNode *>::const_iterator siter;
  
  for(siter= module_node->subnodes.begin() ; siter!=module_node->subnodes.end() ; siter++){
    missionNode *varnode=(missionNode *)*siter;
    if(varnode->script.name==asknode->script.name){
      char buffer[200];
      sprintf(buffer,"FOUND module variable %s in that node",varnode->script.name.c_str());
      debug(4,varnode,SCRIPT_RUN,buffer);
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
	  fatalError(node,mode,"did not find variable");
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

  if(node->subnodes.size()!=1){
    fatalError(node,mode,"setvar takes exactly one argument");
    assert(0);
  }

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
  if(mode==SCRIPT_PARSE){
    string name=node->attr_value("name");
    if(name.empty()){
      fatalError(node,mode,"you have to give name to exec");
      assert(0);
    }
    node->script.name=name;

    missionNode *module=current_module;

    missionNode *script=module->script.scripts[name];

    if(script==NULL){
      fatalError(node,mode,"script "+name+" not found in module");
      assert(0);
    }

    node->script.exec_node=script;
  }
  
  missionNode *arg_node=node->script.exec_node->script.argument_node;

  int nr_arguments;

  if(arg_node==NULL){
    nr_arguments=0;
  }
  else{
    nr_arguments=arg_node->script.nr_arguments;
  }
  int nr_exec_args=node->subnodes.size();

  if(nr_arguments!=nr_exec_args){
    fatalError(node,mode,"wrong nr of arguments in doExec");
    assert(0);
  }

  varInstMap *varmap=NULL;
 if(nr_arguments>0){
  varmap=new varInstMap;

  for(int i=0;i<nr_arguments;i++){
    missionNode *defnode=(missionNode *)arg_node->subnodes[i];
    missionNode *callnode=(missionNode *)node->subnodes[i];

    varInst *vi=new varInst;
    vi->type=defnode->script.vartype;


    if(defnode->script.vartype==VAR_FLOAT){
      debug(4,node,mode,"doExec checking floatExpr");
      float res=checkFloatExpr(callnode,mode);
      vi->float_val=res;
    }
    else if(defnode->script.vartype==VAR_BOOL){
      debug(4,node,mode,"doExec checking boolExpr");
      bool ok=checkBoolExpr(callnode,mode);
      vi->bool_val=ok;
    }
    else{
      fatalError(node,mode,"unsupported vartype in doExec");
      assert(0);
    }

    (*varmap)[defnode->script.name]=vi;
    
  }
 }

  if(mode==SCRIPT_RUN){
    // SCRIPT_RUN

    debug(4,node,mode,"executing "+node->script.name);

    doScript(node->script.exec_node,mode,varmap);

    delete varmap;
  }

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
  tagmap["arguments"]=DTAG_ARGUMENTS;
  tagmap["globals"]=DTAG_GLOBALS;
}

void Mission::assignVariable(varInst *v1,varInst *v2){
  if(v1->type!=v2->type){
    fatalError(NULL,SCRIPT_RUN,"wrong types in assignvariable");
    assert(0);
  }

  v1->float_val=v2->float_val;
  v1->bool_val=v2->bool_val;
}

varInst *Mission::checkExpression(missionNode *node,int mode){

  varInst *ret=NULL;
  debug(0,node,mode,"checking expression");
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
