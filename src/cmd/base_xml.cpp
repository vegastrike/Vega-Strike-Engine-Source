#include <vector>
#include <string>
#include <math.h>
#include <Python.h>
#include "python/python_class.h"
#include "base.h"
#include "base_util.h"
#include "vsfilesystem.h"
#ifdef USE_BOOST_129
#include <boost/python/object.hpp>
#else
#include <boost/python/objects.hpp>
#endif

static FILE * withAndWithout (std::string filename, std::string time_of_day_hint) {
  string with (filename+"_"+time_of_day_hint+BASE_EXTENSION);
  FILE * fp = VSFileSystem::vs_open (with.c_str(),"r");
  if (!fp) {
    string without (filename+BASE_EXTENSION);
    fp = VSFileSystem::vs_open (without.c_str(),"r");
  }
  return fp;
}
static FILE * getFullFile (std::string filename, std::string time_of_day_hint,std::string faction) {
  FILE * fp = withAndWithout (filename+"_"+faction,time_of_day_hint);
  if (!fp) {
    fp = withAndWithout (filename,time_of_day_hint);
  }
  return fp;
}
void BaseInterface::Load(const char * filename,const char * time_of_day_hint, const char * faction) {
#if 0

  std::string full_filename = string("bases/") + filename;
  std::string daynight_filename = full_filename + "_"+string(time_of_day_hint);
  full_filename+=BASE_EXTENSION;
  daynight_filename+=BASE_EXTENSION;
  std::string newfile=daynight_filename;
  cout << "BaseInterface::LoadXML " << full_filename << endl;
  FILE * inFile = VSFileSystem::vs_open (daynight_filename.c_str(),"r");
  if (!inFile) {
    newfile=full_filename;
    inFile = VSFileSystem::vs_open (full_filename.c_str(), "r");
  }
  if(!inFile) {
    Unit *baseun=this->baseun.GetUnit();
    if (baseun) {
      if (baseun->isUnit()==PLANETPTR){
	daynight_filename = string("bases/planet_")+time_of_day_hint+string(BASE_EXTENSION);
	inFile = VSFileSystem::vs_open (daynight_filename.c_str(),"r");
	newfile=daynight_filename;
	if (!inFile) {
      newfile="bases/planet"BASE_EXTENSION;
	  inFile=VSFileSystem::vs_open(newfile.c_str(),"r");
	}
      }else{ 
	daynight_filename = string("bases/unit_")+time_of_day_hint+string(BASE_EXTENSION);
	inFile = VSFileSystem::vs_open (daynight_filename.c_str(),"r");
	newfile=daynight_filename;
	if (!inFile) {
	  newfile="bases/unit"BASE_EXTENSION;
	  inFile=VSFileSystem::vs_open(newfile.c_str(),"r");
	}
      }
    }
	if (!inFile)
      return;
  }
#else
  FILE * inFile = getFullFile (string("bases/")+filename,time_of_day_hint,faction);
  if (!inFile){
    bool planet=false;
    Unit *baseun=this->baseun.GetUnit();
    if (baseun) {
      planet = (baseun->isUnit()==PLANETPTR);
    }
    string basestring ("bases/unit");
    if (planet) {
      basestring = "bases/planet";
    }
    inFile = getFullFile (basestring,time_of_day_hint,faction);
    if (!inFile)
      return;
  }
#endif
  //now that we have a FILE * named inFile and a std::string named newfile we can finally begin the python
  string compilefile = string(filename)+time_of_day_hint+string(faction)+BASE_EXTENSION;
  char *pyfile=strdup(compilefile.c_str());
  Python::reseterrors();
  PyRun_SimpleFile(inFile,pyfile);
  Python::reseterrors();
  free (pyfile);
  VSFileSystem::vs_close(inFile);
}
