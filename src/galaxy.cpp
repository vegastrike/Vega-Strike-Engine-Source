#include "star_system.h"
#include "cmd/script/mission.h"
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
  string backgrounds;
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

  generateStarSystem (getUniversePath(),si.seed,si.sector,si.name,si.filename,si.sunradius,si.compactness,si.numstars,si.numgas,si.numplanets,si.nummoons,si.nebulae,si.asteroids, si.numun1,si.numun2,si.faction,si.jumps,si.names,si.stars,si.planets,si.gasgiants,si.moons,si.smallun,si.nebulaelist,si.asteroidslist,si.backgrounds);

}
string getVarEitherSectionOrSub (Galaxy *galaxy, string section, string subsection, string variable, string defaultst) {
  return galaxy->getVariable (section,subsection,variable,
			      galaxy->getVariable (section,variable,defaultst));
  
}

SystemInfo GetSystemMin (Galaxy * galaxy) {
  SystemInfo si;
  si.sunradius=parse_float(getVarEitherSectionOrSub(galaxy,"unknown_sector","min","sun_radius","5000"));
  si.compactness=parse_float(getVarEitherSectionOrSub(galaxy,"unknown_sector","min","compactness","1.5"));
  si.numstars=parse_int(getVarEitherSectionOrSub(galaxy,"unknown_sector","min","num_stars","1"));
  si.numgas=parse_int(getVarEitherSectionOrSub(galaxy,"unknown_sector","min","num_gas_giants","0"));
  si.numplanets=parse_int(getVarEitherSectionOrSub(galaxy,"unknown_sector","min","num_planets","1"));
  si.nummoons=parse_int(getVarEitherSectionOrSub(galaxy,"unknown_sector","min","num_moons","1"));
  si.nebulae=parse_bool(getVarEitherSectionOrSub(galaxy,"unknown_sector","min","nebulae","true"));
  si.asteroids=parse_bool(getVarEitherSectionOrSub(galaxy,"unknown_sector","min","asteroids","true"));
  si.numun1=parse_int(getVarEitherSectionOrSub(galaxy,"unknown_sector","min","num_natural_phenomena","0"));
  si.numun2=parse_int(getVarEitherSectionOrSub(galaxy,"unknown_sector","min","num_starbases","0"));
  si.faction=getVarEitherSectionOrSub(galaxy,"unknown_sector","min","faction","unknown");
  
  si.seed=parse_int(getVarEitherSectionOrSub(galaxy,"unknown_sector","min","data","0"));
  si.numjumps=parse_int(getVarEitherSectionOrSub(galaxy,"unknown_sector","min","num_jump","2"));
  si.names=getVarEitherSectionOrSub(galaxy,"unknown_sector","min","namelist","names.txt");  
  si.stars=getVarEitherSectionOrSub(galaxy,"unknown_sector","min","starlist","stars.txt");  
  si.gasgiants=getVarEitherSectionOrSub(galaxy,"unknown_sector","min","gasgiantlist","gas_giants.txt");  
  si.planets=getVarEitherSectionOrSub(galaxy,"unknown_sector","min","planetlist","planets.txt");  
  si.moons=getVarEitherSectionOrSub(galaxy,"unknown_sector","min","moonlist","moons.txt");  
  si.smallun=getVarEitherSectionOrSub(galaxy,"unknown_sector","min","unitlist","smallunits.txt");  
  si.asteroidslist=getVarEitherSectionOrSub(galaxy,"unknown_sector","min","asteroidlist","asteroids.txt");  
  si.nebulaelist=getVarEitherSectionOrSub(galaxy,"unknown_sector","min","nebulalist","nebulae.txt");  
  si.backgrounds=getVarEitherSectionOrSub(galaxy,"unknown_sector","min","backgroundlist","background.txt");  

  return si;
}
SystemInfo GetSystemMax (Galaxy * galaxy) {
  SystemInfo si;
  si.sunradius=parse_float(getVarEitherSectionOrSub(galaxy,"unknown_sector","max","sun_radius","60000"));
  si.compactness =parse_float(getVarEitherSectionOrSub(galaxy,"unknown_sector","max","compactness","2.4"));
  si.numstars=parse_int(getVarEitherSectionOrSub(galaxy,"unknown_sector","max","num_stars","2"));
  si.numgas=parse_int(getVarEitherSectionOrSub(galaxy,"unknown_sector","max","num_gas_giants","3"));
  si.numplanets=parse_int(getVarEitherSectionOrSub(galaxy,"unknown_sector","max","num_planets","10"));
  si.nummoons=parse_int(getVarEitherSectionOrSub(galaxy,"unknown_sector","max","num_moons","10"));
  si.nebulae=parse_bool(getVarEitherSectionOrSub(galaxy,"unknown_sector","max","nebulae","true"));
  si.asteroids=parse_bool(getVarEitherSectionOrSub(galaxy,"unknown_sector","max","asteroids","true"));
  si.numun1=parse_int(getVarEitherSectionOrSub(galaxy,"unknown_sector","max","num_natural_phenomena","2"));
  si.numun2=parse_int(getVarEitherSectionOrSub(galaxy,"unknown_sector","max","num_starbases","4"));
  si.faction=getVarEitherSectionOrSub(galaxy,"unknown_sector","max","faction","unknown");
  si.names=getVarEitherSectionOrSub(galaxy,"unknown_sector","max","namelist","names.txt");
  si.seed=parse_int(getVarEitherSectionOrSub(galaxy,"unknown_sector","max","data","0"));
  si.numjumps=parse_int(getVarEitherSectionOrSub(galaxy,"unknown_sector","max","num_jump","6"));
  si.stars=getVarEitherSectionOrSub(galaxy,"unknown_sector","max","starlist","stars.txt");  
  si.gasgiants=getVarEitherSectionOrSub(galaxy,"unknown_sector","max","gasgiantlist","gas_giants.txt");  
  si.planets=getVarEitherSectionOrSub(galaxy,"unknown_sector","max","planetlist","planets.txt");  
  si.moons=getVarEitherSectionOrSub(galaxy,"unknown_sector","max","moonlist","moons.txt");  
  si.smallun=getVarEitherSectionOrSub(galaxy,"unknown_sector","max","unitlist","smallunits.txt");  
  si.asteroidslist=getVarEitherSectionOrSub(galaxy,"unknown_sector","max","asteroidlist","asteroids.txt");  
  si.nebulaelist=getVarEitherSectionOrSub(galaxy,"unknown_sector","max","nebulalist","nebulae.txt");  
  si.backgrounds=getVarEitherSectionOrSub(galaxy,"unknown_sector","max","backgroundlist","background.txt");  

  return si;
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
extern void SetStarSystemLoading (bool value);

void MakeStarSystem (string file, Galaxy *galaxy, string origin, int forcerandom) {


  SystemInfo Ave  (AvgSystems (GetSystemMin (galaxy),GetSystemMax (galaxy)));
  SystemInfo si;
  si.sector =getStarSystemSector (file); 
  si.name =RemoveDotSystem (getStarSystemName (file).c_str()); 
  si.filename=MakeSharedStarSysPath(file);
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
  si.nebulaelist=getVarEitherSectionOrSub(galaxy,si.sector,si.name,"nebulalist",Ave.nebulaelist);  
  si.backgrounds=getVarEitherSectionOrSub(galaxy,si.sector,si.name,"backgroundlist",Ave.backgrounds);  


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
#ifdef _WIN32
#include <windows.h>
static HWND hWnd;
static HINSTANCE hInst;								// current instance
LRESULT CALLBACK DLOG_start(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
	case WM_INITDIALOG:
		return TRUE;
	case WM_CLOSE:
		DestroyWindow(hWnd);
		return TRUE;
	case WM_DESTROY:
		return FALSE;
	}
    return FALSE;
}
#include "../resource.h"


volatile HANDLE hMutex; // Global hMutex Object
DWORD WINAPI DrawStartupDialog(
  LPVOID lpParameter   // thread data
  ) {
	int dumbi;
	MSG msg;

	hWnd=CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_START),NULL, (DLGPROC)DLOG_start, 0);
	ShowWindow(hWnd, SW_SHOW);
	for (dumbi=0;dumbi<6;dumbi++) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) {

			if(GetMessage(&msg, NULL, 0, 0)) {
				DispatchMessage(&msg);
			} else {
				break;
			}
		}
		Sleep(100);
	}
	WaitForSingleObject(hMutex,INFINITE); // wait for ownership
	DestroyWindow(hWnd);
	return 0;
}
#endif


StarSystem * Universe::GenerateStarSystem (const char * file, const char * jumpback, Vector center) {
  static bool firsttime=true;
  if (!firsttime) {
#ifdef _WIN32
  hMutex=CreateMutex(NULL,FALSE,NULL); // nameless mutex object
  WaitForSingleObject(hMutex,INFINITE); // wait for ownership + print
  DWORD id;
  HANDLE hThr=CreateThread(NULL,  // pointer to security attributes
		0,                         // initial thread stack size
	    DrawStartupDialog,     // pointer to thread function
        NULL,                        // argument for new thread
        0,                     // creation flags
        &id                         // pointer to receive thread ID
         );
//	DialogBox (hInst,(LPCTSTR)IDD_START,hWnd,(DLGPROC)DLOG_start);
#endif
  }
  int count=0;
  SetStarSystemLoading (true);
  while (GetCorrectStarSysPath (file).length()==0) {
    MakeStarSystem(file, galaxy,RemoveDotSystem (jumpback),count);
    count++;
  }

  StarSystem * ss = new StarSystem (file,center);

  LoadStarSystem (ss);
  pushActiveStarSystem(ss);
  // notify the director that a new system is loaded (gotta have at least one active star system)
  StarSystem *old_script_system=script_system;

  script_system=ss;
  mission->DirectorStartStarSystem(ss);

  script_system=old_script_system;
  popActiveStarSystem();
  if (active_star_system.empty()) {
    pushActiveStarSystem (ss);
  } else {
    ss->SwapOut();
    activeStarSystem()->SwapIn();
  }
  if (firsttime) {
  	firsttime=false;
  }else {
#ifdef _WIN32
    ReleaseMutex(hMutex);
#endif
  }
  SetStarSystemLoading (false);
  return ss;

}
