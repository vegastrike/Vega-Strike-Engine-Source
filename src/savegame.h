//#include "cmd/script/mission.h"
#ifndef __SAVEGAME_H
#define __SAVEGAME_H

#include "gfx/vec.h"

// WARNING, SAVE FILES ARE LIMITED TO MAXBUFFER SIZE !!! (LOOK IN NETWORKING/CONST.H)

#include <string>
#include <vector>
  struct SavedUnits {
    std::string filename;
    int type;
    std::string faction;
    SavedUnits (const char * filen, int typ, const char * fact) {
      faction=fact;
      filename=filen;
      type = typ;
    }
  };
class MissionFloatDat;
class MissionStringDat;
class SaveGame {
  SaveGame(const SaveGame &){}//not used!
  std::string savestring;
  std::string ForceStarSystem ;
  QVector PlayerLocation;
  std::string last_pickled_data;
  std::string last_written_pickled_data;
  std::string outputsavegame;
  std::string originalsystem;
  std::string callsign;
  std::string WriteMissionData();
  void WriteMissionStringData(std::vector <char> &output);
  std::string WriteNewsData ();
  void ReadStardate(char * &buf);
  void ReadNewsData(char * &buf);
  void ReadMissionData (char * &buf);
  void ReadMissionStringData (char * &buf);
  MissionStringDat *missionstringdata;
  MissionFloatDat *missiondata;
  std::string playerfaction;
 public:

  ~SaveGame();
  void ReloadPickledData();
  std::string GetCallsign() {return callsign;}
  std::vector<float> &getMissionData(const std::string &magic_number);
  std::vector<std::string> &getMissionStringData(const std::string &magic_number);
  SaveGame(const std::string &pilotname);
  float GetSavedCredits();
  void SetSavedCredits (float);
  void SetPlayerLocation (const QVector &playerloc);
  QVector GetPlayerLocation ();
  void SetStarSystem (std::string sys);
  std::string GetStarSystem();
  std::string GetOldStarSystem();
  std::string GetPlayerFaction() { return playerfaction;}
  void	 SetPlayerFaction( std::string faction) { playerfaction = faction;}
  std::string WriteSavedUnit (SavedUnits *su);
  std::string WriteSaveGame (const char * systemname, const class QVector &Pos,float credits, std::vector <std::string> unitname, int player_num, std::string fact="", bool write=true);
  std::string WritePlayerData( const QVector &FP, std::vector<std::string> unitname, const char * systemname, float credits, std::string fact="");
  std::string WriteDynamicUniverse();
  void ReadSavedPackets (char * &buf);
  ///cast address to long (for 64 bits compatibility)
  void AddUnitToSave (const char * unitname, int type, const char * faction, long address);
  void RemoveUnitFromSave (long address);//cast it to a long
  //void ParseSaveGame (std::string filename, std::string &ForceStarSystem, std::string originalstarsystem, QVector & pos, bool &shouldupdatedfighter0pos, float &credits, std::vector <std::string> & originalunit, int player_num, char * savebuf=NULL, bool read=true);
  void ParseSaveGame (std::string filename, std::string &ForceStarSystem, std::string originalstarsystem, QVector & pos, bool &shouldupdatedfighter0pos, float &credits, std::vector <std::string> & originalunit, int player_num, std::string savestr="", bool read=true);
};
void WriteSaveGame (class Cockpit * cp, bool auto_save);
void CopySavedShips(std::string filename, int player_num,const std::vector<std::string>&starships,bool load);
#endif

