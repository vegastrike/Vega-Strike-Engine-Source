#include <vector>
#include <string>
#include <Python.h>
#include "python/python_class.h"
#include "base.h"
#include "base_util.h"
#include "vs_path.h"
#include <boost/python/objects.hpp>

static boost::python::tuple GetRandomBarMessage () {
	gameMessage * last;
	int i=0;
	vector <std::string> who;
	vector <std::string> say;
	vector <std::string> sounds;
	string newmsg;
	string newsound;
	who.push_back ("bar");
	while ((last= mission->msgcenter->last(i++,who))!=NULL) {
		newmsg=last->message;
		newsound="";
		int first=newmsg.find_first_of("[");
		int last=newmsg.find_first_of("]");
		if (first!=string::npos&&(first+1)<newmsg.size()) {
			newsound=newmsg.substr(first+1,last-first-1);
			newmsg=newmsg.substr(0,first);
		}
		sounds.push_back(newsound);
		say.push_back(newmsg);
	}
	if (say.size()) {
		int index=rand()%say.size();
		return boost::python::tuple(say[index],sounds[index]);
	} else {
		return boost::python::tuple("","");
	}
}
PYTHON_BEGIN_MODULE(Base)
	Base.def (&BaseUtil::Room,"Room");
	Base.def (&BaseUtil::GetCurRoom,"GetCurRoom");
	Base.def (&BaseUtil::GetNumRoom,"GetNumRoom");
	Base.def (&BaseUtil::Comp,"Comp");
	Base.def (&BaseUtil::CompPython,"CompPython");
	Base.def (&BaseUtil::Launch,"Launch");
	Base.def (&BaseUtil::LaunchPython,"LaunchPython");
	Base.def (&BaseUtil::Link,"Link");
	Base.def (&BaseUtil::LinkPython,"LinkPython");
	Base.def (&BaseUtil::Python,"Python");
	Base.def (&BaseUtil::EraseLink,"EraseLink");
	Base.def (&BaseUtil::Ship,"Ship");
	Base.def (&BaseUtil::Texture,"Texture");
	Base.def (&BaseUtil::Message,"Message");
	Base.def (&BaseUtil::EnqueueMessage,"EnqueueMessage");
	Base.def (&BaseUtil::EraseObj,"EraseObj");
	Base.def (&GetRandomBarMessage,"GetRandomBarMessage");
PYTHON_END_MODULE(Base)

void InitBase() {
	Python::reseterrors();
	PYTHON_INIT_MODULE(Base);
	Python::reseterrors();
}
static FILE * withAndWithout (std::string filename, std::string time_of_day_hint) {
  string with (filename+"_"+time_of_day_hint+BASE_EXTENSION);
  FILE * fp = fopen (with.c_str(),"r");
  if (!fp) {
    string without (filename+BASE_EXTENSION);
    fp = fopen (without.c_str(),"r");
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
void Base::Load(const char * filename,const char * time_of_day_hint, const char * faction) {
#if 0

  std::string full_filename = string("bases/") + filename;
  std::string daynight_filename = full_filename + "_"+string(time_of_day_hint);
  full_filename+=BASE_EXTENSION;
  daynight_filename+=BASE_EXTENSION;
  std::string newfile=daynight_filename;
  cout << "Base::LoadXML " << full_filename << endl;
  FILE * inFile = fopen (daynight_filename.c_str(),"r");
  if (!inFile) {
    newfile=full_filename;
    inFile = fopen (full_filename.c_str(), "r");
  }
  if(!inFile) {
    Unit *baseun=this->baseun.GetUnit();
    if (baseun) {
      if (baseun->isUnit()==PLANETPTR){
	daynight_filename = string("bases/planet_")+time_of_day_hint+string(BASE_EXTENSION);
	inFile = fopen (daynight_filename.c_str(),"r");
	newfile=daynight_filename;
	if (!inFile) {
      newfile="bases/planet"BASE_EXTENSION;
	  inFile=fopen(newfile.c_str(),"r");
	}
      }else{ 
	daynight_filename = string("bases/unit_")+time_of_day_hint+string(BASE_EXTENSION);
	inFile = fopen (daynight_filename.c_str(),"r");
	newfile=daynight_filename;
	if (!inFile) {
	  newfile="bases/unit"BASE_EXTENSION;
	  inFile=fopen(newfile.c_str(),"r");
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
  fclose(inFile);
}
