#include "cmd/unit.h"
#include "hashtable.h"
#include <float.h>
#include "vs_path.h"
#include <vector>
#include <string>
#include "config_xml.h"
#include "vs_globals.h"
#include "config_xml.h"
#include "savegame.h"
#include <algorithm>
using namespace std;
SaveGame::SaveGame(const std::string &pilot) {
  callsign=pilot;
  ForceStarSystem=string("");
  PlayerLocation.Set(FLT_MAX, FLT_MAX,FLT_MAX);
}

void SaveGame::SetStarSystem (string sys) {
  ForceStarSystem = sys;
}
string SaveGame::GetStarSystem () {
  return ForceStarSystem;
}

void SaveGame::SetPlayerLocation (const Vector &v) {
  PlayerLocation =v;
}
Vector SaveGame::GetPlayerLocation () {
  return PlayerLocation;
}

Hashtable<int,SavedUnits,char[47]> SaveGame::savedunits;
void SaveGame::RemoveUnitFromSave (int address) {
  SavedUnits *tmp;
  if (NULL!=(tmp =savedunits.Get (address))) {
    savedunits.Delete (address);
    delete tmp;
  }
}
void SaveGame::WriteNewsData (FILE * fp) {
  gameMessage * last;
  vector <gameMessage *> tmp;
  int i=0;
  vector <string> newsvec;
  newsvec.push_back ("news");
  while (NULL!=(last=mission->msgcenter->last(i++,newsvec))) {
    tmp.push_back (last);
  }
  fprintf (fp,"%d\n",i);
  for (int j=tmp.size()-1;j>=0;j--) {
    char * msg = strdup (tmp[j]->message.c_str());
    int k=0;
    while (msg[k]) {
      if (msg[k]=='\n'||msg[k]=='\r')
	msg[k]=' ';
      k++;
    }
    fprintf (fp,"%s\n",msg);
    free (msg);
  }
}
void SaveGame::ReadNewsData (FILE * fp) {
  int numnews;
  fscanf (fp,"%d\n",&numnews);
  char news [1024];
  for (unsigned int i=0;i<numnews;i++) {
    fgets (news,1023,fp);
    news[1023]='\0';
    if (news[0]!='\r'&&news[0]!='\n') {
      mission->msgcenter->add ("game","news",news);
    }
  }
}
void SaveGame::AddUnitToSave (const char * filename, enum clsptr type, const char * faction, int address) {
  static string s = vs_config->getVariable ("physics","Drone","drone");
  if (0==strcmp (s.c_str(),filename)/*||type==ENHANCEMENTPTR*/) {
    RemoveUnitFromSave (address);
    savedunits.Put (address,new SavedUnits (filename,type,faction));
  }
}
olist_t &SaveGame::getMissionData(float magic_number) {
  unsigned int i=std::find (mission_data.begin(),mission_data.end(),magic_number)-mission_data.begin();
  if (i==mission_data.size()) {
    mission_data.push_back(MissionDat(magic_number));
  }
  return mission_data[i].dat;
}
void SaveGame::WriteMissionData (FILE * fp) {
  fprintf (fp," %d ",mission_data.size());
  for( unsigned int i=0;i<mission_data.size();i++) {
    fprintf (fp,"\n%f ",mission_data[i].magic_number);
    fprintf (fp,"%d ",mission_data[i].dat.size());
    for (unsigned int j=0;j<mission_data[i].dat.size();j++) {
      fprintf (fp,"%s ",varToString(mission_data[i].dat[j]).c_str());
    }
  }
}
void SaveGame::ReadMissionData (FILE * fp) {
  int mdsize;
  fscanf (fp," %d ",&mdsize);
  for( unsigned int i=0;i<mdsize;i++) {
    int md_i_size;
    float mag_num;
    fscanf (fp,"\n%f ",&mag_num);
    fscanf (fp,"%d ",&md_i_size);
    mission_data.push_back (MissionDat(mag_num));
    for (unsigned int j=0;j<md_i_size;j++) {
      varInst * vi = new varInst (VI_IN_OBJECT);//not belong to a mission...not sure should inc counter
      vi->type = VAR_FLOAT;
      fscanf (fp,"%f ",&vi->float_val);
      mission_data[i].dat.push_back (vi);
    }
  }
}
vector <SavedUnits> SaveGame::ReadSavedUnits (FILE * fp) {
  vector <SavedUnits> su;
  int a;
  char unitname[1024];
  char factname[1024];
  while (3==fscanf (fp,"%d %s %s",&a,unitname,factname)) {
    if (a==0&&0==strcmp(unitname,"factions")&&0==strcmp(factname,"begin")) {
      _Universe->LoadSerializedFaction(fp);
    }else if (a==0&&0==strcmp(unitname,"mission")&&0==strcmp(factname,"data")) {
      ReadMissionData(fp);
    }else if (a==0&&0==strcmp(unitname,"news")&&0==strcmp(factname,"data")) {
      ReadNewsData(fp);
    }else {
      su.push_back (SavedUnits (unitname,(clsptr)a,factname));
    }
  }
  return su;
}
void SaveGame::WriteSavedUnit (FILE * fp, SavedUnits* su) {
  fprintf (fp,"\n%d %s %s",su->type, su->filename.c_str(),su->faction.c_str());
}
void SaveGame::WriteSaveGame (const char *systemname, const Vector &FP, float credits, std::string unitname) {
  vector<SavedUnits *> myvec = savedunits.GetAll();
  if (outputsavegame.length()!=0) {
    printf ("Writing Save Game %s",outputsavegame.c_str());
    changehome();
    vschdir ("save");
    FILE * fp = fopen (outputsavegame.c_str(),"w");
    vscdup();
    returnfromhome();
    Vector FighterPos= PlayerLocation-FP;
//    if (originalsystem!=systemname) {
      FighterPos=FP;
//    }
    fprintf (fp,"%s^%f^%s %f %f %f",systemname,credits,unitname.c_str(),FighterPos.i,FighterPos.j,FighterPos.k);
    SetSavedCredits (credits);
    while (myvec.empty()==false) {
      WriteSavedUnit (fp,myvec.back());
      delete myvec.back();
      myvec.pop_back();
    }
    fprintf (fp,"\n%d %s %s",0,"mission","data ");
    WriteMissionData(fp);
    fprintf (fp,"\n%d %s %s",0,"news","data ");
    WriteNewsData(fp);
    fprintf (fp,"\n%d %s %s",0,"factions","begin ");
    _Universe->SerializeFaction(fp);
    fclose (fp);
  }
}
static float savedcredits=0;
float SaveGame::GetSavedCredits () {
  return savedcredits;
}
void SaveGame::SetSavedCredits (float c) {
  savedcredits = c;
}


vector<SavedUnits> SaveGame::ParseSaveGame (string filename, string &FSS, string originalstarsystem, Vector &PP, bool & shouldupdatepos,float &credits, string &savedstarship) {
  if (filename.length()>0)
    filename=callsign+filename;
  vector <SavedUnits> mysav;
  shouldupdatepos=!(PlayerLocation.i==FLT_MAX||PlayerLocation.j==FLT_MAX||PlayerLocation.k==FLT_MAX);
  outputsavegame=filename;
  changehome();
  vschdir ("save");
  FILE * fp = NULL;
  if (filename.length()>0)
	  fp = fopen (filename.c_str(),"r");
  vscdup();
  returnfromhome();
  if (fp) {
    char tmp2[10000];
    Vector tmppos;
    if (4==fscanf (fp,"%s %f %f %f\n",tmp2,&tmppos.i,&tmppos.j,&tmppos.k)) {
      for (int j=0;'\0'!=tmp2[j];j++) {
	if (tmp2[j]=='^') {
	  sscanf (tmp2+j+1,"%f",&credits);
	  tmp2[j]='\0';
	  for (int k=j+1;tmp2[k]!='\0';k++) {
	    if (tmp2[k]=='^') {
	      tmp2[k]='\0';
	      savedstarship=string(tmp2+k+1);
	      break;
	    }
	  }
	  break;
	}
      }
      if (ForceStarSystem.length()==0) 
	ForceStarSystem=string(tmp2);
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
  SetSavedCredits(credits);
  return mysav;
}

