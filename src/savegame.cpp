#include "cmd/unit_generic.h"
#include "hashtable.h"
#include <float.h>
#include "vs_path.h"
#include <vector>
#include <string>
#include "configxml.h"
#include "vs_globals.h"
#include "savegame.h"
#include "load_mission.h"
#include <algorithm>
#include "cmd/script/mission.h"
#include "gfx/cockpit_generic.h"
#include "networking/const.h"
using namespace std;
 std::string GetHelperPlayerSaveGame (int num) {

if( Network==NULL)
{
    cout << "Hi helper play " << num << endl;
  static string *res=NULL;
  if (res==NULL) {
    res = new std::string;
    changehome(false);
    //char c[2]={'\0','\0'};
    FILE * fp = fopen (("save.txt"),"r");
    if (!fp) {
      fp = fopen ("save.txt","w");
      if (fp) {
	fwrite ("default\n",8,1,fp);
	fclose (fp);
      }
      fp = fopen (("save.txt"),"r");
    }
    if (fp) {
      fseek (fp,0,SEEK_END);
      int length = ftell (fp);
      if (length>0) {
      char * temp = (char *)malloc (length+1);
      temp[length]='\0';
      fseek (fp,0,SEEK_SET);
      fread (temp,length,1,fp);
      bool end=true;
      for (int i=length-1;i>=0;i--) {
        if (temp[i]=='\r'||temp[i]=='\n') {
          temp[i]=(end?'\0':'_');
        }else if (temp[i]=='\0'||temp[i]==' '||temp[i]=='\t') {
          temp[i]=(end?'\0':'_');
        }else {
          end=false;
        }
      }
      *res = (temp);
      free (temp);
      }
      fclose (fp);

    }
#if 0
    if (fp) {
    while (!feof (fp)) {
      c[0]=fgetc (fp);
      if (!feof(fp)) {
        if (c[0]!='\r'&&c[0]!='\n'&&c[0]!='\0') {
          if (c[0]==' ') {
            c[0]='_';
            if (feof(fp)) {
              continue;
            }
          }
          (*res)+=c;
        }
      }else {
        break;
      }
    }
    fclose (fp);
#endif
    if (!res->empty()) {
      if (*res->begin()=='~') {
	fp = fopen (("save.txt"),"w");
	if (fp) {
	  for (unsigned int i=1;i<res->length();i++) {
	    fputc (*(res->begin()+i),fp);
	  }
	  fputc ('\0',fp);
	  fclose (fp);
	}
      }
    }

    
#if 0
    fp = fopen (("save.txt"),"w");
    if (fp) {
      fputc('\0',fp);
      fclose (fp);
    }
#endif
    returnfromhome();
  }
  if (num==0||res->empty()) {
    cout << "Here";
    return (*res);  
  }
  return (*res)+XMLSupport::tostring(num);
}
else
{
	//return Network[num].getCallsign();
	// Return "" so that the filename argument to ParseSavegame will be used
	return "";
}
}

std::string GetWritePlayerSaveGame(int num) {
  string ret = GetHelperPlayerSaveGame(num);
  if (!ret.empty()) {
    if (*ret.begin()=='~') {
      return ret.substr (1,ret.length());
    }
  }
  return ret;
}

std::string GetReadPlayerSaveGame(int num) {
  string ret = GetHelperPlayerSaveGame(num);
  if (!ret.empty()) {
    if (*ret.begin()=='~') {
      return "";
    }
  }
  return ret;
}

void FileCopy (const char * src, const char * dst) {
  if (dst[0]!='\0'&&src[0]!='\0') {

  FILE * fp = fopen (src,"r");
  if (fp) {
      fseek(fp,0,SEEK_END);
      long length = ftell (fp);
      fseek(fp,0,SEEK_SET);
      char * info = new char [length];
      fread(info,length,sizeof(char),fp);
      fclose (fp);
      fp = fopen (dst,"w");
      if (fp) {
        fwrite (info,length,sizeof(char),fp);
        fclose(fp);
      }
      delete [] info;
  }
  }
}

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
string SaveGame::GetOldStarSystem () {
  return ForceStarSystem;
}

void SaveGame::SetPlayerLocation (const QVector &v) {
  fprintf (stderr,"Set Location %f %f %f",v.i,v.j,v.k);
  if ((FINITE (v.i)&&FINITE(v.j)&&FINITE(v.k))) {
    PlayerLocation =v;
  }else {
    fprintf (stderr,"ERROR saving unit");
    PlayerLocation.Set(1,1,1);
  }
}
QVector SaveGame::GetPlayerLocation () {
  return PlayerLocation;
}

Hashtable<long,SavedUnits,char[47]> *SaveGame::savedunits = new Hashtable<long,SavedUnits,char[47]>();
void SaveGame::RemoveUnitFromSave (long address) {
  SavedUnits *tmp;
  if (NULL!=(tmp =savedunits->Get (address))) {
    savedunits->Delete (address);
    delete tmp;
  }
}
string SaveGame::WriteNewsData () {
  char temp[MAXBUFFER];
  string ret("");
  gameMessage * last;
  vector <gameMessage *> tmp;
  int i=0;
  vector <string> newsvec;
  newsvec.push_back ("news");
  while (NULL!=(last=mission->msgcenter->last(i++,newsvec))) {
    tmp.push_back (last);
  }
  memset( temp, 0, MAXBUFFER);
  sprintf (temp,"%d\n",i);
  ret += string( temp);
  for (int j=tmp.size()-1;j>=0;j--) {
    char * msg = strdup (tmp[j]->message.c_str());
    int k=0;
    while (msg[k]) {
      if (msg[k]=='\n'||msg[k]=='\r')
	msg[k]=' ';
      k++;
    }
    memset( temp, 0, MAXBUFFER);
    sprintf (temp,"%s\n",msg);
	ret += string( temp);
    free (msg);
  }
  return ret;
}
vector <string> parsePipedString(string s) {
  int loc;
  vector <string> ret;
  while ((loc = s.find("|"))!=string::npos) {
    ret.push_back( s.substr (0,loc));
    s = s.substr (loc+1);
  }
  if (s.length())
    ret.push_back(s);
  return ret;
}
string createPipedString(vector <string> s) {
  string ret;
  for (unsigned int i=0;i<s.size()-1;i++) {
    ret += s[i]+"|";
  }
  if (s.size())
    ret+=s.back();
  return ret;
}
void WriteSaveGame (Cockpit * cp,bool auto_save) {
  int player_num= cp-_Universe->AccessCockpit(0);
  Unit * un = cp->GetSaveParent();
  if (!un) {
    return;
  }
  if (un->GetHull()>0) {
    cp->savegame->WriteSaveGame (cp->activeStarSystem->getFileName().c_str(),un->LocalPosition(),cp->credits,cp->unitfilename,auto_save?-1:player_num);
    un->WriteUnit(cp->GetUnitModifications().c_str());
    if (GetWritePlayerSaveGame(player_num).length()&&!auto_save) {
      cp->savegame->SetSavedCredits (_Universe->AccessCockpit()->credits);
      cp->savegame->SetStarSystem(cp->activeStarSystem->getFileName());
      un->WriteUnit(GetWritePlayerSaveGame(player_num).c_str());
      cp->savegame->SetPlayerLocation(un->LocalPosition());    
    }
  }

}
int hopto (char *buf,char endln, char endln2,int readlen) {
  if (endln==' '||endln2==' ') {
    while (buf[readlen]&&buf[readlen]==' ') {
      readlen++;
    }
  }
  for(; buf[readlen]!=0&&buf[readlen]!=endln&&buf[readlen]!=endln2; readlen++) ;
  if (buf[readlen]&&buf[readlen]==endln||buf[readlen]==endln2)
    readlen++;
  return readlen;
}

void SaveGame::ReadNewsData (char * &buf) {
  int numnews;
  int i=0, j=0;
  int offset=hopto (buf,'\n','\n',0);
  if (offset>0) {
    //fgets (news,1023,fp);
    sscanf (buf,"%d\n",&numnews);
    buf+=offset;
    for (i=0;i<numnews;i++) {
      offset = hopto (buf,'\n','\n',0);
      if (offset>0)
	buf[offset-1]=0;
      int l=0;
      for (l=0;l<offset-1;l++) {
	if (buf[l]!='\r'&&buf[l]!='\n') {
	  break;
	}
      }
      if (buf[l]!='\r'&&buf[l]!='\n'&&buf[l]) {
	mission->msgcenter->add ("game","news",buf+l);
      }
      buf+=offset;
    }
  }
}
void SaveGame::AddUnitToSave (const char * filename, enum clsptr type, const char * faction, long address) {
  string s = vs_config->getVariable ("physics","Drone","drone");
  if (0==strcmp (s.c_str(),filename)/*||type==ENHANCEMENTPTR*/) {
    RemoveUnitFromSave (address);
    savedunits->Put (address,new SavedUnits (filename,type,faction));
  }
}
olist_t &SaveGame::getMissionData(const std::string &magic_number) {
  unsigned int i=std::find (mission_data.begin(),mission_data.end(),magic_number)-mission_data.begin();
  if (i==mission_data.size()) {
    mission_data.push_back(MissionDat(magic_number));
  }
  return mission_data[i].dat;
}
string SaveGame::WriteMissionData () {
  char temp[MAXBUFFER];
  string ret("");
  memset( temp, 0, MAXBUFFER);
  sprintf (temp," %d ",mission_data.size());
  ret += string(temp);
  for( unsigned int i=0;i<mission_data.size();i++) {
    memset( temp, 0, MAXBUFFER);
    sprintf (temp,"\n%s ",mission_data[i].magic_number.c_str());
    ret += string(temp);
    memset( temp, 0, MAXBUFFER);
    sprintf (temp,"%d ",mission_data[i].dat.size());
    ret += string(temp);
    for (unsigned int j=0;j<mission_data[i].dat.size();j++) {
      memset( temp, 0, MAXBUFFER);
      sprintf (temp,"%s ",varToString(mission_data[i].dat[j]).c_str());
      ret += string(temp);
    }
  }
  return ret;
}
std::string scanInString (char * &buf) {
  std::string str;
  char c[2]={'\n','\0'};
  while (*buf&&isspace (*buf)) {
    buf++;
  }
  c[0]=*buf;
  while (*buf&&(!isspace (*buf))) {
    c[0]=*buf;
    str+=c;
    buf++;
  }
  return str;
}
void SaveGame::ReadMissionData (char * &buf) {
  int mdsize;
  char * buf2 = buf;
  sscanf (buf2," %d ",&mdsize);
  // Put ptr to point after the number we just read
  buf2 +=hopto (buf2,' ','\n',0);
  for( int i=0;i<mdsize;i++) {
    int md_i_size;
    string mag_num(scanInString (buf2));
    sscanf (buf2,"%d ",&md_i_size);
    // Put ptr to point after the number we just read
    buf2 +=hopto (buf2,' ','\n',0);
    mission_data.push_back (MissionDat(mag_num));
    for (int j=0;j<md_i_size;j++) {
      varInst * vi = new varInst (VI_IN_OBJECT);//not belong to a mission...not sure should inc counter
      vi->type = VAR_FLOAT;
      sscanf (buf2,"%lf ",&vi->float_val);
      // Put ptr to point after the number we just read
      buf2 +=hopto (buf2,' ','\n',0);
      mission_data[i].dat.push_back (vi);
    }
  }
  buf = buf2;
}
vector <SavedUnits> SaveGame::ReadSavedUnits (char * &buf) {
  vector <SavedUnits> su;
  int a, j=0;
  char unitname[1024];
  char factname[1024];
  while (3==sscanf (buf,"%d %s %s",&a,unitname,factname)) {
	// Put i to point after what we parsed (on the 3rd space read)
        buf+=hopto (buf,' ','\n',0);
        buf+=hopto (buf,' ','\n',0);
        buf+=hopto (buf,' ','\n',0);
    if (a==0&&0==strcmp(unitname,"factions")&&0==strcmp(factname,"begin")) {
      FactionUtil::LoadSerializedFaction(buf);
      return su;//GOT TO BE THE LAST>... cus it's stupid :-) and mac requires the factions to be loaded AFTER this function call
    }else if (a==0&&0==strcmp(unitname,"mission")&&0==strcmp(factname,"data")) {
      ReadMissionData(buf);
    }else if (a==0&&0==strcmp(unitname,"python")&&0==strcmp(factname,"data")) {
      last_written_pickled_data=last_pickled_data=UnpickleAllMissions(buf);
    }else if (a==0&&0==strcmp(unitname,"news")&&0==strcmp(factname,"data")) {
      ReadNewsData(buf);
    }else {
      printf ("buf unrecognized %s",buf);
      //su.push_back (SavedUnits (unitname,(clsptr)a,factname));
    }
  }
  return su;
}
string SaveGame::WriteSavedUnit (SavedUnits* su) {
  char temp[MAXBUFFER];
  memset( temp, 0, MAXBUFFER);
  sprintf (temp,"\n%d %s %s",su->type, su->filename.c_str(),su->faction.c_str());
  return string( temp);
}
 extern bool STATIC_VARS_DESTROYED;
string SaveGame::WriteSaveGame (const char *systemname, const QVector &FP, float credits, std::vector<std::string> unitname, int player_num, bool write) {
  char tmp[MAXBUFFER];
  memset( tmp, 0, MAXBUFFER);
  savestring = string("");
  vector<SavedUnits *> myvec = savedunits->GetAll();
  if (outputsavegame.length()!=0) {
    printf ("Writing Save Game %s",outputsavegame.c_str());
    changehome();
    vschdir ("save");
    QVector FighterPos= PlayerLocation-FP;
//    if (originalsystem!=systemname) {
      FighterPos=FP;
//    }
      string pipedunitname = createPipedString(unitname);
    memset( tmp, 0, MAXBUFFER);
    sprintf (tmp,"%s^%f^%s %f %f %f",systemname,credits,pipedunitname.c_str(),FighterPos.i,FighterPos.j,FighterPos.k);
	savestring += string( tmp);
    SetSavedCredits (credits);
    while (myvec.empty()==false) {
      savestring += WriteSavedUnit (myvec.back());
      myvec.pop_back();
    }
    memset( tmp, 0, MAXBUFFER);
    sprintf (tmp,"\n%d %s %s",0,"mission","data ");
	savestring += string( tmp);
    savestring += WriteMissionData();
    if (!STATIC_VARS_DESTROYED)
      last_written_pickled_data=PickleAllMissions(); 

    memset( tmp, 0, MAXBUFFER);
    sprintf (tmp,"\n%d %s %s %s ",0,"python","data",last_written_pickled_data.c_str());
	savestring += string( tmp);

    memset( tmp, 0, MAXBUFFER);
    sprintf (tmp,"\n%d %s %s",0,"news","data ");
	savestring += string( tmp);
    savestring += WriteNewsData();
    memset( tmp, 0, MAXBUFFER);
    sprintf (tmp,"\n%d %s %s",0,"factions","begin ");
	savestring += string( tmp);
    savestring += FactionUtil::SerializeFaction();
	if( write)
	{
		FILE * fp = fopen (outputsavegame.c_str(),"wb");
		fwrite( savestring.c_str(), sizeof( char), savestring.length(), fp);
		fclose (fp);
		if (player_num!=-1) {
		  last_pickled_data =last_written_pickled_data;
		  FileCopy (outputsavegame.c_str(),GetWritePlayerSaveGame(player_num).c_str());
		}
	}
    vscdup();
    returnfromhome();

  }
  return savestring;
}

static float savedcredits=0;
float SaveGame::GetSavedCredits () {
  return savedcredits;
}
void SaveGame::SetSavedCredits (float c) {
  savedcredits = c;
}

vector<SavedUnits> SaveGame::ParseSaveGame (string filename, string &FSS, string originalstarsystem, QVector &PP, bool & shouldupdatepos,float &credits, vector <string> &savedstarship, int player_num, char * buf, bool read) {
  char *tempfullbuf=0;
  int tempfulllength=2048;
  int readlen=0;
  if (filename.length()>0)
    filename=callsign+filename;
  vector <SavedUnits> mysav;
  shouldupdatepos=!(PlayerLocation.i==FLT_MAX||PlayerLocation.j==FLT_MAX||PlayerLocation.k==FLT_MAX);
  outputsavegame=filename;
  changehome();
  vschdir ("save");
  FILE * fp = NULL;
  if( read)
  {
	  if (filename.length()>0) {
		if (GetReadPlayerSaveGame(player_num).length()) {
			  fp = fopen (GetReadPlayerSaveGame(player_num).c_str(),"r");
		}else {
		  fp = fopen (filename.c_str(),"r");
		}
	  }
	  if( fp)
	  {
	    fseek (fp,0,SEEK_END);
	    tempfulllength=ftell (fp);
	    fseek (fp,0,SEEK_SET);
      
	    tempfullbuf = (char *)malloc (tempfulllength+1);
	    tempfullbuf[tempfulllength]=0;
	    fread( tempfullbuf, sizeof( char), tempfulllength, fp);
	    buf = tempfullbuf;
	  }
  }
  vscdup();
  returnfromhome();
  if( fp || (!read && buf))
  {
	  savestring = string( buf);
	  if ( savestring.length()>0) {
		char tmp2[10000];
		QVector tmppos;
		if (4==sscanf (buf,"%s %lf %lf %lf\n",tmp2,&tmppos.i,&tmppos.j,&tmppos.k)) {
		  // Put readlen to point to the end of the line we just parsed
		  readlen = hopto (buf,'\n','\n',readlen);
		  for (int j=0;'\0'!=tmp2[j];j++) {
			if (tmp2[j]=='^') {
				sscanf (tmp2+j+1,"%f",&credits);
				tmp2[j]='\0';
				for (int k=j+1;tmp2[k]!='\0';k++) {
					if (tmp2[k]=='^') {
						tmp2[k]='\0';
						savedstarship.clear();
						savedstarship=parsePipedString(tmp2+k+1);
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
			PlayerLocation=tmppos;//LaunchUnitNear(tmppos);
		  }
		  buf+=readlen;
		  mysav=ReadSavedUnits (buf);
		}
	  }
	  if( read)
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
  if (tempfullbuf)
    free(tempfullbuf);
  return mysav;
}

