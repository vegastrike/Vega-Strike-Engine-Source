void LoadMission (const char *, bool loadfirst);
void delayLoadMission (std::string);
void processDelayedMissions();
void UnpickleMission(std::string pickled);
std::string PickleAllMissions ();
std::string UnpickleAllMissions (FILE *);
std::string UnpickleAllMissions (char * &buf);
std::string PickledDataSansMissionName (std::string file);
