#ifndef _COCKPIT_GENERIC_H_
#define _COCKPIT_GENERIC_H_
#include "xml_support.h"
#include "cmd/container.h"
#include <vector>
#include "gfx/vec.h"
#include "vsfilesystem.h"
using namespace XMLSupport;
enum VIEWSTYLE {CP_FRONT,  CP_BACK,CP_LEFT, CP_RIGHT, CP_VIEWTARGET, CP_CHASE, CP_PAN,  CP_PANTARGET, CP_TARGET};
#ifdef NETCOMM_WEBCAM
#define MAXVDUS 13
#else
#define MAXVDUS 12
#endif
class Unit;
class Camera;
class Animation;
#include "in.h"
#include "cmd/images.h"
/**
 * The Cockpit Contains all displayable information about a particular Unit *
 * Gauges are used to indicate analog controls, and some diagital ones
 * The ones starting from KPS are digital with text readout
 */
class Cockpit {
protected:
  ///style of current view (chase cam, inside)
  enum VIEWSTYLE view;
  int currentcamera;
  float radar_time;
  float gauge_time [UnitImages::NUMGAUGES];
  /// 8 armor vals and 1 for startfuel

    /// 8 armor vals and 1 for startfuel
  float StartArmor[9]; //short fix

  ///saved values to compare with current values (might need more for damage)
  float maxfuel, maxhull;
  ///this is the parent that Cockpit will read data from
  UnitContainer parent; 
  UnitContainer parentturret; 
  int unitfaction;
  ///4 views f/r/l/b
  float shakin;
  ///Video Display Units (may need more than 2 in future)
  std::string unitmodname;
  ///Color of cockpit default text
  ///The font that the entire cockpit will use. Currently without color
  //Gauge *gauges[UnitImages::NUMGAUGES];
  //0 means no autopilot...positive autopilto in progress
  float autopilot_time;
  UnitContainer autopilot_target;//usually null
  /**  
   * two values that represent the adjustment to perspective needed to center teh crosshairs in the perceived view.
   */
  float cockpit_offset, viewport_offset;
  ///style of current view (chase cam, inside)
  //enum VIEWSTYLE view;
  virtual void LocalToEliteRadar (const Vector & pos, float &s, float &t,float &h) {}
  virtual void LocalToRadar (const Vector & pos, float &s, float &t) {}

  virtual void LoadXML (const char *file) {}
  virtual void LoadXML (VSFileSystem::VSFile & f) {}
  static void beginElement(void *userData, const XML_Char *name, const XML_Char **atts);
  static void endElement(void *userData, const XML_Char *name);
  virtual void beginElement(const string &name, const AttributeList &attributes) {}
  virtual void endElement(const string &name) {}
  ///Destructs cockpit info for new loading
  virtual void Delete();
  ///draws the navigation symbol around targetted location
  virtual void DrawNavigationSymbol (const Vector &loc, const Vector &p, const Vector &q, float size) {}
  ///draws the target box around targetted unit
  virtual float computeLockingSymbol(Unit * par) { return 1;}
  virtual void DrawTargetBox () {}
  ///draws the target box around all units
  virtual void DrawTargetBoxes () {}
  ///draws a target cross around all units targeted by your turrets // ** jay
  virtual void DrawTurretTargetBoxes () {}
  ///Draws all teh blips on the radar.
  virtual void DrawBlips(Unit * un) {}
  ///Draws all teh blips on the radar in Elite-style
  virtual void DrawEliteBlips(Unit * un) {}
  ///Draws gauges
  virtual void DrawGauges(Unit * un) {}
  float cockpit_time;
  bool ejecting;
 public:
  double TimeOfLastCollision;
  char jumpok;
  virtual void	ReceivedTargetInfo() {}
  std::vector<std::string> unitfilename;
  ///Sets the current viewstyle
  void SetView (const enum VIEWSTYLE tmp) {view = tmp;}
  enum VIEWSTYLE GetView () {return view;}
  virtual void InitStatic ();
  virtual void Shake (float amt) {}
  float godliness;
  virtual int Autopilot (Unit * target);
  void RestoreGodliness();
 ///Restores the view from the IDentity Matrix needed to draw sprites
  virtual void RestoreViewPort() {}
  std::string& GetUnitFileName () {if ( unitfilename.empty()) unitfilename.push_back(""); return unitfilename.front();}
  std::string GetUnitModifications() {return unitmodname;}
  std::string communication_choices;
  float credits;//how much money player has
  ///How far away chasecam and pan cam is
  float zoomfactor;
  Cockpit (const char * file, Unit * parent, const std::string &pilotname);
  virtual ~Cockpit();
  ///Looks up a particular Gauge stat on target unit
  virtual float LookupTargetStat (int stat, Unit *target) {return 1;}
  ///Loads cockpit info...just as constructor
	virtual void Init (const char * file, bool defaultCockpit=false);
  ///Sets owner of this cockpit
  //  unsigned int whichcockpit;//0 is the first player, 1 is the second and so forth
  class Flightgroup * fg;
  class StarSystem* activeStarSystem;//used for context switch in Universe
  void SetParent(Unit * unit, const char * filename, const char * unitmodname,const QVector &startloc);
  Unit * GetParent () {return parent.GetUnit();}
  Unit * GetSaveParent ();
  ///Draws Cockpit then restores viewport
  virtual void Draw() {}
  void Update(); //respawns and the like.
  virtual void UpdAutoPilot();
  ///Sets up the world for rendering...call before draw
  virtual void SetupViewPort (bool clip=true) {}
  virtual void VDUSwitch (int vdunum) {}
  virtual void ScrollVDU (int vdunum, int howmuch) {}
  virtual void ScrollAllVDU (int howmuch) {}
  virtual int getScrollOffset (unsigned int whichtype) { return 1;}
  virtual void SelectProperCamera () {}
  virtual void Eject ();
  static void Respawn (int,KBSTATE) {}
  static void SwitchControl (int,KBSTATE) {}
  static void TurretControl (int, KBSTATE) {}
  virtual void SetCommAnimation (Animation * ani) {}
  class SaveGame * savegame;
  ///Accesses the current camera
  virtual Camera *AccessCamera() {return NULL;}
  ///Returns the passed in cam
  virtual Camera *AccessCamera(int) {return NULL;}
  ///Changes current camera to selected camera
  virtual void SelectCamera(int) {}
  ///GFXLoadMatrix proper camera
  virtual void SetViewport() {}
  virtual bool SetDrawNavSystem(bool){return false;}
  virtual bool CanDrawNavSystem() {return false;}
  virtual bool DrawNavSystem() {return false;}
};
#endif
