#include "cmd/ai/order.h"

#include "configxml.h"
#include "gfx/cockpit_generic.h"
#ifdef HAVE_PYTHON

#include "Python.h"

#endif

#include "python/python_class.h"

#ifdef USE_BOOST_129
#include <boost/python/class.hpp>
#else
#include <boost/python/detail/extension_class.hpp>
#endif



#include "pythonmission.h"
#include "mission.h"
extern bool have_yy_error;

PYTHON_INIT_INHERIT_GLOBALS(Director,PythonMissionBaseClass);
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

  debuglevel=atoi(vs_config->getVariable("interpreter","debuglevel","0").c_str());
  start_game=XMLSupport::parse_bool(vs_config->getVariable("interpreter","startgame","true"));

  do_trace=XMLSupport::parse_bool(vs_config->getVariable("interpreter","trace","false"));

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

  this->player_num=(_Universe->AccessCockpit()-_Universe->AccessCockpit(0));
  if (nextpythonmission) {
	// CAUSES AN UNRESOLVED EXTERNAL SYMBOL FOR PythonClass::last_instance ?!?!

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
