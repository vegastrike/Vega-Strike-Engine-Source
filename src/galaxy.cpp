#include "star_system_generic.h"
#include "cmd/script/mission.h"
#include "universe_generic.h"
#include "galaxy_xml.h"
#include "galaxy_gen.h"
#include "vsfilesystem.h"
#include "configxml.h"
#include "vs_globals.h"
#include "xml_support.h"
#include "lin_time.h"
#include "star_system_generic.h"
using namespace XMLSupport;
using namespace GalaxyXML;
string RemoveDotSystem (const char *input) {
  int sl = strlen (input);
  if (sl ==0) {
    return string ("");
  }
  char * tmp =strdup (input);
  char * ptr = tmp+sl-1;;
  while (ptr>tmp) {
    if (*ptr=='.') {
      if (0==strcmp (ptr,".system")) {
	*ptr='\0';
	
	char * ttmp=tmp;
	tmp = strdup (RemoveDotSystem(tmp).c_str());
	ptr = (tmp+(ptr-ttmp));
	free (ttmp);

	break;
      }else {
	break;
      }      
    }
    ptr--;
  }
  string retval(tmp);
  free (tmp);
  return retval;
}


struct SystemInfo {
  string sector;
  string name;
  string filename;
  float sunradius;
  float compactness;
  int numstars;
  int numgas;
  int numplanets;
  int nummoons;
  bool nebulae;
  bool asteroids;
  int numun1;
  int numun2;
  string faction;
  string names;
  string stars;
  string planets;
  string gasgiants;
  string moons;
  string smallun;
  string nebulaelist;
  string asteroidslist;
  string ringlist;
  string backgrounds;
  vector <string> jumps;
  int numjumps;
  int seed;
  bool force;
};
string getUniversePath () {
  static string datapath = vs_config->getVariable ("data","universe_path", "universe");
  char del[]= {'/','\0'};
  return datapath+string(del);
}

void MyLoadSystem (SystemInfo si) {

  generateStarSystem (getUniversePath(),si.seed,si.sector,si.name,si.filename,si.sunradius,si.compactness,si.numstars,si.numgas,si.numplanets,si.nummoons,si.nebulae,si.asteroids, si.numun1,si.numun2,si.faction,si.jumps,si.names,si.stars,si.planets,si.gasgiants,si.moons,si.smallun,si.nebulaelist,si.asteroidslist,si.ringlist,si.backgrounds,si.force);

}
string getVarEitherSectionOrSub (Galaxy *galaxy, string section, string subsection, string variable, string defaultst) {
  string d3fault = galaxy->getVariable (section,subsection,variable,
										galaxy->getVariable (section,variable,defaultst));
  if (d3fault.length()==0)
	  d3fault = galaxy->getVariable(section,variable,defaultst);
  if (d3fault.length()==0)
	  return defaultst;
  return d3fault;//this code will prevent the empty planet lists from interfering
}

void ClampIt (float & prop, float min, float max) {
  if (prop<min) {
    prop = min;
  }
  if (prop>max) {
    prop=max;
  }
}
void ClampIt (int & prop, int min, int max) {
  if (prop<min) {
    prop = min;
  }
  if (prop>max) {
    prop=max;
  }
}
static void clampSystem (SystemInfo & si, const SystemInfo & min, const SystemInfo &max) {
  ClampIt (si.sunradius,min.sunradius,max.sunradius);
  ClampIt (si.compactness,min.compactness,max.compactness);
  ClampIt (si.numstars,min.numstars,max.numstars);
  ClampIt (si.numplanets,min.numplanets,max.numplanets);
  ClampIt (si.nummoons,min.nummoons,max.nummoons);
  ClampIt (si.numgas,min.numgas,max.numgas);
  ClampIt (si.numun1,min.numun1,max.numun1);
  ClampIt (si.numun2,min.numun2,max.numun2);
}

SystemInfo GetSystemXProp (Galaxy * galaxy, std::string sector, std::string minmax) {
  SystemInfo si;
  si.sunradius=parse_float(getVarEitherSectionOrSub(galaxy,sector,minmax,"sun_radius","5000"));
  si.compactness=parse_float(getVarEitherSectionOrSub(galaxy,sector,minmax,"compactness","1.5"));
  si.numstars=parse_int(getVarEitherSectionOrSub(galaxy,sector,minmax,"num_stars","1"));
  si.numgas=parse_int(getVarEitherSectionOrSub(galaxy,sector,minmax,"num_gas_giants","0"));
  si.numplanets=parse_int(getVarEitherSectionOrSub(galaxy,sector,minmax,"num_planets","1"));
  si.nummoons=parse_int(getVarEitherSectionOrSub(galaxy,sector,minmax,"num_moons","1"));
  si.nebulae=parse_bool(getVarEitherSectionOrSub(galaxy,sector,minmax,"nebulae","true"));
  si.asteroids=parse_bool(getVarEitherSectionOrSub(galaxy,sector,minmax,"asteroids","true"));
  si.numun1=parse_int(getVarEitherSectionOrSub(galaxy,sector,minmax,"num_natural_phenomena","0"));
  si.numun2=parse_int(getVarEitherSectionOrSub(galaxy,sector,minmax,"num_starbases","0"));
  si.faction=getVarEitherSectionOrSub(galaxy,sector,minmax,"faction","unknown");
  
  si.seed=parse_int(getVarEitherSectionOrSub(galaxy,sector,minmax,"data","0"));
  si.numjumps=parse_int(getVarEitherSectionOrSub(galaxy,sector,minmax,"num_jump","2"));
  si.names=getVarEitherSectionOrSub(galaxy,sector,minmax,"namelist","names.txt");  
  si.stars=getVarEitherSectionOrSub(galaxy,sector,minmax,"starlist","stars.txt");  
  si.gasgiants=getVarEitherSectionOrSub(galaxy,sector,minmax,"gasgiantlist","gas_giants.txt");  
  si.planets=getVarEitherSectionOrSub(galaxy,sector,minmax,"planetlist","planets.txt");  
  si.moons=getVarEitherSectionOrSub(galaxy,sector,minmax,"moonlist","moons.txt");  
  si.smallun=getVarEitherSectionOrSub(galaxy,sector,minmax,"unitlist","smallunits.txt");  
  si.asteroidslist=getVarEitherSectionOrSub(galaxy,sector,minmax,"asteroidlist","asteroids.txt");  
  si.ringlist=getVarEitherSectionOrSub(galaxy,sector,minmax,"ringlist","rings.txt");  
  si.nebulaelist=getVarEitherSectionOrSub(galaxy,sector,minmax,"nebulalist","nebulae.txt");  
  si.backgrounds=getVarEitherSectionOrSub(galaxy,sector,minmax,"backgroundlist","background.txt");  
  si.force=parse_bool (getVarEitherSectionOrSub (galaxy,sector,minmax,"force","false"));
  return si;
}
SystemInfo GetSystemMin (Galaxy * galaxy) {
  return GetSystemXProp (galaxy, "unknown_sector","min");
}

SystemInfo GetSystemMax (Galaxy * galaxy) {
  return GetSystemXProp (galaxy,"unknown_sector","max");
}

static float av01 () {
  return (float (rand()))/((((float)RAND_MAX)+1));
}
static float sqav01() {
  float tmp =av01();
  return tmp*tmp;
}
static float fav (float in1, float in2) {
  return in1+ (in2-in1)*av01();
}
static float fsqav(float in1, float in2) {
  return sqav01()*(in2-in1)+in1;
}
//not including in2
static int rnd (int in1, int in2) {
  return (int)(in1+ (in2-in1)*(float (rand()))/(((float)RAND_MAX)+1));
}
//inclusive
static int iav (int in1, int in2) {
  return rnd (in1,in2+1);
}
static int isqav (int in1, int in2) {
  return (int)(in1+ (in2+1-in1)*sqav01());
}

SystemInfo AvgSystems (SystemInfo a, SystemInfo b) {
  SystemInfo si;
  si = a;//copy all stuff that cna't be averaged
  si.sunradius=fsqav (a.sunradius,b.sunradius);
  si.compactness=fsqav (a.compactness,b.compactness);
  si.numstars=isqav (a.numstars,b.numstars);
  si.numgas=isqav (a.numgas,b.numgas);
  si.numplanets=isqav (a.numplanets,b.numplanets);
  si.nummoons=isqav (a.nummoons,b.nummoons);
  si.nebulae=a.nebulae||b.nebulae;
  si.asteroids=a.asteroids||b.asteroids;
  si.numun1=isqav (a.numun1,b.numun1);
  si.numun2=isqav (a.numun2,b.numun2);
  si.seed=iav (a.seed,b.seed);
  si.numjumps= isqav (a.numjumps,b.numjumps);
  si.force = a.force||b.force;
  return si;
}
vector <char *> ParseDestinations (const string &value) {
  vector <char *> tmp;
  int i;
  int j;
  int k;
  for (j=0;value[j]!=0;){
    for (i=0;value[j]!=' '&&value[j]!='\0';i++,j++) {
    }
    tmp.push_back(new char [i+1]);//leak...somewhere!
    for (k=0;k<i;k++) {
      tmp[tmp.size()-1][k]=value[k+j-i];
    }
    tmp[tmp.size()-1][i]='\0';
    if (value[j]!=0)
      j++;
  }
  return tmp;
}


vector <string> ParseStringyDestinations (vector <char *> v) {
  vector <string> retval;
  while (!v.empty()) {
    retval.push_back (string (v.back()));
    delete [] v.back();
    v.pop_back();
  }
  return retval;
}

void MakeStarSystem (string file, Galaxy *galaxy, string origin, int forcerandom) {


  SystemInfo Ave  (AvgSystems (GetSystemMin (galaxy),GetSystemMax (galaxy)));
  SystemInfo si;
  si.sector =getStarSystemSector (file); 
  si.name =RemoveDotSystem (getStarSystemName (file).c_str()); 
  si.filename=file;
  si.sunradius=parse_float(getVarEitherSectionOrSub(galaxy,si.sector,si.name,"sun_radius",tostring(Ave.sunradius)));
  si.compactness=parse_float(getVarEitherSectionOrSub(galaxy,si.sector,si.name,"compactness",tostring(Ave.compactness)));
  si.numstars=parse_int(getVarEitherSectionOrSub(galaxy,si.sector,si.name,"num_stars",tostring(Ave.numstars)));
  si.numgas=parse_int(getVarEitherSectionOrSub(galaxy,si.sector,si.name,"num_gas_giants",tostring(Ave.numgas)));
  si.numplanets=parse_int(getVarEitherSectionOrSub(galaxy,si.sector,si.name,"num_planets",tostring(Ave.numplanets)));
  si.nummoons=parse_int(getVarEitherSectionOrSub(galaxy,si.sector,si.name,"num_moons",tostring(Ave.nummoons)));
  si.nebulae=parse_bool(getVarEitherSectionOrSub(galaxy,si.sector,si.name,"nebulae",tostring(Ave.nebulae)));
  si.asteroids=parse_bool(getVarEitherSectionOrSub(galaxy,si.sector,si.name,"asteroids",tostring(Ave.asteroids)));
  si.numun1=parse_int(getVarEitherSectionOrSub(galaxy,si.sector,si.name,"num_natural_phenomena",tostring(Ave.numun1)));
  si.numun2=parse_int(getVarEitherSectionOrSub(galaxy,si.sector,si.name,"num_starbases",tostring(Ave.numun2)));
  si.faction=getVarEitherSectionOrSub(galaxy,si.sector,si.name,"faction",Ave.faction);
  si.seed=parse_int(getVarEitherSectionOrSub(galaxy,si.sector,si.name,"data",tostring(Ave.seed)));
  si.numjumps=parse_int(getVarEitherSectionOrSub(galaxy,si.sector,si.name,"num_jump",tostring(Ave.numjumps)));
  si.names=getVarEitherSectionOrSub(galaxy,si.sector,si.name,"namelist", Ave.names);
  si.stars=getVarEitherSectionOrSub(galaxy,si.sector,si.name,"starlist",Ave.stars);  
  si.gasgiants=getVarEitherSectionOrSub(galaxy,si.sector,si.name,"gasgiantlist",Ave.gasgiants);  
  si.planets=getVarEitherSectionOrSub(galaxy,si.sector,si.name,"planetlist",Ave.planets);  
  si.moons=getVarEitherSectionOrSub(galaxy,si.sector,si.name,"moonlist",Ave.moons);  
  si.smallun=getVarEitherSectionOrSub(galaxy,si.sector,si.name,"unitlist",Ave.smallun);  
  si.asteroidslist=getVarEitherSectionOrSub(galaxy,si.sector,si.name,"asteroidlist",Ave.asteroidslist);  
  si.ringlist=getVarEitherSectionOrSub(galaxy,si.sector,si.ringlist,"ringlist", Ave.ringlist);
  si.nebulaelist=getVarEitherSectionOrSub(galaxy,si.sector,si.name,"nebulalist",Ave.nebulaelist);  
  si.backgrounds=getVarEitherSectionOrSub(galaxy,si.sector,si.name,"backgroundlist",Ave.backgrounds);  
  si.force=parse_bool (getVarEitherSectionOrSub(galaxy,si.sector,si.name,"force",Ave.force?"true":"false"));
  
static bool always_force = XMLSupport::parse_float(vs_config->getVariable("galaxy","PushValuesToMean","true"));
  if (always_force)
	  si.force=true;
  string dest = galaxy->getVariable (si.sector,si.name,"jumps","");
  if (dest.length()) {
    si.numjumps=0;
    si.jumps = ParseStringyDestinations (ParseDestinations (dest));
  }
  bool canret=origin.length()==0;
  for (unsigned int i=0;i<si.jumps.size();i++) {
    if (si.jumps[i]==origin) {
      canret=true;
      break;
    }
  }
  if (!canret) {
    si.jumps.push_back (origin);
  }
  if ((int)si.jumps.size()<(int)si.numjumps) {
    vector <string> entities;
    readnames (entities, (getUniversePath()+si.names).c_str());
    while ((int)si.jumps.size()<si.numjumps) {
	si.jumps.push_back (si.sector+string("/")+galaxy->getRandSystem (si.sector,"celeste"));
    }
    
  }
  if (!si.force) {
    clampSystem (si,GetSystemXProp (galaxy,"unknown_sector","minlimit"),GetSystemXProp (galaxy,"unknown_sector","maxlimit"));
  }
  MyLoadSystem (si);
}

std::string Universe::getGalaxyProperty (const std::string & sys, const std::string & prop) {
  string sector = getStarSystemSector (sys);
  string name = RemoveDotSystem (getStarSystemName (sys).c_str());
  return galaxy->getVariable (sector,name,prop,galaxy->getVariable (sector,prop,galaxy->getVariable ("unknown_sector","min",prop,"")));
}
std::string Universe::getGalaxyPropertyDefault (const std::string & sys, const std::string & prop, const std::string def) {
  string sector = getStarSystemSector (sys);
  string name = RemoveDotSystem (getStarSystemName (sys).c_str());
  return galaxy->getVariable (sector,name,prop,def);
}
vector <std::string> Universe::getAdjacentStarSystems (const std::string &file) {
  string sector =getStarSystemSector (file); 
  string name =RemoveDotSystem (getStarSystemName (file).c_str()); 
  return ParseStringyDestinations (ParseDestinations (galaxy->getVariable (sector,name,"jumps","")));  
}
