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
#include <hash_map>
#include "configxml.h"
#include "xml_support.h"
#include "easydom.h"
#include "in_kb.h"

using std::string;

using XMLSupport::AttributeList;

typedef stdext::hash_map<string,KBHandler> CommandMap;
typedef stdext::hash_map<string,int> KeyMap;

class GameVegaConfig: public VegaConfig {
 public:
  GameVegaConfig(const char *configfile);

#define AXIS_X 0
#define AXIS_Y 1
#define AXIS_Z 2
#define AXIS_THROTTLE 3

 private:
  void initCommandMap();
  void initKeyMap();

  CommandMap command_map;
  KeyMap     key_map;

  int hs_value_index;

  //  vector<vColor *> colors;

  void bindKeys();
  void doBindings(configNode *node);
  void checkBind(configNode *node);
  void doAxis(configNode *node);
  void checkHatswitch(int nr,configNode *node);
};

#endif // _VEGACONFIG_H_
