#ifndef _VEGACONFIG_H_
#define _VEGACONFIG_H_

#include <expat.h>
#include <string>
#include "xml_support.h"

using std::string;

using XMLSupport::AttributeList;

class VegaConfig {
 public:
  VegaConfig(char *configfile);

  void getColor(char *name, float color[4]);
  char *getVariable(char *section,char *name);

  struct VegaConfigXML {
  } *xml;

  void LoadXML(const char*);
  static void beginElement(void *userData, const XML_Char *name, const XML_Char **atts);
  static void endElement(void *userData, const XML_Char *name);

  void beginElement(const string &name, const AttributeList &attributes);
  void endElement(const string &name);

};

#endif // _VEGACONFIG_H_
