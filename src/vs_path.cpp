#include "vegastrike.h"
#include "vs_path.h"
#include "configxml.h"
#include "vs_globals.h"
#include "xml_support.h"

#include "common/common.h"

#if defined(_WIN32) && !defined(__CYGWIN__)
#include <direct.h>				// definitions of getcwd() and chdir() on win32
#else
#include <unistd.h>
#include <pwd.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif
#include "galaxy_gen.h"

char *CONFIGFILE;

string floattostringh (float f) {
	char c[128];
	sprintf (c,"%2.2f",f);
	return string(c);
}
string VectorToString(const Vector & v) {
	string ret(floattostringh(v.i));
	if (v.i!=v.j||v.j!=v.k) {
		ret+=string(",")+floattostringh(v.j)+string(",")+floattostringh(v.k);
	}
	return ret;
}
std::vector <std::string> savedpwd;
std::string sharedtextures;
std::string sharedunits;
std::string sharedsounds;
std::string sharedmeshes;
std::string datadir;
std::string homedir;
std::string HOMESUBDIR(".vegastrike");
std::vector <std::string> curdir;//current dir starting from datadir
std::vector <std::vector <std::string> > savedcurdir;//current dir starting from datadir
void vsmkdir(const std::string &s) {
	mkdir (s.c_str()
#ifndef _WIN32
		   ,0xFFFFFFFF
#endif
		);
}
void changehome(bool makehomedir) {
  static char pw_dir[2000];
#ifndef _WIN32
  struct passwd *pwent;
  pwent = getpwuid (getuid());
  vssetdir (pwent->pw_dir);
#else
  if (!makehomedir)
    vssetdir (pw_dir);
#endif
  if (makehomedir) {
  getcwd (pw_dir,1998);
#ifndef _WIN32
  vsresetdir();
#endif
  vssetdir (pw_dir);
  if (chdir (HOMESUBDIR.c_str())==-1) {
      //      system ("mkdir " HOMESUBDIR);
    mkdir (HOMESUBDIR.c_str()
#if !defined(_WIN32) || defined(__CYGWIN__) 
		  , 0xFFFFFFFF
#endif		  
		  );
    } else {
      chdir ("..");
    }
  std::string genbsp(HOMESUBDIR +"/generatedbsp");
    if (chdir (genbsp.c_str())==-1) {
                        mkdir (genbsp.c_str()
#if !defined(_WIN32) || defined(__CYGWIN__) 
			, 0xFFFFFFFF
#endif
		);

		
    }else {
		chdir (pw_dir);
    }
    std::string savetmp  (HOMESUBDIR+"/save");
    if (chdir (savetmp.c_str())==-1) {
      mkdir (savetmp.c_str()
#if !defined(_WIN32) || defined(__CYGWIN__) 
		  , 0xFFFFFFFF
#endif		  
		  );
      //system ("mkdir " HOMESUBDIR "/generatedbsp");
    }else {
      chdir (pw_dir);
    }
  }
  vschdir (HOMESUBDIR.c_str());
}
void returnfromhome() {
  vscdup();
  vsresetdir();
}


char pwd[65536];
void initpaths (const std::string& modname) {
#ifndef _WIN32
	
  datadir = getdatadir();
  if (modname.size())
    HOMESUBDIR = string(".")+modname;
  if (modname.size())
	  datadir+=string(DELIMSTR)+modname;
#else
  if (modname.size())
    chdir (modname.c_str());
  getcwd(pwd,65534);
  
  pwd[65534]=0;
  datadir=pwd;
#endif
  sharedsounds = datadir;

  cerr << "Data directory is " << datadir << endl;
  FILE *fp= fopen (CONFIGFILE,"r");

  //check if we have a config file in home dir
  changehome(true);
  char myhomedir[8192];
  getcwd (myhomedir,8191);
  myhomedir[8191]='\0';
  homedir = myhomedir;
  FILE *fp1= fopen (CONFIGFILE,"r");
  if (fp1) {
    //  we have a config file in home directory
    fclose (fp1);
    vs_config=createVegaConfig(CONFIGFILE); // move config to global or some other struct
    cout << "using config file in home dir" << endl;
  }else if (fp) {
    // we don't have a config file in home dir
    // but we have one in the data dir
    chdir(datadir.c_str());

    fclose (fp);
    fp =NULL;
    returnfromhome();
    vs_config = createVegaConfig (CONFIGFILE);
    cout << "using config file in data dir " << datadir << endl;
    changehome();
  } else {

    // no config file in home dir or data dir
    fprintf (stderr,"Could not open config file in either %s/%s\nOr in ~/.vegastrike/%s\n",datadir.c_str(),CONFIGFILE,CONFIGFILE);
    exit (-1);
  }
  if (fp)
    fclose (fp);
  returnfromhome();

  simulation_atom_var=atof(vs_config->getVariable("general","simulation_atom","0.1").c_str());
  char mycwd [256];
  getcwd(mycwd,255);
  mycwd[254]=mycwd[255]=0;
  cout << "SIMULATION_ATOM: " << SIMULATION_ATOM << endl;

  string config_datadir = vs_config->getVariable ("data","directory",datadir);
  if(config_datadir!=datadir){
    cout << "using data dir " << config_datadir << " from config file" << endl;
    datadir=config_datadir;
  }
  vschdir (vs_config->getVariable ("data","sharedtextures","textures").c_str());
  getcwd (pwd,8191);
  sharedtextures = string (pwd);
  vscdup();
  vschdir (vs_config->getVariable ("data","sharedsounds","sounds").c_str());
  getcwd (pwd,8191);
  sharedsounds = string (pwd);
  vscdup();
  vschdir (vs_config->getVariable ("data","sharedmeshes","meshes").c_str());
  getcwd (pwd,8191);
  sharedmeshes = string (pwd);
  vscdup();
  vschdir (vs_config->getVariable ("data","sharedunits","units").c_str());
  getcwd (pwd,8191);
  sharedunits = string (pwd);


  vscdup();
  if (datadir.end()!=datadir.begin()) {
    if (*(datadir.end()-1)!='/'&&*(datadir.end()-1)!='\\') {
      datadir+=DELIM;
    }
  }
  if (sharedtextures.end()!=sharedtextures.begin()) {
    if (*(sharedtextures.end()-1)!='/'&&*(sharedtextures.end()-1)!='\\') {
      sharedtextures+=DELIM;
    }
  }
  if (sharedmeshes.end()!=sharedmeshes.begin()) {
    if (*(sharedmeshes.end()-1)!='/'&&*(sharedmeshes.end()-1)!='\\') {
      sharedmeshes+=DELIM;
    }
  }
  if (sharedunits.end()!=sharedunits.begin()) {
    if (*(sharedunits.end()-1)!='/'&&*(sharedunits.end()-1)!='\\') {
      sharedunits+=DELIM;
    }
  }
  if (sharedsounds.end()!=sharedsounds.begin()) {
    if (*(sharedsounds.end()-1)!='/'&&*(sharedsounds.end()-1)!='\\') {
      sharedsounds+=DELIM;
    }
  }
}
std::string GetHashName (const std::string &name) {
  std::string result("");
  for (unsigned int i=0;i<curdir.size();i++) {
    result+=curdir[i];
  }
  result+=name;
  return result;
}
std::string GetHashName (const std::string &name, const Vector &scale, int faction) {
  std::string result("");
  for (unsigned int i=0;i<curdir.size();i++) {
    result+=curdir[i];
  }
  result+=name;
  
  result+=VectorToString(scale)+"|"+XMLSupport::tostring(faction);
  return result;
}


std::string GetSharedMeshPath (const std::string &name) {
  return sharedmeshes+name;
}
std::string GetSharedUnitPath () {
  return sharedunits;
}
std::string GetSharedMeshHashName (const std::string &name,const Vector & scale, int faction) {
    return (string ("#")+VectorToString(scale)+ string("#")+name+string("#")+XMLSupport::tostring(faction));
}

std::string GetSharedTexturePath (const std::string &name) {
  return sharedtextures+name;
}
std::string GetSharedTextureHashName (const std::string &name) {
  return (string ("#stex#")+name);
}
std::string GetSharedSoundPath (const std::string &name) {
  return sharedsounds+name;
}
std::string GetSharedSoundHashName (const std::string &name) {
  return (string ("#ssnd#")+name);
}

std::string MakeSharedPathReturnHome (const std::string &newpath) {
  changehome();
  getcwd (pwd,8191);
 
  if (chdir (newpath.c_str())==-1) {
#if defined(_WIN32) && !defined(__CYGWIN__)
    mkdir (newpath.c_str());
#else
    mkdir (newpath.c_str(), 0xFFFFFFFF);
#endif
  }else {
    chdir ("..");
  }
  returnfromhome();
  return 
    string(pwd)+string("/");	  
}
std::string MakeSharedPath (const std::string &s) {
  fprintf (stderr,"MakingSharedPath %s",s.c_str());
  return MakeSharedPathReturnHome (s)+s;
}
std::string MakeSharedStarSysPath (const std::string &s){
  return MakeSharedPathReturnHome (getStarSystemSector (s))+s;
}
std::string GetCorrectStarSysPath (const std::string &name, bool &autogenerated) {
  autogenerated=false;
  if (name.length()==0) {
    return string ("");
  }
   static string sys = vs_config->getVariable("data","sectors","sectors");
   string temp (sys+"/"+name);
  FILE * fp= fopen (temp.c_str(),"r");
  if (fp){
    fclose (fp);
    return temp;
  }else {
      fp = fopen (name.c_str(),"r");
  }
  if (fp){
    fclose (fp);
    return name;
  }
  changehome();
  fp = fopen (name.c_str(),"r");

  if (fp!=NULL) {
    fclose (fp);
    autogenerated=true;
    getcwd (pwd,8191);
  }
  returnfromhome();
  char delim []={DELIM,'\0'};
  return (fp==NULL)?string (""):string (pwd)+string (delim)+name;
}


void vschdir (const char *path) {
  if (path[0]!='\0') {
    if (path[0]=='.'&&path[1]=='.') {
      vscdup();
      return;
    }
  }
  if (chdir (path)!=-1) {
    std::string tpath = path;
    if (tpath.end()!=tpath.begin())
      if ((*(tpath.end()-1)!='/')&&((*(tpath.end()-1))!='\\'))
	tpath+='/';
    curdir.push_back (tpath);
  } else {
    curdir.push_back (string("~"));
  }
}
void vssetdir (const char * path) {
  getcwd (pwd,8191);
  savedpwd.push_back (string (pwd));
  savedcurdir.push_back (curdir);
  curdir.clear();
  curdir.push_back (path);
  chdir (path);
}
void vsresetdir () {
  chdir (savedpwd.back().c_str());
  curdir = savedcurdir.back();
  savedcurdir.pop_back();
  savedpwd.pop_back();
}

void vscdup() {
  if (!curdir.empty()) {
    if ((*curdir.back().begin())!='~') {
      chdir ("..");
    }
    curdir.pop_back ();
  }
}
