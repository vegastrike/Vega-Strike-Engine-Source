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
#include "vdu.h"


/**
 * The Cockpit Contains all displayable information about a particular Unit *
 * Gauges are used to indicate analog controls, and some diagital ones
 * The ones starting from KPS are digital with text readout
 */
class Cockpit {
  enum GAGUES {ARMORF,ARMORB,ARMORR,ARMORL,FUEL, SHIELDF,SHIELDR,SHIELDL,SHIELDB, ENERGY, EJECT, LOCK, HULL, KPS, SETKPS, FPS, NUMGAUGES};
  /// 4 armor vals and 1 for startfuel
  unsigned short StartArmor[5];
  ///saved values to compare with current values (might need more for damage)
  float maxfuel, maxhull;
  ///this is the parent that Cockpit will read data from
  UnitContainer parent; 
  ///4 views f/r/l/b
  Sprite *Pit [4];
  Sprite *Radar;
  ///Video Display Units (may need more than 2 in future)
  std::vector <VDU *> vdu;
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
  ///flag to decide whether to draw all target boxes
  bool draw_all_boxes;
  bool draw_line_to_target,draw_line_to_targets_target;
  bool draw_line_to_itts;
  ///flag to tell wheter to draw the itts, even if the ship has none
  bool always_itts;
  // colors of blips/targetting boxes
  GFXColor friendly,enemy,neutral,targeted,targetting,planet;
  // gets the color by relation
  GFXColor relationToColor (float relation);
  // gets the color by looking closer at the unit
  GFXColor unitToColor (Unit *un,Unit *target);
  // the style of the radar (WC|Elite)
  string radar_type;
  void LocalToEliteRadar (const Vector & pos, float &s, float &t,float &h);
  void LocalToRadar (const Vector & pos, float &s, float &t);

  void LoadXML (const char *file);
  static void beginElement(void *userData, const XML_Char *name, const XML_Char **atts);
  static void endElement(void *userData, const XML_Char *name);
  void beginElement(const string &name, const AttributeList &attributes);
  void endElement(const string &name);
  ///Destructs cockpit info for new loading
  void Delete();
  ///draws the navigation symbol around targetted location
  void DrawNavigationSymbol (const Vector &loc, const Vector &p, const Vector &q, float size);
  ///draws the target box around targetted unit
  void DrawTargetBox ();
  ///draws the target box around all units
  void DrawTargetBoxes ();
  ///Draws all teh blips on the radar.
  void DrawBlips(Unit * un);
  ///Draws all teh blips on the radar in Elite-style
  void DrawEliteBlips(Unit * un);
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
  Unit * GetParent () {return parent.GetUnit();}
  ///Draws Cockpit then restores viewport
  void Draw();
  ///Sets the current viewstyle
  void SetView (const enum VIEWSTYLE tmp);
  ///Sets up the world for rendering...call before draw
  void SetupViewPort (bool clip=true);
  void VDUSwitch (int vdunum);
  void ScrollVDU (int vdunum, int howmuch);
  void ScrollAllVDU (int howmuch);
  void SelectProperCamera ();
};
#endif
