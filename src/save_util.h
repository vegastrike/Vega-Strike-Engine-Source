#ifndef __SAVE_UTIL_H__
#define __SAVE_UTIL_H__
extern const char *mission_key;
float getSaveData (int whichcp, string key, unsigned int num);
unsigned int pushSaveData (int whichcp, string key, float val);
unsigned int getSaveDataLength (int whichcp, string key);
void putSaveData (int whichcp, string key, unsigned int num, float val);
vector <string> loadStringList (int playernum,string mykey);
void saveStringList (int playernum,string mykey,vector<string> names);
void DockToSavedBases(int playernum);
#endif
