void LoadMission (const char *, const std::string &scriptname,bool loadfirst);
void delayLoadMission (std::string missionfile);
void delayLoadMission (std::string missionfile, string script);
void processDelayedMissions();
void UnpickleMission(std::string pickled);
std::string PickleAllMissions ();
std::string UnpickleAllMissions (FILE *);
std::string UnpickleAllMissions (char * &buf);
std::string PickledDataSansMissionName (std::string file);
