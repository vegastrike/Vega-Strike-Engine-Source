#include <stdio.h>
#include <fcntl.h>
#include <algorithm>
#include "universe_generic.h"
#include "galaxy_xml.h"
//#include "config_xml.h"
#include "cmd/unit_generic.h"
#include "vs_globals.h"
#include "xml_support.h"

using namespace std;
void SortStarSystems (std::vector <StarSystem *> &ss, StarSystem * drawn) {
  if ((*ss.begin())==drawn) {
    return;
  }
  vector<StarSystem*>::iterator drw = std::find (ss.begin(),ss.end(),drawn);
  if (drw!=ss.end()) {
    StarSystem * tmp=drawn;
    vector<StarSystem*>::iterator i=ss.begin();
    while (i<=drw) {
      StarSystem * t=*i;
      *i=tmp;
      tmp = t;
      i++;
    }
  }
}
void Universe::Init()
{
	// No need to load weapons on server side
	//LoadWeapons("weapon_list.xml");
	LoadFactionXML("factions.xml");	
	//this->galaxy = new GalaxyXML::Galaxy (galaxy);

	script_system=NULL;
}
Universe::Universe(int argc, char** argv, const char * galaxy)
{
	this->Init();
}
Universe::Universe()
{
	script_system=NULL;
}

Universe::~Universe()
{
  unsigned int i;
  for (i=0;i<factions.size();i++) {
    delete factions[i];
  }
  for (i=0;i<cockpit.size();i++) {
    delete cockpit[i];
  }
  factions.clear();
  cockpit.clear();
}

void Universe::LoadStarSystem(StarSystem * s) {
  star_system.push_back (s);
  SortStarSystems(star_system,s);//dont' want instadie
}
bool Universe::StillExists (StarSystem * s) {
  return std::find (star_system.begin(),star_system.end(),s)!=star_system.end();
}

void Universe::UnloadStarSystem (StarSystem * s) {
  //not sure what to do here? serialize?
}
StarSystem * Universe::Init (string systemfile, const Vector & centr,const string planetname) {
  string fullname=systemfile+".system";
  return GenerateStarSystem((char *)fullname.c_str(),"",centr);
}

extern void micro_sleep (unsigned int howmuch);
extern int getmicrosleep ();

StarSystem *Universe::getStarSystem(string name){

  vector<StarSystem*>::iterator iter;

  for(iter = star_system.begin();iter!=star_system.end();iter++){
    StarSystem *ss=*iter;
    if(ss->getName()==name){
      return ss;
    }
  }

  return NULL;
}
