/*
 * images.h
 *
 * Copyright (C) 2001-2023 Daniel Horn, pyramid3d, Stephen G. Tuggy,
 * Benjamen R. Meyer, and other Vega Strike Contributors
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * Vega Strike is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef VEGA_STRIKE_ENGINE_CMD_IMAGES_H
#define VEGA_STRIKE_ENGINE_CMD_IMAGES_H

#include <string>
#include <vector>
#include "gfx/vec.h"
#include "container.h"
#include "../SharedPool.h"
#include "gfx/sprite.h"
#include "gfx/animation.h"

struct DockingPorts
{
    struct Type
    {
        enum Value
        {
            CATEGORY_CONNECTED = 10,
            CATEGORY_WAYPOINT = 20,

            // Unconnected types corresponds to the old true/false values
            OUTSIDE = 0,
            INSIDE = 1,

            CONNECTED_OUTSIDE = CATEGORY_CONNECTED + OUTSIDE,
            CONNECTED_INSIDE = CATEGORY_CONNECTED + INSIDE,

            WAYPOINT_OUTSIDE = CATEGORY_WAYPOINT + OUTSIDE,
            WAYPOINT_INSIDE = CATEGORY_WAYPOINT + INSIDE,

            DEFAULT = OUTSIDE
        };

        static bool IsConnected(const Value& type)
        {
            switch (type)
            {
            case OUTSIDE:
            case INSIDE:
                return false;
            default:
                return true;
            }
        }

        static bool IsInside(const Value& type)
        {
            switch (type)
            {
            case INSIDE:
            case CONNECTED_INSIDE:
            case WAYPOINT_INSIDE:
                return true;
            default:
                return false;
            }
        }

        static bool IsWaypoint(const Value& type)
        {
            switch (type)
            {
            case WAYPOINT_OUTSIDE:
            case WAYPOINT_INSIDE:
                return true;
            default:
                return false;
            }
        }
    };

    DockingPorts()
        : radius(0),
          isInside(false),
          isConnected(false),
          isWaypoint(false),
          isOccupied(false)
    {}

    DockingPorts(const Vector &center, float radius, float minradius, const Type::Value& type)
        : center(center),
          radius(radius),
          isInside(Type::IsInside(type)),
          isConnected(Type::IsConnected(type)),
          isWaypoint(Type::IsWaypoint(type)),
          isOccupied(isWaypoint) // Waypoints are always occupied
    {}

    DockingPorts(const Vector &min, const Vector &max, float minradius, const Type::Value& type)
        : center((min + max) / 2.0f),
          radius((max - min).Magnitude() / 2.0f),
          isInside(Type::IsInside(type)),
          isConnected(Type::IsConnected(type)),
          isWaypoint(Type::IsWaypoint(type)),
          isOccupied(isWaypoint) // Waypoints are always occupied
    {}

    float GetRadius() const { return radius; }

    const Vector& GetPosition() const { return center; }

    // Waypoints are always marked as occupied.
    bool IsOccupied() const { return isOccupied; }
    void Occupy(bool yes) { isOccupied = yes; }

    // Does port have connecting waypoints?
    bool IsConnected() const { return isConnected; }

    // Port is located inside or outside the station
    bool IsInside() const { return isInside; }

    bool IsDockable() const { return !isWaypoint; }

private:
    Vector center;
    float radius;
    bool isInside;
    bool isConnected;
    bool isWaypoint;
    bool isOccupied;
};

struct DockedUnits
{
    UnitContainer uc;
    unsigned int  whichdock;
    DockedUnits( Unit *un, unsigned int w ) : uc( un )
        , whichdock( w ) {}
};

class Cargo
{
public:
    StringPool::Reference content;
    StringPool::Reference category;
    StringPool::Reference description;
    int   quantity;
    float price;
    float mass;
    float volume;
    bool  mission;
    bool  installed;
    float functionality;
    float maxfunctionality;
    Cargo()
    {
        mass     = 0;
        volume   = 0;
        price    = 0;
        quantity = 1;
        mission  = false;
        installed = false;
        functionality = maxfunctionality = 1.0f;
    }
    Cargo( std::string name, std::string cc, float pp, int qq, float mm, float vv, float func, float maxfunc ) :
        content( name )
        , category( cc )
    {
        quantity = qq;
        price    = pp;
        mass     = mm;
        volume   = vv;
        mission  = false;
        installed = false;
        functionality = func;
        maxfunctionality = maxfunc;
    }
    Cargo( std::string name, std::string cc, float pp, int qq, float mm, float vv ) :
        content( name )
        , category( cc )
    {
        quantity = qq;
        price    = pp;
        mass     = mm;
        volume   = vv;
        mission  = false;
        installed = false;
    }
    float GetFunctionality()
    {
        return functionality;
    }
    float GetMaxFunctionality()
    {
        return maxfunctionality;
    }
    void SetFunctionality( float func )
    {
        functionality = func;
    }
    void SetMaxFunctionality( float func )
    {
        maxfunctionality = func;
    }
    void SetMissionFlag( bool flag )
    {
        this->mission = flag;
    }
    void SetPrice( float price )
    {
        this->price = price;
    }
    void SetMass( float mass )
    {
        this->mass = mass;
    }
    void SetVolume( float vol )
    {
        this->volume = vol;
    }
    void SetQuantity( int quantity )
    {
        this->quantity = quantity;
    }
    void SetContent( const std::string &content )
    {
        this->content = content;
    }
    void SetCategory( const std::string &category )
    {
        this->category = category;
    }

    bool GetMissionFlag() const
    {
        return this->mission;
    }
    const std::string& GetCategory() const
    {
        return category.get();
    }
    const std::string& GetContent() const
    {
        return content.get();
    }
    const std::string& GetDescription() const
    {
        return description.get();
    }
    std::string GetCategoryPython()
    {
        return GetCategory();
    }
    std::string GetContentPython()
    {
        return GetContent();
    }
    std::string GetDescriptionPython()
    {
        return GetDescription();
    }
    int GetQuantity() const
    {
        return quantity;
    }
    float GetVolume() const
    {
        return volume;
    }
    float GetMass() const
    {
        return mass;
    }
    float GetPrice() const
    {
        return price;
    }
    bool operator==( const Cargo &other ) const
    {
        return content == other.content;
    }
    bool operator<( const Cargo &other ) const
    {
        return (category == other.category) ? (content < other.content) : (category < other.category);
    }
};

class Box;
class VSSprite;
class Animation;

template < typename BOGUS >
//added by chuck starchaser, to try to break dependency to VSSprite in vegaserver
struct UnitImages
{
    UnitImages(){ VSCONSTRUCT1('i'); pHudImage=NULL; pExplosion=NULL;}
/*    {
*       VSCONSTRUCT1( 'i' )
*  //        pHudImage = NULL;
*       pExplosion = NULL;
*   }*/
    virtual ~UnitImages();
/*    {
*       delete pExplosion;
*  //        delete pHudImage;
*       VSDESTRUCT1
*   }*/
    StringPool::Reference cockpitImage;
    StringPool::Reference explosion_type;
    Vector CockpitCenter;
    VSSprite     *pHudImage;
    ///The explosion starts at null, when activated time explode is incremented and ends at null
    Animation    *pExplosion;
    float timeexplode;
    float        *cockpit_damage;     //0 is radar, 1 to MAXVDU is vdus and >MAXVDU is gauges
    ///how likely to fool missiles
    /// -2 = inactive L2, -1 = inactive L1, 0 = not available, 1 = active L1, 2 = active L2, etc...
    int  ecm;    
    ///holds the info for the repair bot type. 0 is no bot;
    unsigned char repair_droid;
    float next_repair_time;
    unsigned int  next_repair_cargo;    //(~0 : select randomly)
    ///How much energy cloaking takes per frame
    float cloakenergy;
    ///how fast this starship decloaks/close...if negative, decloaking
    int   cloakrate;   //short fix
    ///If this unit cloaks like glass or like fading
    bool  cloakglass;
    ///if the unit is a wormhole
    bool  forcejump;
    float UpgradeVolume;
    float CargoVolume;     ///mass just makes you turn worse
    float equipment_volume;     //this one should be more general--might want to apply it to radioactive goods, passengers, ships (hangar), etc
    float HiddenCargoVolume;
    std::vector< Cargo > cargo;
    std::vector< string >destination;
    std::vector< DockingPorts >dockingports;
    ///warning unreliable pointer, never dereference!
    std::vector< Unit* > clearedunits;
    std::vector< DockedUnits* >dockedunits;
    UnitContainer DockedTo;
    float unitscale;     //for output
    class XMLSerializer *unitwriter;
    float fireControlFunctionality;
    float fireControlFunctionalityMax;
    float SPECDriveFunctionality;
    float SPECDriveFunctionalityMax;
    float CommFunctionality;
    float CommFunctionalityMax;
    float LifeSupportFunctionality;
    float LifeSupportFunctionalityMax;
    enum GAUGES
    {
        //Image-based gauges
        ARMORF, ARMORB, ARMORR, ARMORL, ARMOR4, ARMOR5, ARMOR6, ARMOR7, FUEL, SHIELDF, SHIELDR, SHIELDL, SHIELDB, SHIELD4,
        SHIELD5, SHIELD6, SHIELD7,
        ENERGY, AUTOPILOT, COLLISION, EJECT, LOCK, MISSILELOCK, JUMP, ECM, HULL, WARPENERGY,
        //target gauges
        TARGETSHIELDF, TARGETSHIELDB, TARGETSHIELDR, TARGETSHIELDL,
        KPS,         //KEEP KPS HERE - it marks the start of text-based gauges
        SETKPS, COCKPIT_FPS, WARPFIELDSTRENGTH, MAXWARPFIELDSTRENGTH, MAXKPS, MAXCOMBATKPS, MAXCOMBATABKPS, MASSEFFECT,
        AUTOPILOT_MODAL,         //KEEP first multimodal gauge HERE -- it marks the start of multi-modal gauges
        SPEC_MODAL, FLIGHTCOMPUTER_MODAL, TURRETCONTROL_MODAL, ECM_MODAL, CLOAK_MODAL, TRAVELMODE_MODAL,
        RECIEVINGFIRE_MODAL, RECEIVINGMISSILES_MODAL, RECEIVINGMISSILELOCK_MODAL, RECEIVINGTARGETLOCK_MODAL,
        COLLISIONWARNING_MODAL, CANJUMP_MODAL, CANDOCK_MODAL,
        NUMGAUGES         //KEEP THIS LAST - obvious reasons, marks the end of all gauges
    };
    enum MODALGAUGEVALUES
    {
        OFF, ON, SWITCHING, ACTIVE, FAW, MANEUVER, TRAVEL, NOTAPPLICABLE, READY, NODRIVE, TOOFAR, NOTENOUGHENERGY, WARNING,
        NOMINAL, AUTOREADY
    };
};



//From star_system_jump.cpp
class StarSystem;
struct unorigdest
{
    UnitContainer un;
    UnitContainer jumppoint;
    StarSystem   *orig;
    StarSystem   *dest;
    float   delay;
    int     animation;
    bool    justloaded;
    bool    ready;
    QVector final_location;
    unorigdest( Unit *un,
                Unit *jumppoint,
                StarSystem *orig,
                StarSystem *dest,
                float delay,
                int ani,
                bool justloaded,
                QVector use_coordinates /*set to 0,0,0 for crap*/ ) : un( un )
        , jumppoint( jumppoint )
        , orig( orig )
        , dest( dest )
        , delay( delay )
        , animation( ani )
        , justloaded( justloaded )
        , ready( true )
        , final_location( use_coordinates ) {}
};

#endif

