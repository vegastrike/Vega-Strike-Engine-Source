/***************************************************************************
 *                           file.h  -  description
 *                           ----------------------------
 *                           begin                : January 18, 2002
 *                           copyright            : (C) 2002 by David Ranger
 *                           email                : sabarok@start.com.au
 **************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   any later version.                                                    *
 *                                                                         *
 **************************************************************************/

// Check to see if we're already loaded
#ifndef FILE_H
#define FILE_H

#include "central.h"
#include "easydom.h"
#include "general.h"
#include "glob.h"
#include <vector>
#include <list.h>
#include <string.h>
#include <string>
using namespace std;
void FindMissions(char *path);
void LoadMission(char *filename);
void ScanNode (string *parent, easyDomNode *node);
void CheckVar(string parent, string current, string name, string value);

using std::string;
//using XMLSupport::AttributeList;

class missionNode;

/*
class missionThread {
 public:
  vector<contextStack *> exec_stack;
  vector<missionNode *> module_stack;
  vector<unsigned int>  classid_stack;
};
*/

struct script_t {
  string name; // script,defvar,module
//    varInstMap variables; // script,module
//    vector<varInstMap *>  classvars; //module
//    varInst *varinst; // defvar,const
  missionNode *if_block[3]; // if
  missionNode *while_arg[2]; // while
  int tester; // test
  missionNode *test_arg[2]; // test
//    enum var_type vartype; // defvar,script
  string initval;
  missionNode *context_block_node; // defvar
//    map<string,missionNode *> scripts; // module
  missionNode *exec_node; // exec, return
  int nr_arguments; // script
  missionNode *argument_node; //script
  missionNode *module_node; // exec
  unsigned int classinst_counter;
};


class missionNode : public tagDomNode {
 public:
  struct script_t script;
};


#endif
