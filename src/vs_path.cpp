#include "vs_path.h"
#include "config_xml.h"
#include "vs_globals.h"
#include "xml_support.h"
#ifdef _WIN32

#else
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#endif

#define CONFIGFILE ".vegastrikerc"
std::string sharedtextures;
std::string sharedsounds;
std::string datadir;

void changehome() {
#ifndef _WIN32
  struct passwd *pwent;
  pwent = getpwuid (getuid());
  chdir (pwent->pw_dir);
#endif
}


void initpaths () {
  char pwd[8192];
  getcwd (pwd,8191);
  datadir= string (pwd);
  sharedsounds = datadir;
  FILE *fp= fopen (CONFIGFILE,"r");
  changehome();
  FILE *fp1= fopen (CONFIGFILE,"r");
  if (fp1) {
    fclose (fp1);
    vs_config=new VegaConfig(CONFIGFILE); // move config to global or some other struct
  }else if (fp) {
    chdir (pwd);
    fclose (fp);
    fp =NULL;
    vs_config = new VegaConfig (CONFIGFILE);
  } else {
    fprintf (stderr,"Could not open config file %s\n",CONFIGFILE);
    exit (-1);
  }
  if (fp)
    fclose (fp);
  datadir = vs_config->getVariable ("data","directory",sharedsounds);
  chdir (datadir.c_str());
  chdir (vs_config->getVariable ("data","sharedtextures","textures").c_str());
  getcwd (pwd,8191);
  sharedtextures = string (pwd);
  chdir (datadir.c_str());
  chdir (vs_config->getVariable ("data","sharedsounds","sounds").c_str());
  getcwd (pwd,8191);
  sharedsounds = string (pwd);
  chdir (datadir.c_str());
}

