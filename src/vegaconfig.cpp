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

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <ctype.h>
#include <unistd.h>

#include <expat.h>
#include "xml_support.h"

#include "vegastrike.h"

#include "vegaconfig.h"
#include "easydom.h"

//#include "vs_globals.h"
//#include "vegastrike.h"

VegaConfig::VegaConfig(char *configfile){
  //LoadXML(configfile);

  easyDomFactory *domf = new easyDomFactory();

  easyDomNode *top=domf->LoadXML(configfile);

  if(top==NULL){
    cout << "Panic exit - no configuration" << endl;
    exit(0);
  }
  //top->walk(0);
 
  checkConfig(top);
}

bool VegaConfig::checkConfig(easyDomNode *node){
  if(node->Name()!="vegaconfig"){
    cout << "this is no Vegastrike config file" << endl;
    return false;
  }

  vector<easyDomNode *>::const_iterator siter;
  
  for(siter= node->subnodes.begin() ; siter!=node->subnodes.end() ; siter++){
    if((*siter)->Name()=="variables"){
      doVariables(*siter);
    }
    else if(((*siter)->Name()=="colors")){
      doColors(*siter);
    }
    else if(((*siter)->Name()=="bindings")){
      doBindings(*siter);
    }
    else{
      cout << "Unknown tag: " << (*siter)->Name() << endl;
    }
  }
  return true;
}

void VegaConfig::doVariables(easyDomNode *node){
  if(variables!=NULL){
    cout << "only one variable section allowed" << endl;
    return;
  }
  variables=node;

  vector<easyDomNode *>::const_iterator siter;
  
  for(siter= node->subnodes.begin() ; siter!=node->subnodes.end() ; siter++){
    checkSection(*siter);
  }
}

void VegaConfig::checkSection(easyDomNode *node){
    if(node->Name()!="section"){
      cout << "not a section" << endl;
    return;
  }

  string section=node->attr_value("name");
  if(section.empty()){
    cout << "no name given for section" << endl;
  }
  
  vector<easyDomNode *>::const_iterator siter;
  
  for(siter= node->subnodes.begin() ; siter!=node->subnodes.end() ; siter++){
    checkVar(*siter);
  }

}

void VegaConfig::checkVar(easyDomNode *node){
    if(node->Name()!="var"){
      cout << "not a variable" << endl;
    return;
  }

  string name=node->attr_value("name");
  string value=node->attr_value("value");
  if(name.empty() || value.empty()){
    cout << "no name or value given for variable" << endl;
  }
}

void VegaConfig::doColors(easyDomNode *node){
}

void VegaConfig::doBindings(easyDomNode *node){
}

string VegaConfig::getVariable(string section,string name,string defaultval){
   vector<easyDomNode *>::const_iterator siter;
  
  for(siter= variables->subnodes.begin() ; siter!=variables->subnodes.end() ; siter++){
    string scan_name=(*siter)->attr_value("name");
    //    cout << "scanning section " << scan_name << endl;

    if(scan_name==section){
      return getVariable(*siter,name,defaultval);
    }
  }

  cout << "WARNING: no section with name " << section << endl;

  return defaultval;
 
}
string VegaConfig::getVariable(easyDomNode *section,string name,string defaultval){
    vector<easyDomNode *>::const_iterator siter;
  
  for(siter= section->subnodes.begin() ; siter!=section->subnodes.end() ; siter++){
    if((*siter)->attr_value("name")==name){
      return (*siter)->attr_value("value");
    }
  }

  cout << "WARNING: no variable with name " << name << endl;

  return defaultval;
 
}

