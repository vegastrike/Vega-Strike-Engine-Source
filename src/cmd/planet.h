/* planet.h
 *  ok *
 * Unit type that is a gravity source, and follows a set orbit pattern
 */
#ifndef _PLANET_H_
#define _PLANET_H_

#include <stdio.h>

#include "unit.h"
#include "planet_generic.h"
#include "ai/order.h"
#include "collection.h"
#include <vector>
#include <string>

struct GFXMaterial;
/* Orbits in the xy plane with the given radius. Depends on a reorientation of coordinate bases */

class Texture;
class Atmosphere;

class ContinuousTerrain;

class GamePlanet : public GameUnit< class Planet >
{
private:
    Animation *shine;
protected:
/// default constructor - only to be called by UnitFactory
    GamePlanet();

/// constructor - only to be called by UnitFactory
    GamePlanet( QVector x,
                QVector y,
                float vely,
                const Vector &rotvel,
                float pos,
                float gravity,
                float radius,
                const std::string &filename,
                const std::string &technique,
                const std::string &unitname,
                BLENDFUNC blendsrc,
                BLENDFUNC blenddst,
                const std::vector< std::string > &dest,
                const QVector &orbitcent,
                Unit *parent,
                const GFXMaterial &ourmat,
                const std::vector< GFXLightLocal >&,
                int faction,
                string fullname,
                bool inside_out = false );

    friend class UnitFactory;

public:
    UnitCollection satellites;
    virtual ~GamePlanet();
    virtual void AddFog( const std::vector< AtmosphericFogMesh >&, bool optical_illusion );
    Vector AddSpaceElevator( const std::string &name, const std::string &faction, char direction );
    void AddAtmosphere( const std::string &texture, float radius, BLENDFUNC blendSrc, BLENDFUNC blendDst, bool inside_out );
    void AddRing( const std::string &texture,
                  float iradius,
                  float oradius,
                  const QVector &r,
                  const QVector &s,
                  int slices,
                  int numwrapx,
                  int numwrapy,
                  BLENDFUNC blendSrc,
                  BLENDFUNC blendDst );
    void AddCity( const std::string &texture,
                  float radius,
                  int numwrapx,
                  int numwrapy,
                  BLENDFUNC blendSrc,
                  BLENDFUNC blendDst,
                  bool inside_out = false,
                  bool reverse_normals = true );
    void DisableLights();
    void EnableLights();
    virtual void Draw( const Transformation &quat = identity_transformation, const Matrix &m = identity_matrix );
    void DrawTerrain();
    static void ProcessTerrains();
    virtual void Kill( bool erasefromsave = false );

    PlanetaryTransform * setTerrain( ContinuousTerrain*, float ratiox, int numwraps, float scaleatmos );
    ContinuousTerrain * getTerrain( PlanetaryTransform* &t )
    {
        t = terraintrans;
        return terrain;
    }
    void setAtmosphere( Atmosphere* );
    Atmosphere * getAtmosphere()
    {
        return atmosphere;
    }
    void reactToCollision( Unit *smaller,
                           const QVector &biglocation,
                           const Vector &bignormal,
                           const QVector &smalllocation,
                           const Vector &smallnormal,
                           float dist );

    friend class Planet::PlanetIterator;
    friend class PlanetaryOrbit;

private:
/// copy constructor forbidden
    GamePlanet( const Planet& );
/// assignment operator forbidden
    GamePlanet& operator=( const Planet& );
};

#endif

