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

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <ctype.h>
#include <unistd.h>

#include <expat.h>
#include "xml_support.h"

#include "vegastrike.h"

#include "mission.h"
#include "easydom.h"

//#include "vs_globals.h"
//#include "vegastrike.h"

Mission::Mission(char *configfile){
  //LoadXML(configfile);

  easyDomFactory *domf = new easyDomFactory();

  easyDomNode *top=domf->LoadXML(configfile);

  if(top==NULL){
    cout << "Panic exit - mission file not found" << endl;
    exit(0);
  }
  //top->walk(0);

  variables=NULL;

  checkMission(top);
}

bool Mission::checkMission(easyDomNode *node){
  if(node->Name()!="mission"){
    cout << "this is no Vegastrike mission file" << endl;
    return false;
  }

  vector<easyDomNode *>::const_iterator siter;
  
  for(siter= node->subnodes.begin() ; siter!=node->subnodes.end() ; siter++){
    if((*siter)->Name()=="variables"){
      doVariables(*siter);
    }
    else if(((*siter)->Name()=="flightgroups")){
      doFlightgroups(*siter);
    }
    else{
      cout << "Unknown tag: " << (*siter)->Name() << endl;
    }
  }
  return true;
}

void Mission::doVariables(easyDomNode *node){
  if(variables!=NULL){
    cout << "only one variable section allowed" << endl;
    return;
  }
  variables=node;

  vector<easyDomNode *>::const_iterator siter;
  
  for(siter= node->subnodes.begin() ; siter!=node->subnodes.end() ; siter++){
    checkVar(*siter);
  }
}


void Mission::checkVar(easyDomNode *node){
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

void Mission::doFlightgroups(easyDomNode *node){
  vector<easyDomNode *>::const_iterator siter;
  
  for(siter= node->subnodes.begin() ; siter!=node->subnodes.end() ; siter++){
    checkFlightgroup(*siter);
  }

}

void Mission::checkFlightgroup(easyDomNode *node){
  if(node->Name()!="flightgroup"){
    cout << "not a flightgroup" << endl;
    return;
  }

  string name=node->attr_value("name");
  string faction=node->attr_value("faction");
  string type=node->attr_value("type");
  string ainame=node->attr_value("ainame");
  string waves=node->attr_value("waves");
  string nr_ships=node->attr_value("nr_ships");

  if(name.empty() || faction.empty() || type.empty() || ainame.empty() || waves.empty() || nr_ships.empty() ){
    cout << "no valid flightgroup decsription" << endl;
    return;
  }

  int waves_i=atoi(waves.data());
  int nr_ships_i=atoi(nr_ships.data());

  bool have_pos=false;
  bool have_rot=false;

  float pos[3];
  float rot[3];

  rot[0]=rot[1]=rot[2]=0.0;

  vector<easyDomNode *>::const_iterator siter;
  
  for(siter= node->subnodes.begin() ; siter!=node->subnodes.end() ; siter++){
    if((*siter)->Name()=="pos"){
      have_pos=doPosition(*siter,pos);
    }
    else if((*siter)->Name()=="rot"){
      have_rot=doRotation(*siter,rot);
    }
  }

  if(!have_pos){
    cout << "don;t have a position" << endl;
    return;
  }

  // now enter the stuff (not yet)
  cout << "flightgroup: " << name << " " << faction << " " << type << " " << ainame << " " << waves << " " << nr_ships << endl;
  cout << "   pos: " << pos[0] << " " << pos[0] << " " << pos[0] << " " << endl;
  cout << "   rot: " << rot[0] << " " << rot[0] << " " << rot[0] << " " << endl;

}

bool Mission::doPosition(easyDomNode *node,float pos[3]){
  string x=node->attr_value("x");
  string y=node->attr_value("y");
  string z=node->attr_value("z");
  string offset=node->attr_value("offset");

  if(x.empty() || y.empty() || z.empty() ){
    cout << "no valid position" << endl;
    return false;
  }

  pos[0]=atof(x.data());
  pos[1]=atof(y.data());
  pos[2]=atof(z.data());

  // offset not yet

  return true;
}

bool Mission::doRotation(easyDomNode *node,float rot[3]){
  //not yet
  return true;
}
