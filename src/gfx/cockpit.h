#ifndef _COCKPIT_H_
#define _COCKPIT_H_
#include "gfx/cockpit_generic.h"
#include "gfxlib.h"
#include "gfxlib_struct.h"
#include <vector>
#include <list>
using namespace XMLSupport;
class TextPlane;
class VSSprite;
class Gauge;
class Unit;
class NavigationSystem;
#include "vdu.h"
#include "camera.h"
#include "nav/navscreen.h"
#define NUM_CAM		12
/**
 * The Cockpit Contains all displayable information about a particular Unit *
 * Gauges are used to indicate analog controls, and some diagital ones
 * The ones starting from KPS are digital with text readout
 */

struct soundContainer  {//used to contain static sounds that will only be
                        //created once and will get deleted automatically
	int sound;
	soundContainer () {
		sound=-2;
	}
	void loadsound (string sooundfile,bool looping=false);
	void playsound ();
	~soundContainer ();
};

struct soundArray {
	soundContainer *ptr;
	soundArray() {ptr=NULL;}
	void deallocate () {
		if (ptr!=NULL) {
			delete []ptr;
			ptr=NULL;
		}
	}
	void allocate (int siz) {
		deallocate();
		ptr=new soundContainer[siz];
	}
	~soundArray() {deallocate();}
};

class GameCockpit: public Cockpit {
private:
  Camera cam[NUM_CAM];
  float vdu_time [MAXVDUS];
  ///saved values to compare with current values (might need more for damage)
  std::list <Matrix> headtrans;
  float shake_time;
  int shake_type;
  Vector oaccel;

  float last_locktime;
  float last_mlocktime;

  std::vector<class Mesh *> mesh;

  int soundfile;
  VSSprite *Pit [4];
  VSSprite *Radar[2];
  ///Video Display Units (may need more than 2 in future)
  std::vector <VDU *> vdu;
  ///Color of cockpit default text
  GFXColor textcol;
  ///The font that the entire cockpit will use. Currently without color
  TextPlane *text;
  Gauge *gauges[UnitImages::NUMGAUGES];
  ///holds misc panels.  Panel[0] is always crosshairs (and adjusted to be in center of view screen, not cockpit)
  std::vector <VSSprite *> Panel;
  ///flag to decide whether to draw all target boxes
  bool draw_all_boxes;
  bool draw_line_to_target,draw_line_to_targets_target;
  bool draw_line_to_itts;
  ///flag to tell wheter to draw the itts, even if the ship has none
  bool always_itts;
  ///flag controlling whether to use old school, less jumpy (and less accurate) itts // ** jay
  bool steady_itts;
  // colors of blips/targetting boxes
  GFXColor friendly,enemy,neutral,targeted,targetting,planet;
  // gets the color by relation
  GFXColor relationToColor (float relation);
  // gets the color by looking closer at the unit
  GFXColor unitToColor (Unit *un,Unit *target, char ifflevel);
  // the style of the radar (WC|Elite)
  string radar_type;
  /// Used to display the arrow pointing to the currently selected target.
  float projection_limit_x, projection_limit_y;
  float inv_screen_aspect_ratio;  // Precomputed division 1 / g_game.aspect.

  void LocalToEliteRadar (const Vector & pos, float &s, float &t,float &h);
  void LocalToRadar (const Vector & pos, float &s, float &t);

  void LoadXML (const char *file);
  void LoadXML (VSFileSystem::VSFile & f);
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
  ///draws a target cross around all units targeted by your turrets // ** jay
  void DrawTurretTargetBoxes ();
  void DrawTacticalTargetBox ();
  void DrawCommunicatingBoxes ();
  ///Draws all teh blips on the radar.
  void DrawBlips(Unit * un);
  ///Draws all teh blips on the radar in Elite-style
  void DrawEliteBlips(Unit * un);
  ///Draws gauges
  void DrawGauges(Unit * un);
  NavigationSystem ThisNav;
  // Draw the arrow pointing to the target.
  void DrawArrowToTarget(Unit *, Unit *);
 public:
  void	ReceivedTargetInfo();
  static void NavScreen (const KBData&,KBSTATE k); // scheherazade
  static string getsoundending(int which=0);
  static string getsoundfile(string filename);
  void InitStatic ();
  void Shake (float amt, int level /*0= shield 1=armor 2=hull*/);
  int Autopilot (Unit * target);
 ///Restores the view from the IDentity Matrix needed to draw sprites
  void RestoreViewPort();
  GameCockpit (const char * file, Unit * parent, const std::string &pilotname);
  ~GameCockpit();
  ///Looks up a particular Gauge stat on target unit
  float LookupTargetStat (int stat, Unit *target);
  ///Loads cockpit info...just as constructor
  void Init (const char * file);
  ///Draws Cockpit then restores viewport
  void Draw();
  //void Update();//respawns and the like.
  void UpdAutoPilot();
  ///Sets up the world for rendering...call before draw
  void SetupViewPort (bool clip=true);
  int getVDUMode(int vdunum);
  void VDUSwitch (int vdunum);
  void ScrollVDU (int vdunum, int howmuch);
  void ScrollAllVDU (int howmuch);
  int getScrollOffset (unsigned int whichtype);
  void SelectProperCamera ();
  void Eject ();
  void EjectDock ();
  static void Respawn (const KBData&,KBSTATE);
  static void SwitchControl (const KBData&,KBSTATE);
  static void ForceSwitchControl (const KBData&,KBSTATE);
  static void TurretControl (const KBData&,KBSTATE);
  void SetSoundFile (std::string sound);
  int GetSoundFile () {return soundfile;}
  void SetCommAnimation (Animation * ani, Unit * un);
  void SetStaticAnimation ();
  ///Accesses the current navigationsystem
  NavigationSystem *AccessNavSystem() {return &ThisNav;}
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
  void drawUnToTarget(Unit* a,Unit * b, float xcent, float ycent, float xsize, float ysize, bool reardar);
  virtual bool SetDrawNavSystem(bool);
  virtual bool CanDrawNavSystem();
  virtual bool DrawNavSystem();
  virtual bool CheckCommAnimation(Unit *un);
};
#endif
