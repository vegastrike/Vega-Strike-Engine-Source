float GetSavedCredits();
void SetSavedCredits (float);
void SetPlayerLocation (const Vector &playerloc);
Vector GetPlayerLocation ();
void SetStarSystem (string sys);
string GetStarSystem();
void WriteSaveGame (const char * systemname, const class Vector &Pos,float credits, std::string unitname);
///cast address to int
void AddUnitToSave (const char * unitname, enum clsptr type, const char * faction, int address);
void RemoveUnitFromSave (int address);//cast it to an int
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
vector<SavedUnits> ParseSaveGame (const string filename, string &ForceStarSystem, string originalstarsystem, Vector & pos, bool &shouldupdatedfighter0pos, float &credits, string &originalunit);
