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
#endif

#include <expat.h>

#include <fstream>

#include "xml_support.h"

#include "vegastrike.h"

#include "mission.h"
#include "easydom.h"

#include "vs_globals.h"
#include "config_xml.h"

#include "msgcenter.h"

#ifdef HAVE_PYTHON
#include "Python.h"
#endif

//#include "vegastrike.h"

extern bool have_yy_error;

/* *********************************************************** */


void Mission::DirectorStart(missionNode *node){

  cout << "DIRECTOR START" << endl;

  debuglevel=atoi(vs_config->getVariable("interpreter","debuglevel","0").c_str());
  bool start_game=XMLSupport::parse_bool(vs_config->getVariable("interpreter","startgame","true"));

  vi_counter=0;
  old_vi_counter=0;

  missionThread *main_thread=new missionThread;
  runtime.thread_nr=0;
  runtime.threads.push_back(main_thread);
  runtime.cur_thread=main_thread;

  director=NULL;

  msgcenter->add("game","all","parsing programmed mission");

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

  if(director==NULL){
    return;
  }


#ifdef HAVE_PYTHON
  Py_Initialize();
  PyRun_SimpleString("import test1");
  //  PyRun_SimpleString("director=VSdirector()");
#endif

  missionNode *initgame=director->script.scripts["initgame"];

  if(initgame==NULL){
    warning("initgame not found");
  }
  else{
    runtime.cur_thread->module_stack.push_back(director);
    runtime.cur_thread->classid_stack.push_back(0);

    varInst *vi=doScript(initgame,SCRIPT_RUN);
    deleteVarInst(vi);

    runtime.cur_thread->module_stack.pop_back();
    runtime.cur_thread->classid_stack.pop_back();
  }

  msgcenter->add("game","all","initialization of programmed missions done");

  if(debuglevel>=1 && start_game==false){
    while(true){
      DirectorLoop();
#ifndef _WIN32
      sleep(1);
#endif
    }
  }

}

void Mission::DirectorLoop(){

  if(director==NULL){
    return;
  }

#ifdef HAVE_PYTHON
  PyRun_SimpleString("test1.director.gameloop()");
#endif

  //  cout << "DIRECTOR LOOP" << endl;

  //  saveVariables(cout);

  missionNode *gameloop=director->script.scripts["gameloop"];

  if(gameloop==NULL){
    warning("no gameloop");
    return;
  }
  else{

    if(vi_counter!=old_vi_counter){
      char buf[200];
      sprintf(buf,"VI_COUNTER %d\n",vi_counter);
      debug(2,NULL,0,buf);
    }

    old_vi_counter=vi_counter;

    runtime.cur_thread->module_stack.push_back(director);
    runtime.cur_thread->classid_stack.push_back(0);

    varInst *vi=doScript(gameloop,SCRIPT_RUN);
    deleteVarInst(vi);

    runtime.cur_thread->module_stack.pop_back();
    runtime.cur_thread->classid_stack.pop_back();
 
    //    doModule(director,SCRIPT_RUN);
  }
}

void Mission::DirectorEnd(){
  if(director==NULL){
    return;
  }

  missionNode *endgame=director->script.scripts["endgame"];

  if(endgame==NULL){
    warning("endgame not found");
  }
  else{
    cout << "ENDGAME" << endl;

    runtime.cur_thread->module_stack.push_back(director);
    runtime.cur_thread->classid_stack.push_back(0);

    varInst *vi=doScript(endgame,SCRIPT_RUN);
    deleteVarInst(vi);

    runtime.cur_thread->module_stack.pop_back();
    runtime.cur_thread->classid_stack.pop_back();
  }


#ifdef WIN32
  var_out.open("c:\tmp\default-player.variables");
#else
  var_out.open("/tmp/default-player.variables");
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

void Mission::runScript(string modulename,string scriptname,uint classid){
  missionNode *module_node=runtime.modules[modulename];
  if(module_node==NULL){
    fatalError(NULL,SCRIPT_RUN,"module "+modulename+" not found");
    assert(0);
  }

  missionNode *script_node=module_node->script.scripts[scriptname];
  if(script_node==NULL){
    fatalError(NULL,SCRIPT_RUN,"can't run "+modulename+"."+scriptname);
    assert(0);
  }
  
  runtime.cur_thread->module_stack.push_back(module_node);
  runtime.cur_thread->classid_stack.push_back(classid);

  varInst *vi=doScript(script_node,SCRIPT_RUN);
  deleteVarInst(vi);

  runtime.cur_thread->classid_stack.pop_back();
  runtime.cur_thread->module_stack.pop_back();
}
