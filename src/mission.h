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

#include <expat.h>
#include <string>
#include "xml_support.h"
#include "easydom.h"

using std::string;

using XMLSupport::AttributeList;

class Flightgroup {
 public:
  string name,faction,type,ainame;
  int waves,nr_ships;
  float pos[3];
  float rot[3];
};

class Mission {
 public:
  Mission(char *configfile);

  int number_of_flightgroups;

  vector<Flightgroup *> flightgroups;

  Flightgroup *findFlightgroup(string offset_name);

 private:
  //  string getVariable(easyDomNode *section,string name,string defaultval);

  easyDomNode *variables;


  bool checkMission(easyDomNode *node);
  void doVariables(easyDomNode *node);
  void checkVar(easyDomNode *node);
  void doFlightgroups(easyDomNode *node);
  void doOrder(easyDomNode *node);
  void checkFlightgroup(easyDomNode *node);
  bool doPosition(easyDomNode *node,float pos[3]);
  bool doRotation(easyDomNode *node,float rot[3]);
};

#endif // _MISSION_H_
