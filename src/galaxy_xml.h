#ifndef GALAXY_XML_H_
#define GALAXY_XML_H_

#include <string>
#include <map>
#include <vector>
#include "gfx/vec.h"
#include "vsfilesystem.h"

void ComputeSerials( std::vector<std::string> & stak);

namespace GalaxyXML {
using std::string;
class StringWrapper : public string {
public:
	StringWrapper() {}
	StringWrapper(const string &s) : string(s) {}
};
class StringMap : public std::map<StringWrapper, StringWrapper> {};
class SubHeirarchy;
class Galaxy {
	class SubHeirarchy * subheirarchy;
	Galaxy *planet_types; // will be null unless top level
  StringMap data;
	Galaxy & operator = (const Galaxy & a);

 public:
  Galaxy () {subheirarchy=NULL;planet_types=NULL;}
  Galaxy(const char *configfile);
  void writeGalaxy(VSFileSystem::VSFile &f);
	void writeSector (VSFileSystem::VSFile & f, int tabs, string sectorType="sector");
  void processGalaxy(string sys);
  void processSystem(string sys,const QVector &suggested_coordinates);
  Galaxy( const Galaxy & g);
  ~Galaxy();
  string getVariable(std::vector<string> section, string name, string default_value);
  string getRandSystem(string section,string default_value);
  string getVariable(string section,string name,string defaultvalue);
  string getVariable(string section,string subsection,string name,string defaultvalue);
  string getPlanetVariable(string name,string defaultvalue);
  string getPlanetVariable(string planet,string name,string defaultvalue);
  bool setVariable(string section,string name,string value);
  bool setVariable(string section,string subsection,string name,string value);
  bool setPlanetVariable(string name,string value);
  bool setPlanetVariable(string planet,string name,string value);
  void addSection(std::vector<string> section);
  void addPlanetSection(std::vector<string> section);
  void setVariable (std::vector<string> section, string name, string value);
  SubHeirarchy & getHeirarchy();
  Galaxy * getPlanetTypes();
  std::string operator [](const std::string &s) {
	  return data[StringWrapper(s)];
  }
};
class SubHeirarchy : public std::map <std::string,class Galaxy> {};

}

string getStarSystemFileName (string input);
string getStarSystemName (string in);
string getStarSystemSector (string in);
#endif
