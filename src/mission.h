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

class missionNode : public tagDomNode {
 public:
  struct script_t {
    string name; // script
    tagDomNode *if_block[3]; // if
    int tester; // test
    tagDomNode *test_arg[2]; // test
  } script;
};

class Mission {
 public:
  Mission(char *configfile);

  int number_of_flightgroups,number_of_ships;

  vector<Flightgroup *> flightgroups;

  Flightgroup *findFlightgroup(string fg_name);

  string getVariable(string name,string defaultval);

 private:
  //  string getVariable(easyDomNode *section,string name,string defaultval);

  easyDomNode *variables;
  tagDomNode *director;

  bool checkMission(easyDomNode *node);
  void doVariables(easyDomNode *node);
  void checkVar(easyDomNode *node);
  void doFlightgroups(easyDomNode *node);
  void doOrder(easyDomNode *node,Flightgroup *fg);
  void checkFlightgroup(easyDomNode *node);
  bool doPosition(easyDomNode *node,float pos[3]);
  bool doRotation(easyDomNode *node,float rot[3]);
};

#endif // _MISSION_H_
