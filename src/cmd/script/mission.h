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
#include <fstream>

#include "xml_support.h"
#include "easydom.h"
#include "vs_globals.h"

using std::string;

using XMLSupport::AttributeList;

class Unit;
class Order;
class MessageCenter;

#define qu(x)	("\""+x+"\"")


/* *********************************************************** */

class Flightgroup {
 public:
  string name; // flightgroup name
  string faction;
  string type; // unit type
  string ainame; // .agg.xml and .int.xml get appended to this
  int terrain_nr;// which terrain to use. -1 for normal unit -2 for mission ter
  enum {UNIT,VEHICLE, BUILDING} unittype;
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
#define SCRIPT_PARSE_DECL 2

enum parsemode_type { PARSE_FULL,PARSE_DECL };

enum tag_type { 
  DTAG_UNKNOWN,
  DTAG_MISSION,
  DTAG_SETTINGS,DTAG_ORIGIN,DTAG_VARIABLES,DTAG_FLIGHTGROUPS,
  DTAG_ROT,DTAG_POS,DTAG_ORDER,
  DTAG_MODULE, DTAG_SCRIPT, DTAG_IF, DTAG_BLOCK,
  DTAG_SETVAR, DTAG_EXEC, DTAG_CALL, DTAG_WHILE,
  DTAG_AND_EXPR,DTAG_OR_EXPR,DTAG_NOT_EXPR,DTAG_TEST_EXPR,
  DTAG_FMATH,DTAG_VMATH,
  DTAG_VAR_EXPR, DTAG_DEFVAR,
  DTAG_CONST,
  DTAG_ARGUMENTS,
  DTAG_GLOBALS,
  DTAG_RETURN,
  DTAG_IMPORT
};

enum var_type { VAR_FAILURE,VAR_BOOL,VAR_FLOAT,VAR_INT,VAR_OBJECT,VAR_VOID,VAR_ANY };

enum tester_type { TEST_GT,TEST_LT,TEST_EQ,TEST_NE,TEST_GE,TEST_LE };

/* *********************************************************** */

class missionNode;

enum scope_type { VI_GLOBAL,VI_MODULE,VI_LOCAL,VI_TEMP, VI_IN_OBJECT,VI_ERROR,VI_CONST,VI_CLASSVAR };

class varInst {
 public:
  varInst(scope_type sctype) { scopetype=sctype; };
  varInst() { cout << "varInst() obsolete\n" << endl; assert(0); };

  string name;
  var_type type;

  scope_type scopetype;
  float  float_val;
  bool   bool_val;
  int    int_val;
  string string_val;
  
  string objectname;
  void   *object;

  missionNode *defvar_node;
  missionNode *block_node;
};

/* *********************************************************** */

class varInstMap : public map<string,varInst *> {
 public:

};

/* *********************************************************** */

class scriptContext {
 public:
  varInstMap *varinsts;
  missionNode *block_node;
};

/* *********************************************************** */

class contextStack {
 public:
 vector<scriptContext *> contexts;
 varInst *return_value;

};

/* *********************************************************** */

class missionNode;

class missionThread {
 public:
  vector<contextStack *> exec_stack;
  vector<missionNode *> module_stack;
  vector<uint>  classid_stack;
};

/* *********************************************************** */

class missionNode : public tagDomNode {
 public:
  struct script_t {
    string name; // script,defvar,module
    varInstMap variables; // script,module
    vector<varInstMap *>  classvars; //module
    varInst *varinst; // defvar,const
    missionNode *if_block[3]; // if
    missionNode *while_arg[2]; // while
    int tester; // test
    missionNode *test_arg[2]; // test
    enum var_type vartype; // defvar,script
    string initval;
    missionNode *context_block_node; // defvar
    map<string,missionNode *> scripts; // module
    missionNode *exec_node; // exec, return
    int nr_arguments; // script
    missionNode *argument_node; //script
    missionNode *module_node; // exec
    uint classinst_counter;
  } script;
};

/* *********************************************************** */

typedef vector<varInst *> olist_t;

class Mission {
 public:
  Mission(char *configfile);

  int number_of_flightgroups,number_of_ships;

  vector<Flightgroup *> flightgroups;

  Flightgroup *findFlightgroup(string fg_name);

  string getVariable(string name,string defaultval);

  void GetOrigin(Vector &pos,string &planetname);

  void DirectorLoop();
  void DirectorStart(missionNode *node);
  void DirectorEnd();
  void  loadMissionModules();
 void  loadModule(string modulename);
void addModule(string modulename);
void runScript(string modulename,string scriptname,uint classid=0);
uint  createClassInstance(string modulename);
 void setCurrentAIUnit(Unit *unit) { current_ai_unit=unit; };
 void setCurrentAIOrder(Order *order) { current_ai_order=order; };
 
  MessageCenter *msgcenter;

 private:
  //  string getVariable(easyDomNode *section,string name,string defaultval);

  Unit *current_ai_unit;
  Order *current_ai_order;

  int debuglevel;

  ofstream var_out;

  parsemode_type parsemode;

  easyDomNode *variables;
  easyDomNode *origin_node;

  missionNode *director;
  easyDomFactory<missionNode> *importf;

  tagMap tagmap;

  struct Runtime {
    vector<missionThread *> threads;
    map<string,missionNode *> modules;
    int thread_nr;
    missionThread *cur_thread;
    map<string,missionNode *> global_variables;
    //    vector<const void *()> callbacks;
  } runtime;

  // used only for parsing
  vector<missionNode *> scope_stack;
  missionNode *current_module;
  missionNode *current_script;

  vector<string> import_stack;

  void saveVariables(const ostream& out);
  void initTagMap();

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
  void removeContextStack();
  void addContextStack(missionNode *node);

varInst *  doScript(missionNode *node,int mode,varInstMap *varmap=NULL);
void  doBlock(missionNode *node,int mode);
bool  doBooleanVar(missionNode *node,int mode);
varInst * lookupLocalVariable(missionNode *asknode);
varInst * lookupModuleVariable(string mname,missionNode *asknode);
varInst * lookupModuleVariable(missionNode *asknode);
varInst * lookupClassVariable(missionNode *asknode);
varInst * lookupGlobalVariable(missionNode *asknode);
varInst * doVariable(missionNode *node,int mode);
void  checkStatement(missionNode *node,int mode);
void  doIf(missionNode *node,int mode);
void  doWhile(missionNode *node,int mode);
bool  checkBoolExpr(missionNode *node,int mode);
bool  doAndOr(missionNode *node,int mode);
bool  doNot(missionNode *node,int mode);
bool  doTest(missionNode *node,int mode);
 void doDefVar(missionNode *node,int mode,bool global_var=false);
 void doSetVar(missionNode *node,int mode);
 varInst * doCall(missionNode *node,int mode);
 varInst* doExec(missionNode *node,int mode);
 varInst *doConst(missionNode *node,int mode);

int doIntVar(missionNode *node,int mode);
int checkIntExpr(missionNode *node,int mode);
int  doIMath(missionNode *node,int mode);
varInst * doMath(missionNode *node,int mode);
int intMath(string mathname,int res1,int res2);
float floatMath(string mathname,float res1,float res2);
 varInst *checkExpression(missionNode *node,int mode);

 void assignVariable(varInst *v1,varInst *v2);

scriptContext *makeContext(missionNode *node);
 bool checkVarType(varInst *var,enum var_type check_type);

 float checkFloatExpr(missionNode *node,int mode);
 float doFloatVar(missionNode *node,int mode);
 float doFMath(missionNode *node,int mode);

 void doArguments(missionNode *node,int mode,varInstMap *varmap=NULL);
 void doReturn(missionNode *node,int mode);
 void doGlobals(missionNode *node,int mode);
 void doImport(missionNode *node,int mode);

 bool have_return(int mode);
 missionNode * lookupScript(string scriptname,string modulename);

 var_type vartypeFromString(string type);


 varInst * doObjectVar(missionNode *node,int mode);
varInst * checkObjectExpr(missionNode *node,int mode);

 void fatalError(missionNode *node,int mode,string message);
 void runtimeFatal(string message);
 void warning(string message);
 void debug(missionNode *node,int mode,string message);
 void debug(int level,missionNode *node,int mode,string message);

void printNode(missionNode *node,int mode);

 void printRuntime();
 void printThread(missionThread *thread);
 void printVarmap(const varInstMap & vmap);
 void printVarInst(varInst *vi);
 void saveVarInst(varInst *vi,ostream &out);
 void printVarInst(int dbg_level,varInst *vi);
 void printGlobals(int dbg_level);
 void printModules();

 string modestring(int mode);

 varInst *searchScopestack(string name);

 varInst * callRnd(missionNode *node,int mode);
 varInst * callPrintFloats(missionNode *node,int mode);
 varInst * callGetGameTime(missionNode *node,int mode);
varInst *  call_isNull(missionNode *node,int mode);
varInst * call_isequal(missionNode *node,int mode);
varInst * callGetCurrentAIUnit(missionNode *node,int mode);
varInst * callGetCurrentAIOrder(missionNode *node,int mode);

 varInst *call_int_cast(missionNode *node,int mode);
 varInst *call_float_cast(missionNode *node,int mode);

varInst * call_io_sprintf(missionNode *node,int mode);
varInst * call_io_printf(missionNode *node,int mode);
string  replaceNewline(string origstr);

varInst * call_io_message(missionNode *node,int mode);
varInst * call_io_printmsglist(missionNode *node,int mode);

string method_str(missionNode *node);
 missionNode *getArgument(missionNode *node,int mode,int arg_nr);

 varInst *call_olist(missionNode *node,int mode);
 olist_t *getOListObject(missionNode *node,int mode,varInst *ovi);
 varInst *call_olist_new(missionNode *node,int mode);
 void call_olist_pop_back(missionNode *node,int mode,varInst *ovi);
 void call_olist_push_back(missionNode *node,int mode,varInst *ovi,varInst *push);
varInst * call_olist_at(missionNode *node,int mode,varInst *ovi,int index);
varInst * call_olist_back(missionNode *node,int mode,varInst *ovi);
void  call_olist_toxml(missionNode *node,int mode,varInst *ovi);
 Vector call_olist_tovector(missionNode *node,int mode,varInst *ovi);

 varInst *getObjectArg(missionNode *node,int mode);

varInst * call_unit(missionNode *node,int mode);
 Unit *getUnitObject(missionNode *node,int mode,varInst *ovi);

 // void call_unit_launch(missionNode *node,int mode,string name,string faction,string type,string ainame,int nr_ships,Vector &pos);
 void call_unit_launch(Flightgroup *fg);
 void call_unit_toxml(missionNode *node,int mode,varInst *ovi);

 varInst *call_string(missionNode *node,int mode);
void  call_string_print(missionNode *node,int mode,varInst *ovi);
varInst * call_string_new(missionNode *node,int mode,string initstring);
 string *getStringObject(missionNode *node,int mode,varInst *ovi);
string call_string_getstring(missionNode *node,int mode,varInst *ovi);
 string getStringArgument(missionNode *node,int mode,int arg_nr);

 void findNextEnemyTarget(Unit *my_unit);

 varInst *doCall(missionNode *node,int mode,string module,string method);
 void doCall_toxml(string module,varInst *ovi);

varInst *newVarInst(scope_type scopetype);
void  deleteVarInst(varInst *vi,bool del_local=false);
 int  vi_counter,old_vi_counter; 
void deleteVarMap(varInstMap *vmap);

varInst * call_order(missionNode *node,int mode);
Order * getOrderObject(missionNode *node,int mode,varInst *ovi);


};

#endif // _MISSION_H_
