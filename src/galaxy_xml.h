#ifndef GALAXY_XML_H_
#define GALAXY_XML_H_

#include <expat.h>
#include <string>
#include <map>
#include "xml_support.h"
#include "easydom.h"
#include "in_kb.h"
namespace GalaxyXML {
  using std::string;
  
  using XMLSupport::AttributeList;
  
  
  class configNode : public easyDomNode {
  public:
    string name;
  };

  enum section_t { SECTION_VAR };
  
  class configNodeFactory : public easyDomFactory<configNode> {
  };

class Galaxy {
 public:
  Galaxy(const char *configfile);

  string getRandSystem(string section,string default_value);
  string getVariable(string section,string name,string defaultvalue);
  string getVariable(string section,string subsection,string name,string defaultvalue);
  configNode *findSection(string section,configNode *startnode);
  configNode *findEntry(string name,configNode *startnode);

  void setVariable(configNode *entry,string value);
  bool setVariable(string section,string name,string value);
  bool setVariable(string section,string subsection,string name,string value);


  easyDomNode *Variables() { return variables; };

 private:
  string getVariable(configNode *section,string name,string defaultval);

  configNode *variables;

  int hs_value_index;

  bool checkConfig(configNode *node);


  void checkSection(configNode *node,enum section_t section_type);
  void checkVar(configNode *node);
  void doSection(configNode *node,enum section_t section_type);
  void doVar(configNode *node);
  void doVariables(configNode *node);
};

}
#endif
