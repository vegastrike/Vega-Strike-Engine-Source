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
  easyDom - easy DOM for expat - written by Alexander Rawass <alexannika@users.sourceforge.net>
*/

#ifndef _EASYDOM_H_
#define _EASYDOM_H_

#include <expat.h>
#include <string>
#include <vector>
#include <stack>
#include "xml_support.h"

using std::string;
using std::vector;
using std::stack;

using XMLSupport::AttributeList;

class easyDomNode {
 public:
  easyDomNode();

  void set(easyDomNode *parent,string name,  AttributeList *attributes);
  void walk(int level);

  void addChild(easyDomNode *child);

  string Name() { return name ; }

  string attr_value(string attr_name);
  vector<easyDomNode *> subnodes;

 private:
  easyDomNode *parent;
  AttributeList *attributes;

  vector<string> att_name;
  vector<string> att_value;

  string name;
};

class easyDomFactory {
 public:
  easyDomFactory();

  void getColor(char *name, float color[4]);
  char *getVariable(char *section,char *name);

  struct easyDomFactoryXML {
  } *xml;

  easyDomNode *LoadXML(const char*);

  static void beginElement(void *userData, const XML_Char *name, const XML_Char **atts);
  static void endElement(void *userData, const XML_Char *name);

  void beginElement(const string &name, const AttributeList &attributes);
  void endElement(const string &name);

  stack<easyDomNode *> nodestack;

  easyDomNode *topnode;
};

#endif // _VEGACONFIG_H_
