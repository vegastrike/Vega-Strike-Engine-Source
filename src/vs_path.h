#ifndef VS_PATH_H_
#define VS_PATH_H_

#include <string>
#include <vector>
#include "configxml.h"
extern char *CONFIGFILE;
extern VegaConfig * createVegaConfig( char * file);

#define HOMESUBDIR ".vegastrike"
#define DELIM '/'
#define DELIMSTR "/"

extern std::vector <std::string> curdir;//current dir starting from datadir
extern std::vector <std::string> savedpwd;
extern std::string sharedtextures;
extern std::string sharedsounds;
extern std::string sharedmeshes;
extern std::string sharedunits;
extern std::string datadir;
extern std::string homedir;//including subdir
///gets the hash name for an item with name in curdir
std::string GetHashName (const std::string &name);
std::string GetHashName (const std::string &name,float scale, int faction);
std::string GetSharedSoundPath (const std::string &name);
std::string GetSharedTexturePath (const std::string &name);
std::string GetSharedTextureHashName(const std::string &);
std::string GetSharedSoundHashName(const std::string&);
std::string GetSharedMeshPath (const std::string &name);
std::string GetSharedMeshHashName(const std::string &, float scale, int faction);
std::string GetSharedUnitPath ();
///takes out all forward slasshes
std::string MakeSharedStarSysPath (const std::string &);
std::string MakeSharedPath (const std::string &path);
std::string GetCorrectStarSysPath (const std::string &);

///sets up the data dir and loads VS config
void initpaths(const std::string& modname="");
///Changes to the users home directory in an OS independent way
void changehome(bool makehomedir=false);
///Returns from home directory to calling directory
void returnfromhome();
//changes the directory and changes the current path to reflect it. Note may only change ONE level of directory or it breaks!
void vschdir (const char *path);
//Goes up one level, and changes the directory accordingly
void vscdup();

void vssetdir (const char * path);
void vsresetdir ();
#endif
