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
  xml Configuration written by Alexander Rawass <alexannika@users.sourceforge.net>
*/

#ifndef _VEGACONFIG_H_
#define _VEGACONFIG_H_

#include <expat.h>
#include <string>
#include <map>
#include "xml_support.h"
#include "easydom.h"
#include "in_kb.h"

using std::string;

using XMLSupport::AttributeList;

typedef map<string,KBHandler> CommandMap;
typedef map<string,int> KeyMap;

class vColor {
 public:
  string name;
  float r,g,b,a;
};


class configNode : public easyDomNode {
 public:
  vColor *color;
};

enum section_t { SECTION_COLOR,SECTION_VAR };

class configNodeFactory : public easyDomFactory<configNode> {
};

class VegaConfig {
 public:
  VegaConfig(char *configfile);

  void getColor(configNode *node,string name, float color[4]);
  void getColor(string section, string name, float color[4]);
  void getColor(string section,string name,float color[4],int hexcolor);
  void getColor(string name, float color[4]) { getColor("default",name,color); };

  string getVariable(string section,string name,string defaultvalue);
  string getVariable(string section,string subsection,string name,string defaultvalue);

  configNode *findSection(string section,configNode *startnode);
  configNode *findEntry(string name,configNode *startnode);

  void setVariable(configNode *entry,string value);
  bool setVariable(string section,string name,string value);

  void bindKeys();

  easyDomNode *Variables() { return variables; };

#define MAX_AXIS 4

#define AXIS_X 0
#define AXIS_Y 1
#define AXIS_Z 2
#define AXIS_THROTTLE 3

  int axis_axis[MAX_AXIS];
  int axis_joy[MAX_AXIS];

#define MAX_HATSWITCHES 4
#define MAX_VALUES 8

  float hatswitch[MAX_HATSWITCHES][MAX_VALUES];
  float hatswitch_margin[MAX_HATSWITCHES];

  int hatswitch_axis[MAX_HATSWITCHES];
  int hatswitch_joystick[MAX_HATSWITCHES];

 private:
  string getVariable(configNode *section,string name,string defaultval);

  void initCommandMap();
  void initKeyMap();

  CommandMap command_map;
  KeyMap     key_map;

  configNode *variables;
  configNode *bindings;
  configNode *colors;

  int hs_value_index;

  //  vector<vColor *> colors;

  bool checkConfig(configNode *node);
  void doVariables(configNode *node);
  void doBindings(configNode *node);
  void checkSection(configNode *node,enum section_t section_type);
  void checkVar(configNode *node);
  void doSection(configNode *node,enum section_t section_type);
  void doVar(configNode *node);
  void doColors(configNode *node);
  bool checkColor(configNode *node);
  void checkBind(configNode *node);
  void doAxis(configNode *node);
  void checkHatswitch(int nr,configNode *node);


};

#endif // _VEGACONFIG_H_
