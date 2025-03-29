/*
 * Copyright (C) 2001-2023 Daniel Horn, David Ranger, pyramid3d,
 * Stephen G. Tuggy, Benjamen R. Meyer, and other Vega Strike contributors.
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Vega Strike is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef VEGA_STRIKE_ENGINE_MISSION_FILE_H
#define VEGA_STRIKE_ENGINE_MISSION_FILE_H

#include "mission/include/central.h"
#include "mission/include/easydom.h"
#include "mission/include/general.h"
#include <glob.h>
#include <vector>
#include <list>
#include <string>
#include <string>
using namespace std;
void FindMissions(char *path);
void LoadMission(char *filename);
void ScanNode(string *parent, easyDomNode *node, string filename);
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

#endif //VEGA_STRIKE_ENGINE_MISSION_FILE_H
