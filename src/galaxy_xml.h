#ifndef GALAXY_XML_H_
#define GALAXY_XML_H_

#include <string>
#include <map>
#include "gfx/vec.h"
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
  StringMap data;
	Galaxy & operator = (const Galaxy & a);
 public:
  Galaxy () {subheirarchy=NULL;}
  Galaxy(const char *configfile);
  void writeGalaxy(const char * filename);
  void writeSector (FILE * fp, int tabs);
  void processGalaxy(string sys);
  void processSystem(string sys,const QVector &suggested_coordinates);
  Galaxy( const Galaxy & g);
  ~Galaxy();
  string getVariable(std::vector<string> section, string name, string default_value);
  string getRandSystem(string section,string default_value);
  string getVariable(string section,string name,string defaultvalue);
  string getVariable(string section,string subsection,string name,string defaultvalue);
  bool setVariable(string section,string name,string value);
  bool setVariable(string section,string subsection,string name,string value);
  void addSection(std::vector<string> section);
  void setVariable (std::vector<string> section, string name, string value);
  SubHeirarchy & getHeirarchy();
};
class SubHeirarchy : public std::map <std::string,class Galaxy> {};
}
#endif
