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
#include "in_kb.h"
#include "camera.h"
#define MAXVDUS 10
#define NUM_CAM		12
/**
 * The Cockpit Contains all displayable information about a particular Unit *
 * Gauges are used to indicate analog controls, and some diagital ones
 * The ones starting from KPS are digital with text readout
 */
class Cockpit {
public:
  enum GAGUES {ARMORF,ARMORB,ARMORR,ARMORL,FUEL, SHIELDF,SHIELDR,SHIELDL,SHIELDB, ENERGY, AUTOPILOT,EJECT, LOCK, HULL, KPS, SETKPS,  FPS, NUMGAUGES};
private:
  Camera cam[NUM_CAM];
  int currentcamera;
  float radar_time;
  float gauge_time [NUMGAUGES];
  float vdu_time [MAXVDUS];
  /// 4 armor vals and 1 for startfuel
  unsigned short StartArmor[5];
  ///saved values to compare with current values (might need more for damage)
  float maxfuel, maxhull;
  ///this is the parent that Cockpit will read data from
  UnitContainer parent; 
  UnitContainer parentturret; 
  int unitfaction;
  ///4 views f/r/l/b
  struct MyMat {
    float m[16];
  };
  float shakin;
  std::list <MyMat> headtrans;
  class Mesh * mesh;
  Sprite *Pit [4];
  Sprite *Radar;
  ///Video Display Units (may need more than 2 in future)
  std::vector <VDU *> vdu;
  std::string unitfilename;
  std::string unitmodname;
  Vector unitlocation;
  ///Color of cockpit default text
  GFXColor textcol;
  ///The font that the entire cockpit will use. Currently without color
  TextPlane *text;
  Gauge *gauges[NUMGAUGES];
  ///holds misc panels.  Panel[0] is always crosshairs (and adjusted to be in center of view screen, not cockpit)
  std::vector <Sprite *> Panel;
  //0 means no autopilot...positive autopilto in progress
  float autopilot_time;
  UnitContainer autopilot_target;//usually null
  /**  
   * two values that represent the adjustment to perspective needed to center teh crosshairs in the perceived view.
   */
  float cockpit_offset, viewport_offset;
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
  float computeLockingSymbol(Unit * par);
  void DrawTargetBox ();
  ///draws the target box around all units
  void DrawTargetBoxes ();
  ///Draws all teh blips on the radar.
  void DrawBlips(Unit * un);
  ///Draws all teh blips on the radar in Elite-style
  void DrawEliteBlips(Unit * un);
  ///Draws gauges
  void DrawGauges(Unit * un);
  float cockpit_time;
  bool ejecting;
 public:
  void Shake (float amt);
  float godliness;
  void Autopilot (Unit * target);
  void RestoreGodliness();
 ///Restores the view from the IDentity Matrix needed to draw sprites
  void RestoreViewPort();
  std::string GetUnitFileName () {return unitfilename;}
  std::string GetUnitModifications() {return unitmodname;}
  std::string communication_choices;
  float credits;//how much money player has
  ///How far away chasecam and pan cam is
  float zoomfactor;
  Cockpit (const char * file, Unit * parent, const std::string &pilotname);
  ~Cockpit();
  ///Looks up a particular Gauge stat on target unit
  float LookupTargetStat (int stat, Unit *target);
  ///Loads cockpit info...just as constructor
  void Init (const char * file);
  ///Sets owner of this cockpit
  //  unsigned int whichcockpit;//0 is the first player, 1 is the second and so forth
  class StarSystem* activeStarSystem;//used for context switch in Universe
  void SetParent(Unit * unit, const char * filename, const char * unitmodname,const Vector &startloc);
  Unit * GetParent () {return parent.GetUnit();}
  ///Draws Cockpit then restores viewport
  void Draw();
  void Update();//respawns and the like.
  ///Sets the current viewstyle
  void SetView (const enum VIEWSTYLE tmp);
  enum VIEWSTYLE GetView () {return view;}
  ///Sets up the world for rendering...call before draw
  void SetupViewPort (bool clip=true);
  void VDUSwitch (int vdunum);
  void ScrollVDU (int vdunum, int howmuch);
  void ScrollAllVDU (int howmuch);
  int getScrollOffset (unsigned int whichtype);
  void SelectProperCamera ();
  void Eject ();
  static void Respawn (int,KBSTATE);
  static void SwitchControl (int,KBSTATE);
  static void TurretControl (int, KBSTATE);
  void SetCommAnimation (Animation * ani);
  class SaveGame * savegame;
  ///Accesses the current camera
  Camera *AccessCamera() {return &cam[currentcamera];}
  ///Returns the passed in cam
  Camera *AccessCamera(int);
  ///Changes current camera to selected camera
  void SelectCamera(int);
  ///GFXLoadMatrix proper camera
  void SetViewport() {
    cam[currentcamera].UpdateGFX();
  }

};
#endif
