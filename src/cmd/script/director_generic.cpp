#ifdef HAVE_PYTHON

#include <Python.h>

#endif
#include "cmd/unit_generic.h"
#include "cmd/ai/order.h"

#include "configxml.h"
#include "gfx/cockpit_generic.h"

#include "python/python_class.h"

#include <boost/version.hpp>
#if BOOST_VERSION != 102800
#include <boost/python/class.hpp>
#else
#include <boost/python/detail/extension_class.hpp>
#endif



#include "pythonmission.h"
#include "mission.h"
#include "savegame.h"
using std::cout;
using std::cerr;
using std::endl;
PYTHON_INIT_INHERIT_GLOBALS(Director,PythonMissionBaseClass);
float getSaveData (int whichcp, string key, unsigned int num) {
  if (whichcp < 0|| whichcp >= _Universe->numPlayers()) {
    return 0;
  }
  vector<float> * ans =&(_Universe->AccessCockpit(whichcp)->savegame->getMissionData (key));
  if (num >=ans->size()) {
    return 0;
  }
  return (*ans)[num];
}
string getSaveString (int whichcp, string key, unsigned int num) {
  if (whichcp < 0|| whichcp >= _Universe->numPlayers()) {
    return "";
  }
  vector<std::string> * ans = &(_Universe->AccessCockpit(whichcp)->savegame->getMissionStringData (key));
  if (num >=ans->size()) {
    return "";
  }
  return (*ans)[num];  
}
unsigned int getSaveDataLength (int whichcp, string key) {
  if (whichcp < 0|| whichcp >= _Universe->numPlayers()) {
    return 0;
  }
  return (_Universe->AccessCockpit(whichcp)->savegame->getMissionDataLength(key));
}
unsigned int getSaveStringLength (int whichcp, string key) {
  if (whichcp < 0|| whichcp >= _Universe->numPlayers()) {
    return 0;
  }
  return (_Universe->AccessCockpit(whichcp)->savegame->getMissionStringDataLength(key));
}
unsigned int pushSaveData (int whichcp, string key, float val) {
  if (whichcp < 0|| whichcp >= _Universe->numPlayers()) {
    return 0;
  }
  vector<float> * ans =&((_Universe->AccessCockpit(whichcp)->savegame->getMissionData (key)));
  ans->push_back (val);
  return ans->size()-1;

}

unsigned int eraseSaveData (int whichcp, string key, unsigned int index) {
  if (whichcp < 0|| whichcp >= _Universe->numPlayers()) {
    return 0;
  }
  vector<float> * ans =&((_Universe->AccessCockpit(whichcp)->savegame->getMissionData (key)));
  if (index <ans->size()) {
    ans->erase (ans->begin()+index);
  }
  return ans->size();

}
unsigned int pushSaveString (int whichcp, string key, string value) {
  if (whichcp < 0|| whichcp >= _Universe->numPlayers()) {
    return 0;
  }
  vector<std::string> * ans =&((_Universe->AccessCockpit(whichcp)->savegame->getMissionStringData (key)));
  ans->push_back (std::string(value));
  return ans->size()-1;
}

void putSaveString (int whichcp, string key, unsigned int num, string val) {
  if (whichcp < 0|| whichcp >= _Universe->numPlayers()) {
    return;
  }
  vector<std::string> *ans =&((_Universe->AccessCockpit(whichcp)->savegame->getMissionStringData (key)));
  if (num<ans->size()) {
    (*ans)[num]= val;
  }
}

void putSaveData (int whichcp, string key, unsigned int num, float val) {
  if (whichcp < 0|| whichcp >= _Universe->numPlayers()) {
    return;
  }
  vector<float> * ans =&((_Universe->AccessCockpit(whichcp)->savegame->getMissionData (key)));
  if (num<ans->size()) {
    (*ans)[num] = val;
  }
}

unsigned int eraseSaveString (int whichcp, string key, unsigned int index) {
  if (whichcp < 0|| whichcp >= _Universe->numPlayers()) {
    return 0;
  }
  vector<std::string> *ans =&((_Universe->AccessCockpit(whichcp)->savegame->getMissionStringData (key)));
  if (index <ans->size()) {
    ans->erase (ans->begin()+index);
  }
  return ans->size();

}

vector <string> loadStringList (int playernum,string mykey) {
	if (playernum<0||playernum>=_Universe->numPlayers()) {
		return vector<string> ();
	}
	vector<float> * ans =&((_Universe->AccessCockpit(playernum)->savegame->getMissionData (mykey)));
	int lengt = ans->size();
	if (lengt<1) {
		return vector<string> ();
	}
	vector<string> rez;
	string curstr;
	int length = (int)(*ans)[0];
	for (int j=0;j<length&&j<lengt;j++) {
		char myint=(char)(*ans)[j+1];
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
	vector <float>* ans =&((_Universe->AccessCockpit(playernum)->savegame->getMissionData (mykey)));
	int length=ans->size();
	int k=1;
	int tot=0;
	int i;
	for (i=0;i<(int)names.size();i++) {
		tot += names[i].size()+1;
	}
	if (length==0) {
		pushSaveData(playernum,mykey,tot);
	} else {
		(*ans)[0]=tot;
	}
	for (i=0;i<(int)names.size();i++) {
		for (int j=0;j<(int)names[i].size();j++) {
			if (k < length) {
				(*ans)[k]=(float)names[i][j];
			} else {
				pushSaveData(playernum,mykey,(float)names[i][j]);
			}
			k+=1;
		}
		if (k < length) {
			(*ans)[k]=0;
		} else {
			pushSaveData(playernum,mykey,0);
		}
		k+=1;
	}
}

PYTHON_BEGIN_MODULE(Director)
PYTHON_BEGIN_INHERIT_CLASS(Director,pythonMission,PythonMissionBaseClass,"Mission")
  PYTHON_DEFINE_METHOD_DEFAULT(Class,&PythonMissionBaseClass::Pickle,"Pickle",pythonMission::default_Pickle);
  PYTHON_DEFINE_METHOD_DEFAULT(Class,&PythonMissionBaseClass::UnPickle,"UnPickle",pythonMission::default_UnPickle);
  PYTHON_DEFINE_METHOD_DEFAULT(Class,&PythonMissionBaseClass::Execute,"Execute",pythonMission::default_Execute);
PYTHON_END_CLASS(Director,pythonMission)
  PYTHON_DEFINE_GLOBAL(Director,&putSaveData,"putSaveData");
  PYTHON_DEFINE_GLOBAL(Director,&pushSaveData,"pushSaveData");
  PYTHON_DEFINE_GLOBAL(Director,&eraseSaveData,"eraseSaveData");
  PYTHON_DEFINE_GLOBAL(Director,&getSaveData,"getSaveData");
  PYTHON_DEFINE_GLOBAL(Director,&getSaveDataLength,"getSaveDataLength");
  PYTHON_DEFINE_GLOBAL(Director,&putSaveString,"putSaveString");
  PYTHON_DEFINE_GLOBAL(Director,&pushSaveString,"pushSaveString");
  PYTHON_DEFINE_GLOBAL(Director,&getSaveString,"getSaveString");
  PYTHON_DEFINE_GLOBAL(Director,&getSaveStringLength,"getSaveStringLength");
  PYTHON_DEFINE_GLOBAL(Director,&eraseSaveString,"eraseSaveString");
PYTHON_END_MODULE(Director)

void InitDirector() {
	Python::reseterrors();
	PYTHON_INIT_MODULE(Director);
	Python::reseterrors();
}


void Mission::loadModule(string modulename){
  missionNode *node=director;
  
  debug(3,node,SCRIPT_PARSE,"loading module "+modulename);
  
  cout << "  loading module " << modulename << endl;
  
  string filename="modules/"+modulename+".module";
  missionNode *import_top=importf->LoadXML(filename.c_str());
  
  if(import_top==NULL){
    
  }
  
  import_top->Tag(&tagmap);
  
  doModule(import_top,SCRIPT_PARSE);

}
void Mission::loadMissionModules(){
  missionNode *node=director;

    while(import_stack.size()>0){
    string importname=import_stack.back();
    import_stack.pop_back();

    missionNode *module=runtime.modules[importname];
    if(module==NULL){
      loadModule(importname);
    }
    else{
      debug(3,node,SCRIPT_PARSE,"already have module "+importname);
    }
  }

}

void Mission::RunDirectorScript (const string& script){
  runScript (director,script,0);
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

bool Mission::runScript(string modulename,const string &scriptname,unsigned int classid){

  return runScript (runtime.modules[modulename],scriptname,classid);
}
double Mission::getGametime(){
  return gametime;
}

void Mission::addModule(string modulename){
  import_stack.push_back(modulename);
}

void Mission::DirectorStartStarSystem(StarSystem *ss){
  RunDirectorScript ("initstarsystem");
}

std::string Mission::Pickle () {
  if (!runtime.pymissions) {
    return "";
  }else {
    return runtime.pymissions->Pickle();
  }
}
void Mission::UnPickle (string pickled) {
  if (runtime.pymissions)
    runtime.pymissions->UnPickle(pickled);  
}

void Mission::DirectorStart(missionNode *node){
  cout << "DIRECTOR START" << endl;

  static int st_debuglevel=atoi(vs_config->getVariable("interpreter","debuglevel","0").c_str());
  static bool st_start_game=XMLSupport::parse_bool(vs_config->getVariable("interpreter","startgame","true"));
  static bool st_do_trace=XMLSupport::parse_bool(vs_config->getVariable("interpreter","trace","false"));

  debuglevel=st_debuglevel;
  start_game=st_start_game;
  do_trace=st_do_trace;

  vi_counter=0;
  old_vi_counter=0;

  olist_counter=0;
  old_olist_counter=0;

  string_counter=0;
  old_string_counter=0;
  missionThread *main_thread= new missionThread;
  runtime.thread_nr=0;
  runtime.threads.push_back(main_thread);
  runtime.cur_thread=main_thread;

  director=NULL;
  //  msgcenter->add("game","all","parsing programmed mission");
  std::string doparse = node->attr_value ("do_parse");
  if (!doparse.empty()) {
    if (XMLSupport::parse_bool (doparse)==false) {
      return;
    }
  }
  cout << "parsing declarations for director" << endl;

  parsemode=PARSE_DECL;

  doModule(node,SCRIPT_PARSE);

  importf=new easyDomFactory<missionNode>();

  loadMissionModules();

  parsemode=PARSE_FULL;

  doModule(node,SCRIPT_PARSE);

  stdext::hash_map<string,missionNode *>::iterator iter;
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

  this->player_num=_Universe->CurrentCockpit();
  if (nextpythonmission) {
	// CAUSES AN UNRESOLVED EXTERNAL SYMBOL FOR PythonClass::last_instance ?!?!
#ifndef _WIN32
    char * tmp=nextpythonmission;
    while (*tmp) {
      if (tmp[0]=='\r') tmp[0]='\n';
      tmp++;
    }
#endif
    runtime.pymissions=(pythonMission::FactoryString (nextpythonmission));
    delete [] nextpythonmission; //delete the allocated memory
    nextpythonmission=NULL;
	if (!this->unpickleData.empty()) {
		if (runtime.pymissions) {
			runtime.pymissions->UnPickle(unpickleData);
			unpickleData="";
		}
	}
  }
  if(director==NULL){
    return;
  }
  RunDirectorScript("initgame");
}
