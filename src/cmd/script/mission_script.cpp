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


/* *********************************************************** */

void Mission::DirectorStart(missionNode *node){

  cout << "DIRECTOR START" << endl;

  debuglevel=atoi(vs_config->getVariable("interpreter","debuglevel","0").c_str());

  missionThread *main_thread=new missionThread;
  runtime.thread_nr=0;
  runtime.threads.push_back(main_thread);
  runtime.cur_thread=main_thread;

  parsemode=PARSE_DECL;

  doModule(node,SCRIPT_PARSE);

  easyDomFactory<missionNode> *importf=new easyDomFactory<missionNode>();



  while(import_stack.size()>0){
    missionNode *import=import_stack.back();
    import_stack.pop_back();

    missionNode *module=runtime.modules[import->script.name];
    if(module==NULL){
      debug(3,node,SCRIPT_PARSE,"loading module "+import->script.name);

      string filename="modules/"+import->script.name+".module";

      missionNode *import_top=importf->LoadXML(filename.c_str());

      if(import_top==NULL){
	fatalError(node,SCRIPT_PARSE,"could not load module file "+filename);
	assert(0);
      }

      import_top->Tag(&tagmap);

      doModule(import_top,SCRIPT_PARSE);

    }
    else{
      debug(3,node,SCRIPT_PARSE,"already have module "+import->script.name);
    }
  }



  parsemode=PARSE_FULL;

  doModule(node,SCRIPT_PARSE);

  map<string,missionNode *>::iterator iter;
  //=runtime.modules.begin()

  for(iter=runtime.modules.begin();iter!=runtime.modules.end();iter++){
    string mname=(*iter).first ;
    missionNode *mnode=(*iter).second;

    if(mname!="director"){
      cout << "  parsing full module " << mname << endl;
      doModule(mnode,SCRIPT_PARSE);
    }
  }


  if(director==NULL){
    return;
  }


  missionNode *initgame=director->script.scripts["initgame"];

  if(initgame==NULL){
    warning("initgame not found");
  }
  else{
    runtime.cur_thread->module_stack.push_back(director);

    varInst *vi=doScript(initgame,SCRIPT_RUN);

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

  //  saveVariables(cout);

  missionNode *gameloop=director->script.scripts["gameloop"];

  if(gameloop==NULL){
    warning("no gameloop");
    return;
  }
  else{
    runtime.cur_thread->module_stack.push_back(director);

    varInst *vi=doScript(gameloop,SCRIPT_RUN);

    runtime.cur_thread->module_stack.pop_back();
  
    //    doModule(director,SCRIPT_RUN);
  }
}


/* *********************************************************** */

void Mission::doImport(missionNode *node,int mode){
  if(mode==SCRIPT_PARSE && parsemode==PARSE_DECL){
    string name=node->attr_value("name");
    if(name.empty()){
      fatalError(node,mode,"you have to give a name to import");
      assert(0);
    }
    node->script.name=name;
    import_stack.push_back(node);
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
      varInst *vi=doScript(snode,mode);
    }
    else if(snode->tag==DTAG_DEFVAR){
      doDefVar(snode,mode);
    }
    else if(snode->tag==DTAG_GLOBALS){
      doGlobals(snode,mode);
    }
    else if(snode->tag==DTAG_IMPORT){
      doImport(snode,mode);
    }
    
    else{
      fatalError(node,mode,"unkown node type");
      assert(0);
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

  cstack->return_value=NULL;

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

varInst * Mission::doScript(missionNode *node,int mode, varInstMap *varmap){
  if(mode==SCRIPT_PARSE){
    current_script=node;

    if(parsemode==PARSE_DECL){
      node->script.name=node->attr_value("name");

      if(node->script.name.empty()){
	fatalError(node,mode,"you have to give a script name");
      }
      current_module->script.scripts[node->script.name]=node;
      node->script.nr_arguments=0;

      string retvalue=node->attr_value("return");
      if(retvalue.empty()){
	node->script.vartype=VAR_VOID;
      }
      else{
	node->script.vartype=vartypeFromString(retvalue);
      }
    }
    scope_stack.push_back(node);

    
  }

  debug(5,node,mode,"executing script name="+node->script.name);

  if(mode==SCRIPT_RUN){
    addContextStack(node);
    addContext(node);
  }

  vector<easyDomNode *>::const_iterator siter;

     if(mode==SCRIPT_PARSE && parsemode==PARSE_DECL){
       node->script.nr_arguments=0;
       node->script.argument_node=NULL;
     }

  for(siter= node->subnodes.begin() ; siter!=node->subnodes.end() && !have_return(mode) ; siter++){
    missionNode *snode=(missionNode *)*siter;
    if(snode->tag==DTAG_ARGUMENTS){
	doArguments(snode,mode,varmap);
      if(mode==SCRIPT_PARSE && parsemode==PARSE_DECL){
	node->script.argument_node=snode;
      }
      char buffer[200];
      sprintf(buffer,"nr of arguments=%d",node->script.nr_arguments);
      debug(3,node,mode,buffer);
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
    contextStack *cstack=runtime.cur_thread->exec_stack.back();
    varInst *vi=cstack->return_value;
    if(vi!=NULL){
      if(node->script.vartype!=vi->type){
	fatalError(node,mode,"doScript: return type not set correctly");
	assert(0);
      }
    }
    else{
      // vi==NULL
      if(node->script.vartype!=VAR_VOID){
	fatalError(node,mode,"no return set from doScript");
	assert(0);
      }
    }
    removeContextStack();

    return vi;
  }
  else{
    scope_stack.pop_back();
    return NULL;
  }
}


/* *********************************************************** */

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

void Mission::doReturn(missionNode *node,int mode){
  if(mode==SCRIPT_PARSE){
    missionNode *script=current_script;

    node->script.exec_node=script;
    
}
  int len=node->subnodes.size();
  varInst *vi=new varInst;

  missionNode *script=node->script.exec_node;

  if(script->script.vartype==VAR_VOID){
    if(len!=0){
      fatalError(node,mode,"script returning void, but return statement with node");
      assert(0);
    }
  }
  else{
    // return something non-void

    if(len!=1){
      fatalError(node,mode,"return statement needs only one subnode");
      assert(0);
    }

    missionNode *expr=(missionNode *)node->subnodes[0];

    if(script->script.vartype==VAR_BOOL){
      bool res=checkBoolExpr(expr,mode);
      vi->bool_val=res;
    }
    else if(script->script.vartype==VAR_FLOAT){
      float res=checkFloatExpr(expr,mode);
      vi->float_val=res;
    }
    else if(script->script.vartype==VAR_OBJECT){
      varInst *vi2=checkObjectExpr(expr,mode);
      vi->type=VAR_OBJECT;
      //      if(mode==SCRIPT_RUN){
	assignVariable(vi,vi2);
	//      }
    }
    else{
      fatalError(node,mode,"unkown variable type");
      assert(0);
    }
  }

  if(mode==SCRIPT_RUN){

    contextStack *cstack=runtime.cur_thread->exec_stack.back();

    vi->type=script->script.vartype;

    cstack->return_value=vi;
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
  
  for(siter= node->subnodes.begin() ; siter!=node->subnodes.end() && !have_return(mode); siter++){
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

varInst *Mission::doExec(missionNode *node,int mode){
  if(mode==SCRIPT_PARSE){
    string name=node->attr_value("name");
    if(name.empty()){
      fatalError(node,mode,"you have to give name to exec");
      assert(0);
    }
    node->script.name=name;

    string use_modstr=node->attr_value("module");
    missionNode *module=NULL;
    missionNode *script=NULL;
    if(!use_modstr.empty()){
      //missionNode *use_script=lookupScript(name,use_modstr);
      
      module=runtime.modules[use_modstr];
    }
    else{
      module=current_module;
    }

    if(module==NULL){
      fatalError(node,mode,"module "+use_modstr+" not found");
      assert(0);
    }
      script=module->script.scripts[name];

      if(script==NULL){
	fatalError(node,mode,"script "+name+" not found in module "+use_modstr);
	assert(0);
      }
    
    node->script.exec_node=script;
    node->script.vartype=script->script.vartype;
    node->script.module_node=module;
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
    char buffer[200];
    sprintf(buffer,"wrong nr of arguments in doExec=%d doScript=%d",nr_exec_args,nr_arguments);
    fatalError(node,mode,buffer);
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
    else if(defnode->script.vartype==VAR_OBJECT){
      debug(3,node,mode,"doExec checking objectExpr");
      varInst *ovi=checkObjectExpr(callnode,mode);
      vi->type=VAR_OBJECT;
      if(mode==SCRIPT_RUN){
	assignVariable(vi,ovi);
      }
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

    missionNode *module=node->script.module_node;

    runtime.cur_thread->module_stack.push_back(module);

    varInst *vi=doScript(node->script.exec_node,mode,varmap);

    runtime.cur_thread->module_stack.pop_back();

    delete varmap;
    return vi;
  }

  // SCRIPT_PARSE

  varInst *vi=new varInst;

  vi->type=node->script.exec_node->script.vartype;

  return vi;
}


