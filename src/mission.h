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
  xml Mission written by Alexander Rawass <alexannika@users.sourceforge.net>
*/

#ifndef _MISSION_H_
#define _MISSION_H_

#include <map>
#include <expat.h>
#include <string>
#include "xml_support.h"
#include "easydom.h"

using std::string;

using XMLSupport::AttributeList;

/* *********************************************************** */

class Flightgroup {
 public:
  string name; // flightgroup name
  string faction;
  string type; // unit type
  string ainame; // .agg.xml and .int.xml get appended to this
  int waves,nr_ships; // number of waves, number of ships per wave
  float pos[3];
  float rot[3];

  int flightgroup_nr; // running fg number
  int ship_nr; // total ships nr

  easyDomNode *domnode;

  map<string,string> ordermap;
};

/* *********************************************************** */

#define SCRIPT_PARSE 0
#define SCRIPT_RUN 1

enum tag_type { 
  DTAG_UNKNOWN,
  DTAG_MODULE, DTAG_SCRIPT, DTAG_IF, DTAG_BLOCK,
  DTAG_SETVAR, DTAG_EXEC, DTAG_CALL, DTAG_WHILE,
  DTAG_AND_EXPR,DTAG_OR_EXPR,DTAG_NOT_EXPR,DTAG_TEST_EXPR,
  DTAG_FMATH,DTAG_VMATH,
  DTAG_VAR_EXPR
};

enum var_type { VAR_BOOL,VAR_FLOAT,VAR_VECTOR,VAR_OBJECT,VAR_STRING,VAR_VOID };

enum tester_type { TEST_GT,TEST_LT,TEST_EQ,TEST_GE,TEST_LE };

/* *********************************************************** */

class varInst {
 public:
  string name;
  var_type type;

  float  float_val;
  bool   bool_val;
  vector<varInst> vector_val;
  string string_val;
};

/* *********************************************************** */

class varInstMap : public map<string,varInst *> {
};

/* *********************************************************** */

class scriptContext {
 public:
  varInstMap *varinsts;
};

/* *********************************************************** */

class contextStack {
 public:
 vector<scriptContext *> contexts;
};

/* *********************************************************** */

class missionNode;

class missionThread {
 public:
  stack<contextStack *> exec_stack;
  stack<missionNode *> module_stack;
};

/* *********************************************************** */

class missionNode : public tagDomNode {
 public:
  struct script_t {
    string name; // script,defvar,module
    vector<missionNode *> variables; // script,module
    varInst *varinst; // defvar
    missionNode *if_block[3]; // if
    missionNode *while_arg[2]; // while
    int tester; // test
    missionNode *test_arg[2]; // test
    enum var_type vartype; // defvar
    string initval;
  } script;
};

/* *********************************************************** */

class Mission {
 public:
  Mission(char *configfile);

  int number_of_flightgroups,number_of_ships;

  vector<Flightgroup *> flightgroups;

  Flightgroup *findFlightgroup(string fg_name);

  string getVariable(string name,string defaultval);

  void GetOrigin(float pos[3],string &planetname);

 private:
  //  string getVariable(easyDomNode *section,string name,string defaultval);

  easyDomNode *variables;
  easyDomNode *origin_node;

  tagDomNode *director;

  struct Runtime {
    vector<missionThread *> threads;
    map<string,missionNode *> modules;
    int thread_nr;
    missionThread *cur_thread;
    map<string,missionNode *> global_variables;
  } runtime;

  // used only for parsing
  stack<missionNode *> scope_stack;

  bool checkMission(easyDomNode *node);
  void doVariables(easyDomNode *node);
  void checkVar(easyDomNode *node);
  void doFlightgroups(easyDomNode *node);
  void doOrder(easyDomNode *node,Flightgroup *fg);
  void checkFlightgroup(easyDomNode *node);
  bool doPosition(easyDomNode *node,float pos[3]);
  bool doRotation(easyDomNode *node,float rot[3]);
  void doOrigin(easyDomNode *node);
  void doSettings(easyDomNode *node);



void  doModule(missionNode *node,int mode);
 scriptContext * addContext(missionNode *node);
  void  removeContext();
void  doScript(missionNode *node,int mode);
void  doBlock(missionNode *node,int mode);
bool  doBooleanVar(missionNode *node,int mode);
varInst * lookupLocalVariable(missionNode *asknode);
varInst * lookupModuleVariable(string mname,missionNode *asknode);
varInst * lookupModuleVariable(missionNode *asknode);
varInst * lookupGlobalVariable(missionNode *asknode);
varInst * doVariable(missionNode *node,int mode);
void  checkStatement(missionNode *node,int mode);
void  doIf(missionNode *node,int mode);
void  doWhile(missionNode *node,int mode);
bool  checkBoolExpr(missionNode *node,int mode);
bool  doAndOr(missionNode *node,int mode);
bool  doNot(missionNode *node,int mode);
bool  doTest(missionNode *node,int mode);
 void doDefVar(missionNode *node,int mode);
 void doSetVar(missionNode *node,int mode);
 void doCall(missionNode *node,int mode);
 void doExec(missionNode *node,int mode);

scriptContext *makeContext(missionNode *node);
 bool checkVarType(varInst *var,enum var_type check_type);

 float checkFloatExpr(missionNode *node,int mode);

 void fatalError(string message);
 void runtimeFatal(string message);
 void warning(string message);








};

#endif // _MISSION_H_
