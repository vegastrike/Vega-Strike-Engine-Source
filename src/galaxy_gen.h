#ifndef _STARSYSGEN_H_
#define _STARSYSGEN_H_
#include <vector>
#include <string>
///appends .system
std::string getStarSystemFileName (std::string input);
///finds the name after all / characters and capitalizes the first letter
std::string getStarSystemName (std::string in);
///finds the name before the first /  this is the sector name
std::string getStarSystemSector (std::string in);
string getUniversePath ();
void readnames (vector <string> &entity, const char *filename);
void generateStarSystem (std::string datapath, int seed, std::string sector, std::string system, std::string outputfile, float sunradius, float compactness, int numstars, int numgasgiants, int numrockyplanets, int nummoons, bool nebulae, bool asteroids, int numnaturalphenomena, int numstarbases, std::string factions, std::string namelist, const std::vector <std::string> &jumplocations);

#endif
