#ifndef _COCKPIT_H_
#define _COCKPIT_H_
#include "xml_support.h"
#include "cmd/container.h"
#include "gfxlib.h"
#include "gfxlib_struct.h"
#include <vector>
using namespace XMLSupport;
class TextPlane;
class Sprite;
class Gauge;
class Unit;
class VDU;
enum VIEWSTYLE {CP_FRONT, CP_LEFT, CP_RIGHT, CP_BACK, CP_CHASE, CP_PAN};

/**
 * The Cockpit Contains all displayable information about a particular Unit *
 * Gauges are used to indicate analog controls, and some diagital ones
 * The ones starting from KPS are digital with text readout
 */
class Cockpit {
  enum GAGUES {ARMORF,ARMORR,ARMORL,ARMORB,FUEL, SHIELDF,SHIELDR,SHIELDL,SHIELDB, ENERGY, EJECT, LOCK, HULL, KPS, SETKPS, NUMGAUGES};
  /// 4 armor vals and 1 for startfuel
  unsigned short StartArmor[ARMORB+1];
  ///saved values to compare with current values (might need more for damage)
  float maxfuel, maxhull;
  ///this is the parent that Cockpit will read data from
  UnitContainer parent; 
  ///4 views f/r/l/b
  Sprite *Pit [4];
  Sprite *Radar;
  ///Video Display Units (may need more than 2 in future)
  VDU *vdu [2];
  ///Color of cockpit default text
  GFXColor textcol;
  ///The font that the entire cockpit will use. Currently without color
  TextPlane *text;
  Gauge *gauges[NUMGAUGES];
  ///holds misc panels.  Panel[0] is always crosshairs (and adjusted to be in center of view screen, not cockpit)
  std::vector <Sprite *> Panel;
  /**  
   * two values that represent the adjustment to perspective needed to center teh crosshairs in the perceived view.
   */
  float cockpit_offset, viewport_offset;
  ///Restores the view from the IDentity Matrix needed to draw sprites
  void RestoreViewPort();
  ///style of current view (chase cam, inside)
  enum VIEWSTYLE view;
  void LoadXML (const char *file);
  static void beginElement(void *userData, const XML_Char *name, const XML_Char **atts);
  static void endElement(void *userData, const XML_Char *name);
  void beginElement(const string &name, const AttributeList &attributes);
  void endElement(const string &name);
  ///Destructs cockpit info for new loading
  void Delete();
  ///draws the target box around targetted unit
  void DrawTargetBox ();
  ///Draws all teh blips on the radar. No color info (yet)
  void DrawBlips(Unit * un);
  ///Draws gauges
  void DrawGauges(Unit * un);
 public:
  ///How far away chasecam and pan cam is
  float zoomfactor;
  Cockpit (const char * file, Unit * parent);
  ~Cockpit();
  ///Looks up a particular Gauge stat on target unit
  float LookupTargetStat (int stat, Unit *target);
  ///Loads cockpit info...just as constructor
  void Init (const char * file);
  ///Sets owner of this cockpit
  void SetParent(Unit * unit);
  ///Draws Cockpit then restores viewport
  void Draw();
  ///Sets the current viewstyle
  void SetView (const enum VIEWSTYLE tmp);
  ///Sets up the world for rendering...call before draw
  void SetupViewPort ();
  void VDUSwitch (int vdunum);
};
#endif
