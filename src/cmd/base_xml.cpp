#include <vector>
#include <string>
#include <python.h>
#include "python/python_class.h"
#include "base.h"
#include "base_util.h"
#include "vs_path.h"

PYTHON_BEGIN_MODULE(Base)
	Base.def (&BaseUtil::Room,"Room");
	Base.def (&BaseUtil::GetCurRoom,"GetCurRoom");
	Base.def (&BaseUtil::GetNumRoom,"GetNumRoom");
	Base.def (&BaseUtil::Comp,"Comp");
	Base.def (&BaseUtil::Launch,"Launch");
	Base.def (&BaseUtil::Link,"Link");
	Base.def (&BaseUtil::Python,"Python");
	Base.def (&BaseUtil::EraseLink,"EraseLink");
	Base.def (&BaseUtil::Ship,"Ship");
	Base.def (&BaseUtil::Texture,"Texture");
	Base.def (&BaseUtil::Message,"Message");
	Base.def (&BaseUtil::EraseObj,"EraseObj");
PYTHON_END_MODULE(Base)

void InitBase() {
	Python::reseterrors();
	PYTHON_INIT_MODULE(Base);
	Python::reseterrors();
}

void Base::Load(const char * filename,const char * time_of_day_hint) {
  const int chunk_size = 16384;
  std::string full_filename = string("bases/") + filename;
  std::string daynight_filename = full_filename + "_"+string(time_of_day_hint);
  full_filename+=BASE_EXTENSION;
  daynight_filename+=BASE_EXTENSION;
  std::string newfile=daynight_filename;
  cout << "Base::LoadXML " << full_filename << endl;
  FILE * inFile = fopen (daynight_filename.c_str(),"r");
  if (!inFile)
    newfile=full_filename;
    inFile = fopen (full_filename.c_str(), "r");
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
    assert(inFile);
	if (!inFile)
      return;
  }
  //now that we have a FILE * named inFile and a std::string named newfile we can finally begin the python
  const char *filnam=newfile.c_str();
  int length=strlen(filnam);
  char *pyfile=new char[length+1];
  strncpy(pyfile,filnam,length);
  pyfile[length]='\0';
  PyRun_SimpleFile(inFile,pyfile);
  fclose(inFile);
}
