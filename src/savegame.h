//#include "cmd/script/mission.h"
#ifndef __SAVEGAME_H
#define __SAVEGAME_H

#include <vector>
typedef vector<class varInst *> olist_t;
  struct SavedUnits {
    string filename;
    clsptr type;
    string faction;
    SavedUnits (const char * filen, enum clsptr typ, const char * fact) {
      faction=fact;
      filename=filen;
      type = typ;
    }
  };
class SaveGame {
  struct MissionDat {
    olist_t dat;
    std::string magic_number;
    bool operator == (const MissionDat&i) {
      return (magic_number==i.magic_number);
    }
    MissionDat (const std::string &magic_number) {
      this->magic_number = magic_number;
    }
  };
  std::string ForceStarSystem ;
  QVector PlayerLocation;
  std::string last_pickled_data;
  std::string last_written_pickled_data;
  std::string outputsavegame;
  std::string originalsystem;
  static Hashtable<long,SavedUnits,char[47]> *savedunits;
  std::string callsign;
  void WriteMissionData(FILE * fp);
  void WriteNewsData (FILE * fp);
  void ReadNewsData(FILE * fp);
  void ReadMissionData (FILE * fp);
  vector <SavedUnits> ReadSavedUnits (FILE * fp);
  vector <MissionDat> mission_data;
 public:
  void ReloadPickledData();
  olist_t &getMissionData(const std::string &magic_number);
  SaveGame(const std::string &pilotname);
  float GetSavedCredits();
  void SetSavedCredits (float);
  void SetPlayerLocation (const QVector &playerloc);
  QVector GetPlayerLocation ();
  void SetStarSystem (string sys);
  string GetStarSystem();
  void WriteSavedUnit (FILE *, SavedUnits *su);
  void WriteSaveGame (const char * systemname, const class QVector &Pos,float credits, std::vector <std::string> unitname, int player_num);
  ///cast address to long (for 64 bits compatibility)
  void AddUnitToSave (const char * unitname, enum clsptr type, const char * faction, long address);
  void RemoveUnitFromSave (long address);//cast it to an long
  vector<SavedUnits> ParseSaveGame (string filename, string &ForceStarSystem, string originalstarsystem, QVector & pos, bool &shouldupdatedfighter0pos, float &credits, std::vector <std::string> & originalunit, int player_num);
};
void WriteSaveGame (class Cockpit * cp, bool auto_save);

#endif

