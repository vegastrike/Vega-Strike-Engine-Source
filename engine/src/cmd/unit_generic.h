// -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil -*-

#ifndef __UNIT_GENERIC_H__
#define __UNIT_GENERIC_H__

/**
 * unit_generic.cpp
 *
 * Copyright (C) 2001-2002 Daniel Horn
 * Copyright (C) 2002-2019 pyramid3d and other Vega Strike Contributors
 * Copyright (C) 2020-2021 pyramid3d, Stephen G. Tuggy, Roy Falk,
 * and other Vega Strike contributors
 * Copyright (C) 2022 Stephen G. Tuggy
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
 * along with Vega Strike.  If not, see <https://www.gnu.org/licenses/>.
 */

/***** Unit is the Unit class without GFX/Sound with AI *****/

#ifndef _UNIT_H_
#define _UNIT_H_

#include "armed.h"
#include "audible.h"
#include "damageable.h"
#include "drawable.h"
#include "movable.h"
#include "computer.h"
#include "intelligent.h"
#include "energetic.h"
#include "carrier.h"
#include "jump_capable.h"

#include "mount.h"
#include "damage/damage.h"

#ifdef VS_DEBUG
#define CONTAINER_DEBUG
#endif
#ifdef CONTAINER_DEBUG
#include "hashtable.h"
class Unit;
void CheckUnit( class Unit* );
void UncheckUnit( class Unit*un );
#endif
#include "vegastrike.h"
#include "vs_globals.h"

#include <string>
#include <set>
#include <map>
#include "gfx/matrix.h"
#include "gfx/quaternion.h"
#include "gfxlib_struct.h"
#include "xml_support.h"
#include "container.h"
#include "collection.h"
#include "script/flightgroup.h"
#include "faction_generic.h"
#include "star_system_generic.h"
#include "gfx/cockpit_generic.h"
#include "vsfilesystem.h"
#include "collide_map.h"
#include "SharedPool.h"
#include "role_bitmask.h"

#include "configuration/game_config.h"

extern char *GetUnitDir(const char *filename);

Unit *getMasterPartList();
bool CloseEnoughToAutotrack(Unit *me, Unit *targ, float &cone);

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
class Pilot;
class Limits;
class MissileGeneric;
class AsteroidGeneric;

/**
 * Currently the only inheriting function is planet
 * Needed by star system to determine whether current unit
 * is orbitable
 */
enum _UnitType {
    unit,
    planet,
    building,
    nebula,
    asteroid,
    enhancement,
    missile
};

class VDU;
//template
template<typename BOGUS>
struct UnitImages;
class Cargo;
class Mesh;
struct PlanetaryOrbitData;

/**
 * Unit contains any physical object that may collide with something
 * And may be physically affected by forces.
 * Units are assumed to have various damage and explode when they are dead.
 * Units may have any number of weapons which, themselves may be units
 * the aistate indicates how the unit will behave in the upcoming phys frame
 */

// TODO: move Armed to subclasses
class Unit : public Armed, public Audible, public Drawable, public Damageable, public Energetic,
        public Intelligent, public Movable, public JumpCapable, public Carrier {
protected:
//How many lists are referencing us
    int ucref = 0;
    StringPool::Reference csvRow;
public:

    /// Radar and related systems
    // TODO: take a deeper look at this much later...
    //how likely to fool missiles
    // -2 = inactive L2, -1 = inactive L1, 0 = not available, 1 = active L1, 2 = active L2, etc...
    int ecm;

    /// Repair
    // TODO: Maybe move to damageable
    // holds the info for the repair bot type. 0 is no bot;
    unsigned char repair_droid;
    float next_repair_time;
    unsigned int next_repair_cargo;    //(~0 : select randomly)

    float fireControlFunctionality;
    float fireControlFunctionalityMax;
    float SPECDriveFunctionality;
    float SPECDriveFunctionalityMax;
    float CommFunctionality;
    float CommFunctionalityMax;
    float LifeSupportFunctionality;
    float LifeSupportFunctionalityMax;

    /// Volume
    // This isn't mass. Denser materials translate to more mass
    // TODO: move this to ship class
    float UpgradeVolume = 0;
    float CargoVolume = 0;     ///mass just makes you turn worse
    float equipment_volume =
            0;     //this one should be more general--might want to apply it to radioactive goods, passengers, ships (hangar), etc
    float HiddenCargoVolume = 0;

//The name (type) of this unit shouldn't be public
    StringPool::Reference name;
    StringPool::Reference filename;

/*
 **************************************************************************************
 **** CONSTRUCTORS / DESCTRUCTOR                                                    ***
 **************************************************************************************
 */

protected:
//forbidden
    Unit(const Unit &) = delete;

//forbidden
    Unit &operator=(const Unit &) = delete;

public:
    Unit();
    virtual ~Unit();

/** Default constructor. This is just to figure out where default
 *  constructors are used. The useless argument will be removed
 *  again later.
 */
    Unit(int dummy);

/** Constructor that creates aa mesh with meshes as submeshes (number
 *  of them) as either as subunit with faction faction
 */
    Unit(std::vector<Mesh *> &meshes, bool Subunit, int faction);

/** Constructor that creates a mesh from an XML file If it is a
 *  customizedUnit, it will check in that directory in the home dir for
 *  the unit.
 */
//Uses a lot of stuff that does not belong to here
    Unit(const char *filename, bool SubUnit, int faction, std::string customizedUnit = std::string(
            ""), Flightgroup *flightgroup = NULL, int fg_subnumber = 0);

public:
//Initialize many of the defaults inherant to the constructor

    // This is called from python and so left in place for now
    // TODO: get rid of this
    void Init();
    void Init(const char *filename, bool SubUnit, int faction, std::string customizedUnit = std::string(
            ""), Flightgroup *flightgroup = NULL, int fg_subnumber = 0);
//table can be NULL, but setting it appropriately may increase performance
    void LoadRow(class CSVRow &row, std::string unitMod, bool saved_game);

    // TODO: implement enum class as type safe bitmask...
    // http://blog.bitwigglers.org/using-enum-classes-as-type-safe-bitmasks/
    enum Damages {
        NO_DAMAGE = 0x0,
        SHIELD_DAMAGED = 0x1,
        COMPUTER_DAMAGED = 0x2,
        MOUNT_DAMAGED = 0x4,
        CARGOFUEL_DAMAGED = 0x8,
        JUMP_DAMAGED = 0x10,
        CLOAK_DAMAGED = 0x20,
        LIMITS_DAMAGED = 0x40,
        ARMOR_DAMAGED = 0x80
    };
    unsigned int damages = Damages::NO_DAMAGE;

/*
 **************************************************************************************
 **** UPGRADE/CUSTOMIZE STUFF                                                       ***
 **************************************************************************************
 */

//Uses mmm... stuff not desired here ?
    bool UpgradeSubUnitsWithFactory(const Unit *up, int subunitoffset, bool touchme, bool downgrade, int &numave,
            double &percentage, Unit *(*createupgradesubunit)(std::string s,
            int faction));
    bool UpgradeSubUnits(const Unit *up,
            int subunitoffset,
            bool touchme,
            bool downgrade,
            int &numave,
            double &percentage);
    bool UpgradeMounts(const Unit *up,
            int subunitoffset,
            bool touchme,
            bool downgrade,
            int &numave,
            const Unit *templ,
            double &percentage);
//the turrets and spinning parts fun fun stuff
    UnitCollection SubUnits;

/**
 * Contains information about a particular Mount on a unit.
 * And the weapons it has, where it is, where it's aimed,
 * The ammo and the weapon type. As well as the possible weapons it may fit
 * Warning: type has a string inside... cannot be memcpy'd
 */
    bool hasSubUnits() const {
        return !SubUnits.empty();
    };
    un_iter getSubUnits();
    un_kiter viewSubUnits() const;
#define NO_MOUNT_STAR
    bool inertialmode = false;
    bool autopilotactive = false;

    bool isSubUnit() const {
        return graphicOptions.SubUnit ? true : false;
    }

    void setFaceCamera();
    bool UpAndDownGrade(const Unit *up,
            const Unit *templ,
            int mountoffset,
            int subunitoffset,
            bool touchme,
            bool downgrade,
            int additive,
            bool forcetransaction,
            double &percentage,
            const Unit *downgrade_min,
            bool force_change_on_nothing,
            bool gen_downgrade_list);
    void ImportPartList(const std::string &category, float price, float pricedev, float quantity, float quantdev);

    void ClearMounts();
//Loads a user interface for the user to upgrade his ship
//Uses base stuff -> only in Unit
    virtual void UpgradeInterface(Unit *base);

    bool canUpgrade(const Unit *upgrador,
            int mountoffset,
            int subunitoffset,
            int additive,
            bool force,
            double &percentage,
            const Unit *templ = NULL,
            bool force_change_on_nothing = false,
            bool gen_downgrade_list = true);
    bool Upgrade(const Unit *upgrador,
            int mountoffset,
            int subunitoffset,
            int additive,
            bool force,
            double &percentage,
            const Unit *templ = NULL,
            bool force_change_on_nothing = false,
            bool gen_downgrade_list = true);
    int RepairCost();                            //returns how many things need to be repaired--if nothing is damaged it will return 1 for labor.  doesn't assume any given cost on such thigns.
    int RepairUpgrade();                 //returns how many things were repaired
//returns percentOperational,maxPercentOperational,and whether mount is damaged (1 is damaged, 0 is fine, -1 is invalid mount)
    bool RepairUpgradeCargo(Cargo *item,
            Unit *baseUnit,
            float *credits);           //item must not be NULL but baseUnit/credits are only used for pricing.
    Vector MountPercentOperational(int whichmount);
    bool ReduceToTemplate();
    double Upgrade(const std::string &file, int mountoffset, int subunitoffset, bool force, bool loop_through_mounts);
    bool canDowngrade(const Unit *downgradeor,
            int mountoffset,
            int subunitoffset,
            double &percentage,
            const Unit *downgradelimit,
            bool gen_downgrade_list = true);
    bool Downgrade(const Unit *downgradeor,
            int mountoffset,
            int subunitoffset,
            double &percentage,
            const Unit *downgradelimit,
            bool gen_downgrade_list = true);

protected:
//Mount may access unit
    friend class Mount;
//no collision table presence.

/*
 **************************************************************************************
 **** GFX/PLANET STUFF                                                              ***
 **************************************************************************************
 */

private:
    // TODO: move from GFX/Planet stuff
    // These two are probably zero.
    // TODO: check and if so, replace with 0.
    // Making private and re-adding trivial getter
    unsigned char attack_preference = ROLES::getRole("INERT");
    unsigned char unit_role = ROLES::getRole("INERT");

public:
    unsigned char getAttackPreferenceChar() {
        return attack_preference;
    }

    unsigned char getUnitRoleChar() {
        return unit_role;
    }

    unsigned char getAttackPreferenceChar() const {
        return attack_preference;
    }

    unsigned char getUnitRoleChar() const {
        return unit_role;
    }

//following 2 are legacy functions for python export only
    void setCombatRole(const std::string &s);
    const std::string &getCombatRole() const;
//end legacy functions
    const std::string &getUnitRole() const;
    void setUnitRole(const std::string &s);
    const std::string &getAttackPreference() const;
    void setAttackPreference(const std::string &s);

    Nebula *nebula = nullptr;

protected:

//The orbit needs to have access to the velocity directly to disobey physics laws to precalculate orbits
    friend class PlanetaryOrbit;
    friend class ContinuousTerrain;
//VDU needs mount data to draw weapon displays
    friend class VDU;
//needed to actually upgrade unit through interface
    friend class UpgradingInfo;
public:
//Have to pass the randnum and degrees in networking and client side since they must not be random in that case
    void DamageRandSys(float dam, const Vector &vec, float randum = 1, float degrees = 1);
    void SetNebula(Nebula *);

    inline Nebula *GetNebula() const {
        return nebula;
    }

//Should draw selection box?
//Process all meshes to be deleted
    static void ProcessDeleteQueue();
//Returns the cockpit name so that the controller may load a new cockpit
    const std::string &getCockpit() const;

//Shouldn't do anything here - but needed by Python
    class Cockpit *GetVelocityDifficultyMult(float &) const;

//Takes out of the collide table for this system.
    void RemoveFromSystem();
    void RequestPhysics();               //Requeues the unit so that it is simulated ASAP

//Uses planet stuff
/* Updates the collide Queue with any possible change in sectors
 *  Split this mesh with into 2^level submeshes at arbitrary planes
 *  Uses Mesh so only in Unit and maybe in NetUnit */
    virtual void Split(int level) {
    }

    virtual void addHalo(const char *filename,
            const Matrix &trans,
            const Vector &size,
            const GFXColor &col,
            std::string halo_type,
            float activation);

//Uses Mesh -> in NetUnit and Unit only
    std::vector<Mesh *> StealMeshes();
/* Begin and continue explosion
 *  Uses GFX so only in Unit class
 *  But should always return true on server side = assuming explosion time=0 here */
    bool Explode(bool draw, float timeit);

//Uses GFX so only in Unit class
    //virtual void DrawNow( const Matrix &m = identity_matrix, float lod = 1000000000 ) override {}
    virtual std::string drawableGetName() override {
        return name;
    }

//Sets the camera to be within this unit.
//Uses Universe & GFX so not needed here -> only in Unit class
//What's the HudImage of this unit

//Not needed just in Unit class

    // Uses GFX, so generic version is a no-op.
    // GameUnit variants (clientside) would apply the overrides to their GFX techniques
    virtual void applyTechniqueOverrides(const std::map<std::string, std::string> &overrides);

/*
 **************************************************************************************
 **** NAVIGATION STUFF                                                              ***
 **************************************************************************************
 */

public:
    bool TransferUnitToSystem(unsigned int whichJumpQueue,
            StarSystem *&previouslyActiveStarSystem,
            bool DoSightAndSound);

    Computer computer;
    void SwitchCombatFlightMode();
    bool CombatMode();

    Pilot *pilot;
    bool selected = false;

    Computer &GetComputerData() {
        return computer;
    }

    const Computer &ViewComputerData() const {
        return computer;
    }


/*
 **************************************************************************************
 **** XML STUFF                                                                     ***
 **************************************************************************************
 */
// TODO: remove all of this. It's unused.
public:
//Unit XML Load information
    struct XML;
//Loading information
    XML *xml = nullptr;

    static void beginElement(void *userData, const XML_Char *name, const XML_Char **atts);
    static void endElement(void *userData, const XML_Char *name);

    void beginElement(const std::string &name, const XMLSupport::AttributeList &attributes);
    void endElement(const std::string &name);

protected:
    static std::string massSerializer(const struct XMLType &input, void *mythis);
    static std::string mountSerializer(const struct XMLType &input, void *mythis);
    static std::string subunitSerializer(const struct XMLType &input, void *mythis);

public:
//tries to warp as close to un as possible abiding by the distances of various enemy ships...it might not make it all the way
    void WriteUnit(const char *modificationname = "");
    std::string WriteUnitString();
//Loads a unit from an xml file into a complete datastructure
    void LoadXML(const char *filename, const char *unitModifications = "", std::string *xmlbuffer = NULL);
    void LoadXML(VSFileSystem::VSFile &f, const char *unitModifications = "", std::string *xmlbuffer = NULL);

/*
 **************************************************************************************
 **** PHYSICS STUFF                                                                 ***
 **************************************************************************************
 */

private:
    void RechargeEnergy();
protected:
    virtual float ExplosionRadius();
public:
    QVector realPosition() override;

    ///Updates physics given unit space transformations and if this is the last physics frame in the current gfx frame
    virtual void UpdatePhysics2(const Transformation &trans,
            const Transformation &old_physical_state,
            const Vector &accel,
            float difficulty,
            const Matrix &transmat,
            const Vector &CumulativeVelocity,
            bool ResolveLast,
            UnitCollection *uc = NULL);

    // Act out a unit's turn
    void ActTurn();

    // TODO: move to cloakable
    void UpdateCloak();

    // TODO: move up to ship
    void UpdatePhysics3(const Transformation &trans,
            const Matrix &transmat,
            bool lastframe,
            UnitCollection *uc,
            Unit *superunit) override;
    bool isPlayerShip() override;

//The owner of this unit. This may not collide with owner or units owned by owner. Do not dereference (may be dead pointer)
    void *owner = nullptr;   //void ensures that it won't be referenced by accident



//Whether or not to schedule subunits for deferred physics processing - if not, they're processed at the same time the parent unit is being processed
    bool do_subunit_scheduling = false;
//Does this unit require special scheduling?
    enum schedulepriorityenum { scheduleDefault, scheduleAField, scheduleRoid }
            schedule_priority = scheduleDefault;
//number of meshes (each with separate texture) this unit has


//The image that will appear on those screens of units targetting this unit
    UnitImages<void> *pImage = nullptr;
//positive for the multiplier applied to nearby spec starships (1 = planetary/inert effects) 0 is default (no effect), -X means 0 but able to be enabled
    float specInterdiction = 0;

    float HeatSink = 0;
public:
    //BUCO! Must add shield tightness back into units.csv for great justice.
    //are shields tight to the hull.  zero means bubble
    float shieldtight = GameConfig::GetVariable("physics",
            "default_shield_tightness",
            0);

public:
    // TODO: move cloak to Cloakable?
    ///How much energy cloaking takes per frame
    float cloakenergy = 0;
    ///how fast this starship decloaks/close...if negative, decloaking
    int cloakrate = 100;   //short fix
    ///If this unit cloaks like glass or like fading
    bool cloakglass = false;

    //-1 is not available... ranges between 0 32767 for "how invisible" unit currently is (32768... -32768) being visible)
    // Despite the above comment, Init() would set it to -1
    int cloaking = -1;                              //short fix
//the minimum cloaking value...
    int cloakmin = cloakglass ? 1 : 0;              //short fix

    // TODO: move to jump_capable?
    ///if the unit is a wormhole
    bool forcejump = false;

//Should not be drawn
    enum INVIS { DEFAULTVIS = 0x0, INVISGLOW = 0x1, INVISUNIT = 0x2, INVISCAMERA = 0x4 };
    unsigned char invisible = DEFAULTVIS;             //1 means turn off glow, 2 means turn off ship
//corners of object

public:

    Vector LocalCoordinates(const Unit *un) const {
        return ToLocalCoordinates((un->Position() - Position()).Cast());
    }

//how visible the ship is from 0 to 1
    float CloakVisible() const {
        if (cloaking < 0) {
            return 1;
        }
        return ((float) cloaking) / 2147483647;
    }

//cloaks or decloaks the starship depending on the bool
    virtual void Cloak(bool cloak);
//deletes
    void Kill(bool eraseFromSave = true, bool quitting = false);

//Is dead yet?
    inline bool Killed() const {
        return killed;
    }

    bool IsExploding() const {
        return pImage->timeexplode > 0;
    }

    // 0 = not stated, 1 = done
    float ExplodingProgress() const;

    ///Resolves forces of given unit on a physics frame
    Vector ResolveForces(const Transformation &, const Matrix &);

//What's the size of this unit
    float rSize() const {
        return radial_size;
    }

//Returns the current world space position
    QVector Position() const {
        return cumulative_transformation.position;
    }

    const Matrix &GetTransformation() const {
        return cumulative_transformation_matrix;
    }

//Returns the unit-space position
    QVector LocalPosition() const {
        return curr_physical_state.position;
    }

    ///Sets the cumulative transformation matrix's position...for setting up to be out in the middle of nowhere
    void SetCurPosition(const QVector &pos) {
        curr_physical_state.position = pos;
    }

    void SetPosAndCumPos(const QVector &pos) {
        SetPosition(pos);
        cumulative_transformation_matrix.p = pos;
        cumulative_transformation.position = pos;
    }

//Sets the state of drawing
    void SetVisible(bool isvis);
    void SetAllVisible(bool isvis);
    void SetGlowVisible(bool isvis);


/**
 * Fire engine takes a unit vector for direction
 * and how fast the fuel speed and mass coming out are
 */
/*unit vector... might default to "r"*/
    void FireEngines(const Vector &Direction, float FuelSpeed, float FMass);
//applies a force for the whole gameturn upon the center of mass
    void ApplyForce(const Vector &Vforce);
//applies a force for the whole gameturn upon the center of mass, using local coordinates
    void ApplyLocalForce(const Vector &Vforce);
//applies a force that is multipled by the mass of the ship
    void Accelerate(const Vector &Vforce);
//Apply a torque in world level coords
    void ApplyTorque(const Vector &Vforce, const QVector &Location);
//Applies a torque in local level coordinates
    void ApplyLocalTorque(const Vector &Vforce, const Vector &Location);
//usually from thrusters remember if I have 2 balanced thrusters I should multiply their effect by 2 :)
    void ApplyBalancedLocalTorque(const Vector &Vforce, const Vector &Location);

//convenient shortcut to applying torques with vector and position
    void ApplyLocalTorque(const Vector &torque);
//Applies damage to the local area given by pnt
    float ApplyLocalDamage(const Vector &pnt,
            const Vector &normal,
            Damage damage,
            Unit *affectedSubUnit,
            const GFXColor &);
//Applies damage from network data
    void ApplyNetDamage(Vector &pnt, Vector &normal, float amt, float ppercentage, float spercentage, GFXColor &color);

//Deals remaining damage to the hull at point and applies lighting effects
//short fix
//    virtual void ArmorDamageSound( const Vector &pnt ) {}
//    virtual void HullDamageSound( const Vector &pnt ) {}
//Clamps thrust to the limits struct
    Vector ClampThrust(const Vector &thrust, bool afterburn);
//Takes a unit vector for direction of thrust and scales to limits
    Vector MaxThrust(const Vector &thrust);
//Thrusts by ammt and clamps accordingly (afterburn or not)
    virtual void Thrust(const Vector &amt, bool afterburn = false);
//Applies lateral thrust
    void LateralThrust(float amt);
//Applies vertical thrust
    void VerticalThrust(float amt);
//Applies forward thrust
    void LongitudinalThrust(float amt);
//Clamps desired velocity to computer set limits
    Vector ClampVelocity(const Vector &velocity, const bool afterburn);
//Clamps desired angular velocity to computer set limits
    Vector ClampAngVel(const Vector &vel);
//Clamps desired torque to computer set limits of thrusters
    Vector ClampTorque(const Vector &torque);
//scales unit size torque to limits in that direction
    Vector MaxTorque(const Vector &torque);
//Applies a yaw of amt
    void YawTorque(float amt);
//Applies a pitch of amt
    void PitchTorque(float amt);
//Applies a roll of amt
    void RollTorque(float amt);
//executes a repair if the repair bot is up to it
    void Repair();

//Useful if you want to override subunit processing, but not self-processing (Asteroids, people?)
    virtual void UpdateSubunitPhysics(const Transformation &trans,
            const Matrix &transmat,
            const Vector &CumulativeVelocity,
            bool ResolveLast,
            UnitCollection *uc,
            Unit *superunit);
//A helper for those who override UpdateSubunitPhysics - Process one subunit (also, an easier way of overriding subunit processing uniformly)
    virtual void UpdateSubunitPhysics(Unit *subunit,
            const Transformation &trans,
            const Matrix &transmat,
            const Vector &CumulativeVelocity,
            bool ResolveLast,
            UnitCollection *uc,
            Unit *superunit);



/*
 **************************************************************************************
 **** WEAPONS/SHIELD STUFF                                                          ***
 **************************************************************************************
 */


public:

//returns percentage of course deviation for contraband searches.  .5 causes error and 1 causes them to get mad




/*
 **************************************************************************************
 **** TARGETTING STUFF                                                              ***
 **************************************************************************************
 */

public:
//not used yet
    StringPool::Reference target_fgid[3];

    bool InRange(const Unit *target, bool cone = true, bool cap = true) const {
        double mm;
        return InRange(target, mm, cone, cap, true);
    }

    bool InRange(const Unit *target, double &mm, bool cone, bool cap, bool lock) const;
    Unit *Target();
    const Unit *Target() const;
    Unit *VelocityReference();
    const Unit *VelocityReference() const;
    Unit *Threat();
//Uses Universe stuff so only in Unit class
    void VelocityReference(Unit *targ);
    void TargetTurret(Unit *targ);
//Threatens this unit with "targ" as aggressor. Danger should be cos angle to target
    void Threaten(Unit *targ, float danger);

//Rekeys the threat level to zero for another turn of impending danger
    void ResetThreatLevel() {
        computer.threatlevel = 0;
        graphicOptions.missilelock = 0;
    }

//The cosine of the angle to the target given passed in speed and range
    float cosAngleTo(Unit *target,
            float &distance,
            float speed = 0.001,
            float range = 0.001,
            bool turnmargin = true) const;
//Highest cosine from given mounts to target. Returns distance and cosine
    float cosAngleFromMountTo(Unit *target, float &distance) const;
//how locked are we
    float computeLockingPercent();
//Turns on selection box
    void Select();
//Turns off selection box
    void Deselect();

//Shouldn't do anything here - but needed by Python
    void Target(Unit *targ);

//not used yet
    void setTargetFg(std::string primary, std::string secondary = std::string(), std::string tertiary = std::string());
//not used yet
    void ReTargetFg(int which_target = 0);
//not used yet



/*
 **************************************************************************************
 **** COLLISION STUFF                                                               ***
 **************************************************************************************
 */

public:
//The information about the minimum and maximum ranges of this unit. Collide Tables point to this bit of information.
    enum COLLIDELOCATIONTYPES { UNIT_ONLY = 0, UNIT_BOLT = 1, NUM_COLLIDE_MAPS = 2 };
//location[0] is for units only, location[1] is for units + bolts
    // This used to be initialized by set_null (see collide_map)
    // Right now, there's an ifdef that assigns NULL but it could be something else.
    CollideMap::iterator location[2] = {nullptr, nullptr};
    struct collideTrees *colTrees = nullptr;
//Sets the parent to be this unit. Unit never dereferenced for this operation
    void SetCollisionParent(Unit *name);
//won't collide with ownery
    void SetOwner(Unit *target);
    void SetRecursiveOwner(Unit *target);

//Shouldn't do anything here - but needed by Python
//Queries the ray collider with a world space st and end point. Returns the normal and distance on the line of the intersection
    Unit *rayCollide(const QVector &st, const QVector &end, Vector &normal, float &distance);

//fils in corner_min,corner_max and radial_size
//Uses Box stuff -> only in NetUnit and Unit
    void calculate_extent(bool update_collide_queue);

//Uses mesh stuff (only rSize()) : I have to find something to do
    bool Inside(const QVector &position, const float radius, Vector &normal, float &dist);
//Uses collide and Universe stuff -> put in NetUnit
    void UpdateCollideQueue(StarSystem *ss, CollideMap::iterator hint[NUM_COLLIDE_MAPS]);
//Uses collision stuff so only in NetUnit and Unit classes
    bool querySphere(const QVector &pnt, float err) const;
//queries the sphere for beams (world space start,end)  size is added to by my_unit_radius
    float querySphere(const QVector &start, const QVector &end, float my_unit_radius = 0) const;
    float querySphereNoRecurse(const QVector &start, const QVector &end, float my_unit_radius = 0) const;
//queries the ship with a directed ray
//for click list
    float querySphereClickList(const QVector &st, const QVector &dir, float err) const;
//Queries if this unit is within a given frustum
    bool queryFrustum(double frustum[6][4]) const;

/**
 * Queries the bounding sphere with a duo of mouse coordinates that project
 * to the center of a ship and compare with a sphere...pretty fast
 * queries the sphere for weapons (world space point)
 * Only in Unit class
 */
    bool querySphereClickList(int, int, float err, Camera *activeCam) const;

    bool InsideCollideTree(Unit *smaller,
            QVector &bigpos,
            Vector &bigNormal,
            QVector &smallpos,
            Vector &smallNormal,
            bool bigasteroid = false,
            bool smallasteroid = false);
//    virtual void reactToCollision( Unit *smaller,
//                                   const QVector &biglocation,
//                                   const Vector &bignormal,
//                                   const QVector &smalllocation,
//                                   const Vector &smallnormal,
//                                   float dist );

//returns true if jump possible even if not taken
//Uses Universe thing
    bool jumpReactToCollision(Unit *smaller);
//Does a collision between this and another unit
    bool Collide(Unit *target);
//checks for collisions with all beams and other units roughly and then more carefully
    void CollideAll();

/*
 **************************************************************************************
 **** DOCKING STUFF                                                                 ***
 **************************************************************************************
 */

public:

//returns -1 if unit cannot dock, otherwise returns which dock it can dock at
    int CanDockWithMe(Unit *dockingunit, bool forcedock = false);
    int ForceDock(Unit *utdw, unsigned int whichdockport);
    void PerformDockingOperations();
    void FreeDockingPort(unsigned int whichport);
    const std::vector<struct DockingPorts> &DockingPortLocations() const;

    char DockedOrDocking() const {
        return docked;
    }

    bool IsCleared(const Unit *dockignunit) const;
    bool isDocked(const Unit *dockingUnit) const;
    bool UnDock(Unit *unitToDockWith);
//Use AI
    ///returns -1 if unit cannot dock, otherwise returns which dock it can dock at
    // The above is no longer accurate. It returns a bool...
    bool RequestClearance(Unit *dockingunit);
    bool EndRequestClearance(Unit *dockingunit);
    bool hasPendingClearanceRequests() const;
    int Dock(Unit *unitToDockWith);
    void RestoreGodliness();

/*
 **************************************************************************************
 **** FACTION/FLIGHTGROUP STUFF                                                     ***
 **************************************************************************************
 */

public:
//the flightgroup this ship is in
    Flightgroup *flightgroup = nullptr;
//the flightgroup subnumber
    int flightgroup_subnumber = 0;

    void SetFg(Flightgroup *fg, int fg_snumber);
//The faction of this unit
    int faction = 0;
    void SetFaction(int faction);

//get the flightgroup description
    Flightgroup *getFlightgroup() const {
        return flightgroup;
    }

//get the subnumber
    int getFgSubnumber() const {
        return flightgroup_subnumber;
    }

//get the full flightgroup ID (i.e 'green-4')
    const std::string getFgID();
    // Changed next two lines from struct CargoColor to class CargoColor to fit line 70 declaration
    std::vector<class CargoColor> &FilterDowngradeList(std::vector<class CargoColor> &mylist, bool downgrade = true);
    std::vector<class CargoColor> &FilterUpgradeList(std::vector<class CargoColor> &mylist);

    bool IsBase() const;

/*
 **************************************************************************************
 **** MISC STUFF                                                                    ***
 **************************************************************************************
 */

public:
    enum tractorHow { tractorImmune = 0, tractorPush = 1, tractorIn = 2, tractorBoth = 3 };
    bool isTractorable(enum tractorHow how = tractorBoth) const;
    void setTractorability(enum tractorHow how);
    enum tractorHow getTractorability() const;

private:
    unsigned char tractorability_flags = tractorImmune;

protected:
//if the unit is a planet, this contains the long-name 'mars-station'
    std::string fullname;
public:
    void setFullname(std::string name) {
        fullname = name;
    }

    const string &getFullname() const {
        return fullname;
    }

    const string &getFilename() const {
        return filename.get();
    }

//Is this class a unit
    virtual enum _UnitType isUnit() const {
        return _UnitType::unit;
    }

    void Ref();
//Low level list function to reference the unit as being the target of a UnitContainer or Colleciton
//Releases the unit from this reference of UnitContainer or Collection
    void UnRef();
//0 in additive is reaplce  1 is add 2 is mult
//Put that in NetUnit & AcctUnit with string and with Unit
    UnitImages<void> &GetImageInformation();

//sets the full name/fgid for planets
    bool isStarShip() const {
        return isUnit() == _UnitType::unit;
    }

    bool isPlanet() const {
        return isUnit() == _UnitType::planet;
    }

    bool isJumppoint() const {
        return GetDestinations().size() != 0;
    }

    void TurretFAW();

    // Temporary holder for upgrade values
    // Because the upgrade mechanism uses macros to construct code,
    // we can't touch it without a complete rewrite.
    // Example:
    // MACRO_FUNCTION(field_a, object_a, object_b)
    // object_a->field_a = object_b->field_b;
    float temporary_upgrade_float_variable;

};

Unit *findUnitInStarsystem(const void *unitDoNotDereference);

//Holds temporary values for inter-function XML communication Saves deprecated restr info
struct Unit::XML {
    float randomstartframe;
    float randomstartseconds;
    std::vector<Mount *> mountz;
    std::vector<Mesh *> meshes;
    std::vector<std::string> meshes_str;
    Mesh *shieldmesh;
    Mesh *rapidmesh;
    std::string shieldmesh_str;
    std::string rapidmesh_str;
    void *data;
    std::vector<Unit *> units;
    int unitlevel;
    bool hasColTree;
    enum restr { YRESTR = 1, PRESTR = 2, RRESTR = 4 };
    const char *unitModifications;
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

inline Unit *UnitContainer::GetUnit() {
    if (unit != NULL) {
#ifdef CONTAINER_DEBUG
        CheckUnit( unit );
#endif
        if (unit->Killed()) {
            unit->UnRef();
            unit = NULL;
        }
    }
    return unit;
}

#ifdef USE_OLD_COLLECTION
inline void UnitCollection::UnitIterator::GetNextValidUnit()
{
    while (pos->next->unit ? pos->next->unit->Killed() : false)
        remove();
}
#endif

extern std::set<std::string> GetListOfDowngrades();
extern void ClearDowngradeMap();
#endif

/*
 **************************************************************************************
 **** MESH ANIMATION STUFF                                                       ***
 **************************************************************************************
 */



#endif

