#ifndef _CMD_AISCRIPT_H_
#define _CMD_AISCRIPT_H_
#include "cmd_order.h"
#include "cmd_navigation_orders.h"
#include <stack>
using std::stack;
class AIScript : public Order {
  char * filename;
  struct AIXML {
	  int unitlevel;
	  int acc;
	  int executefor;
	  bool afterburn;
	  char lin;
	  Vector defaultvec;
	  float defaultf;
	  stack <Vector> vectors;
	  stack <float> floats;
	  vector <Order *> orders;
  } *xml;
  void LoadXML(); //load the xml
  static void beginElement(void *userData, const XML_Char *name, const XML_Char **atts);
  static void endElement(void *userData, const XML_Char *name);
  float& topf();
  void popf();
  Vector& topv();
  void popv ();
  void beginElement(const string &name, const AttributeList &attributes);
  void endElement(const string &name);
public:
  AIScript(const char * scriptname);
  ~AIScript();
  void Execute();
};
#endif
