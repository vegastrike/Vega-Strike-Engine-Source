#ifndef _STARSYSGEN_H_
#define _STARSYSGEN_H_
#include <vector>
#include <string>
using std::string;
using std::vector;

/// All the properties from the galaxy in a system.
struct SystemInfo {
  string sector;
  string name;
  string filename;
  float sunradius;
  float compactness;
  int numstars;
  bool nebulae;
  bool asteroids;
  int numun1;
  int numun2;
  string faction;
  string names;
  string stars;
  string planetlist;
  string smallun;
  string nebulaelist;
  string asteroidslist;
  string ringlist;
  string backgrounds;
  vector <string> jumps;
  int seed;
  bool force;
};

///appends .system
std::string getStarSystemFileName (std::string input);
///finds the name after all / characters and capitalizes the first letter
std::string getStarSystemName (std::string in);
///finds the name before the first /  this is the sector name
std::string getStarSystemSector (std::string in);
string getUniversePath ();
void readnames (vector <string> &entity, const char *filename);
void generateStarSystem (SystemInfo &si);
#endif
