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

varInst * Mission::doObjectVar(missionNode *node,int mode){
  varInst *var=doVariable(node,mode);

  bool ok=checkVarType(var,VAR_OBJECT);

  debug(3,node,mode,"doObjectVar got variable :");
  printVarInst(3,var);

  if(!ok){
    fatalError(node,mode,"expected a object variable - got a different type");
    assert(0);
  }
  
  return var;
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

void Mission::doGlobals(missionNode *node,int mode){

  if(mode==SCRIPT_RUN || (mode==SCRIPT_PARSE && parsemode==PARSE_FULL)){
    // nothing to do
    return;
  }

  debug(3,node,mode,"doing global variables");

  vector<easyDomNode *>::const_iterator siter;

  for(siter= node->subnodes.begin() ; siter!=node->subnodes.end() && !have_return(mode) ; siter++){
    missionNode *snode=(missionNode *)*siter;
    if(snode->tag==DTAG_DEFVAR){
      doDefVar(snode,mode,true);
    }
    else{
      fatalError(node,mode,"only defvars allowed below globals node");
      assert(0);
    }
  }

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
      missionNode *global_var=runtime.global_variables[node->script.name];
      if(global_var==NULL){
	fatalError(node,mode,"no variable "+node->script.name+" found on the scopestack (dovariable)");
	assert(0);
      }
      vi=global_var->script.varinst;
    }

    return vi;


  }
}

/* *********************************************************** */

void Mission::doDefVar(missionNode *node,int mode,bool global_var){
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

  // SCRIPT_PARSE

  node->script.name=node->attr_value("name");
    if(node->script.name.empty()){
      fatalError(node,mode,"you have to give a variable name");
      assert(0);
    }

    string value=node->attr_value("initvalue");

    debug(5,node,mode,"defining variable "+node->script.name);

    string type=node->attr_value("type");
    //    node->initval=node->attr_value("init");

    node->script.vartype=vartypeFromString(type);

    missionNode *scope=NULL;

    if(global_var==false){
      scope=scope_stack.back();
    }
    
  varInst *vi=new varInst;
  vi->defvar_node=node;
  vi->block_node=scope;
  vi->type=node->script.vartype;
  vi->name=node->script.name;

  if(global_var || scope->tag==DTAG_MODULE){
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
      else{
	fatalError(node,mode,"this datatype can;t be initialized");
	assert(0);
      }
      printVarInst(vi);
    }
  }
  else{
    //local variable
    if(!value.empty()){
      fatalError(node,mode,"initvalue is not allowed for a local variable");
      assert(0);
    }
  }

  node->script.varinst=vi;//FIXME (not for local var)

  if(global_var){
    debug(3,node,mode,"defining global variable");
    runtime.global_variables[node->script.name]=node;
    printGlobals(3);
  }
  else{
    scope->script.variables[node->script.name]=vi;
    node->script.context_block_node=scope;
    debug(5,scope,mode,"defined variable in that scope");
  }

}

/* *********************************************************** */

void Mission::doSetVar(missionNode *node,int mode){

  if(mode==SCRIPT_PARSE){
    node->script.name=node->attr_value("name");
    if(node->script.name.empty()){
      fatalError(node,mode,"you have to give a variable name");
    }
  }

  debug(3,node,mode,"trying to set variable "+node->script.name);

  //    varInst *var_expr=checkExpression((missionNode *)node->subnodes[0],mode);

  if(node->subnodes.size()!=1){
    fatalError(node,mode,"setvar takes exactly one argument");
    assert(0);
  }

  missionNode *expr=(missionNode *)node->subnodes[0];

  if(mode==SCRIPT_PARSE){
    varInst *vi=searchScopestack(node->script.name);

    if(vi==NULL){
      missionNode *global_var=runtime.global_variables[node->script.name];
      if(global_var==NULL){

	fatalError(node,mode,"no variable "+node->script.name+" found on the scopestack (setvar)");
	assert(0);
      }
      
      vi=global_var->script.varinst;
    }

    if(vi->type==VAR_BOOL){
      bool res=checkBoolExpr(expr,mode);
    }
    else if(vi->type==VAR_FLOAT){
      float res=checkFloatExpr(expr,mode);
    }
    else if(vi->type==VAR_OBJECT){
      debug(3,node,mode,"setvar object");
      varInst *ovi=checkObjectExpr(expr,mode);
    }
    else{
      fatalError(node,mode,"unsupported type in setvar");
      assert(0);
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
    else if(var_inst->type==VAR_OBJECT){
      debug(3,node,mode,"setvar object (before)");
      varInst *ovi=checkObjectExpr(expr,mode);
      assignVariable(var_inst,ovi);
      debug(3,node,mode,"setvar object left,right");
      printVarInst(3,var_inst);
      printVarInst(3,ovi);
    }
    else{
      fatalError(node,mode,"unsupported datatype");
      assert(0);
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

/* *********************************************************** */

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
#if 0
    else if(typestr=="string"){
      node->script.vartype=VAR_STRING;
    }
    else if(typestr=="vector"){
      node->script.vartype=VAR_VECTOR;
    }
#endif
    else if(typestr=="object"){
      fatalError(node,mode,"you cant have a const object");
      assert(0);
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

/* *********************************************************** */

void Mission::assignVariable(varInst *v1,varInst *v2){
  if(v1->type!=v2->type){
    fatalError(NULL,SCRIPT_RUN,"wrong types in assignvariable");
    saveVarInst(v1,cout);
    saveVarInst(v2,cout);
    assert(0);
  }
  if(v1->type==VAR_OBJECT){
    if(v1->objectname.empty()){
      // the object has not been set
      // we set it below
    }
    else{
      if(v1->objectname!=v2->objectname){
	fatalError(NULL,SCRIPT_RUN,"wrong object types in assignment ("+v1->objectname+" , "+v2->objectname);
	assert(0);
      }
    }
  }
  v1->float_val=v2->float_val;
  v1->bool_val=v2->bool_val;
  v1->objectname=v2->objectname;
  v1->object=v2->object;
}

/* *********************************************************** */

var_type Mission::vartypeFromString(string type){
  var_type vartype;

    if(type=="float"){
      vartype=VAR_FLOAT;
    }
    else if(type=="bool"){
      vartype=VAR_BOOL;
    }
#if 0
    else if(type=="string"){
      vartype=VAR_STRING;
    }
    else if(type=="vector"){
      vartype=VAR_VECTOR;
    }
#endif
    else if(type=="object"){
      vartype=VAR_OBJECT;
    }
    else{
      fatalError(NULL,SCRIPT_PARSE,"unknown var type "+type);
      vartype=VAR_FAILURE;
    }
    return vartype;

}

/* *********************************************************** */

void Mission::saveVariables(ostream& out){

  out << "<saved-variables>" << endl << endl;

  out << "    <globals>" << endl;
  map<string,missionNode *>::iterator iter;

  for(iter=runtime.global_variables.begin();iter!=runtime.global_variables.end();iter++){
    string name=(*iter).first;
    missionNode *gnode=(*iter).second;

    varInst *vi=gnode->script.varinst;

    //    out << "      <defvar name=\"" << name << "\" type=\"" << typestr << "\" value=\"" << valuestr << "\" />" << endl;

    out << "      <defvar name=\"" << name << "\" ";

    saveVarInst(vi,out);

    out << "/> " << endl;
  }

  out << "    </globals>" << endl << endl;

  {
  // modules
  map<string,missionNode *>::iterator iter;

  for(iter=runtime.modules.begin();iter!=runtime.modules.end();iter++){
    string mname=(*iter).first ;
    missionNode *module_node=(*iter).second;

    out << "    <module name=\"" << mname << "\" >" << endl;

    // each module

    vector<easyDomNode *>::const_iterator siter;
  
    for(siter= module_node->subnodes.begin() ; siter!=module_node->subnodes.end() ; siter++){

    missionNode *varnode=(missionNode *)*siter;
    if(varnode->tag==DTAG_DEFVAR){
      // found a module var node
      out << "      <defvar name=\"" << varnode->script.name << "\" ";
      saveVarInst(varnode->script.varinst,out);
      out << "/> " << endl;
    }
  }

    out << "    </module>" << endl << endl;
  }
  }

  out << endl << "</saved-variables>" << endl;
}

