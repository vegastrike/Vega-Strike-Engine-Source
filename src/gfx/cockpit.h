#ifndef _COCKPIT_H_
#define _COCKPIT_H_
#include "xml_support.h"
#include "cmd/container.h"
using namespace XMLSupport;
class Sprite;
class Unit;
enum VIEWSTYLE {CP_FRONT, CP_LEFT, CP_RIGHT, CP_BACK, CP_CHASE, CP_PAN};
class Cockpit {
  UnitContainer parent;
  Sprite *Pit [4];
  Sprite *Radar;
  Sprite *VDU [2];
  Sprite *Shield[2];
  Sprite *Crosshairs;//have a bunch of these...like radar
  float cockpit_offset;
  float viewport_offset;
  void RestoreViewPort();
  enum VIEWSTYLE view;
  void LoadXML (const char *file);
  static void beginElement(void *userData, const XML_Char *name, const XML_Char **atts);
  static void endElement(void *userData, const XML_Char *name);
  void beginElement(const string &name, const AttributeList &attributes);
  void endElement(const string &name);
  void Delete();
  void DrawBlips(Unit * un);
 public:
  float zoomfactor;
  Cockpit (const char * file, Unit * parent);
  ~Cockpit();
  void Init (const char * file);
  void SetParent(Unit * unit);
  void Draw();//restores viewport
  void SetView (const enum VIEWSTYLE tmp);
  void SetupViewPort ();
};
#endif
