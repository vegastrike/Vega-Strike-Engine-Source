#include "cmd/unit_generic.h"
#include "hashtable.h"
#include <float.h>
#include "vsfilesystem.h"
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
#include "vsfilesystem.h"
using namespace std;
using namespace VSFileSystem;
std::string CurrentSaveGameName="";
 std::string GetHelperPlayerSaveGame (int num) {

if( Network==NULL)
{
	if (CurrentSaveGameName.length()>0){
		VSFile f;
		VSError err = f.OpenCreateWrite( "save.4.x.txt", UnknownFile);
		if (err<=Ok) {
			f.Write(CurrentSaveGameName);
			f.Close();
		}		
		if (num!=0) {
			return CurrentSaveGameName+XMLSupport::tostring(num);
		}
		return CurrentSaveGameName;
	}
    cout << "Hi helper play " << num << endl;
  static string *res=NULL;
  if (res==NULL) {
    res = new std::string;
    //char c[2]={'\0','\0'};
	VSFile f;
	// TRY TO OPEN THE save.4.x.txt FILE WHICH SHOULD CONTAIN THE NAME OF THE SAVE TO USE
	VSError err = f.OpenReadOnly( "save.4.x.txt", UnknownFile);
    if (err>Ok) {
	  // IF save.4.x.txt DOES NOT EXIST WE CREATE ONE WITH "default" AS SAVENAME
	  err = f.OpenCreateWrite( "save.4.x.txt", UnknownFile);
      if (err<=Ok) {
		f.Write("New_Game\n",9);
		f.Close();
      }
	  else
	  {
	  	fprintf( stderr, "!!! ERROR : Creating default save.4.x.txt file : %s\n", f.GetFullPath().c_str());
		exit(1);
	  }
	  err = f.OpenReadOnly( "save.4.x.txt", UnknownFile);
	  if( err>Ok)
	  {
	  	fprintf( stderr, "!!! ERROR : Opening the default save we just created\n");
		exit(1);
	  }
    }
    if (err<=Ok) {
	  long length=f.Size();
      if (length>0) {
      char * temp = (char *)malloc (length+1);
      temp[length]='\0';
      f.Read (temp,length);
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
      f.Close();

    }
#if 0
    if (err<=Ok) {
    while (!f.Eof()) {
      f.Read( &c[0], sizeof( char));
      if (!f.Eof()) {
        if (c[0]!='\r'&&c[0]!='\n'&&c[0]!='\0') {
          if (c[0]==' ') {
            c[0]='_';
            if (f.Eof()) {
              continue;
            }
          }
          (*res)+=c;
        }
      }else {
        break;
      }
    }
    f.Close();
#endif
    if (!res->empty())
	{
	  // Set filetype to Unknown so that it is searched in homedir/
      if (*res->begin()=='~')
	  {
		  err = f.OpenCreateWrite( "save.4.x.txt", VSFileSystem::UnknownFile);
		if (err<=Ok) {
	  	for (unsigned int i=1;i<res->length();i++)
		{
			char cc = *(res->begin()+i);
	    	f.Write ( &cc,sizeof(char));
	  	}
	 	char cc=0;
	  	f.Write (&cc,sizeof(char));
	  	f.Close();
	  }
    }
  } 

    
#if 0
	err = f.OpenReadOnly( "save.4.x.txt", SaveFile);
    if (err<=Ok) {
	  char cc=0;
      f.Write(&cc,sizeof( char));
      f.Close();
    }
#endif
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

// Used only to copy a savegame to a different named one
void SaveFileCopy (const char * src, const char * dst) {
  if (dst[0]!='\0'&&src[0]!='\0') {

  VSFile f;
  VSError err = f.OpenReadOnly( src, SaveFile);
  if (err<=Ok) {
      string savecontent = f.ReadFull();
	  f.Close();
      err = f.OpenCreateWrite( dst, SaveFile);
      if (err<=Ok) {
		f.Write( savecontent);
	    f.Close();
      }
	  else
	  	fprintf( stderr, "WARNING : couldn't open savegame to copy to : %s as SaveFile", dst);
  }
  else
  	fprintf(stderr, "WARNING : couldn't find the savegame to copy : %s as SaveFile", src);
  }
}
class MissionStringDat {
public:
  typedef std::map <string,vector <string> >MSD;
  MSD m;
};
class MissionFloatDat {
public:
  typedef std::map <string,vector <float> >MFD;
  MFD m;
};
SaveGame::SaveGame(const std::string &pilot) {
  callsign=pilot;
  ForceStarSystem=string("");
  PlayerLocation.Set(FLT_MAX, FLT_MAX,FLT_MAX);
  missionstringdata = new MissionStringDat;
  missiondata = new MissionFloatDat;

}
SaveGame::~SaveGame() {
  delete missionstringdata;
  delete missiondata;
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
  //VSFileSystem::vs_fprintf (stderr,"Set Location %f %f %f",v.i,v.j,v.k);
  if ((FINITE (v.i)&&FINITE(v.j)&&FINITE(v.k))) {
    PlayerLocation =v;
  }else {
    VSFileSystem::vs_fprintf (stderr,"ERROR saving unit");
    PlayerLocation.Set(1,1,1);
  }
}
QVector SaveGame::GetPlayerLocation () {
  return PlayerLocation;
}

void SaveGame::RemoveUnitFromSave (long address) {
  /*
  SavedUnits *tmp;
  if (NULL!=(tmp =savedunits->Get (address))) {
    savedunits->Delete (address);
    delete tmp;
  }
  */
}
string SaveGame::WriteNewsData () {
  string ret("");
  gameMessage last;
  vector <gameMessage> tmp;
  int i=0;
  vector <string> newsvec;
  newsvec.push_back ("news");
  while ((mission->msgcenter->last(i++,last,newsvec))) {
    tmp.push_back (last);
  }
  ret += XMLSupport::tostring(i)+"\n";
  for (int j=tmp.size()-1;j>=0;j--) {
    char * msg = strdup (tmp[j].message.c_str());
    int k=0;
    while (msg[k]) {
      if (msg[k]=='\r')
	msg[k]=' ';
      if (msg[k]=='\n')
	msg[k]='/';
      k++;
    }
    ret += string( msg)+"\n";
    free (msg);
  }
  return ret;
}
vector <string> parsePipedString(string s) {
  string::size_type loc;
  vector <string> ret;
  while ((loc = s.find("|"))!=string::npos) {
    ret.push_back( s.substr (0,loc));
	cout<<"Found ship named : "<<s.substr( 0, loc)<<endl;
    s = s.substr (loc+1);
  }
  if (s.length())
  {
    ret.push_back(s);
	cout<<"Found ship named : "<<s<<endl;
  }
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
      //      un->WriteUnit(GetWritePlayerSaveGame(player_num).c_str());
      cp->savegame->SetPlayerLocation(un->LocalPosition());    
      for (int i=0;i<cp->unitfilename.size();i+=2) {
        if (i==2) i=1;
        VSFile src,dst;
        string srcnam=cp->GetUnitModifications();

        VSError e = src.OpenReadOnly(srcnam+"/"+cp->unitfilename[i]+".csv",UnitSaveFile);
        if (e<=Ok) {
          string dstnam= GetWritePlayerSaveGame(player_num);
          VSFileSystem::CreateDirectoryHome (VSFileSystem::savedunitpath+"/"+dstnam);
          VSError f = dst.OpenCreateWrite(dstnam+"/"+cp->unitfilename[i]+".csv",UnitFile);
          if (f<=Ok) {
            string srcdata=src.ReadFull();
            dst.Write(srcdata);
          }else {
            printf ("Error: Cannot Copy Unit %s from save file %s to %s\n",
                    cp->unitfilename[i].c_str(),
                    srcnam.c_str(),
                    dstnam.c_str());
                    
          }
        }else {
            printf ("Error: Cannot Open Unit %s from save file %s.\n",
                    cp->unitfilename[i].c_str(),
                    srcnam.c_str());
                    
        }        
      }
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
  int i=0;
  vector <string> n00s;
  n00s.push_back ("news");
  vector<string> nada;
  mission->msgcenter->clear (n00s,nada);  
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
void SaveGame::AddUnitToSave (const char * filename, int type, const char * faction, long address) {
  string s = vs_config->getVariable ("physics","Drone","drone");
  if (0==strcmp (s.c_str(),filename)/*||type==ENHANCEMENTPTR*/) {
    RemoveUnitFromSave (address);
    //    savedunits->Put (address,new SavedUnits (filename,type,faction));//not no more
  }
}
std::vector<float> &SaveGame::getMissionData(const std::string &magic_number) {
  return missiondata->m[magic_number];
}
std::vector<string> &SaveGame::getMissionStringData(const std::string &magic_number) {
  return missionstringdata->m[magic_number];
}
template <class MContainerType> void RemoveEmpty (MContainerType &t) {
	bool done=false;
	done=true;
	typename MContainerType::iterator i;
	for (i=t.begin();i!=t.end();) {
		typename MContainerType::key_type k = (*i).first;
		if ((*i).second.empty()) {
			t.erase(i);
			i = t.lower_bound(k);
		}else {
			i++;
		}
	}
}
string SaveGame::WriteMissionData () {
  string ret(" ");
  RemoveEmpty<MissionFloatDat::MFD> (missiondata->m);
  ret+=XMLSupport::tostring ((int)missiondata->m.size());
  for( MissionFloatDat::MFD::iterator i=missiondata->m.begin();i!=missiondata->m.end();i++) {
    unsigned int siz = (*i).second.size();
    ret += string("\n")+(*i).first+string(" ")+XMLSupport::tostring(siz)+" ";
    for (unsigned int j=0;j<siz;j++) {
      ret += XMLSupport::tostring((*i).second[j])+" ";
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
  missiondata->m.clear();
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
    missiondata->m[mag_num] = vector<float>();
    vector <float> * vecfloat=&missiondata->m[mag_num];
    for (int j=0;j<md_i_size;j++) {
      double float_val;
      sscanf (buf2,"%lf ",&float_val);
      // Put ptr to point after the number we just read
      buf2 +=hopto (buf2,' ','\n',0);
      vecfloat->push_back (float_val);
    }
  }
  buf = buf2;
}
string AnyStringScanInString (char * &buf) {
  unsigned int size=0;
  bool found=false;
  while ((*buf)&&((*buf)!=' '||(!found))) {
    if ((*buf)>='0'&&(*buf)<='9') {
      size*=10;
      size+=(*buf)-'0';
      found=true;
    }
    buf++;
  }
  if (*buf)
    buf++;
  char duo[2]={0,0};
  string ret;
  for (unsigned int i=0;i<size&&(*buf);++i) {
    duo[0]=*buf;
    ret+=duo;
    buf++;
  }
  return ret;
}
string AnyStringWriteString (string input) {
  return XMLSupport::tostring ((int)input.length())+" "+input;
}
void SaveGame::ReadMissionStringData (char * &buf) {
  missionstringdata->m.clear();
  int mdsize;
  char * buf2 = buf;
  sscanf (buf2," %d ",&mdsize);
  // Put ptr to point after the number we just read
  buf2 +=hopto (buf2,' ','\n',0);
  for( int i=0;i<mdsize;i++) {
    int md_i_size;
    string mag_num(AnyStringScanInString (buf2));
    //sscanf (buf2,"%d ",&md_i_size);
	md_i_size = strtol(buf2,(char **)NULL,10);
    // Put ptr to point after the number we just read
    buf2 +=hopto (buf2,' ','\n',0);
    missionstringdata->m[mag_num] = vector<string>();
    vector <string> * vecstring=&missionstringdata->m[mag_num];
    for (int j=0;j<md_i_size;j++) {
      vecstring->push_back (AnyStringScanInString(buf2));
    }
  }
  buf = buf2;
}



void PushBackFloat(float f, vector <char> &ret) {
	char c[128];
    sprintf(c,"%f",f);
	char * k=&c[0];
	while (*k) {
		ret.push_back(*k);
	}
}
void PushBackUInt(unsigned int i, vector<char> & ret) {
	if (!i)
		ret.push_back('0');
	else {
		unsigned int start=ret.size();
		while(i){
			ret.push_back(i%10+'0');
			i/=10;
		}
		unsigned int fin = ret.size();
		unsigned int jend = (fin-start)/2;
		for (unsigned int j=0;j<jend;++j) {
			char tmp = ret[j+start];
			ret[j+start]=ret[fin-j-1];
			ret[fin-j-1]=tmp;
		}
	}
}
void PushBackInt(int i, vector<char> &ret) {
	if (i<0)
		ret.push_back('-');
	PushBackUInt(i<0?-i:1,ret);
}
void PushBackChars(const char * c,vector<char> & ret) {
	while (*c) {
		ret.push_back(*c);
		++c;
	}
}
void PushBackString (string input,vector<char> &ret) {
	PushBackUInt(input.length(),ret);
	PushBackChars(" ",ret);
	PushBackChars(input.c_str(),ret);
}

void SaveGame::WriteMissionStringData (vector <char> & ret) {
  RemoveEmpty<MissionStringDat::MSD> (missionstringdata->m);
  PushBackUInt(missionstringdata->m.size(),ret);
  for( MissionStringDat::MSD::iterator i=missionstringdata->m.begin();i!=missionstringdata->m.end();i++) {
    unsigned int siz = (*i).second.size();
	PushBackChars("\n",ret);
	PushBackString((*i).first,ret);
	PushBackUInt(siz,ret);
	PushBackChars(" ",ret);
    for (unsigned int j=0;j<siz;j++) {
		PushBackString((*i).second[j],ret);
    }
  }
}

void SaveGame::ReadStardate( char * &buf)
{
	string stardate( AnyStringScanInString( buf));
	cout<<"Read stardate : "<<stardate<<endl;
	_Universe->current_stardate.InitTrek( stardate);
}

void SaveGame::ReadSavedPackets (char * &buf) {
  int a=0;
  char unitname[1024];
  char factname[1024];
  while (3==sscanf (buf,"%d %s %s",&a,unitname,factname)) {
	// Put i to point after what we parsed (on the 3rd space read)
    while ((*buf)&&isspace(*buf))++buf;
        buf+=hopto (buf,' ','\n',0);
        buf+=hopto (buf,' ','\n',0);
        buf+=hopto (buf,' ','\n',0);
    if (a==0&&0==strcmp(unitname,"factions")&&0==strcmp(factname,"begin")) {
      FactionUtil::LoadSerializedFaction(buf);
      return;//GOT TO BE THE LAST>... cus it's stupid :-) and mac requires the factions to be loaded AFTER this function call
    }else if (a==0&&0==strcmp(unitname,"mission")&&0==strcmp(factname,"data")) {
      ReadMissionData(buf);
    }else if (a==0&&0==strcmp(unitname,"missionstring")&&0==strcmp(factname,"data")) {
      ReadMissionStringData(buf);
    }else if (a==0&&0==strcmp(unitname,"python")&&0==strcmp(factname,"data")) {
      last_written_pickled_data=last_pickled_data=UnpickleAllMissions(buf);
    }else if (a==0&&0==strcmp(unitname,"news")&&0==strcmp(factname,"data")) {
      ReadNewsData(buf);
    }else if (a==0&&0==strcmp(unitname,"stardate")&&0==strcmp(factname,"data")) {
	  // On server side we expect the latest saved stardate in dynaverse.dat too
      ReadStardate(buf);
    }else {
      printf ("buf unrecognized %s",buf);
      //su.push_back (SavedUnits (unitname,(clsptr)a,factname));
    }
  }
 cout<<"\tExiting ReadSavedPackets"<<endl;
}
string SaveGame::WriteSavedUnit (SavedUnits* su) {
  return string("\n")+XMLSupport::tostring(su->type)+string(" ")+su->filename+" "+su->faction;
}
 extern bool STATIC_VARS_DESTROYED;
static char * tmprealloc (char * var, int &oldlength, int newlength) {
  if (oldlength<newlength) {
    oldlength= newlength;
    var= (char *)realloc(var,newlength);
  }
  memset (var,0,newlength);
  return var;
}

string SaveGame::WritePlayerData( const QVector &FP, std::vector<std::string> unitname, const char * systemname, float credits, std::string fact)
{
	  string playerdata("");
  	  int MB = MAXBUFFER;
  	  char * tmp=(char *)malloc (MB);
  	  memset( tmp, 0, MB);

      QVector FighterPos= PlayerLocation-FP;
//    if (originalsystem!=systemname) {
      FighterPos=FP;
//    }
      string pipedunitname = createPipedString(unitname);
      tmp = tmprealloc(tmp,MB,pipedunitname.length()+strlen(systemname)+256/*4 floats*/);
	  // If we specify no faction, it won't be saved in there
	  if( fact != "")
		sprintf (tmp,"%s^%f^%s %f %f %f %s",systemname,credits,pipedunitname.c_str(),FighterPos.i,FighterPos.j,FighterPos.k, fact.c_str());
	  else
		sprintf (tmp,"%s^%f^%s %f %f %f",systemname,credits,pipedunitname.c_str(),FighterPos.i,FighterPos.j,FighterPos.k);
      playerdata = string( tmp);
	  this->playerfaction = fact;
      SetSavedCredits (credits);
  	  free(tmp);tmp=NULL;

	  return playerdata;
}

string SaveGame::WriteDynamicUniverse()
{
	string dyn_univ("");
  	int MB = MAXBUFFER;
  	char * tmp=(char *)malloc (MB);
  	memset( tmp, 0, MB);

	// Write mission data
	// On server side we save the stardate
	if( SERVER)
	{
		cerr<<"SAVING STARDATE - SERVER="<<SERVER<<endl;
		dyn_univ += "\n0 stardate data"+AnyStringWriteString( _Universe->current_stardate.GetFullTrekDate());
	}
    memset( tmp, 0, MB);
    sprintf (tmp,"\n%d %s %s",0,"mission","data ");
    dyn_univ += string( tmp);
    dyn_univ += WriteMissionData();
    memset( tmp, 0, MB);
    sprintf (tmp,"\n%d %s %s",0,"missionstring","data ");
    dyn_univ += string( tmp);
	vector <char> missionstringdata1;
	WriteMissionStringData(missionstringdata1);
    dyn_univ +=string(&missionstringdata1[0],missionstringdata1.size());
		
    if (!STATIC_VARS_DESTROYED)
      last_written_pickled_data=PickleAllMissions(); 
    tmp = tmprealloc(tmp,MB,last_written_pickled_data.length()+256/*4 floats*/);
    sprintf (tmp,"\n%d %s %s %s ",0,"python","data",last_written_pickled_data.c_str());
    dyn_univ += string( tmp);

	// Write news data
    memset( tmp, 0, MB);
    sprintf (tmp,"\n%d %s %s",0,"news","data ");
    dyn_univ += string( tmp);
    dyn_univ += WriteNewsData();
	// Write faction relationships
    memset( tmp, 0, MB);
    sprintf (tmp,"\n%d %s %s",0,"factions","begin ");
    dyn_univ += string( tmp);
    dyn_univ += FactionUtil::SerializeFaction();

  	free(tmp);tmp=NULL;

	return dyn_univ;
}

using namespace VSFileSystem;

string SaveGame::WriteSaveGame (const char *systemname, const QVector &FP, float credits, std::vector<std::string> unitname, int player_num, std::string fact, bool write) {
  savestring = string("");
  if (outputsavegame.length()!=0) {
    printf ("Writing Save Game %s",outputsavegame.c_str());

	savestring += WritePlayerData( FP, unitname, systemname, credits, fact);
	savestring += WriteDynamicUniverse();

    if( write){
	VSFile f;
	VSError err = f.OpenCreateWrite( outputsavegame, SaveFile);
	// WRITE THE SAVEGAME TO THE MISSION SAVENAME
	f.Write( savestring.c_str(), savestring.length());
	f.Close();
	if (player_num!=-1) {
			// AND THEN COPY IT TO THE SPECIFIED SAVENAME (from save.4.x.txt)
          last_pickled_data =last_written_pickled_data;
          string sg =GetWritePlayerSaveGame(player_num);
          SaveFileCopy (outputsavegame.c_str(),sg.c_str());
          
	}
    }

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

void SaveGame::ParseSaveGame (string filename, string &FSS, string originalstarsystem, QVector &PP, bool & shouldupdatepos,float &credits, vector <string> &savedstarship, int player_num, string str, bool read) {
	char *tempfullbuf=0;
	int tempfulllength=2048;
	int readlen=0;
	if (filename.length()>0)
			filename=callsign+filename;
	shouldupdatepos=!(PlayerLocation.i==FLT_MAX||PlayerLocation.j==FLT_MAX||PlayerLocation.k==FLT_MAX);
	// WE WILL ALWAYS SAVE THE CURRENT SAVEGAME IN THE MISSION SAVENAME (IT WILL BE COPIED TO THE SPECIFIED SAVENAME)
	outputsavegame=filename;
	VSFile f;
	VSError err=FileNotFound;
	if( read)
	{
		if (filename.length()>0) {
				// TRY TO GET THE SPECIFIED SAVENAME TO LOAD
				string plsave = GetReadPlayerSaveGame(player_num);
				if (plsave.length()) {
					err = f.OpenReadOnly( plsave, SaveFile);
				}else {
					// IF NONE SIMPLY LOAD THE MISSION DEFAULT ONE
					err = f.OpenReadOnly( filename, SaveFile);
				}
		}
		if( err<=Ok)
		{
				str = savestring = f.ReadFull();
		}
  }
  if( err<=Ok || (!read && str!=""))
  {
	  savestring = str;
	  if ( savestring.length()>0) {
	    char * buf = new char[str.length()+1];
		buf[str.length()]='\0';

		memcpy( buf, str.c_str(), str.length());
		char * deletebuf = buf;
		char *tmp2= (char *)malloc(savestring.length()+2);
		char * freetmp2 = tmp2;
		char * factionname = new char[1024];
		memset( factionname, 0, 1024);
		QVector tmppos;
		int res = sscanf (buf,"%s %lf %lf %lf %s\n",tmp2,&tmppos.i,&tmppos.j,&tmppos.k, factionname);
		if (res==4 || res==5) {
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
		  // In networking save we include the faction at the end of the first line
		  if( res==5)
		  {
		  	playerfaction = string( factionname);
			cout<<"Found faction in save file : "<<playerfaction<<endl;
		  }
		  else
		  {
		    // If no faction -> default to privateer
		  	playerfaction = string( "privateer");
			cout<<"Faction not found assigning default one : privateer !!!"<<endl;
		  }
		  delete []factionname;

		  if (ForceStarSystem.length()==0)
			ForceStarSystem=string(tmp2);
		  if (PlayerLocation.i==FLT_MAX||PlayerLocation.j==FLT_MAX||PlayerLocation.k==FLT_MAX) {
			shouldupdatepos=true;
			PlayerLocation=tmppos;//LaunchUnitNear(tmppos);
		  }
		  buf+=readlen;
		  ReadSavedPackets (buf);
		}
		free(freetmp2);freetmp2=NULL;
		tmp2=NULL;
		delete []deletebuf;
	  }
	  if( read)
	  	f.Close();
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
  //  return mysav;
}

