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
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <ctype.h>
#include <Python.h>
#ifndef WIN32
// this file isn't available on my system (all win32 machines?) i dun even know what it has or if we need it as I can compile without it
#include <unistd.h>
#endif
//#include "gfx/aux_texture.h"
//#include <expat.h>
//#include "xml_support.h"

//#include "vegastrike.h"
#include <assert.h>
#include "mission.h"
#include "flightgroup.h"
//#include "gldrv/winsys.h"
#ifdef HAVE_PYTHON
#include "Python.h"
#endif
#include "python/python_class.h"
//#include "easydom.h"

//#include "msgcenter.h"

//#include "vs_globals.h"
//#include "vegastrike.h"

/* *********************************************************** */
Mission::~Mission() {
  VSFileSystem::vs_fprintf (stderr,"Mission Cleanup Not Yet Implemented");
  //do not delete msgcenter...could be vital
}
double   Mission::gametime=0.0;
int Mission::number_of_ships=0;
int Mission::number_of_flightgroups=0;
int Mission::total_nr_frames=0;
vector<Flightgroup *> Mission::flightgroups;

Mission::Mission (const char * filename,const std::string & script, bool loadscripts) {
    ConstructMission(filename,script,loadscripts);
}
Mission::Mission (const char * filename, bool loadscripts) {
    ConstructMission(filename,string(""),loadscripts);
}
void Mission::ConstructMission(const char *configfile, const std::string &script, bool loadscripts){
  player_autopilot=global_autopilot=AUTO_NORMAL;
  briefing=NULL;
  director=NULL;
  runtime.pymissions=NULL;
  nextpythonmission=NULL;
  if (script.length()>0) {
      nextpythonmission=new char [script.length()+2];
      nextpythonmission[script.length()+1]=0;
      nextpythonmission[script.length()]=0;
      strcpy (nextpythonmission,script.c_str());
  }
  easyDomFactory<missionNode> *domf= new easyDomFactory<missionNode>();//such a bloody leak!

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

  if (loadscripts) {
    initTagMap();
    initCallbackMaps();

    top->Tag(&tagmap);
  }

#endif

}
MessageCenter * Mission::msgcenter=NULL;
void Mission::initMission(bool loadscripts){
  if (!top)
    return;
  if (msgcenter==NULL) {
    msgcenter=new MessageCenter();
  }
  //  msgcenter->add("game","all","Welcome to Vegastrike");
  /*
  msgcenter->add("game","news","A hero is born");
  msgcenter->add("game","news","2 hero is born");
  msgcenter->add("game","news","3 hero is born");
  msgcenter->add("game","news","4 hero is born.                That day, in teh news hero was born without even boring the born of bachunkuphe threw his last baseball throw he chewed his last hay the man was a hero a heayrinagia sf of a hero...who knw that ith all things iblicatedly obfusictaroed he arced his bunko bunka\nbachunk!jtkgjdfljg fdlg fdj glkdf jglkdfj glkdf jgjfd lkgj dflkg jlfkdjg ldjg  j");
  */
  checkMission(top,loadscripts);
  mission_name = getVariable("mission_name","");
  //  objectives.push_back (Objective(0.0,"cachunk"));
  //  objectives.push_back (Objective(1,"cachunk.org"));
  //  objectives.push_back (Objective(-1,"cachunk.com"));
}

/* *********************************************************** */

bool Mission::checkMission(easyDomNode *node, bool loadscripts){
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
      if (loadscripts) {
	DirectorStart((missionNode *)*siter);
      }
    } else if (((*siter)->Name()=="python") && (!this->nextpythonmission)){ //I need to get rid of an extra whitespace at the end that expat may have added... Python is VERY strict about that... :(
		string locals = (*siter)->attr_value(textAttr);
      const char *constdumbstr=locals.c_str(); //get the text XML attribute
      int i=strlen(constdumbstr); //constdumbstr is the string I wish to copy... i is its length.
      char *dumbstr=new char [i+2]; //allocate 2 extra bytes for a double-null-terminated string.
      strncpy(dumbstr,constdumbstr,i); //i copy constdumbstr to dumbstr.
      dumbstr[i]='\0'; //I make sure that it has 2 null bytes at the end.
      dumbstr[i+1]='\0'; //I am allowed to use i+1 because I allocated 2 extra bytes
      for (i-=1;i>=0;i--) { //start from the end-1, or i-1 and set i to that value(i-=1)
        if (dumbstr[i]=='\t'||dumbstr[i]==' '||dumbstr[i]=='\r'||dumbstr[i]=='\n') { //I check if dumbstr[i] contains whitespace
          dumbstr[i]='\0'; //if so, I truncate the string
        } else {
          dumbstr[i+1]='\n'; //otherwise I add a new line (python sometimes gets mad...)
          dumbstr[i+2]='\0'; //and add a null byte (If i is at the end of the allocated memory, I will use the extra byte
          break; //get out of the loop so that it doesn't endlessly delete the newlines that I added.
        }
      }
	  this->nextpythonmission=dumbstr;
    } else{
      cout << "warning: Unknown tag: " << (*siter)->Name() << endl;
    }
  }
  return true;
}

static std::vector <Mission *> Mission_delqueue;
void Mission::wipeDeletedMissions() {
  while (!Mission_delqueue.empty()) {
    delete Mission_delqueue.back();
    Mission_delqueue.pop_back();
  }
}

void Mission::terminateMission(){
	vector<Mission *> *active_missions = ::active_missions.Get();
	vector<Mission *>::iterator f = std::find (active_missions->begin(),active_missions->end(),this);
	if (f!=active_missions->end()) {
		active_missions->erase (f);
	}
	if (this!=(*active_missions)[0]) {
		Mission_delqueue.push_back(this);//only delete if we arent' the base mission
	}
	if (runtime.pymissions)
		runtime.pymissions->Destroy();
	runtime.pymissions=NULL;
}


/* *********************************************************** */

void Mission::doOrigin(easyDomNode *node){
  origin_node=node;
}

/* *********************************************************** */

#ifndef VS_MIS_SEL
void Mission::GetOrigin(QVector &pos,string &planetname){
  //  float pos[3];

  if(origin_node==NULL){
    pos.i=pos.j=pos.k=0.0;
    planetname=string();
    return;
  }

  bool ok=doPosition(origin_node,&pos.i,NULL);

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
void Mission::AddFlightgroup (Flightgroup * fg) {
  fg->flightgroup_nr = flightgroups.size();
  flightgroups.push_back (fg);
  number_of_flightgroups = flightgroups.size();
  
}
/* *********************************************************** */

void Mission::checkFlightgroup(easyDomNode *node){
  if(node->Name()!="flightgroup"){
    cout << "not a flightgroup" << endl;
    return;
  }

  // nothing yet
  string texture=node->attr_value("logo");
  string texture_alpha=node->attr_value("logo_alpha");
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

  double pos[3];
  float rot[3];

  rot[0]=rot[1]=rot[2]=0.0;
  CreateFlightgroup cf;
  cf.fg = Flightgroup::newFlightgroup(name,type,faction,ainame,nr_ships_i,waves_i,texture,texture_alpha,this);
  vector<easyDomNode *>::const_iterator siter;

  for(siter= node->subnodes.begin() ; siter!=node->subnodes.end() ; siter++){
    if((*siter)->Name()=="pos"){
      have_pos=doPosition(*siter,pos,&cf);
    }
    else if((*siter)->Name()=="rot"){
      have_rot=doRotation(*siter,rot,&cf);
    }
    else if((*siter)->Name()=="order"){
      doOrder(*siter,cf.fg);
    }     
  }
  
  if(!have_pos){
    cout << "don;t have a position in flightgroup " << name << endl;
  }
  if (terrain_nr.empty()) {
    cf.terrain_nr=-1;
  } else {
    if (terrain_nr=="mission") {
      cf.terrain_nr=-2;
    } else {
      cf.terrain_nr = atoi (terrain_nr.c_str());
    }
  }
  cf.unittype = CreateFlightgroup::UNIT;
  if (unittype=="vehicle")
    cf.unittype= CreateFlightgroup::VEHICLE;
  if (unittype=="building")
    cf.unittype=CreateFlightgroup::BUILDING;

  cf.nr_ships=nr_ships_i;
  cf.domnode=(node);//don't hijack node

    cf.fg->pos.i=pos[0];
    cf.fg->pos.j=pos[1];
    cf.fg->pos.k=pos[2];
  for(int i=0;i<3;i++){

    cf.rot[i]=rot[i];
  }
  cf.nr_ships=nr_ships_i;

  if(ainame[0]=='_'){
#ifndef VS_MIS_SEL
    addModule(ainame.substr(1));
#endif
  }
  number_of_ships+=nr_ships_i;
}

/* *********************************************************** */

bool Mission::doPosition(easyDomNode *node,double pos[3], CreateFlightgroup * cf){
  string x=node->attr_value("x");
  string y=node->attr_value("y");
  string z=node->attr_value("z");
  ///  string offset=node->attr_value("offset");

  //  cout << "POS: x=" << x << " y=" << y << " z=" << z << endl;

  if(x.empty() || y.empty() || z.empty() ){
    cout << "no valid position" << endl;
    return false;
  }

  pos[0]=strtod(x.c_str(),NULL);
  pos[1]=strtod(y.c_str(),NULL);
  pos[2]=strtod(z.c_str(),NULL);

  if(cf!=NULL){
    pos[0]+=cf->fg->pos.i;
    pos[1]+=cf->fg->pos.j;
    pos[2]+=cf->fg->pos.k;
  }
  return true;
}

/* *********************************************************** */

Flightgroup *Mission::findFlightgroup(const string &offset_name, const string &fac) {
  vector<Flightgroup *>::const_iterator siter;

  for(siter= flightgroups.begin() ; siter!=flightgroups.end() ; siter++){
    // cout << "checking " << offset_name << " against " << (*siter)->name << endl;
    if((*siter)->name==offset_name&&(fac.empty()||(*siter)->faction==fac)){
      //cout << "found " << offset_name << " against " << (*siter)->name << endl;
      return *siter;
    }
  }
  return NULL;
}

/* *********************************************************** */

bool Mission::doRotation(easyDomNode *node,float rot[3], class CreateFlightgroup *){
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
