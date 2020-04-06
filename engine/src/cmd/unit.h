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

#ifdef VS_DEBUG
#define CONTAINER_DEBUG
#endif

#include <string>
#include <vector>
#include <map>
#include <memory>

class HaloSystem;
struct GFXColor; // labeled as class, declared as struct in gfxlib_struct.h
class QVector;
struct Transformation;
class Matrix;
class Vector;
class StarSystem;

class Mesh;
class Flightgroup;
template < typename BOGUS >
struct UnitImages;
class Unit;
class VSSprite;
class Camera;
class UnitCollection;

/**
 * GameUnit contains any physical object that may collide with something
 * And may be physically affected by forces.
 * Units are assumed to have various damage and explode when they are dead.
 * Units may have any number of weapons which, themselves may be units
 * the aistate indicates how the unit will behave in the upcoming phys frame
 */
template < class UnitType >
class GameUnit : public UnitType
{
    friend class UnitFactory;
protected:

public:
  //Default constructor. This is just to figure out where default
  //constructors are used. The useless argument will be removed
  //again later.
  GameUnit( int dummy );
  //Constructor that creates aa mesh with meshes as submeshes (number
  //of them) as either as subunit with faction faction
    GameUnit( std::vector< Mesh* > &meshes, bool Subunit, int faction );
  //Constructor that creates a mesh from an XML file If it is a
  //customizedUnit, it will check in that directory in the home dir for
  //the unit.
    GameUnit( const char *filename, bool SubUnit, int faction, std::string customizedUnit = std::string(
                  "" ), Flightgroup *flightgroup = NULL, int fg_subnumber = 0, std::string *netxml = NULL );
    virtual ~GameUnit();
    unsigned int nummesh() const {
        return UnitType::nummesh();
    }
  ///fils in corner_min,corner_max and radial_size
  ///returns -1 if unit cannot dock, otherwise returns which dock it can dock at
    UnitImages< void >& GetImageInformation();
    bool RequestClearance( Unit *dockingunit );
  ///Loads a user interface for the user to upgrade his ship
    void UpgradeInterface( Unit *base );
  ///The name (type) of this unit shouldn't be public
    virtual void Cloak( bool cloak );

/*
 **************************************************************************************
 **** GFX/MESHES STUFF                                                              ***
 **************************************************************************************
 */
 	double sparkle_accum;
    std::unique_ptr< HaloSystem >phalos;
///Process all meshes to be deleted
///Split this mesh with into 2^level submeshes at arbitrary planes
    void Split( int level );
    void FixGauges();
///Sets the camera to be within this unit.
    void UpdateHudMatrix( int whichcam );
///What's the HudImage of this unit
    VSSprite * getHudImage() const;
///Draws this unit with the transformation and matrix (should be equiv) separately
    virtual void Draw( const Transformation &quat, const Matrix &m );
    virtual void Draw( const Transformation &quat );
    virtual void Draw();
    virtual void DrawNow( const Matrix &m, float lod = 1000000000 );
    virtual void DrawNow();
///Deprecated
    void addHalo( const char *filename,
                  const Matrix &trans,
                  const Vector &size,
                  const GFXColor &col,
                  std::string halo_type,
                  float halo_speed );
    
    virtual void applyTechniqueOverrides(const std::map<std::string, std::string> &overrides);
    
/*
 **************************************************************************************
 **** STAR SYSTEM STUFF                                                             ***
 **************************************************************************************
 */
    bool TransferUnitToSystem( unsigned int whichJumpQueue, StarSystem*&previouslyActiveStarSystem, bool DoSightAndSound );
///Begin and continue explosion
    bool Explode( bool draw, float timeit );
/*
 **************************************************************************************
 **** COLLISION STUFF                                                               ***
 **************************************************************************************
 */
///Updates the collide Queue with any possible change in sectors
///Queries if this unit is within a given frustum
    bool queryFrustum( double frustum[6][4] ) const;

///Queries the bounding sphere with a duo of mouse coordinates that project
///to the center of a ship and compare with a sphere...pretty fast*/
    bool querySphereClickList( int, int, float err, Camera *activeCam ) const;
///returns true if jump possible even if not taken
/*
 **************************************************************************************
 **** PHYSICS STUFF
 **************************************************************************************
 */
///Updates physics given unit space transformations and if this is the last physics frame in the current gfx frame
    virtual void UpdatePhysics2( const Transformation &trans,
                                 const Transformation &old_physical_state,
                                 const Vector &accel,
                                 float difficulty,
                                 const Matrix &transmat,
                                 const Vector &CumulativeVelocity,
                                 bool ResolveLast,
                                 UnitCollection *uc = NULL );
///Thrusts by ammt and clamps accordingly (afterburn or not)
    void Thrust( const Vector &amt, bool afterburn = false );
///Resolves forces of given unit on a physics frame
    Vector ResolveForces( const Transformation&, const Matrix& );
//these functions play the damage sounds
    virtual void ArmorDamageSound( const Vector &pnt );
    virtual void HullDamageSound( const Vector &pnt );
///applies damage from the given pnt to the shield, and returns % damage applied and applies lighitn
    float DealDamageToShield( const Vector &pnt, float &Damage );
/*
 **************************************************************************************
 **** CUSTOMIZE/UPGRADE STUFF
 **************************************************************************************
 */
    bool UpgradeSubUnits( const Unit *up, int subunitoffset, bool touchme, bool downgrade, int &numave, double &percentage );
    double Upgrade( const std::string &file, int mountoffset, int subunitoffset, bool force, bool loop_through_mounts );
/*
 *******************************************
 **** XML struct
 *******************************************
 */
///Holds temporary values for inter-function XML communication Saves deprecated restr info
    Matrix WarpMatrix( const Matrix &ctm ) const;
};



/*
 ***************************************************************************
 **** Since we now use templates, we have to include GameUnit related    ***
 **** .cpp files in unit.h to avoid problems with some compilers         ***
 ***************************************************************************
 */

/////////////////////////////////////////////////////
//forward declarations of explicit instantiations, added by chuck_starchaser:

class Asteroid;
template < class Asteroid >
class GameUnit;

class Building;
template < class Building >
class GameUnit;

class Planet;
template < class Planet >
class GameUnit;

class Unit;
template < class Unit >
class GameUnit;

class Missile;
template < class Missile >
class GameUnit;

class Nebula;
template < class Nebula >
class GameUnit;

class Enhancement;
template < class Enhancement >
class GameUnit;

#endif
