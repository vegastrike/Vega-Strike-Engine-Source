#ifndef VS_PATH_H_
#define VS_PATH_H_

#include <string>
#include <vector>
extern std::vector <std::string> curdir;//current dir starting from datadir
extern std::string sharedtextures;
extern std::string sharedsounds;
extern std::string sharedmeshes;
extern std::string datadir;
///gets the hash name for an item with name in curdir
std::string GetHashName (const std::string &name);
std::string GetSharedSoundPath (const std::string &name);
std::string GetSharedTexturePath (const std::string &name);
std::string GetSharedTextureHashName(const std::string &);
std::string GetSharedSoundHashName(const std::string&);
std::string GetSharedMeshPath (const std::string &name);
std::string GetSharedMeshHashName(const std::string &);

///sets up the data dir and loads VS config
void initpaths();
///Changes to the users home directory in an OS independent way
void changehome();
//changes the directory and changes the current path to reflect it. Note may only change ONE level of directory or it breaks!
void vschdir (const char *path);
//Goes up one level, and changes the directory accordingly
void vscdup();


#endif
