void LoadMission (const char *, bool loadfirst);
void UnpickleMission(std::string pickled);
std::string PickleAllMissions ();
std::string UnpickleAllMissions (FILE *);
std::string PickledDataSansMissionName (std::string file);
