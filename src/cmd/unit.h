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
#include "gfxlib.h"
#include "mount.h"

struct CargoColor {
  Cargo cargo;
  GFXColor color;
  CargoColor () 
    : cargo(), color(1,1,1,1) {
  }
};//A stupid struct that is only for grouping 2 different types of variables together in one return value


/**
 * GameUnit contains any physical object that may collide with something
 * And may be physically affected by forces.
 * Units are assumed to have various damage and explode when they are dead.
 * Units may have any number of weapons which, themselves may be units
 * the aistate indicates how the unit will behave in the upcoming phys frame
 */
template <class UnitType> class GameUnit: public UnitType
{ 
public:

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

public:
  ///Initialize many of the defaults inherant to the constructor
  void Init();

/***************************************************************************************/
/**** CARGO STUFF                                                                   ****/
/***************************************************************************************/

  void EjectCargo (unsigned int index);
  bool CanAddCargo (const Cargo &carg) const;
  ///The faction of this unit
  vector <class CargoColor>& FilterDowngradeList (vector <class CargoColor> & mylist,bool downgrade=true);
  vector <class CargoColor>& FilterUpgradeList (vector <class CargoColor> & mylist);

/***************************************************************************************/
/**** DOCKING STUFF                                                                 ****/
/***************************************************************************************/

  void RestoreGodliness() {
	_Universe->AccessCockpit()->RestoreGodliness();
  }

/***************************************************************************************/
/**** GFX/MESHES STUFF                                                              ****/
/***************************************************************************************/

public:
  HaloSystem halos;
protected:
  int nummesh()const {return ((int)meshdata.size())-1;}

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
  void addHalo( const char * filename, const QVector &loc, const Vector &size, const GFXColor & col, std::string halo_type)
  {
	 halos.AddHalo (filename,loc,size,col,halo_type);
  }

/***************************************************************************************/
/**** STAR SYSTEM STUFF                                                             ****/
/***************************************************************************************/

public:
  void SetPlanetOrbitData (PlanetaryTransform *trans);
  PlanetaryTransform *GetPlanetOrbit () const;
  //bool TransferUnitToSystem (StarSystem *NewSystem);
  void TransferUnitToSystem (unsigned int whichJumpQueue, class StarSystem *&previouslyActiveStarSystem, bool DoSightAndSound);
  StarSystem * getStarSystem();
  //void ActivateJumpDrive (int destination=0);
  //void DeactivateJumpDrive ();
  ///Begin and continue explosion
  bool Explode(bool draw, float timeit);
  void Kill(bool eraseFromSave=true);
  const LineCollide &GetCollideInfo () {return CollideInfo;}
  ///Takes out of the collide table for this system.
  void RemoveFromSystem ();
  void Target (Unit * targ);
  void TargetTurret (Unit * targ);
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
  bool queryFrustum (double frustum[6][4]) const;

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
  Unit * BeamInsideCollideTree(const QVector &start, const QVector &end, QVector & pos, Vector & norm, double & distance);
  bool InsideCollideTree (Unit * smaller, QVector & bigpos, Vector & bigNormal, QVector & smallpos, Vector & smallNormal);
  //virtual void reactToCollision(Unit * smaller, const QVector & biglocation, const Vector & bignormal, const QVector & smalllocation, const Vector & smallnormal, float dist);
  ///returns true if jump possible even if not taken
  bool jumpReactToCollision (Unit *smaller);
  ///Does a collision between this and another unit
  bool Collide(Unit * target);
  ///checks for collisions with all beams and other units roughly and then more carefully
  void CollideAll();

/***************************************************************************************/
/**** PHYSICS STUFF                                                                    */
/***************************************************************************************/

  void SetPlanetHackTransformation (Transformation *&ct,Matrix *&ctm);
  bool AutoPilotTo(Unit * un, bool ignore_friendlies=false);

  ///Updates physics given unit space transformations and if this is the last physics frame in the current gfx frame
  virtual void UpdatePhysics (const Transformation &trans, const Matrix &transmat, const Vector & CumulativeVelocity, bool ResolveLast, UnitCollection *uc=NULL);
  class Cockpit * GetVelocityDifficultyMult(float &) const;
  ///executes a repair if the repair bot is up to it
  void Repair();
  ///Thrusts by ammt and clamps accordingly (afterburn or not)
  void Thrust(const Vector &amt,bool afterburn = false);
  ///Resolves forces of given unit on a physics frame
  Vector ResolveForces (const Transformation &, const Matrix&);
  ///Applies damage to the local area given by pnt
  float ApplyLocalDamage (const Vector &pnt, const Vector & normal, float amt, Unit * affectedSubUnit, const GFXColor &, float phasedamage=0);
  ///Applies damage to the pre-transformed area of the ship
  void ApplyDamage (const Vector & pnt, const Vector & normal, float amt, Unit * affectedSubUnit, const GFXColor &,  Unit *ownerDoNotDereference, float phasedamage=0 );
  ///Deals remaining damage to the hull at point and applies lighting effects
  // float DealDamageToHullReturnArmor (const Vector &pnt, float Damage, unsigned short * &t);
  //these functions play the damage sounds
  virtual void ArmorDamageSound( const Vector &pnt);
  virtual void HullDamageSound( const Vector &pnt);
  ///applies damage from the given pnt to the shield, and returns % damage applied and applies lighitn
  float DealDamageToShield (const Vector & pnt, float &Damage);

  void scanSystem();

/***************************************************************************************/
/**** CUSTOMIZE/UPGRADE STUFF                                                          */
/***************************************************************************************/

  bool UpgradeSubUnits (const Unit * up, int subunitoffset, bool touchme, bool downgrade, int &numave, double &percentage);
  double Upgrade (const std::string &file, int mountoffset, int subunitoffset, bool force, bool loop_through_mounts);


/********************************************/
/**** XML struct                            */
/********************************************/

  ///Holds temporary values for inter-function XML communication Saves deprecated restr info
  /*
	struct XML {
	  vector<Mount *> mountz;
	  vector<Mesh*> meshes;
	  vectorstring> meshes_str;
	  Mesh * shieldmesh;
	  Mesh * bspmesh;
	  Mesh * rapidmesh;
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
	};
	*/
};

/****************************************************************************/
/**** Since we now use templates, we have to include GameUnit related    ****/
/**** .cpp files in unit.h to avoid problems with some compilers         ****/
/****************************************************************************/

#include "unit.cpp"
#include "unit_bsp.h"
/***** FROM UNIT_BSP.CPP *****/
#ifndef PROPHECY
template<class UnitType>
void GameUnit<UnitType>::BuildBSPTree(const char *filename, bool vplane, Mesh * hull) {
  bsp_tree * bsp=NULL;
  bsp_tree temp_node;
  vector <bsp_polygon> tri;
  vector <bsp_tree> triplane;
  if (hull!=NULL) {
    hull->GetPolys (tri);
  } else {
    for (int j=0;j<nummesh();j++) {
      meshdata[j]->GetPolys(tri);
    }
  }	
  for (unsigned int i=0;i<tri.size();i++) {
    if (!Cross (tri[i],temp_node)) {
      vector <bsp_polygon>::iterator ee = tri.begin();
      ee+=i;
      tri.erase(ee);
      i--;
      continue;
    }	
    // Calculate 'd'
    temp_node.d = (double) ((temp_node.a*tri[i].v[0].i)+(temp_node.b*tri[i].v[0].j)+(temp_node.c*tri[i].v[0].k));
    temp_node.d*=-1.0;
    triplane.push_back(temp_node);
    //                bsp=put_plane_in_tree3(bsp,&temp_node,&temp_poly3); 
 }
 
 bsp = buildbsp (bsp,tri,triplane, vplane?VPLANE_ALL:0);
 if (bsp) {
   changehome();
   vschdir ("generatedbsp");
   o = fopen (filename, "wb");
   vscdup();
   returnfromhome();
   if (o) {
     write_bsp_tree(bsp,0);
     fclose (o);
     bsp_stats (bsp);
     FreeBSP (&bsp);
   }
 }	

}
#endif

#include "unit_cargo.cpp"
#include "unit_click.cpp"
#include "unit_collide.cpp"
#include "unit_customize.cpp"
#include "unit_damage.cpp"
//#include "base_interface.cpp"
#include "unit_physics.cpp"
#include "unit_repair.cpp"
#include "unit_weapon.cpp"
//#include "unit_xml.cpp"

// From star_system_jump.cpp
extern Hashtable<std::string, StarSystem ,char [127]> star_system_table;
extern std::vector <unorigdest *> pendingjump;

// From star_system_jump.cpp
inline bool CompareDest (Unit * un, StarSystem * origin) {
  for (unsigned int i=0;i<un->GetDestinations().size();i++) {
    if ((origin==star_system_table.Get (string(un->GetDestinations()[i])))||(origin==star_system_table.Get (string(un->GetDestinations()[i])+string (".system")))) 
      return true;
  }
  return false;
}
inline std::vector <Unit *> ComparePrimaries (Unit * primary, StarSystem *origin) {
  std::vector <Unit *> myvec;
  if (CompareDest (primary, origin))
    myvec.push_back (primary);
  /*
  if (primary->isUnit()==PLANETPTR) {
    Iterator *iter = ((Planet *)primary)->createIterator();
    Unit * unit;
    while((unit = iter->current())!=NULL) {
      if (unit->isUnit()==PLANETPTR)
	if (CompareDest ((Planet*)unit,origin)) {
	  myvec.push_back (unit);
	}
      iter->advance();
    }
    delete iter;
  }
  */
  return myvec;
}
extern void DealPossibleJumpDamage (Unit *un);

template <class UnitType>
void GameUnit<UnitType>::TransferUnitToSystem (unsigned int kk, StarSystem * &savedStarSystem, bool dosightandsound) {
  if (pendingjump[kk]->orig==activeStarSystem||activeStarSystem==NULL) {
	  if (Unit::TransferUnitToSystem (pendingjump[kk]->dest)) {
#ifdef JUMP_DEBUG
      fprintf (stderr,"Unit removed from star system\n");
#endif

      ///eradicating from system, leaving no trace
	  Unit::TransferUnitToSystem(pendingjump[kk]->dest);


      UnitCollection::UnitIterator iter = pendingjump[kk]->orig->getUnitList().createIterator();
      Unit * unit;
      while((unit = iter.current())!=NULL) {
	if (unit->Threat()==this) {
	  unit->Threaten (NULL,0);
	}
	if (unit->VelocityReference()==this) {
	  unit->VelocityReference(NULL);
	}
	if (unit->Target()==this) {
	  unit->Target (pendingjump[kk]->jumppoint.GetUnit());
	  unit->ActivateJumpDrive (0);
	}else {
	  Flightgroup * ff = unit->getFlightgroup();
	  if (ff) {
		  if (this==ff->leader.GetUnit()&&(ff->directive=="f"||ff->directive=="F")) {
			unit->Target (pendingjump[kk]->jumppoint.GetUnit());
			unit->getFlightgroup()->directive="F";
			unit->ActivateJumpDrive (0);
		  }
	  }
	}
	iter.advance();
      }
      Cockpit * an_active_cockpit = _Universe->isPlayerStarship(this);
      if (an_active_cockpit!=NULL) {
	an_active_cockpit->activeStarSystem=pendingjump[kk]->dest;
      }
      if (this==_Universe->AccessCockpit()->GetParent()) {
	fprintf (stderr,"Unit is the active player character...changing scene graph\n");
	savedStarSystem->SwapOut();
	savedStarSystem = pendingjump[kk]->dest;
	pendingjump[kk]->dest->SwapIn();
      }
      
      _Universe->setActiveStarSystem(pendingjump[kk]->dest);
      vector <Unit *> possibilities;
      iter = pendingjump[kk]->dest->getUnitList().createIterator();
      Unit * primary;
      while ((primary = iter.current())!=NULL) {
	vector <Unit *> tmp;
	tmp = ComparePrimaries (primary,pendingjump[kk]->orig);
	if (!tmp.empty()) {
	  possibilities.insert (possibilities.end(),tmp.begin(), tmp.end());
	}
	iter.advance();
      }
      if (!possibilities.empty()) {
	static int jumpdest=235034;
	this->SetCurPosition(possibilities[jumpdest%possibilities.size()]->Position());
	jumpdest+=23231;
      }
      DealPossibleJumpDamage (this);
      static int jumparrive=AUDCreateSound(vs_config->getVariable ("unitaudio","jumparrive", "sfx43.wav"),false);
      if (dosightandsound)
	AUDPlay (jumparrive,this->LocalPosition(),this->GetVelocity(),1);
    } else {
#ifdef JUMP_DEBUG
      fprintf (stderr,"Unit FAILED remove from star system\n");
#endif
    }
    if (docked&DOCKING_UNITS) {
      for (unsigned int i=0;i<image->dockedunits.size();i++) {
	Unit * unut;
	if (NULL!=(unut=image->dockedunits[i]->uc.GetUnit())) {
	  unut->TransferUnitToSystem (kk,savedStarSystem,dosightandsound);
	}
      }
    }
    if (docked&(DOCKED|DOCKED_INSIDE)) {
      Unit * un = image->DockedTo.GetUnit();
      if (!un) {
	docked &= (~(DOCKED|DOCKED_INSIDE));
      }else {
	Unit * targ=NULL;
	for (un_iter i=pendingjump[kk]->dest->getUnitList().createIterator();
	     (targ = (*i));
	     ++i) {
	  if (targ==un) {
	    break;
	  }
	}
	if (targ!=un)
	  UnDock (un);
      }
    }
  }else {
    fprintf (stderr,"Already jumped\n");
  }
}

// FROM unit_xml.cpp -> client only stuff
extern void cache_ani (string s);
extern void update_ani_cache ();
extern std::string getRandomCachedAniString ();
extern Animation* getRandomCachedAni ();

#endif
