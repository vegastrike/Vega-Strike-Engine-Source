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
  xml Mission Scripting written by Alexander Rawass <alexannika@users.sourceforge.net>
*/

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <ctype.h>
#include <assert.h>
#ifndef WIN32
// this file isn't available on my system (all win32 machines?) i dun even know what it has or if we need it as I can compile without it
#include <unistd.h>
#include <pwd.h>
#endif

#include <expat.h>

#include <fstream>

#include "xml_support.h"

#include "vegastrike.h"
#include "vs_path.h"
#include "lin_time.h"
#include "cmd/unit.h"
#include "cmd/ai/order.h"

#include "mission.h"
#include "easydom.h"

#include "vs_globals.h"
#include "config_xml.h"

#include "msgcenter.h"
#include "cmd/briefing.h"
#ifdef HAVE_PYTHON
#include "Python.h"
#endif
#include "flightgroup.h"
#include "gldrv/winsys.h"
//#include "vegastrike.h"

extern bool have_yy_error;

/* *********************************************************** */


void Mission::DirectorStart(missionNode *node){

  cout << "DIRECTOR START" << endl;

  debuglevel=atoi(vs_config->getVariable("interpreter","debuglevel","0").c_str());
  start_game=XMLSupport::parse_bool(vs_config->getVariable("interpreter","startgame","true"));

  do_trace=XMLSupport::parse_bool(vs_config->getVariable("interpreter","trace","false"));

  vi_counter=0;
  old_vi_counter=0;

  olist_counter=0;
  old_olist_counter=0;

  string_counter=0;
  old_string_counter=0;

  missionThread *main_thread=new missionThread;
  runtime.thread_nr=0;
  runtime.threads.push_back(main_thread);
  runtime.cur_thread=main_thread;

  director=NULL;

  //  msgcenter->add("game","all","parsing programmed mission");

  cout << "parsing declarations for director" << endl;

  parsemode=PARSE_DECL;

  doModule(node,SCRIPT_PARSE);

  importf=new easyDomFactory<missionNode>();

  loadMissionModules();

  parsemode=PARSE_FULL;

  doModule(node,SCRIPT_PARSE);

  map<string,missionNode *>::iterator iter;
  //=runtime.modules.begin()

  for(iter=runtime.modules.begin();iter!=runtime.modules.end();iter++){
    string mname=(*iter).first ;
    missionNode *mnode=(*iter).second;

    if(mname!="director"){
      cout << "  parsing full module " << mname << endl;
      doModule(mnode,SCRIPT_PARSE);
    }
  }
}

void Mission::DirectorInitgame(){
  if(director==NULL){
    return;
  }
#ifdef HAVE_PYTHON
  Py_Initialize();
  PyRun_SimpleString("import test1");  //  PyRun_SimpleString("director=VSdirector()");
#endif
  RunDirectorScript("initgame");
}

void Mission::DirectorLoop(){
  if(director==NULL){
    return;
  }
#ifdef HAVE_PYTHON
  PyRun_SimpleString("test1.director.gameloop()");
#endif

  if(vi_counter!=old_vi_counter){
    char buf[200];
    sprintf(buf,"VI_COUNTER %d\n",vi_counter);
    debug(2,NULL,0,buf);
  }
  old_vi_counter=vi_counter;
  
  if(olist_counter!=old_olist_counter){
    char buf[200];
    sprintf(buf,"OLIST_COUNTER %d\n",olist_counter);
    debug(2,NULL,0,buf);
  }
  old_olist_counter=olist_counter;
  
  if(string_counter!=old_string_counter){
    char buf[200];
    sprintf(buf,"STRING_COUNTER %d\n",string_counter);
    debug(2,NULL,0,buf);
  }
  old_string_counter=string_counter;
  RunDirectorScript("gameloop");
  BriefingLoop();
}
void Mission::DirectorEnd(){
  if(director==NULL){
    return;
  }
  RunDirectorScript ("endgame");
#ifdef WIN32
  var_out.open("c:\\tmp\\default-player.variables");
#else
  struct passwd *pwent;
  pwent=getpwuid(getuid());
  //  printf("home save dir: %s\n",pwent->pw_dir);

  string var_file=string(pwent->pw_dir)+string(DELIMSTR)+string(HOMESUBDIR)+string(DELIMSTR)+string("default-player.variables");
  //var_out.open("/tmp/default-player.variables");
  var_out.open(var_file.c_str());
#endif

  if(!var_out){
    cout << "ERROR: could not write variables file" << endl;
    return;
  }

  saveVariables(var_out);

  var_out.close();

}

void Mission::loadModule(string modulename){
  missionNode *node=director;

      debug(3,node,SCRIPT_PARSE,"loading module "+modulename);

      cout << "  loading module " << modulename << endl;

      string filename="modules/"+modulename+".module";
      missionNode *import_top=importf->LoadXML(filename.c_str());

      if(import_top==NULL){
	debug(5,node,SCRIPT_PARSE,"could not load "+filename);

	//	fatalError(node,SCRIPT_PARSE,"could not load module file "+filename);
	//assert(0);
	string f2name="modules/"+modulename+".c";
        import_top=importf->LoadCalike(f2name.c_str());

	if(import_top==NULL){
	  //debug(0,node,SCRIPT_PARSE,"could not load "+f2name);
	  fatalError(node,SCRIPT_PARSE,"could not load module "+modulename);
	  assert(0);
	}
	if(have_yy_error){
	  fatalError(NULL,SCRIPT_PARSE,"yy-error while parsing "+modulename);
	  assert(0);
	}
      }

      import_top->Tag(&tagmap);

      doModule(import_top,SCRIPT_PARSE);

}

void Mission::addModule(string modulename){
  import_stack.push_back(modulename);
}

void Mission::loadMissionModules(){
  missionNode *node=director;

    while(import_stack.size()>0){
    string importname=import_stack.back();
    import_stack.pop_back();

    missionNode *module=runtime.modules[importname];
    if(module==NULL){
      loadModule(importname);
#if 0
      debug(3,node,SCRIPT_PARSE,"loading module "+import->script.name);

      cout << "  loading module " << import->script.name << endl;

      string filename="modules/"+import->script.name+".module";
      missionNode *import_top=importf->LoadXML(filename.c_str());

      if(import_top==NULL){
	debug(5,node,SCRIPT_PARSE,"could not load "+filename);

	//	fatalError(node,SCRIPT_PARSE,"could not load module file "+filename);
	//assert(0);
	string f2name="modules/"+import->script.name+".c";
        import_top=importf->LoadCalike(f2name.c_str());

	if(import_top==NULL){
	  //debug(0,node,SCRIPT_PARSE,"could not load "+f2name);
	  fatalError(node,SCRIPT_PARSE,"could not load module "+import->script.name);
	  assert(0);
	}
	if(have_yy_error){
	  fatalError(NULL,SCRIPT_PARSE,"yy-error while parsing "+import->script.name);
	  assert(0);
	}
      }

      import_top->Tag(&tagmap);

      doModule(import_top,SCRIPT_PARSE);
#endif
    }
    else{
      debug(3,node,SCRIPT_PARSE,"already have module "+importname);
    }
  }

}
void Mission::RunDirectorScript (const string& script){
  runScript (director,script,0);
}
bool Mission::runScript(string modulename,const string &scriptname,unsigned int classid){

  return runScript (runtime.modules[modulename],scriptname,classid);
}
bool Mission::runScript(missionNode *module_node,const string &scriptname,unsigned int classid){
  if(module_node==NULL){
    return false;
  }

  missionNode *script_node=module_node->script.scripts[scriptname];
  if(script_node==NULL){
    return false;
  }
  
  runtime.cur_thread->module_stack.push_back(module_node);
  runtime.cur_thread->classid_stack.push_back(classid);

  varInst *vi=doScript(script_node,SCRIPT_RUN);
  deleteVarInst(vi);

  runtime.cur_thread->classid_stack.pop_back();
  runtime.cur_thread->module_stack.pop_back();
  return true;
}


void Mission::DirectorBenchmark(){
  double elapsed=GetElapsedTime();

  gametime+=SIMULATION_ATOM;//elapsed;
  total_nr_frames++;

  //cout << "elapsed= " << elapsed << " fps= " << 1.0/elapsed << " average= " << ((double)total_nr_frames)/gametime << " in " << gametime << " seconds" << endl;

  if(benchmark>0.0 && benchmark<gametime){
    cout << "Game was running for " << gametime << " secs,   av. framerate " << ((double)total_nr_frames)/gametime << endl;
    winsys_exit(0);
  }
}

double Mission::getGametime(){
  return gametime;
}

void Mission::DirectorShipDestroyed(Unit *unit){
  Flightgroup *fg=unit->getFlightgroup();

  if(fg==NULL){
    printf("ship destroyed-no flightgroup\n");
    return;
  }
  if(fg->nr_ships_left<=0 && fg->nr_waves_left>0){
    printf("WARNING: nr_ships_left<=0\n");
    return;
  }
  
  fg->nr_ships_left-=1;

  char buf[200];
  sprintf(buf,"Ship destroyed: %s:%s:%s-%d",fg->faction.c_str(),fg->type.c_str(),fg->name.c_str(),unit->getFgSubnumber());
  
  
  msgcenter->add("game","all",buf);

  if(fg->nr_ships_left==0){
    printf("no ships left in fg %s\n",fg->name.c_str());
    if(fg->nr_waves_left>0){
      //      printf("relaunching wave %d of fg %s\n",fg->waves-fg->nr_waves_left,fg->name.c_str());
      sprintf(buf,"Relaunching %s wave",fg->name.c_str());
      mission->msgcenter->add("game","all",buf);

      // launch new wave
      fg->nr_waves_left-=1;
      fg->nr_ships_left=fg->nr_ships;

      Order *order=unit->getAIState()->findOrderList();
      fg->orderlist=NULL;
      if(order){
	printf("found an orderlist\n");
	fg->orderlist=order->getOrderList();
      }
      CreateFlightgroup cf;
      cf.fg = fg;
      cf.unittype= CreateFlightgroup::UNIT;
      cf.fg->pos=unit->Position();
      cf.waves = fg->nr_waves_left;
      cf.nr_ships = fg->nr_ships;
      
      //      cf.type = fg->type;
      call_unit_launch(&cf,UNITPTR,string(""));
    }
    else{
      mission->msgcenter->add("game","all","Flightgroup "+fg->name+" destroyed");
    }
  }
}

void Mission::DirectorStartStarSystem(StarSystem *ss){
  RunDirectorScript ("initstarsystem");
}
bool Mission::BriefingInProgress() {
  return (briefing!=NULL);
}
void Mission::BriefingStart() {
  if (briefing) {
    BriefingEnd();
  }
  briefing = new Briefing();
  RunDirectorScript ("initbriefing");
}
void Mission::BriefingUpdate() {
  if (briefing){
      briefing->Update();
  }
}

void Mission::BriefingLoop() {
  if (briefing) {
    RunDirectorScript ("loopbriefing");
  }
}
class TextPlane * Mission::BriefingRender() {
  if (briefing) {
    vector <std::string> who;
    who.push_back ("briefing");
    string str1;
    gameMessage * g1 = msgcenter->last(0,who);
    gameMessage * g2=msgcenter->last(1,who);
    if (g1) {
      str1 = g1->message;
    }
    if (g2) {
      str1 = str1 + string("\n")+g2->message;
    }
    briefing->tp.SetText (str1);
    briefing->Render();
    return &briefing->tp;
  }
  return NULL;
}

void Mission::BriefingEnd() {
  if (briefing) {
    RunDirectorScript ("endbriefing");      
    delete briefing;
    briefing = NULL;
  }
}
