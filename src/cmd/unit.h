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
#ifndef _UNIT_H_
#define _UNIT_H_


struct GFXColor;
#include "vegastrike.h"

#include "gfx/matrix.h"
#include "gfx/quaternion.h"
#include <string>
#include "weapon_xml.h"
#include "linecollide.h"
//#include "gfx/vdu.h"
#include "xml_support.h"
#include "container.h"
#include "collection.h"
#include "gfx/cockpit.h"
#include "gfx/halo_system.h"
using std::string;

class Flightgroup;
class Nebula;
extern float capship_size;
//#include "mission.h"
class Beam;
class Animation;
using namespace XMLSupport;

class Sprite;
class Order;
class Box;
class Mesh;
class Camera;


class PlanetaryOrbit;
class UnitCollection;
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

class VDU;
struct UnitImages;
struct UnitSounds;
struct Cargo;

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
  bool UpgradeSubUnits (Unit * up, int subunitoffset, bool touchme, bool downgrade, int &numave, double &percentage);
  bool UpgradeMounts (Unit * up, int subunitoffset, bool touchme, bool downgrade, int &numave, Unit * templ, double &percentage);

  Nebula * nebula;
  PlanetaryOrbitData * planet;
  ///The orbit needs to have access to the velocity directly to disobey physics laws to precalculate orbits
  friend class PlanetaryOrbit;
  friend class ContinuousTerrain;
  ///VDU needs mount data to draw weapon displays
  friend class VDU;
  friend class UpgradingInfo;//needed to actually upgrade unit through interface
 public:
  void DamageRandSys (float dam,const Vector &vec);
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
      ///does this radar support IFF?
      bool color;
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
    float max_speed; float max_ab_speed;
    ///Computer's restrictions of YPR to limit space combat maneuvers
    float max_yaw; float max_pitch; float max_roll;
    ///Whether or not an 'lead' indicator appears in front of target
    unsigned char slide_start;
    unsigned char slide_end;
    bool itts;
  };

 private:
  ///Unit XML Load information
  struct XML;
  ///Loading information
  XML *xml;

  ///Loads a unit from an xml file into a complete datastructure
  void LoadXML(const char *filename, const char * unitModifications="");

  static void beginElement(void *userData, const XML_Char *name, const XML_Char **atts);
  static void endElement(void *userData, const XML_Char *name);

  void beginElement(const std::string &name, const AttributeList &attributes);
  void endElement(const std::string &name);

 protected:
  virtual float ExplosionRadius();
  StarSystem * activeStarSystem;//the star system I'm in
  bool BuyCargo (unsigned int i, unsigned int quantity, Unit * buyer, float &creds);
  bool BuyCargo (const std::string &cargo,unsigned int quantity, Unit * buyer, float & creds);
  
  void SetPlanetHackTransformation (Transformation *&ct, Matrix *&ctm);

  UnitSounds * sound;
  ///The owner of this unit. This may not collide with owner or units owned by owner. Do not dereference (may be dead pointer)
  Unit *owner;
  ///The previous state in last physics frame to interpolate within
  Transformation prev_physical_state;
  ///The state of the current physics frame to interpolate within
  Transformation curr_physical_state;
  ///The cumulative (incl subunits parents' transformation)
  Matrix cumulative_transformation_matrix;
  ///The cumulative (incl subunits parents' transformation)
  Transformation cumulative_transformation;
  ///The velocity this unit has in World Space
  Vector cumulative_velocity;
  ///The information about the minimum and maximum ranges of this unit. Collide Tables point to this bit of information.
  LineCollide CollideInfo;
  ///The image that will appear on those screens of units targetting this unit
  UnitImages *image;
  ///number of meshes (each with separate texture) this unit has
  int nummesh;
  ///The pointers to the mesh data of this mesh
  Mesh **meshdata;
  ///are shields tight to the hull.  zero means bubble
  float shieldtight;
  /// the turrets and spinning parts fun fun stuff
  UnitCollection SubUnits; 
  HaloSystem halos;
  ///NUmber of weapons on this unit
  int nummounts;
  /** 
   * Contains information about a particular Mount on a unit.
   * And the weapons it has, where it is, where it's aimed, 
   * The ammo and the weapon type. As well as the possible weapons it may fit
   * Warning: type has a string inside... cannot be memcpy'd
   */
  class Mount {
    ///Where is it
    Transformation LocalPosition;
  public:
    void SwapMounts (Mount & othermount);
    double Percentage (const Mount& oldmount) const;
    union REF{
      ///only beams are actually coming out of the gun at all times...bolts, balls, etc aren't
      Beam *gun;
      ///Other weapons must track refire times
      float refire;
    } ref;
    ///the size that this mount can hold. May be any bitwise combination of weapon_info::MOUNT_SIZE

    short size;
    ///-1 is infinite
    short ammo;
    short volume;//-1 is infinite
    ///The data behind this weapon. May be accordingly damaged as time goes on
    enum MOUNTSTATUS{PROCESSED,UNFIRED,FIRED} processed;
    ///Status of the selection of this weapon. Does it fire when we hit space
    enum {ACTIVE, INACTIVE, DESTROYED, UNCHOSEN} status;
    ///The sound this mount makes when fired
    const weapon_info *type;
    int sound;
    
    float time_to_lock;
    Mount();
    Mount(const std::string& name, short int ammo=-1, short int volume=-1);
    ///Sets this gun to active, unless unchosen or destroyed
    void Activate (bool Missile);
    ///Sets this gun to inactive, unless unchosen or destroyed
    void DeActive (bool Missile);
    ///Sets this gun's position on the mesh
    void SetMountPosition (const Transformation &t) {LocalPosition = t;}
    ///Gets the mount's position and transform
    Transformation &GetMountLocation () {return LocalPosition;}
    ///Turns off a firing beam (upon key release for example)
    void UnFire();
    /**
     *  Fires a beam when the firing unit is at the Cumulative location/transformation 
     * owner (won't crash into)  as owner and target as missile target. bool Missile indicates if it is a missile
     * should it fire
     */ 
    void PhysicsAlignedUnfire();
    bool PhysicsAlignedFire (const Transformation &Cumulative, const Matrix & mat, const Vector & Velocity, Unit *owner,  Unit *target, signed char autotrack, float trackingcone);//0 is no track...1 is target 2 is target + lead
    bool Fire (Unit *owner, bool Missile=false);
  } *mounts;
  ///Mount may access unit
  friend class Unit::Mount;
  ///Activates all guns of that size
  void ActivateGuns (weapon_info::MOUNT_SIZE, bool Missile);
  ///Armor values: how much damage armor can withhold before internal damage accrues
  struct {
    unsigned short front, back, right, left;
  } armor;
  ///Shielding Struct holding values of current shields
  struct {
    ///How much the shield recharges per second
    float recharge; 
    ///A union containing the different shield values and max values depending on number
    union {
      ///if shield is 2 big, 2 floats make this shield up, and 2 floats for max {front,back,frontmax,backmax}
      float fb[4];
      ///If the shield if 4 big, 4 floats make the shield up, and 4 keep track of max recharge value
      struct {
	unsigned short front, back, right, left;
	unsigned short frontmax, backmax, rightmax, leftmax;
      }fbrl;
      ///If the shield is 6 sided, 6 floats make it up, 2 indicating the max value of various sides, and 6 being the actual vals
      struct {
	unsigned short v[6];
	unsigned short fbmax,rltbmax;
      }fbrltb;
    };
    ///the number of shields in the current shielding struct
    signed char number;
    ///What percentage leaks (divide by 100%)
    char leak; 
  } shield;
  float MaxShieldVal() const;
  ///regenerates all 2,4, or 6 shields for 1 SIMULATION_ATOM
  void RegenShields();
  ///The structual integ of the current unit
  float hull;
  ///The radar limits (range, cone range, etc) 
  ///the current order
  Order *aistate;
  ///how much the energy recharges per second
  float recharge;
  ///maximum energy
  unsigned short maxenergy;
  ///current energy
  unsigned short energy;
  ///mass of this unit (may change with cargo)
  float mass;
  ///fuel of this unit
  float fuel;
  unsigned short afterburnenergy;
  ///-1 means it is off. -2 means it doesn't exist. otherwise it's engaged to destination (positive number)
  struct UnitJump {
    short energy;
    signed char drive;
    unsigned char delay;
    unsigned char damage;
    //negative means fuel
  } jump;
  ///Moment of intertia of this unit
  float MomentOfInertia;
  ///The force applied from outside accrued over the whole physics frame
  Vector NetForce;
  ///The force applied by internal objects (thrusters)
  Vector NetLocalForce;
  ///The torque applied from outside objects
  Vector NetTorque;
  ///The torque applied from internal objects
  Vector NetLocalTorque;
  ///the current velocities in LOCAL space (not world space)
  Vector AngularVelocity;  Vector Velocity;

  /*
    //FIXME Deprecated turret restrictions (may bring back)
  enum restr {YRESTR=1, PRESTR=2, RRESTR=4};

  char yprrestricted;

  float ymin, ymax, ycur;
  float pmin, pmax, pcur;
  float rmin, rmax, rcur;
  */
  /// thrusting limits (acceleration-wise
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
  } limits;
  Computer computer;
  ///no collision table presence.
  bool SubUnit;
  ///-1 is not available... ranges between 0 32767 for "how invisible" unit currently is (32768... -32768) being visible)
  short cloaking;
  ///the minimum cloaking value...
  short cloakmin;
  ///Should draw selection box?
  bool selected;  

  ///Is dead already?
  bool killed;
  ///Should not be drawn
  bool invisible;
  
  enum {NOT_DOCKED=0x0, DOCKED_INSIDE=0x1, DOCKED=0x2, DOCKING_UNITS=0x4} DOCKENUM;
  unsigned char docked;
  ///How many lists are referencing us
  int ucref;
  ///How big is this unit
  float radial_size;
  /// corners of object  
  Vector corner_min, corner_max; 
  struct collideTrees * colTrees;
  ///fils in corner_min,corner_max and radial_size
  void calculate_extent(bool update_collide_queue);
  ///applies damage from the given pnt to the shield, and returns % damage applied and applies lighitn
  float DealDamageToShield (const Vector & pnt, float &Damage);
  ///Sets the parent to be this unit. Unit never dereferenced for this operation
  void SetCollisionParent (Unit *name);
  ///If the shields are up from this position
  bool ShieldUp (const Vector &) const;
  ///Builds a BSP tree from either the hull or else the current meshdata[] array
  void BuildBSPTree (const char *filename, bool vplane=false, Mesh * hull=NULL);//if hull==NULL, then use meshdata **
  ///returns -1 if unit cannot dock, otherwise returns which dock it can dock at
  int CanDockWithMe (Unit * dockingunit) ;
  void PerformDockingOperations ();
  void FreeDockingPort(unsigned int whichport);
  void SetRecursiveOwner(Unit *target);
  //0 in additive is reaplce  1 is add 2 is mult
  bool UpAndDownGrade (Unit * up, Unit * templ, int mountoffset, int subunitoffset, bool touchme, bool downgrade, int additive, bool forcetransaction, double &percentage);
public:
  int LockMissile();//-1 is no lock necessary 1 is locked
  float TrackingGuns(bool &missileLock);

  void EjectCargo (unsigned int index);
  void FixGauges();
  bool CanAddCargo (const Cargo &carg) const;
  void AddCargo (const Cargo &carg,bool sort=true);
  int RemoveCargo (unsigned int i, int quantity, bool eraseZero=true);
  float PriceCargo (const std::string &s);
  //  void SwapOutHalos();
  //  void SwapInHalos();
  void SetFg (Flightgroup * fg, int fg_snumber);
  UnitImages &GetImageInformation();
  Cargo & GetCargo (unsigned int i);
  void GetCargoCat (const std::string &category, vector <Cargo> &cat);
  ///below function returns NULL if not found
  Cargo * GetCargo (const std::string &s, unsigned int &i);
  unsigned int numCargo ()const;
  std::string GetManifest (unsigned int i, Unit * scanningUnit, const Vector & original_velocity) const;
  bool SellCargo (unsigned int i, int quantity, float &creds, Cargo & carg, Unit *buyer);
  bool SellCargo (const std::string &s, int quantity, float & creds, Cargo &carg, Unit *buyer);
  bool BuyCargo (const Cargo &carg, float & creds);
  bool IsCleared (Unit * dockignunit);
  const vector <struct DockingPorts> &DockingPortLocations();
  void ImportPartList (const std::string& category, float price, float pricedev,  float quantity, float quantdev);
  bool RequestClearance (Unit * dockingunit);
  bool isDocked (Unit *dockingUnit);
  bool Dock (Unit * unitToDockWith);
  bool UnDock (Unit * unitToDockWith);
  ///Does a one way collision between smaller target and this
  bool Inside (const QVector &position, const float radius, Vector & normal, float &dist);
  void SetPlanetOrbitData (PlanetaryTransform *trans);
  PlanetaryTransform *GetPlanetOrbit () const;
  ///Updates the collide Queue with any possible change in sectors
  void UpdateCollideQueue();
  ///Loads a user interface for the user to upgrade his ship
  void UpgradeInterface (Unit * base);
  ///The name of this unit
  std::string name;
  ///The faction of this unit
  int faction;
  void SetFaction (int faction);
  bool canUpgrade (Unit * upgrador, int mountoffset,  int subunitoffset, int additive, bool force,  double & percentage, Unit * templ=NULL);
  bool Upgrade (Unit * upgrador, int mountoffset,  int subunitoffset, int additive, bool force,  double & percentage, Unit * templ=NULL);
  bool canDowngrade (Unit *downgradeor, int mountoffset, int subunitoffset, double & percentage);
  bool Downgrade (Unit * downgradeor, int mountoffset, int subunitoffset,  double & percentage);
  vector <struct Cargo>& FilterDowngradeList (vector <struct Cargo> & mylist);
  vector <struct Cargo>& FilterUpgradeList (vector <struct Cargo> & mylist);

private:
  /// forbidden
  Unit( const Unit& ); 
  
  /// forbidden
  Unit& operator=( const Unit& );
  
  /** default constructor
   */
  Unit();
    
protected:
  /** Default constructor. This is just to figure out where default
   *  constructors are used. The useless argument will be removed
   *  again later.
   */
  Unit( int dummy );
  
  /** Constructor that creates aa mesh with meshes as submeshes (number
   *  of them) as either as subunit with faction faction
   */
  Unit (Mesh ** meshes  , int num, bool Subunit, int faction);

  /** Constructor that creates a mesh from an XML file If it is a
   *  customizedUnit, it will check in that directory in the home dir for
   *  the unit.
   */
  Unit( const char *filename,
        bool        SubUnit,
       int         faction,
       std::string customizedUnit=string(""),
       Flightgroup *flightgroup=NULL,
       int         fg_subnumber=0 );
    
  friend class UnitFactory;
    
public:
  virtual ~Unit();

  vector <Mesh *> StealMeshes();
  ///Changes currently selected weapon
  void ToggleWeapon (bool Missile);
  ///Selects all weapons
  void SelectAllWeapon (bool Missile);
  ///Is this class a unit
  virtual enum clsptr isUnit() {return UNITPTR;}
  ///Process all meshes to be deleted
  static void ProcessDeleteQueue();
  ///Split this mesh with into 2^level submeshes at arbitrary planes
  void Split (int level);
  bool TransferUnitToSystem (StarSystem *NewSystem);
  void TransferUnitToSystem (unsigned int whichJumpQueue, class StarSystem *&previouslyActiveStarSystem, bool DoSightAndSound);
  ///Initialize many of the defaults inherant to the constructor
  void Init();
  void ActivateJumpDrive (int destination=0);
  void DeactivateJumpDrive ();
  const UnitJump &GetJumpStatus() const {return jump;}
  ///Begin and continue explosion
  bool Explode(bool draw, float timeit);
  ///explodes then deletes
  void Destroy();
  const LineCollide &GetCollideInfo () {return CollideInfo;}
  bool InRange (Unit *target, Vector &localcoord, bool cone=true) const{
    QVector delta( target->Position()-Position());
    localcoord =ToLocalCoordinates(Vector (delta.i,delta.j,delta.k));
    float mm= localcoord.Magnitude();
    if (owner==target||this==target||((mm-rSize()-target->rSize())>computer.radar.maxrange&&target->isUnit()!=PLANETPTR)||((localcoord.k/mm)<computer.radar.maxcone&&cone)||target->CloakVisible()<.8||target->rSize()<computer.radar.mintargetsize) {
		
		if (target->rSize()<capship_size||this==target) 
			return false;
    }
    return true;
  }
  ///how visible the ship is from 0 to 1
  float CloakVisible() const {
    if (cloaking<0)
      return 1;
    return ((float)cloaking)/32767;
  }

  ///cloaks or decloaks the starship depending on the bool
  void Cloak (bool cloak);
  ///deletes
  virtual void Kill(bool eraseFromSave=true);
  ///Is dead yet?
  inline bool Killed() const {return killed;}
  ///Takes out of the collide table for this system.
  void RemoveFromSystem ();
  ///Low level list function to reference the unit as being the target of a UnitContainer or Colleciton
  inline void Ref() {ucref++;}
  ///Releases the unit from this reference of UnitContainer or Collection
  void UnRef();
  ///Gets the average gun speed of the unit::caution SLOW
  void getAverageGunSpeed (float & speed, float & range) const;
  ///Finds the position from the local position if guns are aimed at it with speed
  QVector PositionITTS (const QVector & local_posit, float speed) const;
  ///The cosine of the angle to the target given passed in speed and range
  float cosAngleTo (Unit * target, float & distance, float speed= 0.001, float range=0.001) const;
  ///Highest cosine from given mounts to target. Returns distance and cosine
  float cosAngleFromMountTo (Unit * target, float & distance) const;
  ///won't collide with owner
  void SetOwner(Unit *target);
  bool InCorrectStarSystem (StarSystem *active) {return active==activeStarSystem;}
  Unit *Target() {return computer.target.GetUnit(); }
  Unit *VelocityReference() {return computer.velocity_ref.GetUnit(); }
  Unit *Threat() {return computer.threat.GetUnit(); }
  void Target (Unit * targ);
  void VelocityReference (Unit *targ);
  void TargetTurret (Unit * targ);
  ///Threatens this unit with "targ" as aggressor. Danger should be cos angle to target
  void Threaten (Unit * targ, float danger);
  ///Rekeys the threat level to zero for another turn of impending danger
  void ResetThreatLevel() {computer.threatlevel=0;}
  ///Fires all active guns that are or arent Missiles
  void Fire(bool Missile);
  ///Stops all active guns from firing
  void UnFire();
  ///returns percentage of course deviation for contraband searches.  .5 causes error and 1 causes them to get mad 
  float CourseDeviation (const Vector &OriginalCourse, const Vector &FinalCourse)const ;
  Computer & GetComputerData () {return computer;}
  const Computer & ViewComputerData () const {return computer;}
  float computeLockingPercent();//how locked are we
  float FShieldData() const;  float RShieldData() const;  float LShieldData() const;  float BShieldData() const;
  void ArmorData(unsigned short armor[4])const;
  ///returns the current ammt of armor left
  float FuelData() const;
  ///Returns the current ammt of energy left
  float EnergyData() const;
  ///Gets the current status of the hull
  float GetHull() const{return hull;}
  ///Sets the camera to be within this unit.
  void UpdateHudMatrix(int whichcam);
  ///Returns the current AI state of the current unit for modification
  Order *getAIState() const{return aistate;}
  ///Should we resolve forces on this unit (is it free to fly or in orbit)
  bool resolveforces;
  ///What's the size of this unit
  float rSize () const {return radial_size;}
  ///What's the HudImage of this unit
  Sprite * getHudImage ()const ;
  ///Returns the cockpit name so that the controller may load a new cockpit
  std::string getCockpit ()const;
  ///Draws this unit with the transformation and matrix (should be equiv) separately
  virtual void Draw(const Transformation & quat = identity_transformation, const Matrix &m = identity_matrix);
  ///Deprecated
  //virtual void ProcessDrawQueue();
  ///Gets the minimum distance away from the point in 3space
  double getMinDis(const QVector &pnt);//for clicklist
  ///queries the sphere for weapons (world space point)
  bool querySphere (const QVector &pnt, float err) const;
  ///queries the sphere for beams (world space start,end)  size is added to by my_unit_radius
  float querySphere (const QVector &start, const QVector & end, float my_unit_radius=0) const;
  float querySphereNoRecurse (const QVector &start, const QVector &end, float my_unit_radius=0) const ;
  ///queries the ship with a directed ray
  float querySphereClickList (const QVector &st, const QVector &dir, float err) const;//for click list
  ///Queries the BSP tree with a world space st and end point. Returns the normal and distance on the line of the intersection
  Unit * queryBSP (const QVector &st, const QVector & end, Vector & normal, float &distance, bool ShieldBSP=true);
  ///queries the BSP with a world space pnt, radius err.  Returns the normal and distance of the plane to the shield. If Unit returned not NULL, that subunit hit
  Unit * queryBSP (const QVector &pnt, float err, Vector & normal, float &dist,  bool ShieldBSP);
  ///Queries if this unit is within a given frustum
  bool queryFrustum (float frustum[6][4]) const;

  /**
   *Queries bounding box with a point, radius err
   */
  bool queryBoundingBox (const QVector &pnt, float err);
  /**
   *Queries the bounding box with a ray.  1 if ray hits in front... -1 if ray
   * hits behind.
   * 0 if ray misses 
   */
  int queryBoundingBox(const QVector &origin,const Vector &direction, float err);
  /**Queries the bounding sphere with a duo of mouse coordinates that project
   * to the center of a ship and compare with a sphere...pretty fast*/
  bool querySphereClickList (int,int, float err, Camera *activeCam);
  ///Turns on selection box
  void Select();
  ///Turns off selection box
  void Deselect();
  ///Sets up a null queue for orders
  void PrimeOrders();
  ///Sets the AI to be a specific order
  void SetAI(Order *newAI);
  ///Enqueues an order to the unit's order queue
  void EnqueueAI(Order *newAI);
  ///EnqueuesAI first
  void EnqueueAIFirst (Order * newAI);
  ///num subunits

  un_iter getSubUnits();
  un_kiter viewSubUnits() const;
  bool InsideCollideTree (Unit * smaller, QVector & bigpos, Vector & bigNormal, QVector & smallpos, Vector & smallNormal);
  virtual void reactToCollision(Unit * smaller, const QVector & biglocation, const Vector & bignormal, const QVector & smalllocation, const Vector & smallnormal, float dist);
  ///returns true if jump possible even if not taken
  bool jumpReactToCollision (Unit *smaller);
  ///Does a collision between this and another unit
  bool Collide(Unit * target);
  ///checks for collisions with all beams and other units roughly and then more carefully
  void CollideAll();
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
  void ApplyLocalDamage (const Vector &pnt, const Vector & normal, float amt, Unit * affectedSubUnit, const GFXColor &, float phasedamage=0);
  ///Applies damage to the pre-transformed area of the ship
  void ApplyDamage (const Vector & pnt, const Vector & normal, float amt, Unit * affectedSubUnit, const GFXColor &,  Unit *ownerDoNotDereference, float phasedamage=0 );
  ///Deals remaining damage to the hull at point and applies lighting effects
  float DealDamageToHull (const Vector &pnt, float Damage);

  ///Clamps thrust to the limits struct
  Vector ClampThrust(const Vector &thrust, bool afterburn);
  ///Takes a unit vector for direction of thrust and scales to limits
  Vector MaxThrust(const Vector &thrust);
  ///Thrusts by ammt and clamps accordingly (afterburn or not)
  void Thrust(const Vector &amt,bool afterburn = false);
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
  virtual void UpdatePhysics (const Transformation &trans, const Matrix &transmat, const Vector & CumulativeVelocity, bool ResolveLast, UnitCollection *uc=NULL);
  ///Resolves forces of given unit on a physics frame
  void ResolveForces (const Transformation &, const Matrix&);
  ///Returns the pqr oritnattion of the unit in world space
  void SetOrientation (Vector q, Vector r);
  void GetOrientation(Vector &p, Vector &q, Vector &r) const;
  ///Transforms a orientation vector Up a coordinate level. Does not take position into account
  Vector UpCoordinateLevel(const Vector &v) const;
  ///Transforms a orientation vector from world space to local space. Does not take position into account 
  Vector ToLocalCoordinates(const Vector &v) const;
  ///Transforms a orientation vector to world space. Does not take position into account
  Vector ToWorldCoordinates(const Vector &v) const;
  ///Returns unit-space ang velocity
  const Vector &GetAngularVelocity() const { return AngularVelocity; }
  ///Return unit-space velocity
  const Vector &GetVelocity() const { return cumulative_velocity; }
  void SetVelocity (const Vector & v) {Velocity = v;}
  void SetAngularVelocity (const Vector & v) {AngularVelocity = v;}
  
  float GetMoment() const { return MomentOfInertia; }
  float GetMass() const { return mass; }
  ///returns the ammt of elasticity of collisions with this unit
  float GetElasticity ();
  ///returns given limits (should not be necessary with clamping functions)
  const Limits &Limits() const { return limits; }
  ///Sets if forces should resolve on this unit or not
  void SetResolveForces(bool);
  ///Executes 1 frame of physics-based AI
  void ExecuteAI();

 private:
  ///the flightgroup this ship is in
  Flightgroup *flightgroup;
  ///the flightgroup subnumber
  int flightgroup_subnumber;
  ///not used yet
  string target_fgid[3];
  void RechargeEnergy();
 protected:
  static std::string massSerializer(const struct XMLType &input, void*mythis);
  static std::string cargoSerializer(const struct XMLType &input, void*mythis);
  static std::string mountSerializer(const struct XMLType &input, void*mythis);
  static std::string shieldSerializer(const struct XMLType &input, void*mythis);
  static std::string subunitSerializer(const struct XMLType &input, void*mythis);
  ///if the unit is a planet, this contains the long-name 'mars-station'
  string fullname;
  void SortCargo();
 public:
  ///tries to warp as close to un as possible abiding by the distances of various enemy ships...it might not make it all the way
  bool AutoPilotTo(Unit * un);
  void WriteUnit(const char * modificationname="");
  void SetTurretAI ();
  void DisableTurretAI ();
  ///get the flightgroup description
  Flightgroup *getFlightgroup() const { return flightgroup; };
  ///reduces shields to X percentage and reduces shield recharge to Y percentage
  void leach (float XshieldPercent, float YrechargePercent, float ZenergyPercent);

  ///get the subnumber
  int getFgSubnumber() const { return flightgroup_subnumber; };
  ///get the full flightgroup ID (i.e 'green-4')
  const string getFgID();
  /// sets the full name/fgid for planets
  void setFullname(string name)  { fullname=name; };
  string getFullAIDescription();
  ///not used yet
  void setTargetFg(string primary,string secondary=string(),string tertiary=string());
  ///not used yet
  void ReTargetFg(int which_target=0);
  ///not used yet
  int getNumAttackers();

  bool isStarShip(){ if(isUnit()==UNITPTR){ return true;} return false; };
  bool isPlanet(){ if(isUnit()==PLANETPTR){ return true;} return false; };
  bool isJumppoint(){ if(GetDestinations().size()!=0){ return true; } return false; }

  bool isEnemy(Unit *other){ if(_Universe->GetRelation(this->faction,other->faction)<0.0){ return true; } return false; };
  bool isFriend(Unit *other){ if(_Universe->GetRelation(this->faction,other->faction)>0.0){ return true; } return false; };
  bool isNeutral(Unit *other){ if(_Universe->GetRelation(this->faction,other->faction)==0.0){ return true; } return false; };
  float getRelation(Unit *other){ return(_Universe->GetRelation(this->faction,other->faction)); };

  // for scanning purposes
 private:
  struct Scanner scanner;
 public:
  void scanSystem();
  struct Scanner *getScanner() { return &scanner; };
};
///Holds temporary values for inter-function XML communication Saves deprecated restr info
struct Unit::XML {
  //  vector<Halo*> halos;
  vector<Unit::Mount *> mountz;
  vector<Mesh*> meshes;
  Mesh * shieldmesh;
  Mesh * bspmesh;
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
};
inline void UnitCollection::UnitIterator::GetNextValidUnit () {
  while (pos->next->unit?pos->next->unit->Killed():false) {
    remove();
  }
}

inline Unit * UnitContainer::GetUnit() {
  if (unit==NULL)
    return NULL;
  if (unit->Killed()) {
    unit->UnRef();
    unit = NULL;
    return NULL;
  }
  return unit;
}


#endif

