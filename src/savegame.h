#include "cmd/script/mission.h"
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
    float magic_number;
    bool operator == (const MissionDat&i) {
      float eps=.00001;
      return fabs (magic_number-i.magic_number)<eps;
    }
    MissionDat (float magic_number) {
      this->magic_number = magic_number;
    }
  };
  std::string ForceStarSystem ;
  Vector PlayerLocation;
  std::string outputsavegame;
  std::string originalsystem;
  static Hashtable<int,SavedUnits,char[47]> savedunits;
  std::string callsign;
  void WriteMissionData(FILE * fp);
  void WriteNewsData (FILE * fp);
  void ReadNewsData(FILE * fp);
  void ReadMissionData (FILE * fp);
  vector <SavedUnits> ReadSavedUnits (FILE * fp);
  vector <MissionDat> mission_data;
 public:
  olist_t &getMissionData(float magic_number);
  SaveGame(const std::string &pilotname);
  float GetSavedCredits();
  void SetSavedCredits (float);
  void SetPlayerLocation (const Vector &playerloc);
  Vector GetPlayerLocation ();
  void SetStarSystem (string sys);
  string GetStarSystem();
  void WriteSavedUnit (FILE *, SavedUnits *su);
  void WriteSaveGame (const char * systemname, const class Vector &Pos,float credits, std::string unitname);
  ///cast address to int
  void AddUnitToSave (const char * unitname, enum clsptr type, const char * faction, int address);
  void RemoveUnitFromSave (int address);//cast it to an int
  vector<SavedUnits> ParseSaveGame (string filename, string &ForceStarSystem, string originalstarsystem, Vector & pos, bool &shouldupdatedfighter0pos, float &credits, string &originalunit);
};
void WriteSaveGame (class Cockpit * cp);
