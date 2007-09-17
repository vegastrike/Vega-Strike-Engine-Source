#ifndef __SAVE_UTIL_H__
#define __SAVE_UTIL_H__

#include <string>
using std::string;

extern const char *mission_key;
float getSaveData (int whichcp, string key, unsigned int num);
string getSaveString (int whichcp, string key, unsigned int num);
unsigned int getSaveDataLength (int whichcp, string key);
unsigned int getSaveStringLength (int whichcp, string key);
unsigned int pushSaveData (int whichcp, string key, float val);
unsigned int eraseSaveData (int whichcp, string key, unsigned int index);
unsigned int pushSaveString (int whichcp, string key, string value);
void putSaveString (int whichcp, string key, unsigned int num, string val);
void putSaveData (int whichcp, string key, unsigned int num, float val);
unsigned int eraseSaveString (int whichcp, string key, unsigned int index);
vector <string> loadStringList (int playernum,string mykey);
void saveStringList (int playernum,string mykey,vector<string> names);
QVector DockToSavedBases(int playernum);
#endif
