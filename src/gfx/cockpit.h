#ifndef _COCKPIT_H_
#define _COCKPIT_H_
#include "xml_support.h"
#include "cmd/container.h"
using namespace XMLSupport;
class Sprite;
class Unit;
class Cockpit {
  UnitContainer parent;
  Sprite *Pit;
  Sprite *Crosshairs;//have a bunch of these...like radar
  float cockpit_offset;
  float viewport_offset;
  void RestoreViewPort();
  void LoadXML (const char *file);
  static void beginElement(void *userData, const XML_Char *name, const XML_Char **atts);
  static void endElement(void *userData, const XML_Char *name);
  void beginElement(const string &name, const AttributeList &attributes);
  void endElement(const string &name);
 public:
  Cockpit (const char * file, Unit * parent);
  ~Cockpit();
  void Draw();//restores viewport
  void SetupViewPort ();
};
#endif
