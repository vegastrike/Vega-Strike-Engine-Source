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

/* *********************************************************** */

varInst *Mission::checkObjectExpr(missionNode *node,int mode){
  varInst *res=NULL;

  if(node->tag==DTAG_VAR_EXPR){
      res=doObjectVar(node,mode);
    }
    else if(node->tag==DTAG_CALL){
      varInst *vi=doCall(node,mode);
      if(vi->type==VAR_OBJECT){
	res=vi;
      }
      else if(vi->type==VAR_ANY && mode==SCRIPT_PARSE){
	res=vi;
      }
      else{
	fatalError(node,mode,"expected a object call, got a different one");
	assert(0);
      }
    }
    else if(node->tag==DTAG_EXEC){
      varInst *vi=doExec(node,mode);
      if(vi==NULL){
	fatalError(node,mode,"doExec returned NULL");
	assert(0);
      }
      else if(node->script.vartype==VAR_OBJECT){
	res=vi;
      }
      else{
	fatalError(node,mode,"expected a object exec, got a different one");
	assert(0);
      }
    }
    else if(node->tag==DTAG_CONST){
      varInst *vi=doConst(node,mode);
      if(vi->type==VAR_OBJECT && vi->objectname=="string"){
	res=vi;
      }
      else{
	fatalError(node,mode,"expected a string const, got a different one: "+vi->objectname);
	assert(0);
      }
    }
 
    else{
      fatalError(node,mode,"no such object expression tag");
      assert(0);
    }

    return res;

}

/* *********************************************************** */

float Mission::doFMath(missionNode *node,int mode){
  //  if(mode==SCRIPT_PARSE){
    string mathname=node->attr_value("math");

    int len=node->subnodes.size();
    if(len<2){
      fatalError(node,mode,"fmath needs at least 2 arguments");
      assert(0);
    }

    float res=checkFloatExpr((missionNode *)node->subnodes[0],mode);

    char buffer[200];
    sprintf(buffer,"fmath: 1st expr returns %f",res);
    debug(4,node,mode,buffer);

    for(int i=1;i<len;i++){
      float res2=checkFloatExpr((missionNode *)node->subnodes[i],mode);
      if(mode==SCRIPT_RUN){
	if(mathname=="+"){
	  res=res+res2;
	}
	else if(mathname=="-"){
	  res=res-res2;
	}
	else if(mathname=="*"){
	  res=res*res2;
	}
	else if(mathname=="/"){
	  res=res/res2;
	}
	else{
	  fatalError(node,mode,"no such fmath expression");
	  assert(0);
	}
      }
    }

    if(mode==SCRIPT_RUN){
      return res;
    }
    return 0.0;
}

/* *********************************************************** */

float Mission::checkFloatExpr(missionNode *node,int mode){
  float res=0.0;

    if(node->tag==DTAG_VAR_EXPR){
      res=doFloatVar(node,mode);
    }
    else if(node->tag==DTAG_FMATH){
      res=doFMath(node,mode);
    }
    else if(node->tag==DTAG_CONST){
      varInst *vi=doConst(node,mode);
      if(vi && vi->type==VAR_FLOAT){
	res=vi->float_val;
      }
      else{
	fatalError(node,mode,"expected a float const, got a different one");
	assert(0);
      }
    }
    else if(node->tag==DTAG_CALL){
      varInst *vi=doCall(node,mode);
      if(vi->type==VAR_FLOAT){ 
	res=vi->float_val;
      }
      else if(vi->type==VAR_ANY && mode==SCRIPT_PARSE){
	res=vi->float_val;
      }
      else{
	fatalError(node,mode,"expected a float call, got a different one");
	assert(0);
      }
    }
    else if(node->tag==DTAG_EXEC){
      varInst *vi=doExec(node,mode);
      if(vi==NULL){
	fatalError(node,mode,"doExec returned NULL");
	assert(0);
      }
      else if(node->script.vartype==VAR_FLOAT){
	res=vi->float_val;
      }
      else{
	fatalError(node,mode,"expected a float exec, got a different one");
	assert(0);
      }
    }
    else{
      fatalError(node,mode,"no such float expression tag");
      assert(0);
    }

    return res;
}

/* *********************************************************** */

bool Mission::checkBoolExpr(missionNode *node,int mode){
  bool ok;

  // no difference between parse/run

    if(node->tag==DTAG_AND_EXPR){
      ok=doAndOr(node,mode);
    }
    else if(node->tag==DTAG_OR_EXPR){
      ok=doAndOr(node,mode);
    }
    else if(node->tag==DTAG_NOT_EXPR){
      ok=doNot(node,mode);
    }
    else if(node->tag==DTAG_TEST_EXPR){
      ok=doTest(node,mode);
    }
    else if(node->tag==DTAG_VAR_EXPR){
      ok=doBooleanVar(node,mode);
    }
    else if(node->tag==DTAG_CONST){
      varInst *vi=doConst(node,mode);
      if(vi->type==VAR_BOOL){
	ok=vi->bool_val;
      }
      else{
	fatalError(node,mode,"expected a bool const, got a different one");
	assert(0);
      }
    }
    else if(node->tag==DTAG_CALL){
      varInst *vi=doCall(node,mode);
      if(vi->type==VAR_BOOL){
	ok=vi->bool_val;
      }
      else if(vi->type==VAR_ANY && mode==SCRIPT_PARSE){
	ok=vi->bool_val;
      }
      else{
	fatalError(node,mode,"expected a bool call, got a different one");
	assert(0);
      }
    }

    else if(node->tag==DTAG_EXEC){
      varInst *vi=doExec(node,mode);
      if(vi==NULL){
	fatalError(node,mode,"doExec returned NULL");
	assert(0);
 	// parsing?
      }
      else if(node->script.vartype==VAR_BOOL){
	ok=vi->bool_val;
      }
      else{
	fatalError(node,mode,"expected a bool exec, got a different one");
	assert(0);
      }
    }


    else{
      fatalError(node,mode,"no such boolean expression tag");
      assert(0);
    }

    return ok;
}

/* *********************************************************** */

bool Mission::doAndOr(missionNode *node,int mode){
  bool ok;

  // no difference between parse/run

  if(node->tag==DTAG_AND_EXPR){
    ok=true;
  }
  else if(node->tag==DTAG_OR_EXPR){
    ok=false;
  }

  vector<easyDomNode *>::const_iterator siter;
  
  int i=0;
  for(siter= node->subnodes.begin() ; siter!=node->subnodes.end() ; siter++,i++){
    missionNode *snode=(missionNode *)*siter;
    bool res=checkBoolExpr(snode,mode);

    if(node->tag==DTAG_AND_EXPR){
      ok=ok && res;
    }
    else if(node->tag==DTAG_OR_EXPR){
      ok=ok || res;
    }

  }

  if(mode==SCRIPT_PARSE){
    if(i<2){
      warning("less than two arguments for and/or");
    }
  }
  return ok;
}

/* *********************************************************** */

bool Mission::doNot(missionNode *node,int mode){
  bool ok;

  // no difference between parse/run

  missionNode *snode=(missionNode *)node->subnodes[0];
  
  if(snode){
    ok=checkBoolExpr(snode,mode);

    return !ok;
  }
  else{
    fatalError(node,mode,"no subnode in not");
    assert(0);
    return false; // we'll never get here
  }
}

/* *********************************************************** */

bool Mission::doTest(missionNode *node,int mode){
  if(mode==SCRIPT_PARSE){
    string teststr=node->attr_value("test");
    if(teststr.empty()){
      fatalError(node,mode,"you have to give test an argument what to test");
      assert(0);
    }

    if(teststr=="gt"){
      node->script.tester=TEST_GT;
    }
    else if(teststr=="lt"){
      node->script.tester=TEST_LT;
    }
    else if(teststr=="eq"){
      node->script.tester=TEST_EQ;
    }
    else if(teststr=="ne"){
      node->script.tester=TEST_NE;
    }
    else if(teststr=="ge"){
      node->script.tester=TEST_GE;
    }
    else if(teststr=="le"){
      node->script.tester=TEST_LE;
    }
#if 0
    else if(teststr=="between"){
      node->script.tester=TEST_BETWEEN;
    }
#endif
    else {
      fatalError(node,mode,"unknown test argument for test");
      assert(0);
    }

    vector<easyDomNode *>::const_iterator siter;
#if 0
    int i=0;
    for(siter= node->subnodes.begin() ; siter!=node->subnodes.end() && i<2; siter++){
      missionNode *snode=(missionNode *)*siter;
      (node->script.test_arg)[i]=snode;
    }
    if(i<2){
      fatalError(node,mode,"a test-expr needs exact two subnodes");
      assert(0);
    }
#endif

    int len=node->subnodes.size();
    if(len!=2){
      fatalError(node,mode,"a test-expr needs exact two subnodes");
      assert(0);
    }
    
    node->script.test_arg[0]=(missionNode *)node->subnodes[0];
    node->script.test_arg[1]=(missionNode *)node->subnodes[1];

  } // end of parse

    float arg1=checkFloatExpr(node->script.test_arg[0],mode);
    float arg2=checkFloatExpr(node->script.test_arg[1],mode);
    bool res=false;

    if(SCRIPT_RUN){
      switch(node->script.tester){
      case TEST_GT:
	res=(arg1>arg2);
	break;
      case TEST_LT:
	res=(arg1<arg2);
	break;
      case TEST_EQ:
	res=(arg1==arg2);
	break;
      case TEST_NE:
	res=(arg1!=arg2);
	break;
      case TEST_GE:
	res=(arg1>=arg2);
	break;
      case TEST_LE:
	res=(arg1<=arg2);
	break;
      default:
	fatalError(node,mode,"no valid tester");
	assert(0);
      }
    }

    return res;

}



/* *********************************************************** */

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
