#ifndef GALAXY_XML_H_
#define GALAXY_XML_H_

#include <string>
#include <map>
namespace GalaxyXML {
	class SubHeirarchy;
class Galaxy {
  class SubHeirarchy * subheirarchy;
  class std::map <std::string,std::string> data;
	Galaxy & operator = (const Galaxy & a);
 public:
  Galaxy () {subheirarchy=NULL;}
  Galaxy(const char *configfile);
 
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
  std::map <std::string, class Galaxy> & getHeirarchy();
};
class SubHeirarchy : public std::map <std::string,class Galaxy> {};
}
#endif
