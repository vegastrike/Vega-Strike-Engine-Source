#include "cmd/unit.h"
#include "hashtable.h"
#include <float.h>
#include "vs_path.h"
#include <vector>
#include <string>
#include "config_xml.h"
#include "vs_globals.h"
#include "savegame.h"
using namespace std;
static std::string ForceStarSystem("") ;
static Vector PlayerLocation (FLT_MAX,FLT_MAX,FLT_MAX);
static std::string outputsavegame;
static std::string originalsystem;
Hashtable<int,SavedUnits,char[47]> savedunits;
void SetStarSystem (string sys) {
  ForceStarSystem = sys;
}
string GetStarSystem () {
  return ForceStarSystem;
}

void SetPlayerLocation (const Vector &v) {
  PlayerLocation =v;
}
Vector GetPlayerLocation () {
  return PlayerLocation;
}
void RemoveUnitFromSave (int address) {
  SavedUnits *tmp;
  if (NULL!=(tmp =savedunits.Get (address))) {
    savedunits.Delete (address);
    delete tmp;
  }
}
void AddUnitToSave (const char * filename, enum clsptr type, const char * faction, int address) {
  static string s = vs_config->getVariable ("physics","Drone","drone");
  if (0==strcmp (s.c_str(),filename)||type==ENHANCEMENTPTR) {
    RemoveUnitFromSave (address);
    savedunits.Put (address,new SavedUnits (filename,type,faction));
  }
}
vector <SavedUnits> ReadSavedUnits (FILE * fp) {
  vector <SavedUnits> su;
  int a;
  char unitname[1024];
  char factname[1024];
  while (3==fscanf (fp,"%d %s %s",&a,unitname,factname)) {
    su.push_back (SavedUnits (unitname,(clsptr)a,factname));
  }
  return su;
}
void WriteSavedUnit (FILE * fp, SavedUnits* su) {
  fprintf (fp,"\n%d %s %s",su->type, su->filename.c_str(),su->faction.c_str());
}
void WriteSaveGame (const char *systemname, const Vector &FP) {
  vector<SavedUnits *> myvec = savedunits.GetAll();
  if (outputsavegame.length()!=0) {
    printf ("Writing Save Game %s",outputsavegame.c_str());
    changehome();
    vschdir ("save");
    FILE * fp = fopen (outputsavegame.c_str(),"w");
    vscdup();
    returnfromhome();
    Vector FighterPos= PlayerLocation-FP;
    if (originalsystem!=systemname) {
      FighterPos=-FP;
    }
    fprintf (fp,"%s %f %f %f",systemname,FighterPos.i,FighterPos.j,FighterPos.k);
    while (myvec.empty()==false) {
      WriteSavedUnit (fp,myvec.back());
      delete myvec.back();
      myvec.pop_back();
    }
    fclose (fp);
  }
}
vector<SavedUnits> ParseSaveGame (const string filename, string &FSS, string originalstarsystem, Vector &PP, bool & shouldupdatepos) {
  vector <SavedUnits> mysav;
  shouldupdatepos=!(PlayerLocation.i==FLT_MAX||PlayerLocation.j==FLT_MAX||PlayerLocation.k==FLT_MAX);
  outputsavegame=filename;
  changehome();
  vschdir ("save");
  FILE * fp = NULL;
  if (filename.length())
	  fp = fopen (filename.c_str(),"r");
  vscdup();
  returnfromhome();
  if (fp) {
    char tmp[10000];
    Vector tmppos;
    if (4==fscanf (fp,"%s %f %f %f\n",tmp,&tmppos.i,&tmppos.j,&tmppos.k)) {
      if (ForceStarSystem.length()==0) 
	ForceStarSystem=string(tmp);
      if (PlayerLocation.i==FLT_MAX||PlayerLocation.j==FLT_MAX||PlayerLocation.k==FLT_MAX) {
	shouldupdatepos=true;
	PlayerLocation=tmppos;
      }
      mysav=ReadSavedUnits (fp);
    }
    fclose (fp);
  }
  if (PlayerLocation.i==FLT_MAX||PlayerLocation.j==FLT_MAX||PlayerLocation.k==FLT_MAX) {
    shouldupdatepos=false;
    PlayerLocation=PP;
  }else {
    PP = PlayerLocation;
    shouldupdatepos=true;
  }
  if (ForceStarSystem.length()==0) {
    ForceStarSystem = FSS;
    originalsystem=FSS;
  } else {
    originalsystem = ForceStarSystem;
    FSS = ForceStarSystem;
  }
  return mysav;
}

