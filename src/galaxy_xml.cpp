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
  who wrote this? probably DanielH?
*/

#include <expat.h>
#include "xml_support.h"

#include "vegastrike.h"
#include <assert.h>

#include "galaxy_xml.h"
#include "easydom.h"
#include "galaxy_gen.h"
using namespace GalaxyXML;

Galaxy::Galaxy(const char *configfile){

  configNodeFactory *domf = new configNodeFactory();
  FILE * fp = fopen (configfile,"r");
  string cf = configfile;
  if (!fp) {
    
    //    fp = fopen ((vs_config->getVariable (
    cf=getUniversePath()+"/"+configfile;
  }else {
    fclose (fp);
  }
  
  configNode *top=(configNode *)domf->LoadXML(cf.c_str());

  if(top==NULL){
    cout << "Panicing   - no galaxy" << endl;
    return;
  }
  variables=NULL;
  checkConfig(top);
}

/* *********************************************************** */

bool Galaxy::checkConfig(configNode *node){
  if(node->Name()!="galaxy"){
    cout << "this is no Vegastrike galaxy file" << endl;
    return false;
  }

  vector<easyDomNode *>::const_iterator siter;
  
  for(siter= node->subnodes.begin() ; siter!=node->subnodes.end() ; siter++){
    configNode *cnode=(configNode *)(*siter);

    if(cnode->Name()=="systems"){
      doVariables(cnode);
    }
    else{
      cout << "Unknown tag: " << cnode->Name() << endl;
    }
  }
  return true;
}

/* *********************************************************** */

void Galaxy::doVariables(configNode *node){
  if(variables!=NULL){
    cout << "only one variable section allowed" << endl;
    return;
  }
  variables=node;

  vector<easyDomNode *>::const_iterator siter;
  
  for(siter= node->subnodes.begin() ; siter!=node->subnodes.end() ; siter++){
    configNode *cnode=(configNode *)(*siter);
    checkSection(cnode,SECTION_VAR);
  }
}

/* *********************************************************** */

void Galaxy::doSection(configNode *node, enum section_t section_type){
  string section=node->attr_value("name");
  if(section.empty()){
    cout << "no name given for section" << endl;
  }
  
  vector<easyDomNode *>::const_iterator siter;
  
  for(siter= node->subnodes.begin() ; siter!=node->subnodes.end() ; siter++){
    configNode *cnode=(configNode *)(*siter);
    if(section_type==SECTION_VAR){
      if(cnode->Name()=="var"){
	doVar(cnode);
      }
      else if(cnode->Name()=="sector"){
	doSection(cnode,section_type);
      }
      else{
	cout << "neither a variable nor a section" << endl;
      }
    }
  }
}

/* *********************************************************** */

void Galaxy::checkSection(configNode *node, enum section_t section_type){
    if(node->Name()!="sector"){
      cout << "galaxy_xml: not a section" << endl;
      node->printNode(cout,0,1);
    return;
  }

    doSection(node,section_type);
}

/* *********************************************************** */

void Galaxy::doVar(configNode *node){
  string name=node->attr_value("name");
  string value=node->attr_value("value");

  //  cout << "checking var " << name << " value " << value << endl;
  if(name.empty() || value.empty()){
    cout << "no name or value given for variable" << endl;
  }
}

/* *********************************************************** */

void Galaxy::checkVar(configNode *node){
    if(node->Name()!="var"){
      cout << "not a variable" << endl;
    return;
  }

    doVar(node);
}

/* *********************************************************** */
string Galaxy::getRandSystem (string sect, string def) {
  configNode *secnodes=findSection(sect,variables);  
  if (secnodes!=NULL) {
    unsigned int size = secnodes->subnodes.size();
    if (size>0) {
      return secnodes->subnodes[rand()%size]->attr_value("name");
    }
  }
  return def;
}
string Galaxy::getVariable(string section,string subsection,string name,string defaultvalue){
  configNode *secnode=findSection(section,variables);
  if(secnode!=NULL){
    configNode *subnode=findSection(subsection,secnode);
    if(subnode!=NULL){
      configNode *entrynode=findEntry(name,subnode);
      if(entrynode!=NULL){
	return entrynode->attr_value("value");
      }
    }
  }

  return defaultvalue;
}

/* *********************************************************** */

string Galaxy::getVariable(string section,string name,string defaultval){
   vector<easyDomNode *>::const_iterator siter;
  
  for(siter= variables->subnodes.begin() ; siter!=variables->subnodes.end() ; siter++){
    configNode *cnode=(configNode *)(*siter);
    string scan_name=(cnode)->attr_value("name");
    //    cout << "scanning section " << scan_name << endl;

    if(scan_name==section){
      return getVariable(cnode,name,defaultval);
    }
  }

  //cout << "WARNING: no section named " << section << endl;

  return defaultval;
}

/* *********************************************************** */

string Galaxy::getVariable(configNode *section,string name,string defaultval){
    vector<easyDomNode *>::const_iterator siter;
  
  for(siter= section->subnodes.begin() ; siter!=section->subnodes.end() ; siter++){
    configNode *cnode=(configNode *)(*siter);
    if((cnode)->attr_value("name")==name){
      return (cnode)->attr_value("value");
    }
  }

  cout << "WARNING: no var named " << name << " in section " << section->attr_value("name") << " using default: " << defaultval << endl;

  return defaultval; 
}





/* *********************************************************** */

configNode *Galaxy::findEntry(string name,configNode *startnode){
  return findSection(name,startnode);
}

/* *********************************************************** */

configNode *Galaxy::findSection(string section,configNode *startnode){
   vector<easyDomNode *>::const_iterator siter;
  
  for(siter= startnode->subnodes.begin() ; siter!=startnode->subnodes.end() ; siter++){
    configNode *cnode=(configNode *)(*siter);
    string scan_name=(cnode)->attr_value("name");
    //    cout << "scanning section " << scan_name << endl;

    if(scan_name==section){
      return cnode;
    }
  }
  //cout << "WARNING: no section/variable/color named " << section << endl;

  return NULL;
 
  
}

/* *********************************************************** */

void Galaxy::setVariable(configNode *entry,string value){
      entry->set_attribute("value",value);
}

/* *********************************************************** */

bool Galaxy::setVariable(string section,string name,string value){
  configNode *sectionnode=findSection(section,variables);
  if(sectionnode!=NULL){
    configNode *varnode=findEntry(name,sectionnode);

    if(varnode!=NULL){
      // now set the thing
      setVariable(varnode,value);
      return true;
    }
  }
  return false;
}

bool Galaxy::setVariable(string section,string subsection,string name,string value){
  configNode *sectionnode=findSection(section,variables);
  if(sectionnode!=NULL){
    configNode *subnode=findSection(name,sectionnode);

	if(subnode!=NULL) {
		configNode *varnode=findEntry(name,subnode);
		if(varnode!=NULL){
			// now set the thing
			setVariable(varnode,value);
			return true;
		}
	}
  }
  return false;
}
