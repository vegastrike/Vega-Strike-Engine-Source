/*
 * Vega Strike
 * Copyright (C) 2001-2002 Daniel Horn
 *
 * http://vegastrike.sourceforge.net/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

/***** Unit is the Unit class without GFX/Sound with AI *****/

#ifndef _UNIT_H_
#define _UNIT_H_
#define CONTAINER_DEBUG
#ifdef CONTAINER_DEBUG
#include "hashtable.h"
class Unit;
void CheckUnit(class Unit *);
void UncheckUnit (class Unit * un);
#endif
#include "vegastrike.h"
#include "vs_globals.h"

#include <string>
#include "cmd/unit_armorshield.h"
#include "gfx/matrix.h"
#include "gfx/quaternion.h"
#include "gfxlib_struct.h"
#include "weapon_xml.h"
#include "linecollide.h"
//#include "gfx/vdu.h"
#include "xml_support.h"
#include "container.h"
#include "collection.h"
#include "script/flightgroup.h"
#include "faction_generic.h"
#include "star_system_generic.h"
#include "networking/const.h"
#include "networking/lowlevel/vsnet_clientstate.h"
#include "gfx/cockpit_generic.h"
#include "vsfilesystem.h"
using std::string;

extern char * GetUnitDir (const char * filename);
extern float capship_size;
using namespace XMLSupport;

struct CargoColor {
  Cargo cargo;
  GFXColor color;
  CargoColor () 
    : cargo(), color(1,1,1,1) {
  }
};//A stupid struct that is only for grouping 2 different types of variables together in one return value

class PlanetaryOrbit;
class UnitCollection;

class Order;
class Beam;
class Animation;
class Nebula;
class Animation;
class VSSprite;
class Box;
class StarSystem;
struct colTrees;

#include "images.h"

/**
 * Currently the only inheriting function is planet
 * Needed by star system to determine whether current unit
 * is orbitable
 */
enum clsptr {
	UNITPTR,
	PLANETPTR,
	BUILDINGPTR,
	NEBULAPTR,
	ASTEROIDPTR,
	ENHANCEMENTPTR,
	MISSILEPTR
};

class Mount {
  protected:
    ///Where is it
	Vector pos;
    Quaternion orient;
	void ReplaceSound();
  public:
	// Serial used to store missiles serial id before they are really created
	ObjSerial serial;
	float xyscale;float zscale;//for guns!
	float ComputeAnimatedFrame(Mesh * gun);
    void SwapMounts (Mount * other);
    void ReplaceMounts (const Mount * other);
	double Percentage (const Mount *newammo) const;
// Gotta look at that, if we can make Beam a string in AcctUnit and a Beam elsewhere
    union REF{
      ///only beams are actually coming out of the gun at all times...bolts, balls, etc aren't
      Beam *gun;
      ///Other weapons must track refire times
      float refire;
    } ref;
    ///the size that this mount can hold. May be any bitwise combination of weapon_info::MOUNT_SIZE

    int size; //short fix
    ///-1 is infinite
    int ammo; //short
    int volume;//-1 is infinite //short fix
    ///The data behind this weapon. May be accordingly damaged as time goes on
    enum MOUNTSTATUS{REQUESTED,ACCEPTED,PROCESSED,UNFIRED,FIRED} processed;
    ///Status of the selection of this weapon. Does it fire when we hit space
    enum STATUS{ACTIVE, INACTIVE, DESTROYED, UNCHOSEN} status;
    ///The sound this mount makes when fired
    const weapon_info *type;
    float functionality;
    float maxfunctionality;
    int sound;
    float time_to_lock;
    Mount();
    Mount(const std::string& name, int ammo, int volume, float xyscale, float zscale, float functionality, float maxfunctionality); //short fix

    void Activate (bool Missile);
    void DeActive (bool Missile);
    ///Sets this gun's position on the mesh
    void SetMountPosition (const Vector & v) {pos=v;}
	void SetMountOrientation(const Quaternion &t) {orient = t;}
    ///Gets the mount's position and transform
    const Vector &GetMountLocation ()const{return pos;}
    const Quaternion &GetMountOrientation ()const{return orient;}	
    ///Turns off a firing beam (upon key release for example)
	void UnFire();
    /**
     *  Fires a beam when the firing unit is at the Cumulative location/transformation 
     * owner (won't crash into)  as owner and target as missile target. bool Missile indicates if it is a missile
     * should it fire
     */ 
	// Uses Sound Forcefeedback and other stuff
	void PhysicsAlignedUnfire();
	bool PhysicsAlignedFire (const Transformation &Cumulative, const Matrix & mat, const Vector & Velocity, Unit *owner,  Unit *target, signed char autotrack, float trackingcone, int mount_num=0);
	bool Fire (Unit *owner, bool Missile=false, bool collide_only_with_target=false);
};


class VDU;
struct UnitImages;
struct UnitSounds;
class Cargo;
class Mesh;

/// used to scan the system - faster than c_alike code

struct Scanner {
  Unit *nearest_enemy;
  Unit *nearest_friend;
  Unit *nearest_ship;
  Unit *leader;

  float nearest_enemy_dist,nearest_friend_dist,nearest_ship_dist;

  double last_scantime;
};

/**
 * Unit contains any physical object that may collide with something
 * And may be physically affected by forces.
 * Units are assumed to have various damage and explode when they are dead.
 * Units may have any number of weapons which, themselves may be units
 * the aistate indicates how the unit will behave in the upcoming phys frame
 */
class PlanetaryTransform;
struct PlanetaryOrbitData;
class Unit
{
protected:
  ///How many lists are referencing us
  int ucref;
  std::string csvRow;
public:
  UnitSounds * sound;

  ///The name (type) of this unit shouldn't be public
  std::string name;
  
/***************************************************************************************/
/**** CONSTRUCTORS / DESCTRUCTOR                                                    ****/
/***************************************************************************************/

protected:
  /// forbidden
  Unit( const Unit& ); 
  
  /// forbidden
  Unit& operator=( const Unit& );
  
public:
  /** default constructor
   */
  Unit();
    
  /** Default constructor. This is just to figure out where default
   *  constructors are used. The useless argument will be removed
   *  again later.
   */
  Unit( int dummy );
  
  /** Constructor that creates aa mesh with meshes as submeshes (number
   *  of them) as either as subunit with faction faction
   */
  Unit (std::vector <Mesh *> &meshes  , bool Subunit, int faction);


  /** Constructor that creates a mesh from an XML file If it is a
   *  customizedUnit, it will check in that directory in the home dir for
   *  the unit.
   */
// Uses a lot of stuff that does not belong to here
  Unit( const char *filename,
        bool        SubUnit,
       int         faction,
       std::string customizedUnit=string(""),
       Flightgroup *flightgroup=NULL,
       int         fg_subnumber=0, string * netxml=NULL);

private:
  /** Fix all those uninitialized variables by calling this from every
   *  constructor.  */
  void ZeroAll( );

public:
  ///Initialize many of the defaults inherant to the constructor
  void Init();
  void Init( const char *filename,
        bool        SubUnit,
       int         faction,
       std::string customizedUnit=string(""),
       Flightgroup *flightgroup=NULL,
       int         fg_subnumber=0, string * netxml=NULL);
  friend class UnitFactory;
  void LoadRow(class CSVRow &row, std::string unitMod, string*netxml=NULL);
public:    
  virtual ~Unit();

/***************************************************************************************/
/**** NETWORKING STUFF                                                              ****/
/***************************************************************************************/

protected:
  // Tell if networked unit
  bool				networked;
  ObjSerial			serial;
  Vector			net_accel;
public:
  ClientState		old_state;
  enum DAMAGES { NO_DAMAGE=0x00, SHIELD_DAMAGED=0x01, COMPUTER_DAMAGED=0x02, MOUNT_DAMAGED=0x04, CARGOFUEL_DAMAGED=0x08, JUMP_DAMAGED=0x10, CLOAK_DAMAGED=0x20, LIMITS_DAMAGED=0x40, ARMOR_DAMAGED=0x80};
  unsigned short damages;

  void				SetNetworkMode( bool mode=true) {this->networked = mode;}
  ObjSerial 		GetSerial() { return this->serial;}
  void				SetSerial( ObjSerial ser) { this->serial = ser;}
  void				BackupState();

/***************************************************************************************/
/**** UPGRADE/CUSTOMIZE STUFF                                                       ****/
/***************************************************************************************/

// Uses mmm... stuff not desired here ?
  bool UpgradeSubUnitsWithFactory (const Unit * up, int subunitoffset, bool touchme, bool downgrade, int &numave, double &percentage, Unit * (*createupgradesubunit) (std::string s, int faction));
  virtual bool UpgradeSubUnits (const Unit * up, int subunitoffset, bool touchme, bool downgrade, int &numave, double &percentage);
  bool UpgradeMounts (const Unit * up, int subunitoffset, bool touchme, bool downgrade, int &numave, const Unit * templ, double &percentage);
  /// the turrets and spinning parts fun fun stuff
  UnitCollection SubUnits; 

  /** 
   * Contains information about a particular Mount on a unit.
   * And the weapons it has, where it is, where it's aimed, 
   * The ammo and the weapon type. As well as the possible weapons it may fit
   * Warning: type has a string inside... cannot be memcpy'd
   */
public:
  un_iter getSubUnits();
  un_kiter viewSubUnits() const;
public:
#define NO_MOUNT_STAR
  vector <Mount> mounts;
protected:
  ///Mount may access unit
  friend class Mount;
  ///no collision table presence.
public:
  struct graphic_options {
	  unsigned SubUnit:1;
	  unsigned RecurseIntoSubUnitsOnCollision:1;	
	  unsigned missilelock:1;	    
	  unsigned FaceCamera:1;
	  unsigned Animating:1;
	  unsigned InWarp:1;
	  unsigned WarpRamping:1;
	  unsigned unused1:1;
	  float WarpFieldStrength;
	  float RampCounter;
	  unsigned char NumAnimationPoints;
	  graphic_options() {
		  FaceCamera=Animating=missilelock=InWarp=unused1=WarpRamping=0;
		  NumAnimationPoints=0;
		  RampCounter=0;
	  }
  }graphicOptions;
  bool isSubUnit() {return graphicOptions.SubUnit?true:false;}
  void setFaceCamera(){graphicOptions.FaceCamera=1;}
  bool UpAndDownGrade (const Unit * up, const Unit * templ, int mountoffset, int subunitoffset, bool touchme, bool downgrade, int additive, bool forcetransaction, double &percentage, const Unit * downgrade_min, bool force_change_on_nothing);
  void ImportPartList (const std::string& category, float price, float pricedev,  float quantity, float quantdev);
  unsigned char RecomputeRole();//changes own role
  int GetNumMounts ()const  {return mounts.size();}
  void ClearMounts();
  ///Loads a user interface for the user to upgrade his ship
// Uses base stuff -> only in Unit
  virtual void UpgradeInterface (Unit * base) {}

  bool canUpgrade (const Unit * upgrador, int mountoffset,  int subunitoffset, int additive, bool force,  double & percentage, const Unit * templ=NULL, bool force_change_on_nothing=false);
  bool Upgrade (const Unit * upgrador, int mountoffset,  int subunitoffset, int additive, bool force,  double & percentage, const Unit * templ=NULL, bool force_change_on_nothing=false);
  int RepairCost();//returns how many things need to be repaired--if nothing is damaged it will return 1 for labor.  doesn't assume any given cost on such thigns.
  int RepairUpgrade();//returns how many things were repaired
  bool ReduceToTemplate();
  virtual double Upgrade (const std::string &file,int mountoffset, int subunitoffset, bool force, bool loop_through_mounts) { return 1;}
  bool canDowngrade (const Unit *downgradeor, int mountoffset, int subunitoffset, double & percentage, const Unit * downgradelimit);
  bool Downgrade (const Unit * downgradeor, int mountoffset, int subunitoffset,  double & percentage, const Unit * downgradelimit);

/***************************************************************************************/
/**** GFX/PLANET STUFF                                                              ****/
/***************************************************************************************/


public:
  ///number of meshes (each with separate texture) this unit has

  std::vector <Mesh *> meshdata;
  unsigned char combatRole() const {return combat_role;}
  void combatRole(unsigned char c){combat_role=c;}
  void setCombatRole(std::string s);
  std::string getCombatRole()const;
protected:
  unsigned char combat_role;
  Nebula * nebula;
  PlanetaryOrbitData * planet;
  ///The orbit needs to have access to the velocity directly to disobey physics laws to precalculate orbits
  friend class PlanetaryOrbit;
  friend class ContinuousTerrain;
  ///VDU needs mount data to draw weapon displays
  friend class VDU;
  friend class UpgradingInfo;//needed to actually upgrade unit through interface
 public:
  // Have to pass the randnum and degrees in networking and client side since they must not be random in that case
  void DamageRandSys (float dam,const Vector &vec, float randum=1, float degrees=1);
  void SetNebula (Nebula *neb) {
    nebula = neb;
    if (!SubUnits.empty()) {
      un_fiter iter =SubUnits.fastIterator();
      Unit * un;
      while ((un = iter.current())) {
	un->SetNebula (neb);
	iter.advance();
      }
    } 
  }
  inline Nebula * GetNebula () const{return nebula;}
  ///Should draw selection box?
  ///Process all meshes to be deleted
  static void ProcessDeleteQueue();
  ///Returns the cockpit name so that the controller may load a new cockpit
  std::string getCockpit ()const;

  // Shouldn't do anything here - but needed by Python
  class Cockpit * GetVelocityDifficultyMult(float &) const;

  StarSystem * activeStarSystem;//the star system I'm in
  ///Takes out of the collide table for this system.
  void RemoveFromSystem();
  bool InCorrectStarSystem (StarSystem *active) {return active==activeStarSystem;}
  virtual int nummesh()const {return ((int)meshdata.size())-1;}
//void FixGauges();
// Uses planet stuff
  virtual void SetPlanetOrbitData (PlanetaryTransform *trans) {}
  virtual PlanetaryTransform *GetPlanetOrbit () const {return NULL;}
  ///Updates the collide Queue with any possible change in sectors
///Split this mesh with into 2^level submeshes at arbitrary planes
// Uses Mesh so only in Unit and maybe in NetUnit
  virtual void Split (int level){}
  //  void SwapOutHalos();
  //  void SwapInHalos();
  virtual void addHalo( const char * filename, const QVector &loc, const Vector &size, const GFXColor & col, std::string halo_type, float activation) {}

// Uses Mesh -> in NetUnit and Unit only
  vector <Mesh *> StealMeshes();
  ///Begin and continue explosion
// Uses GFX so only in Unit class
// But should always return true on server side = assuming explosion time=0 here
  virtual bool Explode(bool draw, float timeit) {return true;}
  ///explodes then deletes
  void Destroy();

// Uses GFX so only in Unit class
  virtual void Draw(const Transformation & quat = identity_transformation, const Matrix &m = identity_matrix) {}
  virtual void DrawNow(const Matrix &m = identity_matrix, float lod=1000000000) {}
  ///Deprecated
  //virtual void ProcessDrawQueue() {}
  ///Gets the minimum distance away from the point in 3space

  ///Sets the camera to be within this unit.
// Uses Universe & GFX so not needed here -> only in Unit class
  virtual void UpdateHudMatrix(int whichcam) {}
  ///What's the HudImage of this unit
// Uses GFX stuff so only in Unit class
  virtual VSSprite * getHudImage ()const {return NULL;}
// Not needed just in Unit class

/***************************************************************************************/
/**** NAVIGATION STUFF                                                              ****/
/***************************************************************************************/

public:
  const std::vector <char *> &GetDestinations () const;
  void AddDestination (const char *);
  /**
   * The computer holds all data in the navigation computer of the current unit
   * It is outside modifyable with GetComputerData() and holds only volatile
   * Information inside containers so that destruction of containers will not
   * result in segfaults.
   * Maximum speeds and turning restrictions are merely facts of the computer
   * and have nothing to do with the limitations of the physical nature
   * of space combat
   */
  struct Computer {
    struct RADARLIM {
      ///the max range the radar can handle
      float maxrange;
      ///the dot with (0,0,1) indicating the farthest to the side the radar can handle.
      float maxcone;
      float lockcone;
      float trackingcone;
      ///The minimum radius of the target
      float mintargetsize;
      ///what kind of iff support does it have 0 = none, 1=friend/enemy 2=object type
      char iff;
      bool locked;
      bool canlock;
      RADARLIM() : maxrange(0), maxcone(0), lockcone(0), trackingcone(0),
      		mintargetsize(0), iff(0), locked(false), canlock(false) { }
    } radar;
    ///The nav point the unit may be heading for
    Vector NavPoint;
    ///The target that the unit has in computer
    UnitContainer target;
    ///Any target that may be attacking and has set this threat
    UnitContainer threat;
    ///Unit that it should match velocity with (not speed) if null, matches velocity with universe frame (star)
    UnitContainer velocity_ref;
    ///The threat level that was calculated from attacking unit's threat
    float threatlevel;
    ///The speed the flybywire system attempts to maintain
    float set_speed;
    ///Computers limitation of speed
    float max_combat_speed; float max_combat_ab_speed;
    float max_speed () const; float max_ab_speed() const;
    ///Computer's restrictions of YPR to limit space combat maneuvers
    float max_yaw_left; 
    float max_yaw_right; 
    float max_pitch_down; 
    float max_pitch_up; 
    float max_roll_left; 
    float max_roll_right;
    ///Whether or not an 'lead' indicator appears in front of target
    unsigned char slide_start;
    unsigned char slide_end;
    bool itts;
    //tells whether the speed is clamped draconian-like or not
    bool combat_mode;

    Computer() : NavPoint(0,0,0) , threatlevel(0) , set_speed(0)
        , max_combat_speed(0) , max_combat_ab_speed(0)
        , max_yaw_left(0) ,max_yaw_right(0), max_pitch_down(0), max_pitch_up(0), max_roll_left(0) ,max_roll_right(0), slide_start(0) , slide_end(0)
        , itts(false) , combat_mode(false) { }
  };
  Computer computer;
  void SwitchCombatFlightMode();
  bool CombatMode();
// SHOULD TRY TO COME BACK HERE
  virtual bool TransferUnitToSystem (StarSystem *NewSystem);
  virtual bool TransferUnitToSystem (unsigned int whichJumpQueue, class StarSystem *&previouslyActiveStarSystem, bool DoSightAndSound) {return false;}
  StarSystem * getStarSystem();
    struct UnitJump {
    float warpDriveRating;
    float energy; //short fix
    float insysenergy; //short fix
    signed char drive;
    unsigned char delay;
    unsigned char damage;
    //negative means fuel
  } jump;
  bool selected;  
 
  const UnitJump &GetJumpStatus() const {return jump;}
  float CourseDeviation (const Vector &OriginalCourse, const Vector &FinalCourse)const ;
  Computer & GetComputerData () {return computer;}
  const Computer & ViewComputerData () const {return computer;}

  // for scanning purposes
 protected:
  struct Scanner scanner;
 public:
  void scanSystem();
  struct Scanner *getScanner() { return &scanner; };
  void ActivateJumpDrive (int destination=0);
  void DeactivateJumpDrive ();

/***************************************************************************************/
/**** XML STUFF                                                                     ****/
/***************************************************************************************/

 public:
  ///Unit XML Load information
  struct XML;
  ///Loading information
  XML *xml;

  static void beginElement(void *userData, const XML_Char *name, const XML_Char **atts);
  static void endElement(void *userData, const XML_Char *name);

  void beginElement(const std::string &name, const AttributeList &attributes);
  void endElement(const std::string &name);

 protected:
	 static std::string massSerializer(const struct XMLType &input, void*mythis);
	 static std::string cargoSerializer(const struct XMLType &input, void*mythis);
	 static std::string mountSerializer(const struct XMLType &input, void*mythis);
	 static std::string shieldSerializer(const struct XMLType &input, void*mythis);
	 static std::string subunitSerializer(const struct XMLType &input, void*mythis);

public:
  ///tries to warp as close to un as possible abiding by the distances of various enemy ships...it might not make it all the way
  void WriteUnit(const char * modificationname="");
  string WriteUnitString();
  ///Loads a unit from an xml file into a complete datastructure
  void LoadXML(const char *filename, const char * unitModifications="", string * xmlbuffer=NULL);
  void LoadXML(VSFileSystem::VSFile & f, const char * unitModifications="", string * xmlbuffer=NULL);

/***************************************************************************************/
/**** PHYSICS STUFF                                                                 ****/
/***************************************************************************************/

private:
  void RechargeEnergy();
protected:
  virtual float ExplosionRadius();
public:
  virtual void SetPlanetHackTransformation (Transformation *&ct,Matrix *&ctm) {}
  bool AutoPilotTo(Unit * un, bool automaticenergyrealloc,int recursive_level=2);
  ///The owner of this unit. This may not collide with owner or units owned by owner. Do not dereference (may be dead pointer)
  Unit *owner;
  ///The number of frames ahead this was put in the simulation queue
  unsigned int sim_atom_multiplier;
  ///The previous state in last physics frame to interpolate within
  Transformation prev_physical_state;
  ///The state of the current physics frame to interpolate within
  Transformation curr_physical_state;
  ///number of meshes (each with separate texture) this unit has
  ///The cumulative (incl subunits parents' transformation)
  Matrix cumulative_transformation_matrix;
  ///The cumulative (incl subunits parents' transformation)
  Transformation cumulative_transformation;
  ///The velocity this unit has in World Space
  Vector cumulative_velocity;
  ///The force applied from outside accrued over the whole physics frame
  Vector NetForce;
  ///The force applied by internal objects (thrusters)
  Vector NetLocalForce;
  ///The torque applied from outside objects
  Vector NetTorque;
  ///The torque applied from internal objects
  Vector NetLocalTorque;
  ///the current velocities in LOCAL space (not world space)
  Vector AngularVelocity;  Vector Velocity;  ///The image that will appear on those screens of units targetting this unit
  UnitImages *image;
  ///mass of this unit (may change with cargo)
  float Mass;
  ///positive for the multiplier applied to nearby spec starships (1 = planetary/inert effects) 0 is default (no effect), -X means 0 but able to be enabled
  float specInterdiction;
  float HeatSink;
protected:
  ///are shields tight to the hull.  zero means bubble
  float shieldtight;
  ///fuel of this unit
  float fuel;
  float afterburnenergy;  //short fix
  int   afterburntype; // 0--energy, 1--fuel
  ///-1 means it is off. -2 means it doesn't exist. otherwise it's engaged to destination (positive number)
 ///Moment of intertia of this unit
  float Momentofinertia;

public:
  struct Limits {
    ///max ypr--both pos/neg are symmetrical
    float yaw; float pitch; float roll;
    ///side-side engine thrust max
    float lateral;
    ///vertical engine thrust max
    float vertical;
    ///forward engine thrust max
    float forward;
    ///reverse engine thrust max
    float retro;
    ///after burner acceleration max
    float afterburn;
    ///the vector denoting the "front" of the turret cone!
    Vector structurelimits;
    ///the minimum dot that the current heading can have with the structurelimit
    float limitmin;

    Limits( )
       : yaw(0) , pitch(0) , roll(0) , lateral(0) , vertical(0) , forward(0)
       , retro(0) , afterburn(0) , structurelimits(0,0,0) , limitmin(0) { }
  } limits;

public:
  ///-1 is not available... ranges between 0 32767 for "how invisible" unit currently is (32768... -32768) being visible)
  int cloaking; //short fix
  ///the minimum cloaking value...
  int cloakmin; //short fix
  ///How big is this unit
  float radial_size;
protected:
  ///Is dead already?
  bool killed;
  ///Should not be drawn
  enum INVIS {DEFAULTVIS=0x0,INVISGLOW=0x1,INVISUNIT=0x2,INVISCAMERA=0x4};
  unsigned char invisible;//1 means turn off glow, 2 means turn off ship
  /// corners of object  

public:
  Vector corner_min, corner_max;
  Vector LocalCoordinates (Unit * un) const {
    return ToLocalCoordinates ((un->Position()-Position()).Cast());
  }
  ///how visible the ship is from 0 to 1
  float CloakVisible() const {
    if (cloaking<0)
      return 1;
    return ((float)cloaking)/2147483647;
  }

  ///cloaks or decloaks the starship depending on the bool
  void Cloak (bool cloak);
  ///deletes
  void Kill(bool eraseFromSave=true, bool quitting=false);
  ///Is dead yet?
  inline bool Killed() const {return killed;}
  ///returns the current ammt of armor left
  float AfterburnData() const{ return afterburnenergy;} //short fix
  void			 SetAfterBurn( float aft) { afterburnenergy = aft;} //short fix
  float FuelData() const;
  float WarpCapData() const;
  void  SetFuel( float f) { fuel = f;}
  ///Returns the current ammt of energy left
  float EnergyRechargeData() const{return recharge;}
  void  SetEnergyRecharge( float enrech) { recharge = enrech;}
  void  SetMaxEnergy( float maxen) { maxenergy = maxen;}
  float MaxEnergyData() const{return maxenergy;}
  float ShieldRechargeData() const{return shield.recharge;}
  float EnergyData() const;
  float WarpEnergyData() const;
  float GetWarpEnergy() {return warpenergy;} //short fix
  void DecreaseWarpEnergy(bool insystem, float time=1.0f);
  void IncreaseWarpEnergy(bool insystem, float time=1.0f);
  bool RefillWarpEnergy();
  ///Should we resolve forces on this unit (is it free to fly or in orbit)
  bool resolveforces;
  ///What's the size of this unit
  float rSize () const {return radial_size;}

  ///Returns the current world space position
  QVector Position() const{return cumulative_transformation.position;};
  const Matrix &  GetTransformation () const {return cumulative_transformation_matrix;}
  ///Returns the unit-space position
  QVector LocalPosition() const {return curr_physical_state.position;};
  ///Sets the unit-space position
  void SetPosition(const QVector &pos) {prev_physical_state.position = curr_physical_state.position = pos;}
  ///Sets the cumulative transformation matrix's position...for setting up to be out in the middle of nowhere
  void SetCurPosition (const QVector & pos) {curr_physical_state.position=pos;}
  void SetPosAndCumPos (const QVector &pos) {SetPosition (pos);cumulative_transformation_matrix.p=pos;cumulative_transformation.position=pos;}
  ///Sets the state of drawing
  void SetVisible(bool isvis);
  void SetAllVisible(bool isvis);
  void SetGlowVisible(bool isvis);	
	
  ///Rotates about the axis
  void Rotate(const Vector &axis);
  /**
   * Fire engine takes a unit vector for direction
   * and how fast the fuel speed and mass coming out are
   */
  void FireEngines (const Vector &Direction, /*unit vector... might default to "r"*/
					float FuelSpeed,
					float FMass);
  ///applies a force for the whole gameturn upon the center of mass
  void ApplyForce(const Vector &Vforce); 
  ///applies a force for the whole gameturn upon the center of mass, using local coordinates
  void ApplyLocalForce(const Vector &Vforce); 
  /// applies a force that is multipled by the mass of the ship
  void Accelerate(const Vector &Vforce); 
  ///Apply a torque in world level coords
  void ApplyTorque (const Vector &Vforce, const QVector &Location);
  ///Applies a torque in local level coordinates
  void ApplyLocalTorque (const Vector &Vforce, const Vector &Location);
  ///usually from thrusters remember if I have 2 balanced thrusters I should multiply their effect by 2 :)
  void ApplyBalancedLocalTorque (const Vector &Vforce, const Vector &Location); 

  ///convenient shortcut to applying torques with vector and position
  void ApplyLocalTorque(const Vector &torque); 
  ///Applies damage to the local area given by pnt
  float ApplyLocalDamage (const Vector &pnt, const Vector & normal, float amt, Unit * affectedSubUnit, const GFXColor &, float phasedamage=0);
  //Applies damage from network data
  void	ApplyNetDamage( Vector & pnt, Vector & normal, float amt, float ppercentage, float spercentage, GFXColor & color);
  ///Applies damage to the pre-transformed area of the ship
  void ApplyDamage (const Vector & pnt, const Vector & normal, float amt, Unit * affectedSubUnit, const GFXColor &,  Unit *ownerDoNotDereference, float phasedamage=0 );
  ///Deals remaining damage to the hull at point and applies lighting effects
  float DealDamageToHullReturnArmor (const Vector &pnt, float Damage, float * &targ);//short fix
  virtual void ArmorDamageSound( const Vector &pnt) {};
  virtual void HullDamageSound( const Vector &pnt) {};
  float DealDamageToHull (const Vector &pnt, float Damage)
  {
	 float * nullvar = NULL; //short fix
	 return this->DealDamageToHullReturnArmor( pnt, Damage, nullvar);
  }

  ///Clamps thrust to the limits struct
  Vector ClampThrust(const Vector &thrust, bool afterburn);
  ///Takes a unit vector for direction of thrust and scales to limits
  Vector MaxThrust(const Vector &thrust);
  ///Thrusts by ammt and clamps accordingly (afterburn or not)
  virtual void Thrust(const Vector &amt,bool afterburn = false);
  ///Applies lateral thrust
  void LateralThrust(float amt);
  ///Applies vertical thrust
  void VerticalThrust(float amt);
  ///Applies forward thrust
  void LongitudinalThrust(float amt);
  ///Clamps desired velocity to computer set limits
  Vector ClampVelocity (const Vector & velocity, const bool afterburn);
  ///Clamps desired angular velocity to computer set limits
  Vector ClampAngVel (const Vector & vel);
  ///Clamps desired torque to computer set limits of thrusters
  Vector ClampTorque(const Vector &torque);
  ///scales unit size torque to limits in that direction
  Vector MaxTorque(const Vector &torque);
  ///Applies a yaw of amt
  void YawTorque(float amt);
  ///Applies a pitch of amt
  void PitchTorque(float amt);
  ///Applies a roll of amt
  void RollTorque(float amt);
  ///executes a repair if the repair bot is up to it
  void Repair();
  ///Updates physics given unit space transformations and if this is the last physics frame in the current gfx frame
// Not needed here, so only in NetUnit and Unit classes
  void UpdatePhysics (const Transformation &trans, const Matrix &transmat, const Vector & CumulativeVelocity, bool ResolveLast, UnitCollection *uc=NULL);
  void AddVelocity(float difficulty);
  virtual void UpdatePhysics2 (const Transformation &trans, const Transformation & old_physical_state, const Vector & accel, float difficulty, const Matrix &transmat, const Vector & CumulativeVelocity, bool ResolveLast, UnitCollection *uc=NULL);
  ///Resolves forces of given unit on a physics frame
  virtual Vector ResolveForces (const Transformation &, const Matrix&);
  ///Returns the pqr oritnattion of the unit in world space
  void SetOrientation (QVector q, QVector r);
  void SetOrientation (QVector p, QVector q, QVector r);
  void SetOrientation (Quaternion Q);
  void GetOrientation(Vector &p, Vector &q, Vector &r) const {
    Matrix m;
    curr_physical_state.to_matrix(m);
    p=m.getP();
    q=m.getQ();
    r=m.getR();
  }
  Vector GetAcceleration() {
    Vector p, q, r;
    GetOrientation(p,q,r);
	Vector res(NetLocalForce.i*p + NetLocalForce.j*q + NetLocalForce.k*r );
    if (NetForce.i||NetForce.j||NetForce.k) {
      res+=InvTransformNormal(identity_matrix,NetForce);
    }
    res=res/GetMass();

  	return res;
  } //acceleration
  ///Transforms a orientation vector Up a coordinate level. Does not take position into account
  Vector UpCoordinateLevel(const Vector &v) const;
  ///Transforms a orientation vector Down a coordinate level. Does not take position into account
  Vector DownCoordinateLevel(const Vector &v) const;
  ///Transforms a orientation vector from world space to local space. Does not take position into account 
  Vector ToLocalCoordinates(const Vector &v) const;
  ///Transforms a orientation vector to world space. Does not take position into account
  Vector ToWorldCoordinates(const Vector &v) const;
  ///Returns unit-space ang velocity
  const Vector &GetAngularVelocity() const { return AngularVelocity; }
  ///Return unit-space velocity
  const Vector &GetVelocity() const { return cumulative_velocity; }
  Vector GetWarpVelocity() const {
	return cumulative_velocity*graphicOptions.WarpFieldStrength;
  }
  void SetVelocity (const Vector & v) {Velocity = v;}
  void SetAngularVelocity (const Vector & v) {AngularVelocity = v;}
  float GetMoment() const { return Momentofinertia+fuel; }
  float GetMass() const { return Mass+fuel; }
  ///returns the ammt of elasticity of collisions with this unit
  float GetElasticity ();
  ///returns given limits (should not be necessary with clamping functions)
  const Limits &Limits() const { return limits; }
  ///Sets if forces should resolve on this unit or not
  void SetResolveForces(bool);

  /*
    //FIXME Deprecated turret restrictions (may bring back)
  enum restr {YRESTR=1, PRESTR=2, RRESTR=4};

  char yprrestricted;

  float ymin, ymax, ycur;
  float pmin, pmax, pcur;
  float rmin, rmax, rcur;
  */
  /// thrusting limits (acceleration-wise

/***************************************************************************************/
/**** WEAPONS/SHIELD STUFF                                                          ****/
/***************************************************************************************/

protected:
  ///Activates all guns of that size
  void ActivateGuns (const weapon_info *, bool Missile);
  float MaxShieldVal() const;
  ///regenerates all 2,4, or 6 shields for 1 SIMULATION_ATOM
  void RegenShields();
public:
  // Armor and shield structures
  Armor armor;
  Shield shield;
  ///The structual integ of the current unit
  float hull;
protected:
  ///Original hull
  float maxhull;
  ///The radar limits (range, cone range, etc) 
  ///the current order
  ///how much the energy recharges per second
  float recharge;

  ///maximum energy
  float maxenergy;
  ///current energy
  float energy;
  ///maximum energy
  float maxwarpenergy; //short fix
  ///current energy
  float warpenergy; // short fix
  ///applies damage from the given pnt to the shield, and returns % damage applied and applies lighitn
  virtual float DealDamageToShield (const Vector & pnt, float &Damage);
  ///If the shields are up from this position
  bool ShieldUp (const Vector &) const;

public:
  int LockMissile() const;//-1 is no lock necessary 1 is locked
  void LockTarget(bool myboo);
  bool TargetLocked()const {return computer.radar.locked;}
  float TrackingGuns(bool &missileLock);
  ///Changes currently selected weapon
  void ToggleWeapon (bool Missile, bool forward=true);
  ///Selects all weapons
  void SelectAllWeapon (bool Missile);
  ///Gets the average gun speed of the unit::caution SLOW
  void getAverageGunSpeed (float & speed, float & grange, float & mrange) const;
  ///Finds the position from the local position if guns are aimed at it with speed
  QVector PositionITTS (const QVector &firingposit, Vector firingvelocity, float gunspeed, bool smooth_itts) const;
  ///returns percentage of course deviation for contraband searches.  .5 causes error and 1 causes them to get mad 
  float FShieldData() const;  float RShieldData() const;  float LShieldData() const;  float BShieldData() const;
  void ArmorData(float armor[8])const; //short fix
  ///Gets the current status of the hull
  float GetHull() const{return hull;}
  float GetHullPercent() const{return maxhull!=0?hull/maxhull:hull;}
  ///Fires all active guns that are or arent Missiles
  // if bitmask is (1<<31) then fire off autotracking of that type;
  void Fire(unsigned int bitmask, bool beams_target_owner=false);
  ///Stops all active guns from firing
  void UnFire();
  ///reduces shields to X percentage and reduces shield recharge to Y percentage
  void leach (float XshieldPercent, float YrechargePercent, float ZenergyPercent);


/***************************************************************************************/
/**** TARGETTING STUFF                                                              ****/
/***************************************************************************************/

protected:
  ///not used yet
  string target_fgid[3];
public:
  bool InRange (Unit *target, bool cone=true, bool cap=true) const{
    double mm;
    return InRange( target,mm,cone,cap,true);
  }
  bool InRange (Unit *target, double & mm, bool cone, bool cap, bool lock) const{
    if (this==target||target->CloakVisible()<.8)
      return false;
    if (cone&&computer.radar.maxcone>-.98){
      QVector delta( target->Position()-Position());
      mm = delta.Magnitude();
      if ((!lock)||(!(TargetLocked()&&computer.target==target))) {
	double tempmm =mm-target->rSize();
	if (tempmm>0.0001) {
	  if ((ToLocalCoordinates (Vector(delta.i,delta.j,delta.k)).k/tempmm)<computer.radar.maxcone&&cone) {
	    return false;
	  }
	}
      }
    }else {
      mm = (target->Position()-Position()).Magnitude();
    }
    if (((mm-rSize()-target->rSize())>computer.radar.maxrange)||target->rSize()<computer.radar.mintargetsize) {//owner==target?!
      Flightgroup *fg = target->getFlightgroup();
      if ((target->rSize()<capship_size||(!cap))&&(fg==NULL?true:fg->name!="Base")) 
        return target->isUnit()==PLANETPTR;
    }
    return true;
  }
  Unit *Target() {return computer.target.GetUnit(); }
  Unit *VelocityReference() {return computer.velocity_ref.GetUnit(); }
  Unit *Threat() {return computer.threat.GetUnit(); }
// Uses Universe stuff so only in Unit class
  void VelocityReference (Unit *targ);
  void TargetTurret (Unit * targ);
  ///Threatens this unit with "targ" as aggressor. Danger should be cos angle to target
  void Threaten (Unit * targ, float danger);
  ///Rekeys the threat level to zero for another turn of impending danger
  void ResetThreatLevel() {computer.threatlevel=0;graphicOptions.missilelock=0;}
  ///The cosine of the angle to the target given passed in speed and range
  float cosAngleTo (Unit * target, float & distance, float speed= 0.001, float range=0.001) const;
  ///Highest cosine from given mounts to target. Returns distance and cosine
  float cosAngleFromMountTo (Unit * target, float & distance) const;
  float computeLockingPercent();//how locked are we
  ///Turns on selection box
  void Select();
  ///Turns off selection box
  void Deselect();

  // Shouldn't do anything here - but needed by Python
  void Target (Unit * targ);

  ///not used yet
  void setTargetFg(string primary,string secondary=string(),string tertiary=string());
  ///not used yet
  void ReTargetFg(int which_target=0);
  ///not used yet

/***************************************************************************************/
/**** CARGO STUFF                                                                   ****/
/***************************************************************************************/

protected:
  void SortCargo();
public:
  static Unit* makeMasterPartList();
  bool CanAddCargo (const Cargo &carg) const;
  void AddCargo (const Cargo &carg,bool sort=true);
  int RemoveCargo (unsigned int i, int quantity, bool eraseZero=true);
  float PriceCargo (const std::string &s);
  Cargo & GetCargo (unsigned int i);
  void GetCargoCat (const std::string &category, vector <Cargo> &cat);
  ///below function returns NULL if not found
  Cargo * GetCargo (const std::string &s, unsigned int &i);
  unsigned int numCargo ()const;
  std::string GetManifest (unsigned int i, Unit * scanningUnit, const Vector & original_velocity) const;
  bool SellCargo (unsigned int i, int quantity, float &creds, Cargo & carg, Unit *buyer);
  bool SellCargo (const std::string &s, int quantity, float & creds, Cargo &carg, Unit *buyer);
  bool BuyCargo (const Cargo &carg, float & creds);
  bool BuyCargo (unsigned int i, unsigned int quantity, Unit * buyer, float &creds);
  bool BuyCargo (const std::string &cargo,unsigned int quantity, Unit * buyer, float & creds);
  void EjectCargo (unsigned int index);
  float getEmptyCargoVolume(void) const;
  float getCargoVolume(void) const;
  float getEmptyUpgradeVolume(void) const;
  float getUpgradeVolume(void) const;

/***************************************************************************************/
/**** AI STUFF                                                                      ****/
/***************************************************************************************/

public:
  class csRapidCollider * getCollideTree(const Vector &scale=Vector(1,1,1),const std::vector<struct bsp_polygon> * = NULL);
  // Because accessing in daughter classes member function from Unit * instances
  Order *aistate;
  Order *getAIState() const{return aistate;}
  ///Sets up a null queue for orders
// Uses AI so only in NetUnit and Unit classes
  void PrimeOrders();
  void PrimeOrdersLaunched();
  void PrimeOrders(Order * newAI);
  ///Sets the AI to be a specific order
  void SetAI(Order *newAI);
  ///Enqueues an order to the unit's order queue
  void EnqueueAI(Order *newAI);
  ///EnqueuesAI first
  void EnqueueAIFirst (Order * newAI);
  ///num subunits
  void LoadAIScript (const std::string &aiscript);
  bool LoadLastPythonAIScript ();
  bool EnqueueLastPythonAIScript ();
// Uses Order class but just a poiner so ok
// Uses AI so only in NetUnit and Unit classes
  double getMinDis(const QVector &pnt);//for clicklist
// Uses AI stuff so only in NetUnit and Unit classes
  void SetTurretAI ();
  void DisableTurretAI ();
// AI so only in NetUnit and Unit classes
  string getFullAIDescription();
   ///Erases all orders that bitwise OR with that type
// Uses AI so only in NetUnit and Unit classes
  void eraseOrderType (unsigned int type);
  ///Executes 1 frame of physics-based AI
  void ExecuteAI();

/***************************************************************************************/
/**** COLLISION STUFF                                                               ****/
/***************************************************************************************/

public:
  ///The information about the minimum and maximum ranges of this unit. Collide Tables point to this bit of information.
  LineCollide CollideInfo;
  struct collideTrees * colTrees;
  ///Sets the parent to be this unit. Unit never dereferenced for this operation
  void SetCollisionParent (Unit *name);
  ///won't collide with owner
  void SetOwner(Unit *target);
  void SetRecursiveOwner(Unit *target);

  // Shouldn't do anything here - but needed by Python
  ///Queries the BSP tree with a world space st and end point. Returns the normal and distance on the line of the intersection
  Unit * queryBSP (const QVector &st, const QVector & end, Vector & normal, float &distance, bool ShieldBSP=true);
  ///queries the BSP with a world space pnt, radius err.  Returns the normal and distance of the plane to the shield. If Unit returned not NULL, that subunit hit
  Unit * queryBSP (const QVector &pnt, float err, Vector & normal, float &dist,  bool ShieldBSP);

  ///fils in corner_min,corner_max and radial_size
// Uses Box stuff -> only in NetUnit and Unit
  void calculate_extent(bool update_collide_queue);

// To let only in Unit class
///Builds a BSP tree from either the hull or else the current meshdata[] array
void BuildBSPTree (const char *filename, bool vplane=false, Mesh * hull=NULL); //if hull==NULL, then use meshdata **
// Uses mesh stuff (only rSize()) : I have to find something to do
  bool Inside (const QVector &position, const float radius, Vector & normal, float &dist);
// Uses collide and Universe stuff -> put in NetUnit
  void UpdateCollideQueue();
// Uses LineCollide stuff so only in NetUnit and Unit
  const LineCollide &GetCollideInfo () {return CollideInfo;}
// Uses collision stuff so only in NetUnit and Unit classes
  bool querySphere (const QVector &pnt, float err)const;
  ///queries the sphere for beams (world space start,end)  size is added to by my_unit_radius
  float querySphere (const QVector &start, const QVector & end, float my_unit_radius=0)const;
  float querySphereNoRecurse (const QVector &start, const QVector &end, float my_unit_radius=0)const;
  ///queries the ship with a directed ray
  float querySphereClickList (const QVector &st, const QVector &dir, float err) const;//for click list
  ///Queries if this unit is within a given frustum
// Uses GFX -> defined only Unit class
  bool queryFrustum (double frustum[6][4]) const {return false;}

  /**
   *Queries bounding box with a point, radius err
   */
// Uses GFX :(
// Try to use in NetUnit thought
  /**
   *Queries the bounding box with a ray.  1 if ray hits in front... -1 if ray
   * hits behind.
   * 0 if ray misses 
   */
  bool queryBoundingBox (const QVector &pnt, float err);
  int queryBoundingBox(const QVector &origin,const Vector &direction, float err);
  /**Queries the bounding sphere with a duo of mouse coordinates that project
   * to the center of a ship and compare with a sphere...pretty fast*/
  ///queries the sphere for weapons (world space point)
// Only in Unit class
  virtual bool querySphereClickList (int,int, float err, Camera *activeCam) {return false;}


  Unit * BeamInsideCollideTree(const QVector &start, const QVector &end, QVector & pos, Vector & norm, double & distance);
  bool InsideCollideTree (Unit * smaller, QVector & bigpos, Vector & bigNormal, QVector & smallpos, Vector & smallNormal, bool bigasteroid=false, bool smallasteroid=false );
  virtual void reactToCollision(Unit * smaller, const QVector & biglocation, const Vector & bignormal, const QVector & smalllocation, const Vector & smallnormal, float dist);
  ///returns true if jump possible even if not taken
// Uses Universe thing
  bool jumpReactToCollision (Unit *smaller);
  ///Does a collision between this and another unit
  bool Collide(Unit * target);
  ///checks for collisions with all beams and other units roughly and then more carefully
  void CollideAll();

/***************************************************************************************/
/**** DOCKING STUFF                                                                 ****/
/***************************************************************************************/

public:
  unsigned char docked;
  ///returns -1 if unit cannot dock, otherwise returns which dock it can dock at
  enum DOCKENUM {NOT_DOCKED=0x0, DOCKED_INSIDE=0x1, DOCKED=0x2, DOCKING_UNITS=0x4};
  int CanDockWithMe (Unit * dockingunit, bool forcedock=false) ;
  int ForceDock (Unit * utdw, int whichdockport);
  void PerformDockingOperations();
  void FreeDockingPort(unsigned int whichport);
  virtual const vector <struct DockingPorts> &DockingPortLocations() { return image->dockingports;}
  char DockedOrDocking()const {return docked;}
  bool IsCleared (Unit * dockignunit);
  bool isDocked ( Unit *dockingUnit);
  bool UnDock (Unit * unitToDockWith);
// Use AI
  bool RequestClearance (Unit * dockingunit);
  bool EndRequestClearance (Unit * dockingunit);
  int  Dock (Unit * unitToDockWith);
  void RestoreGodliness() {
	_Universe->AccessCockpit()->RestoreGodliness();
  }

/***************************************************************************************/
/**** FACTION/FLIGHTGROUP STUFF                                                     ****/
/***************************************************************************************/

public:
  void SetFg (Flightgroup * fg, int fg_snumber);
  ///The faction of this unit
  int faction;
  void SetFaction (int faction);
protected:
  ///the flightgroup this ship is in
  Flightgroup *flightgroup;
  ///the flightgroup subnumber
  int flightgroup_subnumber;
public:
  ///get the flightgroup description
  Flightgroup *getFlightgroup() const { return flightgroup; };
  ///get the subnumber
  int getFgSubnumber() const { return flightgroup_subnumber; };
  ///get the full flightgroup ID (i.e 'green-4')
  const string getFgID();

  vector <struct CargoColor>& FilterDowngradeList (vector <struct CargoColor> & mylist, bool downgrade =true);
  vector <struct CargoColor>& FilterUpgradeList (vector <struct CargoColor> & mylist);

/***************************************************************************************/
/**** MISC STUFF                                                                    ****/
/***************************************************************************************/

protected:
  ///if the unit is a planet, this contains the long-name 'mars-station'
  string fullname;
public:
  void setFullname(string name)  { fullname=name; };
  string getFullname() const { return fullname; };

  ///Is this class a unit
  virtual enum clsptr isUnit()const {return UNITPTR;}
  inline void Ref() {
#ifdef CONTAINER_DEBUG
    CheckUnit(this);
#endif
    ucref++;
  }
  ///Low level list function to reference the unit as being the target of a UnitContainer or Colleciton
  ///Releases the unit from this reference of UnitContainer or Collection
  void UnRef();

  //0 in additive is reaplce  1 is add 2 is mult
  // Put that in NetUnit & AcctUnit with string and with Unit
  UnitImages &GetImageInformation();

  /// sets the full name/fgid for planets
  bool isStarShip(){ if(isUnit()==UNITPTR){ return true;} return false; };
  bool isPlanet(){ if(isUnit()==PLANETPTR){ return true;} return false; };
  bool isJumppoint(){ if(GetDestinations().size()!=0){ return true; } return false; }

// Uses Universe stuff -> maybe only needed in Unit class
  bool isEnemy(Unit *other){ if(FactionUtil::GetIntRelation(this->faction,other->faction)<0.0){ return true; } return false; };
  bool isFriend(Unit *other){ if(FactionUtil::GetIntRelation(this->faction,other->faction)>0.0){ return true; } return false; };
  bool isNeutral(Unit *other){ if(FactionUtil::GetIntRelation(this->faction,other->faction)==0.0){ return true; } return false; };
  float getRelation(Unit *other);
  //can be eaten by tractor or not?  of the thing, added by chuck_starchaser, refined by spiritplumber
  bool tractorable;
public:
  bool isTractorable() const { return tractorable && !docked; }
};

///Holds temporary values for inter-function XML communication Saves deprecated restr info
struct Unit::XML {
  float randomstartframe;
  float randomstartseconds;
  //  vector<Halo*> halos;
  vector<Mount *> mountz;
  vector<Mesh*> meshes;
  vector<string> meshes_str;
  Mesh * shieldmesh;
  Mesh * bspmesh;
  Mesh * rapidmesh;
  string shieldmesh_str;
  string bspmesh_str;
  string rapidmesh_str;
  void * data;
  vector<Unit*> units;
  int unitlevel;
  bool hasBSP;
  bool hasColTree;
  enum restr {YRESTR=1, PRESTR=2, RRESTR=4};
  const char * unitModifications;
  char yprrestricted;
  float unitscale;
  float ymin, ymax, ycur;
  float pmin, pmax, pcur;
  float rmin, rmax, rcur;
  std::string cargo_category;
  std::string hudimage;
  int damageiterator;
  bool calculated_role;
};

inline void UnitCollection::UnitIterator::GetNextValidUnit () {
  while (pos->next->unit?pos->next->unit->Killed():false) {
    remove();
  }
};

inline Unit * UnitContainer::GetUnit() {
  if (unit==NULL)
    return NULL;
#ifdef CONTAINER_DEBUG
  CheckUnit(unit);
#endif
  if (unit->Killed()) {
    unit->UnRef();
    unit = NULL;
    return NULL;
  }

  return unit;
}


#endif

