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

#ifndef WIN32
// this file isn't available on my system (all win32 machines?) i dun even know what it has or if we need it as I can compile without it
#include <unistd.h>
#endif

//#include <expat.h>
//#include "xml_support.h"

//#include "vegastrike.h"
#include <assert.h>
#include "mission.h"
//#include "easydom.h"

//#include "msgcenter.h"

//#include "vs_globals.h"
//#include "vegastrike.h"

/* *********************************************************** */
Mission::~Mission() {
  fprintf (stderr,"Mission Cleanup Not Yet Implemented");
  //do not delete msgcenter...could be vital
}
Mission::Mission(char *configfile){
  number_of_flightgroups=0;
  number_of_ships=0;

  easyDomFactory<missionNode> *domf= new easyDomFactory<missionNode>();

 top=domf->LoadXML(configfile);
 static bool dontpanic=false;
  if(top==NULL&&!dontpanic){
    cout << "Panic exit - mission file " << configfile << " not found" << endl;
    exit(0);
  } else {
    dontpanic=true;
  }
  if (top==NULL) {
    return;
  }
  //top->walk(0);

  variables=NULL;
  origin_node=NULL;

#ifndef VS_MIS_SEL
  director=NULL;

  gametime=0.0;
  total_nr_frames=0;

  initTagMap();
  initCallbackMaps();

  top->Tag(&tagmap);

#endif

}

void Mission::initMission(){
  if (!top)
    return;
  static bool begin=true;
  if (begin) {
    msgcenter=new MessageCenter();
    begin=false;
  }else {
    msgcenter=active_missions[0]->msgcenter;
  }
  msgcenter->add("game","all","Welcome to Vegastrike");
  msgcenter->add("game","news","A hero is born");
  msgcenter->add("game","news","2 hero is born");
  msgcenter->add("game","news","3 hero is born");
  msgcenter->add("game","news","4 hero is born.                That day, in teh news hero was born without even boring the born of bachunkuphe threw his last baseball throw he chewed his last hay the man was a hero a heayrinagia sf of a hero...who knw that ith all things iblicatedly obfusictaroed he arced his bunko bunka\nbachunk!jtkgjdfljg fdlg fdj glkdf jglkdfj glkdf jgjfd lkgj dflkg jlfkdjg ldjg  j");
  checkMission(top);
}

/* *********************************************************** */

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
    else if(((*siter)->Name()=="settings")){
      doSettings(*siter);
    }
    else if(((*siter)->Name()=="module")){
      //      doModule(*siter);
#ifndef VS_MIS_SEL
      DirectorStart((missionNode *)*siter);
#endif
    }
    else{
      cout << "warning: Unknown tag: " << (*siter)->Name() << endl;
    }
  }
  return true;
}

/* *********************************************************** */

void Mission::doOrigin(easyDomNode *node){
  origin_node=node;
}

/* *********************************************************** */

#ifndef VS_MIS_SEL
void Mission::GetOrigin(Vector &pos,string &planetname){
  //  float pos[3];

  if(origin_node==NULL){
    pos.i=pos.j=pos.k=0.0;
    planetname=string();
    return;
  }

  bool ok=doPosition(origin_node,&pos.i);

  if(!ok){
    pos.i=pos.j=pos.k=0.0;
  }

  planetname=origin_node->attr_value("planet");
}

#endif
/* *********************************************************** */

void Mission::doSettings(easyDomNode *node){
  vector<easyDomNode *>::const_iterator siter;
  
  for(siter= node->subnodes.begin() ; siter!=node->subnodes.end() ; siter++){
    easyDomNode *mnode=*siter;
    if(mnode->Name()=="origin"){
      doOrigin(mnode);
    }
  }
}


/* *********************************************************** */

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

/* *********************************************************** */

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

/* *********************************************************** */

void Mission::doFlightgroups(easyDomNode *node){
  vector<easyDomNode *>::const_iterator siter;

  for(siter= node->subnodes.begin() ; siter!=node->subnodes.end() ; siter++){
    checkFlightgroup(*siter);
  }

}

/* *********************************************************** */

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
  string terrain_nr=node->attr_value("terrain_nr");
  string unittype= node->attr_value("unit_type");
  if(name.empty() || faction.empty() || type.empty() || ainame.empty() || waves.empty() || nr_ships.empty() ){
    cout << "no valid flightgroup decsription" << endl;
    return;
  }
  if (unittype.empty()) {
    unittype = string ("unit");
  }

  int waves_i=atoi(waves.c_str());
  int nr_ships_i=atoi(nr_ships.c_str());

  bool have_pos=false;
  bool have_rot=false;

  float pos[3];
  float rot[3];

  rot[0]=rot[1]=rot[2]=0.0;

  Flightgroup *fg=new Flightgroup();

  vector<easyDomNode *>::const_iterator siter;

  for(siter= node->subnodes.begin() ; siter!=node->subnodes.end() ; siter++){
    if((*siter)->Name()=="pos"){
      have_pos=doPosition(*siter,pos);
    }
    else if((*siter)->Name()=="rot"){
      have_rot=doRotation(*siter,rot);
    }
    else if((*siter)->Name()=="order"){
      doOrder(*siter,fg);
    }
  }

  if(!have_pos){
    cout << "don;t have a position in flightgroup " << name << endl;
    delete fg;
    return;
  }

#if 0
  cout << "flightgroup: " << name << " " << faction << " " << type << " " << ainame << " " << waves << " " << nr_ships << endl;
  cout << "   pos: " << pos[0] << " " << pos[1] << " " << pos[2] << " " << endl;
  cout << "   rot: " << rot[0] << " " << rot[1] << " " << rot[2] << " " << endl;
#endif

  if (terrain_nr.empty()) {
    fg->terrain_nr=-1;
  } else {
    if (terrain_nr=="mission") {
      fg->terrain_nr=-2;
    } else {
      fg->terrain_nr = atoi (terrain_nr.c_str());
    }
  }
  fg->unittype = Flightgroup::UNIT;
  if (unittype=="vehicle")
    fg->unittype= Flightgroup::VEHICLE;
  if (unittype=="building")
    fg->unittype=Flightgroup::BUILDING;
  fg->name=name;
  fg->faction=faction;
  fg->type=type;
  fg->ainame=ainame;
  fg->flightgroup_nr=number_of_flightgroups;
  fg->ship_nr=number_of_ships;
  fg->waves=waves_i;
  fg->nr_ships=nr_ships_i;
  fg->domnode=node;

  for(int i=0;i<3;i++){
    fg->pos[i]=pos[i];
    fg->rot[i]=rot[i];
  }

  fg->nr_waves_left=fg->waves-1;
  fg->nr_ships_left=fg->nr_ships;
  fg->orderlist=NULL;

  if(ainame[0]=='_'){
#ifndef VS_MIS_SEL
    addModule(ainame.substr(1));
#endif
  }

  flightgroups.push_back(fg);

  number_of_flightgroups++;
  number_of_ships+=nr_ships_i;
}

/* *********************************************************** */

bool Mission::doPosition(easyDomNode *node,float pos[3]){
  string x=node->attr_value("x");
  string y=node->attr_value("y");
  string z=node->attr_value("z");
  string offset=node->attr_value("offset");

  //  cout << "POS: x=" << x << " y=" << y << " z=" << z << endl;

  if(x.empty() || y.empty() || z.empty() ){
    cout << "no valid position" << endl;
    return false;
  }

  pos[0]=atof(x.c_str());
  pos[1]=atof(y.c_str());
  pos[2]=atof(z.c_str());

  if(!offset.empty()){
    Flightgroup *fg=findFlightgroup(offset);
    if(fg==NULL){
      cout << "can't find flightgroup " << offset << " for offset" << endl;
    }
    else{
      pos[0]+=fg->pos[0];
      pos[1]+=fg->pos[1];
      pos[2]+=fg->pos[2];
    }
  }


  return true;
}

/* *********************************************************** */

Flightgroup *Mission::findFlightgroup(string offset_name){
  vector<Flightgroup *>::const_iterator siter;

  for(siter= flightgroups.begin() ; siter!=flightgroups.end() ; siter++){
    // cout << "checking " << offset_name << " against " << (*siter)->name << endl;
    if((*siter)->name==offset_name){
      //cout << "found " << offset_name << " against " << (*siter)->name << endl;
      return *siter;
    }
  }
  return NULL;
}

/* *********************************************************** */

bool Mission::doRotation(easyDomNode *node,float rot[3]){
  //not yet
  return true;
}

/* *********************************************************** */

void Mission::doOrder(easyDomNode *node,Flightgroup *fg){
  // nothing yet
  string order=node->attr_value("order");
  string target=node->attr_value("target");

  if(order.empty() || target.empty()){
    cout << "you have to give an order and a target" << endl;
    return;
  }

  // the tmptarget is evaluated later
  // because the target may be a flightgroup that's not yet defined
  fg->ordermap[order]=target;
}

/* *********************************************************** */

string Mission::getVariable(string name,string defaultval){
   vector<easyDomNode *>::const_iterator siter;
  
  for(siter= variables->subnodes.begin() ; siter!=variables->subnodes.end() ; siter++){
    string scan_name=(*siter)->attr_value("name");
    //    cout << "scanning section " << scan_name << endl;

    if(scan_name==name){
      return (*siter)->attr_value("value");
    }
  }

  cout << "WARNING: no variable named " << name << endl;

  return defaultval;
 
}
