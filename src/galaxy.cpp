#include "star_system.h"
#include "universe.h"
#include "galaxy_xml.h"
#include "galaxy_gen.h"
#include "vs_path.h"
#include "config_xml.h"
#include "vs_globals.h"
#include "xml_support.h"
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
  vector <string> jumps;
  int numjumps;
  int seed;
};
string getUniversePath () {
  static string datapath = vs_config->getVariable ("data","universe_path", "universe");
  char del[]= {'/','\0'};
  return datapath+string(del);
}

void MyLoadSystem (SystemInfo si) {

  generateStarSystem (getUniversePath(),si.seed,si.sector,si.name,si.filename,si.sunradius,si.numstars,si.numgas,si.numplanets,si.nummoons,si.nebulae,si.asteroids, si.numun1,si.numun2,si.faction,si.names,si.jumps);

}

SystemInfo GetSystemMin (Galaxy * galaxy) {
  SystemInfo si;
  si.sunradius=parse_float(galaxy->getVariable ("unknown_sector","min","sun_radius","5000"));
  si.numstars=parse_int(galaxy->getVariable ("unknown_sector","min","num_stars","1"));
  si.numgas=parse_int(galaxy->getVariable ("unknown_sector","min","num_gas_giants","0"));
  si.numplanets=parse_int(galaxy->getVariable ("unknown_sector","min","num_planets","1"));
  si.nummoons=parse_int(galaxy->getVariable ("unknown_sector","min","num_moons","1"));
  si.nebulae=parse_bool(galaxy->getVariable ("unknown_sector","min","nebulae","true"));
  si.asteroids=parse_bool(galaxy->getVariable ("unknown_sector","min","asteroids","true"));
  si.numun1=parse_int(galaxy->getVariable ("unknown_sector","min","num_natural_phenomena","0"));
  si.numun2=parse_int(galaxy->getVariable ("unknown_sector","min","num_starbases","0"));
  si.faction=galaxy->getVariable ("unknown_sector","min","faction","unknown");
  si.names=galaxy->getVariable ("unknown_sector","min","namelist","names.txt");
  si.seed=parse_int(galaxy->getVariable ("unknown_sector","min","data","0"));
  si.numjumps=parse_int(galaxy->getVariable ("unknown_sector","min","num_jump","2"));
  return si;
}
SystemInfo GetSystemMax (Galaxy * galaxy) {
  SystemInfo si;
  si.sunradius=parse_float(galaxy->getVariable ("unknown_sector","max","sun_radius","50000"));
  si.numstars=parse_int(galaxy->getVariable ("unknown_sector","max","num_stars","2"));
  si.numgas=parse_int(galaxy->getVariable ("unknown_sector","max","num_gas_giants","3"));
  si.numplanets=parse_int(galaxy->getVariable ("unknown_sector","max","num_planets","10"));
  si.nummoons=parse_int(galaxy->getVariable ("unknown_sector","max","num_moons","10"));
  si.nebulae=parse_bool(galaxy->getVariable ("unknown_sector","max","nebulae","true"));
  si.asteroids=parse_bool(galaxy->getVariable ("unknown_sector","max","asteroids","true"));
  si.numun1=parse_int(galaxy->getVariable ("unknown_sector","max","num_natural_phenomena","2"));
  si.numun2=parse_int(galaxy->getVariable ("unknown_sector","max","num_starbases","4"));
  si.faction=galaxy->getVariable ("unknown_sector","max","faction","unknown");
  si.names=galaxy->getVariable ("unknown_sector","max","namelist","names.txt");
  si.seed=parse_int(galaxy->getVariable ("unknown_sector","max","data","0"));
  si.numjumps=parse_int(galaxy->getVariable ("unknown_sector","max","num_jump","6"));
  return si;
}

static float fav (float in1, float in2) {
  return in1+ (in2-in1)*(float (rand()))/(((float)RAND_MAX));
}
//not including in2
static int rnd (int in1, int in2) {
  return (int)(in1+ (in2-in1)*(float (rand()))/(((float)RAND_MAX)+1));
}
//inclusive
static int iav (int in1, int in2) {
  return rnd (in1,in2+1);
}
SystemInfo AvgSystems (SystemInfo a, SystemInfo b) {
  SystemInfo si;

  si.sunradius=fav (a.sunradius,b.sunradius);
  si.numstars=iav (a.numstars,b.numstars);
  si.numgas=iav (a.numgas,b.numgas);
  si.numplanets=iav (a.numplanets,b.numplanets);
  si.nummoons=iav (a.nummoons,b.nummoons);
  si.nebulae=a.nebulae||b.nebulae;
  si.asteroids=a.asteroids||b.asteroids;
  si.numun1=iav (a.numun1,b.numun1);
  si.numun2=iav (a.numun2,b.numun2);
  si.faction=a.faction;
  si.names=a.names;
  si.seed=iav (a.seed,b.seed);
  si.numjumps= iav (a.numjumps,b.numjumps);
  return si;
}
extern vector <char *> ParseDestinations (const string &value);

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
  si.filename=MakeSharedStarSysPath(file);
  si.sunradius=parse_float(galaxy->getVariable (si.sector,si.name,"sun_radius",tostring(Ave.sunradius)));
  si.numstars=parse_int(galaxy->getVariable (si.sector,si.name,"num_stars",tostring(Ave.numstars)));
  si.numgas=parse_int(galaxy->getVariable (si.sector,si.name,"num_gas_giants",tostring(Ave.numgas)));
  si.numplanets=parse_int(galaxy->getVariable (si.sector,si.name,"num_planets",tostring(Ave.numplanets)));
  si.nummoons=parse_int(galaxy->getVariable (si.sector,si.name,"num_moons",tostring(Ave.nummoons)));
  si.nebulae=parse_bool(galaxy->getVariable (si.sector,si.name,"nebulae",tostring(Ave.nebulae)));
  si.asteroids=parse_bool(galaxy->getVariable (si.sector,si.name,"asteroids",tostring(Ave.asteroids)));
  si.numun1=parse_int(galaxy->getVariable (si.sector,si.name,"num_natural_phenomena",tostring(Ave.numun1)));
  si.numun2=parse_int(galaxy->getVariable (si.sector,si.name,"num_starbases",tostring(Ave.numun2)));
  si.faction=galaxy->getVariable (si.sector,si.name,"faction",Ave.faction);
  si.names=galaxy->getVariable (si.sector,si.name,"namelist",Ave.names);
  si.seed=parse_int(galaxy->getVariable (si.sector,si.name,"data",tostring(Ave.seed)));
  si.numjumps=parse_int(galaxy->getVariable (si.sector,si.name,"num_jump",tostring(Ave.numjumps)));
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
    if (entities.size()) {
      while ((int)si.jumps.size()<si.numjumps) {
	si.jumps.push_back (string("unknown_sector/")+entities[rnd(0,entities.size())]);
      }
    }
  }
  MyLoadSystem (si);
}


StarSystem * Universe::GenerateStarSystem (const char * file, const char * jumpback, Vector center) {
  int count=0;
  while (GetCorrectStarSysPath (file).length()==0) {
    MakeStarSystem(file, galaxy,RemoveDotSystem (jumpback),count);
    count++;
  }

  StarSystem * ss = new StarSystem (file,center);
  LoadStarSystem (ss);
  if (active_star_system.empty()) {
    pushActiveStarSystem (ss);
  } else {
    ss->SwapOut();
    activeStarSystem()->SwapIn();
  }
  return ss;

}
