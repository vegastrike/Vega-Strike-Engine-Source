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

/* *********************************************************** */

void Mission::fatalError(string message){
  cout << "fatalError: " << message << endl;
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
    missionNode *snode=*siter;
    if(snode->tag==DTAG_SCRIPT){
      doScript(snode,mode);
    }
  }
}

/* *********************************************************** */

scriptContext *Mission::makeContext(missionNode *node){
  
}

/* *********************************************************** */

scriptContext *Mission::addContext(missionNode *node)
{
  scriptContext *context=makeContext(node);
  contextStack *stack=cur_thread->exec_stack.top();
  stack->push_back(context);
}

/* *********************************************************** */

void Mission::removeContext()
{
  scriptContext *old=cur_thread->context_stack.pop(context);
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
    missionNode *snode=*siter;
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
    missionNode *snode=*siter;
    checkStatement(snode,mode);
  }
  if(mode==SCRIPT_RUN){
    removeContext();
  }
}

/* *********************************************************** */

bool Mission::doBooleanVar(missionNode *node,int mode){
  scriptVariable *var=doVariable(node,mode);

  bool ok=checkVarType(var,VAR_BOOL);

  if(!ok){
    runtimeFatal("expected a bool variable - got a different type");
    assert(0);
  }

  return var->bool_val;
}

/* *********************************************************** */

scriptVariable *Mission::lookupLocalVariable(missionNode *asknode){
  contextStack *cstack=cur_thread->exec_stack.top();
  missionNode *defnode=NULL;

  for(int i=0;i<stack->size() && defnode==NULL;i++){
    scriptContext *context=cstack[i];
    defnode=context[asknode->script.name];
  }
  if(defnode==NULL){
    return NULL;
  }

  return defnode->script.var;
}

/* *********************************************************** */

varInst *Mission::lookupModuleVariable(string mname,missionNode *asknode){
  // only when runtime
  missionNode *module_node=runtime.modules[mname];

  if(module_node==NULL){
    return NULL;
  }

  vector<missionNode *>::const_iterator siter;
  
  for(siter= module_node->subnodes.begin() ; siter!=module_node->subnodes.end() ; siter++){
    missionNode *varnode=*siter;
    if(varnode->name==asknode->name){
      return varinst;
    }
  }

  return NULL;

}

/* *********************************************************** */

varInst *Mission::lookupModuleVariable(missionNode *asknode){
  // only when runtime
  missionNode *module=cur_thread->module_stack.top();

  string mname=module->name;

  varInst *var=lookupModuleVariable(mname,asknode);


  return var;

}

/* *********************************************************** */

varInst *Mission::lookupGlobalVariable(missionNode *asknode){
  missionNode *varnode=runtime.global_variables[asknode->name];

  if(varnode==NULL){
    return NULL;
  }

  return varnode->varinst;
}

/* *********************************************************** */

scriptVariable *Mission::doVariable(missionNode *node,int mode){
  if(mode==SCRIPT_RUN){
    scriptVariable *var=lookupLocalVariable(node);
    if(var==NULL){
      // search in module namespace
      var=lookupModuleVariable(node);
      if(var==NULL){
	// search in global namespace
	var=lookupGlobalVariable(node);
	if(var==NULL){
	  fatalRuntime("did not find variable");
	  assert(0);
	}
      }
    }
    return var;
  }
  else{
    // SCRIPT_PARSE
    script.name=node->attr_value("name");
    if(script.name.empty()){
      fatalError("you have to give a variable name");
      assert(0);
    }

  }
}

/* *********************************************************** */

void Mission:doDefVar(missionNode *node,int mode){
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
      node->vartype=VAR_FLOAT;
    }
    else if(type=="string"){
      node->vartype=VAR_STRING;
    }
    else if(type=="vector"){
      node->vartype=VAR_VECTOR;
    }
    else if(type=="object"){
      node->vartype=VAR_OBJECT;
    }
    else{
      fatalError("unknown variable type");
      assert(0);
    }

    missionNode *scope=scope_stack.top();
    
    scope->variables.push_back(node);
}
