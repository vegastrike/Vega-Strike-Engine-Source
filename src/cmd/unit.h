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
#ifndef _GAMEUNIT_H_
#define _GAMEUNIT_H_
#define CONTAINER_DEBUG
struct GFXColor;

#include "gfx/matrix.h"
#include "gfx/quaternion.h"
#include <string>
//#include "gfx/vdu.h"
#include "gfx/cockpit.h"
#include "gfx/halo_system.h"
#include "script/flightgroup.h"
#include "unit_generic.h"
using std::string;

/**
 * GameUnit contains any physical object that may collide with something
 * And may be physically affected by forces.
 * Units are assumed to have various damage and explode when they are dead.
 * Units may have any number of weapons which, themselves may be units
 * the aistate indicates how the unit will behave in the upcoming phys frame
 */
class GameUnit: public Unit
{
 public:
	class GameMount;
 private:
  ///Unit XML Load information
  struct XML;
  ///Loading information
  XML *xml;
 
  static void beginElement(void *userData, const XML_Char *name, const XML_Char **atts);
  static void endElement(void *userData, const XML_Char *name);

  void beginElement(const std::string &name, const AttributeList &attributes);
  void endElement(const std::string &name);

public:
  ///tries to warp as close to un as possible abiding by the distances of various enemy ships...it might not make it all the way
  virtual void WriteUnit(const char * modificationname="");
  ///Loads a unit from an xml file into a complete datastructure
  virtual void LoadXML(const char *filename, const char * unitModifications="", char * xmlbuffer=0, int buflength=0);

  UnitImages &GetImageInformation();

  bool RequestClearance (Unit * dockingunit);

  ///Loads a user interface for the user to upgrade his ship
  void UpgradeInterface (Unit * base);
  ///The name (type) of this unit shouldn't be public

protected:

/***************************************************************************************/
/**** CONSTRUCTORS / DESCTRUCTOR                                                    ****/
/***************************************************************************************/
  /** Default constructor. This is just to figure out where default
   *  constructors are used. The useless argument will be removed
   *  again later.
   */
  GameUnit( int dummy );

  /** Constructor that creates aa mesh with meshes as submeshes (number
   *  of them) as either as subunit with faction faction
   */
  GameUnit (std::vector <Mesh *> &meshes  , bool Subunit, int faction);

  /** Constructor that creates a mesh from an XML file If it is a
   *  customizedUnit, it will check in that directory in the home dir for
   *  the unit.
   */
  GameUnit( const char *filename,
        bool        SubUnit,
       int         faction,
       std::string customizedUnit=string(""),
       Flightgroup *flightgroup=NULL,
       int         fg_subnumber=0 );

  ~GameUnit();

  friend class UnitFactory;
  friend class GameUnitFactory;

public:
  ///Initialize many of the defaults inherant to the constructor
  void Init();

/***************************************************************************************/
/**** CARGO STUFF                                                                   ****/
/***************************************************************************************/

  void EjectCargo (unsigned int index);
  bool CanAddCargo (const Cargo &carg) const;
  ///The faction of this unit
  vector <struct Cargo>& FilterDowngradeList (vector <struct Cargo> & mylist);
  vector <struct Cargo>& FilterUpgradeList (vector <struct Cargo> & mylist);

/***************************************************************************************/
/**** DOCKING STUFF                                                                 ****/
/***************************************************************************************/

  void PerformDockingOperations();
  void FreeDockingPort(unsigned int whichport);
  bool Dock (Unit * unitToDockWith);

/***************************************************************************************/
/**** GFX/MESHES STUFF                                                              ****/
/***************************************************************************************/

protected:
  ///number of meshes (each with separate texture) this unit has
  std::vector <Mesh *> meshdata;
  int nummesh()const {return ((int)meshdata.size())-1;}
  HaloSystem halos;

public:
  vector <Mesh *> StealMeshes();
  ///Process all meshes to be deleted
  ///Split this mesh with into 2^level submeshes at arbitrary planes
  void Split (int level);
  void FixGauges();
  ///Sets the camera to be within this unit.
  void UpdateHudMatrix(int whichcam);
  ///What's the HudImage of this unit
  Sprite * getHudImage ()const ;
  ///Draws this unit with the transformation and matrix (should be equiv) separately
  virtual void Draw(const Transformation & quat = identity_transformation, const Matrix &m = identity_matrix);
  virtual void DrawNow(const Matrix &m = identity_matrix, float lod=1000000000);
  ///Deprecated
  //virtual void ProcessDrawQueue() {}

/***************************************************************************************/
/**** STAR SYSTEM STUFF                                                             ****/
/***************************************************************************************/

protected:
  void SetPlanetHackTransformation (Transformation *&ct, Matrix *&ctm);
public:
  void SetPlanetOrbitData (PlanetaryTransform *trans);
  PlanetaryTransform *GetPlanetOrbit () const;
  bool TransferUnitToSystem (StarSystem *NewSystem);
  void TransferUnitToSystem (unsigned int whichJumpQueue, class StarSystem *&previouslyActiveStarSystem, bool DoSightAndSound);
  StarSystem * getStarSystem();
  void ActivateJumpDrive (int destination=0);
  void DeactivateJumpDrive ();
  ///Begin and continue explosion
  bool Explode(bool draw, float timeit);
  ///explodes then deletes
  void Destroy();
  const LineCollide &GetCollideInfo () {return CollideInfo;}
  ///Takes out of the collide table for this system.
  void RemoveFromSystem ();
  bool InCorrectStarSystem (StarSystem *active) {return active==activeStarSystem;}
  void Target (Unit * targ);
  void TargetTurret (GameUnit * targ);
  ///Fires all active guns that are or arent Missiles
  void Fire(bool Missile);

/***************************************************************************************/
/**** COLLISION STUFF                                                               ****/
/***************************************************************************************/

protected:
  ///fils in corner_min,corner_max and radial_size
  void calculate_extent(bool update_collide_queue);
  ///Builds a BSP tree from either the hull or else the current meshdata[] array
  void BuildBSPTree (const char *filename, bool vplane=false, Mesh * hull=NULL);//if hull==NULL, then use meshdata **
  ///returns -1 if unit cannot dock, otherwise returns which dock it can dock at

public:
  ///Updates the collide Queue with any possible change in sectors
  void UpdateCollideQueue();
  bool Inside (const QVector &position, const float radius, Vector & normal, float &dist);
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
  ///Sets up a null queue for orders
  void PrimeOrders();
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
  Unit * BeamInsideCollideTree(const QVector &start, const QVector &end, QVector & pos, Vector & norm, double & distance);
  bool InsideCollideTree (Unit * smaller, QVector & bigpos, Vector & bigNormal, QVector & smallpos, Vector & smallNormal);
  virtual void reactToCollision(Unit * smaller, const QVector & biglocation, const Vector & bignormal, const QVector & smalllocation, const Vector & smallnormal, float dist);
  ///returns true if jump possible even if not taken
  bool jumpReactToCollision (Unit *smaller);
  ///Does a collision between this and another unit
  bool Collide(Unit * target);
  ///checks for collisions with all beams and other units roughly and then more carefully
  void CollideAll();

/***************************************************************************************/
/**** PHYSICS STUFF                                                                    */
/***************************************************************************************/

  ///Updates physics given unit space transformations and if this is the last physics frame in the current gfx frame
  virtual void UpdatePhysics (const Transformation &trans, const Matrix &transmat, const Vector & CumulativeVelocity, bool ResolveLast, UnitCollection *uc=NULL);
  class Cockpit * GetVelocityDifficultyMult(float &) const;
  void DamageRandSys (float dam,const Vector &vec);
  ///executes a repair if the repair bot is up to it
  void Repair();
  ///Thrusts by ammt and clamps accordingly (afterburn or not)
  void Thrust(const Vector &amt,bool afterburn = false);
  ///Resolves forces of given unit on a physics frame
  Vector ResolveForces (const Transformation &, const Matrix&);
  ///deletes
  void Kill(bool eraseFromSave=true);
  ///Applies damage to the local area given by pnt
  void ApplyLocalDamage (const Vector &pnt, const Vector & normal, float amt, Unit * affectedSubUnit, const GFXColor &, float phasedamage=0);
  ///Applies damage to the pre-transformed area of the ship
  void ApplyDamage (const Vector & pnt, const Vector & normal, float amt, Unit * affectedSubUnit, const GFXColor &,  Unit *ownerDoNotDereference, float phasedamage=0 );
  ///Deals remaining damage to the hull at point and applies lighting effects
  float DealDamageToHull (const Vector &pnt, float Damage, unsigned short * t=NULL);
  ///applies damage from the given pnt to the shield, and returns % damage applied and applies lighitn
  float DealDamageToShield (const Vector & pnt, float &Damage);

  void scanSystem();

/***************************************************************************************/
/**** CUSTOMIZE/UPGRADE STUFF                                                          */
/***************************************************************************************/

  bool UpgradeSubUnits (Unit * up, int subunitoffset, bool touchme, bool downgrade, int &numave, double &percentage);
  double Upgrade (const std::string &file, int mountoffset, int subunitoffset, bool force, bool loop_through_mounts);

/***************************************************************************************/
/**** AI STUFF                                                                         */
/***************************************************************************************/

  ///Erases all orders that bitwise OR with that type
  void eraseOrderType (unsigned int type);
  ///Executes 1 frame of physics-based AI
  void ExecuteAI();

 public:
  ///tries to warp as close to un as possible abiding by the distances of various enemy ships...it might not make it all the way
  bool AutoPilotTo(Unit * un, bool ignore_friendlies=false);
  void SetTurretAI ();
  void DisableTurretAI ();
  string getFullAIDescription();
  ///not used yet
  void setTargetFg(string primary,string secondary=string(),string tertiary=string());
  ///not used yet
  void ReTargetFg(int which_target=0);
  ///not used yet

  float getRelation(Unit *other);
};

class GameUnit::GameMount: public Unit::Mount {
    ///Where is it
    //Transformation LocalPosition;
  public:
    void ReplaceMounts (const Mount * othermount);

	GameMount();
	// Requires weapon_xml.cpp stuff so Beam stuff so GFX and AUD stuff
	GameMount(const std::string& name, short int am=-1, short int vol=-1);
    ///Turns off a firing beam (upon key release for example)
    void UnFire();
    /**
     *  Fires a beam when the firing unit is at the Cumulative location/transformation 
     * owner (won't crash into)  as owner and target as missile target. bool Missile indicates if it is a missile
     * should it fire
     */ 
	// Uses Sound Forcefeedback and other stuff
	void PhysicsAlignedUnfire();
	bool PhysicsAlignedFire (const Transformation &Cumulative, const Matrix & mat, const Vector & Velocity, Unit *owner,  Unit *target, signed char autotrack, float trackingcone);//0 is no track...1 is target 2 is target + lead
	bool Fire (Unit *owner, bool Missile=false);
};

///Holds temporary values for inter-function XML communication Saves deprecated restr info
struct GameUnit::XML {
  //  vector<Halo*> halos;
  vector<Unit::Mount *> mountz;
  vector<Mesh*> meshes;
  Mesh * shieldmesh;
  Mesh * bspmesh;
  Mesh * rapidmesh;
  void * data;
  vector<GameUnit*> units;
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

  /** 
   * Contains information about a particular Mount on a unit.
   * And the weapons it has, where it is, where it's aimed, 
   * The ammo and the weapon type. As well as the possible weapons it may fit
   * Warning: type has a string inside... cannot be memcpy'd
   */

#endif
