#ifndef _CMD_AISCRIPT_H_
#define _CMD_AISCRIPT_H_
#include "cmd_order.h"
#include "cmd_navigation_orders.h"
class AIScript : public Order {
  char * filename;
  struct AIXML {
	  int unitlevel;
	  int acc;
	  Vector vec;
	  Vector ang;
	  int executefor;
	  bool afterburn;
	  vector <Order *> orders;
  } *xml;
  void LoadXML(); //load the xml
  static void beginElement(void *userData, const XML_Char *name, const XML_Char **atts);
  static void endElement(void *userData, const XML_Char *name);

  void beginElement(const string &name, const AttributeList &attributes);
  void endElement(const string &name);
public:
  AIScript(const char * scriptname);
  ~AIScript();
  AI *Execute();
};
#endif