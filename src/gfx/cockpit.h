#ifndef _COCKPIT_H_
#define _COCKPIT_H_
#include "xml_support.h"
#include "cmd/container.h"
#include <vector>
using namespace XMLSupport;
class Sprite;
class Gauge;
class Unit;
enum VIEWSTYLE {CP_FRONT, CP_LEFT, CP_RIGHT, CP_BACK, CP_CHASE, CP_PAN};
class Cockpit {
  enum GAGUES {ARMORF,ARMORR,ARMORL,ARMORB,FUEL, SHIELDF,SHIELDR,SHIELDL,SHIELDB, ENERGY, NUMGAUGES};
  unsigned short StartArmor[ARMORB+1];//and startfuel
  float maxfuel;
  UnitContainer parent;
  Sprite *Pit [4];
  Sprite *Radar;
  Sprite *VDU [2];
  Gauge *gauges[NUMGAUGES];
  std::vector <Sprite *> Panel;
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
  void DrawTargetBox ();
  void DrawBlips(Unit * un);
  void DrawGauges(Unit * un);
 public:
  float zoomfactor;
  Cockpit (const char * file, Unit * parent);
  ~Cockpit();
  float LookupTargetStat (int stat, Unit *target);
  void Init (const char * file);
  void SetParent(Unit * unit);
  void Draw();//restores viewport
  void SetView (const enum VIEWSTYLE tmp);
  void SetupViewPort ();
};
#endif
