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

void Mission::doDirector(tagDomNode *node,int mode){
  director=node;

  vector<easyDomNode *>::const_iterator siter;
  
  for(siter= node->subnodes.begin() ; siter!=node->subnodes.end() ; siter++){
    tagDomNode *snode=*siter;
    if(snode->tag==DTAG_SCRIPT){
      doScript(snode,mode);
    }
  }
}

void Mission::doScript(tagDomNode *node,int mode){
  if(mode==SCRIPT_PARSE){
    script.name=node->attr_value("name");

    if(script.name.empty()){
      fatalError("you have to give a script name");
    }
  }

  vector<easyDomNode *>::const_iterator siter;
  
  for(siter= node->subnodes.begin() ; siter!=node->subnodes.end() ; siter++){
    tagDomNode *snode=*siter;
    checkStatement(snode,mode);
  }
}

void Mission::checkStatement(tagDomNode *node,int mode){
    if(node->tag==DTAG_IF){
      doIf(node,mode);
    }
    else if(node->tag==DTAG_BLOCK){
      doBlock(node,mode);
    }
    else if(node->tag==DTAG_SETVAR){
      doSetVar(node,mode);
    }
    else if(node->tag==DTAG_EXEC){
      doExec(node,mode);
    }
    else if(node->tag==DTAG_CALL){
      doCall(node,mode);
    }
    else if(node->tag==DTAG_WHILE){
      doWhile(node,mode);
    }
}

void Mission::doIf(tagDomNode *node,int mode){
  if(mode==SCRIPT_PARSE){
    vector<easyDomNode *>::const_iterator siter;
  
    int i=0;
    for(siter= node->subnodes.begin() ; siter!=node->subnodes.end() && i<3; siter++){
      tagDomNode *snode=*siter;
      if_block[i]=snode;
    }
    if(i<3){
      fatalError("an if-statement needs exact three subnodes");
    }
  }

  bool ok=checkBoolExpr(if_block[0],mode);

  if(mode==SCRIPT_PARSE){
    checkStatement(if_block[1],mode);
    checkStatement(if_block[2],mode);
  }
  else{
    if(ok){
      checkStatement(if_block[1],mode);
    }
    else{
      checkStatement(if_block[2],mode);
    }
  }
}

bool Mission::checkBoolExpr(tagDomNode *node,int mode){
  bool ok;

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
}

bool Mission::doAndOr(tagDomNode *node,int mode){
  bool ok;
  if(node->tag==DTAG_AND_EXPR){
    ok=true;
  }
  else if(node->tag==DTAG_OR_EXPR){
    ok=false;
  }

  vector<easyDomNode *>::const_iterator siter;
  
  int i=0;
  for(siter= node->subnodes.begin() ; siter!=node->subnodes.end() ; siter++,i++){
    tagDomNode *snode=*siter;
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

bool Mission::doNot(tagDomNode *node,int mode){
  bool ok;

  tagDomNode *snode=node->subnodes.begin();
  
  ok=checkBoolExpr(node,mode);

  return !ok;
}

bool Mission::doTest(tagDomNode *node,int mode){
  if(mode==SCRIPT_PARSE){
    string teststr=node->attr_value("test");
    if(teststr.empty()){
      fatalError("you have to give test an argument what to test");
      assert(0);
    }

    if(teststr=="gt"){
      script.tester=TEST_GT;
    }
    else if(teststr=="lt"){
      script.tester=TEST_LT;
    }
    else if(teststr=="eq"){
      script.tester=TEST_EQ;
    }
    else if(teststr=="ge"){
      script.tester=TEST_GE;
    }
    else if(teststr=="le"){
      script.tester=TEST_LE;
    }
#if 0
    else if(teststr=="between"){
      script.tester=TEST_BETWEEN;
    }
#endif
    else {
      fatalError("unknown test argument for test");
      assert(0);
    }

    int i=0;
    for(siter= node->subnodes.begin() ; siter!=node->subnodes.end() && i<2; siter++){
      tagDomNode *snode=*siter;
      test_arg[i]=snode;
    }
    if(i<2){
      fatalError("a test-expr needs exact two subnodes");
      assert(0);
    }

  }

    float arg1=checkFloatExpr(test_arg[0]);
    float arg2=checkFloatExpr(test_arg[1]);
    bool res=false;

    if(SCRIPT_RUN){
      switch(script.tester){
      case TEST_GT:
	res=(arg1>arg2);
	break;
      case TEST_LT:
	res=(arg1<arg2);
	break;
      case TEST_EQ:
	res=(arg1==arg2);
	break;
      case TEST_GE:
	res=(arg1>=arg2);
	break;
      case TEST_LE:
	res=(arg1<=arg2);
	break;
      default:
	fatalError("no valid tester");
	assert(0);
      }

      return res;
    }
}

void Mission::doBlock(tagDomNode *node,int mode){
    vector<easyDomNode *>::const_iterator siter;
  
  for(siter= node->subnodes.begin() ; siter!=node->subnodes.end() ; siter++){
    tagDomNode *snode=*siter;
    checkStatement(snode,mode);
  }

}

void Mission::doWhile(tagDomNode *node,int mode){

  if(SCRIPT_PARSE){
    int i=0;
    for(siter= node->subnodes.begin() ; siter!=node->subnodes.end() && i<2; siter++){
      tagDomNode *snode=*siter;
      while_arg[i]=snode;
    }

    if(i<2){
      fatalError("a while-expr needs exact two subnodes");
      assert(0);
    }

    bool res=doBoolExpr(while_arg[0],mode);

    checkStatement(while_arg[1],mode);
  }
  else{
    // runtime
    while(doBoolExpr(while_arg[0],mode)){
      checkStatement(while_arg[1],mode);
    }
  }
}
