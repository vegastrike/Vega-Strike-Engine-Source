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
#include "cmd/unit_generic.h"

#include "cmd/ai/order.h"
#include "mission.h"
#include "easydom.h"

#include "vs_globals.h"
#include "config_xml.h"
#include "savegame.h"
#include "msgcenter.h"
#include "cmd/briefing.h"
#include "pythonmission.h"
#ifdef HAVE_PYTHON
#include "Python.h"
#endif
#include "flightgroup.h"
#include "gldrv/winsys.h"
#include "python/python_class.h"
#ifdef USE_BOOST_129
#include <boost/python/class.hpp>
#else
#include <boost/python/detail/extension_class.hpp>
#endif
#include "gfx/cockpit_generic.h"
//#include "vegastrike.h"

extern bool have_yy_error;
/* *********************************************************** */
float getSaveData (int whichcp, string key, unsigned int num) {
  if (whichcp < 0|| whichcp >= _Universe->numPlayers()) {
    return 0;
  }
  olist_t * ans =&(_Universe->AccessCockpit(whichcp)->savegame->getMissionData (key));
  if (num >=ans->size()) {
    return 0;
  }
  return (*ans)[num]->float_val;
}
unsigned int getSaveDataLength (int whichcp, string key) {
  if (whichcp < 0|| whichcp >= _Universe->numPlayers()) {
    return 0;
  }
  olist_t * ans =&(_Universe->AccessCockpit(whichcp)->savegame->getMissionData (key));
  return ans->size();
}
unsigned int pushSaveData (int whichcp, string key, float val) {
  if (whichcp < 0|| whichcp >= _Universe->numPlayers()) {
    return 0;
  }
  olist_t * ans =&((_Universe->AccessCockpit(whichcp)->savegame->getMissionData (key)));
  varInst * vi = new varInst (VI_IN_OBJECT);//not belong to a mission...not sure should inc counter
  vi->type = VAR_FLOAT;
  vi->float_val=val;
  ans->push_back (vi);
  return ans->size()-1;

}

void putSaveData (int whichcp, string key, unsigned int num, float val) {
  if (whichcp < 0|| whichcp >= _Universe->numPlayers()) {
    return;
  }
  olist_t * ans =&((_Universe->AccessCockpit(whichcp)->savegame->getMissionData (key)));
  if (num<ans->size()) {
    (*ans)[num]->float_val = val;
  }
}

vector <string> loadStringList (int playernum,string mykey) {
	if (playernum<0||playernum>=_Universe->numPlayers()) {
		return vector<string> ();
	}
	olist_t * ans =&((_Universe->AccessCockpit(playernum)->savegame->getMissionData (mykey)));
	int lengt = ans->size();
	if (lengt<1) {
		return vector<string> ();
	}
	vector<string> rez;
	string curstr;
	int length = (*ans)[0]->float_val;
	for (int j=0;j<length&&j<lengt;j++) {
		char myint=(char)(*ans)[j+1]->float_val;
		if (myint != '\0') {
			curstr += myint;
		} else {
			rez.push_back(curstr);
			curstr="";
		}
	}
	return rez;
}
void saveStringList (int playernum,string mykey,vector<string> names) {
	if (playernum<0||playernum>=_Universe->numPlayers()) {
		return;
	}
	olist_t * ans =&((_Universe->AccessCockpit(playernum)->savegame->getMissionData (mykey)));
	int length=ans->size();
	int k=1;
	int tot=0;
	int i;
	for (i=0;i<names.size();i++) {
		tot += names[i].size()+1;
	}
	if (length==0) {
		pushSaveData(playernum,mykey,tot);
	} else {
		(*ans)[0]->float_val=tot;
	}
	for (i=0;i<names.size();i++) {
		for (int j=0;j<names[i].size();j++) {
			if (k < length) {
				(*ans)[k]->float_val=(float)names[i][j];
			} else {
				pushSaveData(playernum,mykey,(float)names[i][j]);
			}
			k+=1;
		}
		if (k < length) {
			(*ans)[k]->float_val=0;
		} else {
			pushSaveData(playernum,mykey,0);
		}
		k+=1;
	}
}
//ADD_FROM_PYTHON_FUNCTION(pythonMission)
PYTHON_BEGIN_MODULE(Director)
PYTHON_BEGIN_INHERIT_CLASS(Director,pythonMission,PythonMissionBaseClass,"Mission")
  PYTHON_DEFINE_METHOD_DEFAULT(Class,&PythonMissionBaseClass::Pickle,"Pickle",pythonMission::default_Pickle);
  PYTHON_DEFINE_METHOD_DEFAULT(Class,&PythonMissionBaseClass::UnPickle,"UnPickle",pythonMission::default_UnPickle);
  PYTHON_DEFINE_METHOD_DEFAULT(Class,&PythonMissionBaseClass::Execute,"Execute",pythonMission::default_Execute);
PYTHON_END_CLASS(Director,pythonMission)
  PYTHON_DEFINE_GLOBAL(Director,&putSaveData,"putSaveData");
  PYTHON_DEFINE_GLOBAL(Director,&pushSaveData,"pushSaveData");
  PYTHON_DEFINE_GLOBAL(Director,&getSaveData,"getSaveData");
  PYTHON_DEFINE_GLOBAL(Director,&getSaveDataLength,"getSaveDataLength");
PYTHON_END_MODULE(Director)

void InitDirector() {
	Python::reseterrors();
	PYTHON_INIT_MODULE(Director);
	Python::reseterrors();
}

void Mission::DirectorLoop(){
  if (runtime.pymissions)
    runtime.pymissions->Execute();
  BriefingLoop();
  if(director==NULL){
    return;
  }

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

      Order *order=NULL;
      order= unit->getAIState()?unit->getAIState()->findOrderList():NULL;
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

bool Mission::BriefingInProgress() {
  return (briefing!=NULL);
}
void Mission::BriefingStart() {
  if (briefing) {
    BriefingEnd();
  }
  briefing = new Briefing();
  if (runtime.pymissions)
	runtime.pymissions->callFunction("initbriefing");
  //RunDirectorScript ("initbriefing");
}
void Mission::BriefingUpdate() {
  if (briefing){
      briefing->Update();
  }
}

void Mission::BriefingLoop() {
  if (briefing) {
    if (runtime.pymissions)
      runtime.pymissions->callFunction("loopbriefing");
    //RunDirectorScript ("loopbriefing");
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
	if (runtime.pymissions)
		runtime.pymissions->callFunction("endbriefing");
	//RunDirectorScript ("endbriefing");      
    delete briefing;
    briefing = NULL;
  }
}

void Mission::DirectorBenchmark(){
  double oldgametime=gametime;
  gametime+=SIMULATION_ATOM;//elapsed;
  if (getTimeCompression()>=.1) {
    if (gametime<=oldgametime)
      gametime=SIMULATION_ATOM;
  }
  total_nr_frames++;

  //cout << "elapsed= " << elapsed << " fps= " << 1.0/elapsed << " average= " << ((double)total_nr_frames)/gametime << " in " << gametime << " seconds" << endl;

  if(benchmark>0.0 && benchmark<gametime){
    cout << "Game was running for " << gametime << " secs,   av. framerate " << ((double)total_nr_frames)/gametime << endl;
    winsys_exit(0);
  }
}
