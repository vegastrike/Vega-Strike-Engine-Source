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

#include <expat.h>
#include "xml_support.h"

//#include "vegastrike.h"
#include <assert.h>


#include "configxml.h"
#include "easydom.h"

//#include "vs_globals.h"
//#include "vegastrike.h"
using std::cout;
using std::cerr;
using std::endl;

/* *********************************************************** */

VegaConfig::VegaConfig(const char *configfile){

  configNodeFactory *domf = new configNodeFactory();

  configNode *top=(configNode *)domf->LoadXML(configfile);

  if(top==NULL){
    cout << "Panic exit - no configuration" << endl;
    exit(0);
  }
  //top->walk(0);
  
  variables=NULL;
  colors=NULL;

  checkConfig(top);
}

VegaConfig::~VegaConfig()
{
	if( variables!=NULL)
		delete variables;
	if( colors!=NULL)
		delete colors;
	if( bindings!=NULL)
		delete bindings;
}

/* *********************************************************** */

bool VegaConfig::checkConfig(configNode *node){
  if(node->Name()!="vegaconfig"){
    cout << "this is no Vegastrike config file" << endl;
    return false;
  }

  vector<easyDomNode *>::const_iterator siter;
  
  for(siter= node->subnodes.begin() ; siter!=node->subnodes.end() ; siter++){
    configNode *cnode=(configNode *)(*siter);

    if(cnode->Name()=="variables"){
      doVariables(cnode);
    }
    else if(cnode->Name()=="colors"){
      doColors(cnode);
    }
    else if(cnode->Name()=="bindings"){
      bindings=cnode; // delay the bindings until keyboard/joystick is initialized
      //doBindings(cnode);
    }
    else{
      cout << "Unknown tag: " << cnode->Name() << endl;
    }
  }
  return true;
}

/* *********************************************************** */

void VegaConfig::doVariables(configNode *node){
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

void VegaConfig::doSection(string prefix, configNode *node, enum section_t section_type){
  string section=node->attr_value("name");
  if(section.empty()){
    cout << "no name given for section" << endl;
  }
  
  vector<easyDomNode *>::const_iterator siter;
  
  for(siter= node->subnodes.begin() ; siter!=node->subnodes.end() ; siter++){
    configNode *cnode=(configNode *)(*siter);
    if(section_type==SECTION_COLOR){
      checkColor(prefix,cnode);
    }
    else if(section_type==SECTION_VAR){
      if(cnode->Name()=="var"){
	doVar(prefix,cnode);
      }
      else if(cnode->Name()=="section"){
	doSection(prefix+cnode->attr_value("name")+"/",cnode,section_type);
      }
      else{
	cout << "neither a variable nor a section" << endl;
      }
    }
  }
}

/* *********************************************************** */

void VegaConfig::checkSection(configNode *node, enum section_t section_type){
    if(node->Name()!="section"){
      cout << "not a section" << endl;
      node->printNode(cout,0,1);

      return;
  }

    doSection(node->attr_value("name")+"/",node,section_type);
}

/* *********************************************************** */

void VegaConfig::doVar(string prefix, configNode *node){
  string name=node->attr_value("name");
  string value=node->attr_value("value");
  string hashname=prefix+name;
  map_variables[hashname]=value;

  //  cout << "checking var " << name << " value " << value << endl;
  if(name.empty()){
    cout << "no name given for variable " << name << " " << value<< " "<<endl;
  }
}

/* *********************************************************** */

void VegaConfig::checkVar(configNode *node){
    if(node->Name()!="var"){
      cout << "not a variable" << endl;
    return;
  }

    doVar("",node);
}

/* *********************************************************** */

bool VegaConfig::checkColor(string prefix, configNode *node){
  if(node->Name()!="color"){
    cout << "no color definition" << endl;
    return false;
  }

  if(node->attr_value("name").empty()){
    cout << "no color name given" << endl;
    return false;
  }

  string name=node->attr_value("name");
  string hashname=prefix+name;

  vColor *color;

  if(node->attr_value("ref").empty()){
    string r=node->attr_value("r");
    string g=node->attr_value("g");
    string b=node->attr_value("b");
    string a=node->attr_value("a");
    if(r.empty() || g.empty() || b.empty() || a.empty()){
      cout << "neither name nor r,g,b given for color " << node->Name() << endl;
      return false;
    }
    float rf=atof(r.c_str());
    float gf=atof(g.c_str());
    float bf=atof(b.c_str());
    float af=atof(a.c_str());

    vColor &vc = map_colors[hashname];
    vc.name.erase();
    vc.r = rf;
    vc.g = gf;
    vc.b = bf;
    vc.a = af;

    color=new vColor;

    color->r=rf;
    color->g=gf;
    color->b=bf;
    color->a=af;
  }
  else{
    float refcol[4];

    string ref_section=node->attr_value("section");
    string ref_name=node->attr_value("ref");
    if(ref_section.empty()){
      cout << "you have to give a referenced section when referencing colors" << endl;
      ref_section="default";
    }

    //    cout << "refsec: " << ref_section << " ref " << node->attr_value("ref") << endl;
    getColor(ref_section,ref_name,refcol);

    vColor &vc = map_colors[hashname];
    vc.name = ref_section+"/"+ref_name;
    vc.r = refcol[0];
    vc.g = refcol[1];
    vc.b = refcol[2];
    vc.a = refcol[3];

    color=new vColor;

    color->r=refcol[0];
    color->g=refcol[1];
    color->b=refcol[2];
    color->a=refcol[3];
  }

  color->name=node->attr_value("name");

  node->color=color;
  //  colors.push_back(color);

  return true;
}

/* *********************************************************** */

void VegaConfig::doColors(configNode *node){
  if(colors!=NULL){
    cout << "only one variable section allowed" << endl;
    return;
  }
  colors=node;

  vector<easyDomNode *>::const_iterator siter;
  
  for(siter= node->subnodes.begin() ; siter!=node->subnodes.end() ; siter++){
    configNode *cnode=(configNode *)(*siter);
    checkSection(cnode,SECTION_COLOR);
  }
}

/* *********************************************************** */

string VegaConfig::getVariable(string section,string subsection,string name,string defaultvalue){
  string hashname = section + "/" + subsection + "/" + name;
  std::map<string,string>::iterator it;
  if ((it=map_variables.find(hashname)) != map_variables.end())
      return (*it).second; else
      return defaultvalue;

  /*configNode *secnode=findSection(section,variables);
  if(secnode!=NULL){
    configNode *subnode=findSection(subsection,secnode);
    if(subnode!=NULL){
      configNode *entrynode=findEntry(name,subnode);
      if(entrynode!=NULL){
	return entrynode->attr_value("value");
      }
    }
  }

  return defaultvalue;*/
}

/* *********************************************************** */

string VegaConfig::getVariable(string section,string name,string defaultval){
  string hashname = section + "/" + name;
  std::map<string,string>::iterator it;
  if ((it=map_variables.find(hashname)) != map_variables.end())
      return (*it).second; else
      return defaultval;

  /*vector<easyDomNode *>::const_iterator siter;
  
  for(siter= variables->subnodes.begin() ; siter!=variables->subnodes.end() ; siter++){
    configNode *cnode=(configNode *)(*siter);
    string scan_name=(cnode)->attr_value("name");
    //    cout << "scanning section " << scan_name << endl;

    if(scan_name==section){
      return getVariable(cnode,name,defaultval);
    }
  }

  cout << "WARNING: no section named " << section << endl;

  return defaultval;*/
}

/* *********************************************************** */

string VegaConfig::getVariable(configNode *section,string name,string defaultval){
    vector<easyDomNode *>::const_iterator siter;
  
  for(siter= section->subnodes.begin() ; siter!=section->subnodes.end() ; siter++){
    configNode *cnode=(configNode *)(*siter);
    if((cnode)->attr_value("name")==name){
      return (cnode)->attr_value("value");
    }
  }
  static bool foundshouldwarn=false;
  static bool shouldwarn=true;
  if (!foundshouldwarn) {
    if (name!="debug_config") {
      shouldwarn=XMLSupport::parse_bool(getVariable("general","debug_config","true"));
      foundshouldwarn=true;
    }
  }
  if (shouldwarn) {
    cout << "WARNING: no var named " << name << " in section " << section->attr_value("name") << " using default: " << defaultval << endl;
  }
  return defaultval; 
}

/* *********************************************************** */

void VegaConfig::gethColor(string section, string name, float color[4],int hexcolor){
  color[3]=((float)(hexcolor & 0xff))/256.0;
  color[2]=((float)((hexcolor & 0xff00)>>8))/256.0;
  color[1]=((float)((hexcolor & 0xff0000)>>16))/256.0;
  color[0]=((float)((hexcolor & 0xff000000)>>24))/256.0;
  
  getColor(section,name,color,true);
}

/* *********************************************************** */

void VegaConfig::getColor(string section, string name, float color[4],bool have_color){
  string hashname = section + "/" + name;
  std::map<string,vColor>::iterator it;
  if ((it=map_colors.find(hashname)) != map_colors.end()) {
      color[0] = (*it).second.r;
      color[1] = (*it).second.g;
      color[2] = (*it).second.b;
      color[3] = (*it).second.a;
  } else {
      if (!have_color) {
          color[0]=color[1]=color[2]=color[3]=1.0f;
      }
  }

  /*vector<easyDomNode *>::const_iterator siter;
  
  if( colors == NULL )
  {
      cout << "WARNING: no colors defined in vegatrike config file" << endl;
      return;
  }

  for(siter= colors->subnodes.begin() ; siter!=colors->subnodes.end() ; siter++){
    configNode *cnode=(configNode *)(*siter);
    string scan_name=(cnode)->attr_value("name");
    //          cout << "scanning section " << scan_name << endl;

    if(scan_name==section){
      getColor(cnode,name,color,have_color);
      return;
    }
  }

  cout << "WARNING: no section named " << section << endl;

  return;*/
  
}

/* *********************************************************** */

void VegaConfig::getColor(configNode *node,string name,float color[4],bool have_color){
  vector<easyDomNode *>::const_iterator siter;
  
  for(siter= node->subnodes.begin() ; siter!=node->subnodes.end() ; siter++){
    configNode *cnode=(configNode *)(*siter);
    //            cout << "scanning color " << (cnode)->attr_value("name") << endl;
    if((cnode)->attr_value("name")==name){
      color[0]=(cnode)->color->r;
      color[1]=(cnode)->color->g;
      color[2]=(cnode)->color->b;
      color[3]=(cnode)->color->a;
      return;
    }
  }

  if(have_color==false){
    color[0]=1.0;
    color[1]=1.0;
    color[2]=1.0;
    color[3]=1.0;

    cout << "WARNING: color " << name << " not defined, using default (white)" << endl;
  }
  else{
    cout << "WARNING: color " << name << " not defined, using default (hexcolor)" << endl;
  }

}

/* *********************************************************** */

configNode *VegaConfig::findEntry(string name,configNode *startnode){
  return findSection(name,startnode);
}

/* *********************************************************** */

configNode *VegaConfig::findSection(string section,configNode *startnode){
   vector<easyDomNode *>::const_iterator siter;
  
  for(siter= startnode->subnodes.begin() ; siter!=startnode->subnodes.end() ; siter++){
    configNode *cnode=(configNode *)(*siter);
    string scan_name=(cnode)->attr_value("name");
    //    cout << "scanning section " << scan_name << endl;

    if(scan_name==section){
      return cnode;
    }
  }

  cout << "WARNING: no section/variable/color named " << section << endl;

  return NULL;
 
  
}

/* *********************************************************** */

void VegaConfig::setVariable(configNode *entry,string value){
      entry->set_attribute("value",value);
}

/* *********************************************************** */

bool VegaConfig::setVariable(string section,string name,string value){
  configNode *sectionnode=findSection(section,variables);
  if(sectionnode!=NULL){
    configNode *varnode=findEntry(name,sectionnode);

    if(varnode!=NULL){
      // now set the thing
      setVariable(varnode,value);
    }
  }
  string hashname = section + "/" + name;
  map_variables[hashname]=value;
  return true;
}


bool VegaConfig::setVariable(string section,string subsection,string name,string value){
  configNode *sectionnode=findSection(section,variables);
  if(sectionnode!=NULL){
    configNode *subnode=findSection(name,sectionnode);
    
	if(subnode!=NULL) {
		configNode *varnode=findEntry(name,subnode);
		if(varnode!=NULL){
			// now set the thing
			setVariable(varnode,value);
		}
	}
  }
  string hashname = section + "/" + subsection + "/" + name;
  map_variables[hashname]=value;
  return true;
}

