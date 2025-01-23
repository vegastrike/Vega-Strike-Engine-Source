/*
 * star_system.h
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
 *
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
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
#ifndef VEGA_STRIKE_ENGINE_SYSTEM_H
#define VEGA_STRIKE_ENGINE_SYSTEM_H

#include "cmd/collection.h"
#include "cmd/container.h"

#include "gfx/vec.h"
#include "gfxlib.h"
#include "gfxlib_struct.h"

#include "star_xml.h"

#include <string>
#include <vector>
#include <map>

using std::vector;
using std::string;

class Atmosphere;
class Background;
class ClickList;
class CollideMap;

class ContinuousTerrain;
class Planet;

class Stars;

class Terrain;
class TextPlane;
class Texture;
class Unit;
class Universe;
class StarSystem;

const unsigned int SIM_QUEUE_SIZE = 128;
bool PendingJumpsEmpty();

struct Statistics {
    //neutral, friendly, enemy
    std::vector<UnitContainer> navs[3];
    vsUMap<std::string, UnitContainer> jumpPoints;
    int system_faction;
    int newfriendlycount;
    int newenemycount;
    int newcitizencount;
    int newneutralcount;
    int friendlycount;
    int enemycount;
    int neutralcount;
    int citizencount;
    size_t checkIter;
    size_t navCheckIter;
    Statistics();
    void AddUnit(Unit *un);
    void RemoveUnit(Unit *un);
    void CheckVitals(StarSystem *ss);
};

/**
 * Star System
 * Scene management for a star system
 * Per-Frame Drawing & PhysicsConfig simulation
 **/
class StarSystem {
public:
    Statistics stats;
    std::multimap<Unit *, Unit *> last_collisions;
    CollideMap *collide_map[2]; // 0 Unit 1 Bolt
    class CollideTable *collide_table = nullptr;

protected:

    // Fields
    ///PhysicsConfig is divided into 3 stages spread over 3 frames
    enum PHYSICS_STAGE { MISSION_SIMULATION, PROCESS_UNIT, PHY_NUM }
            current_stage = MISSION_SIMULATION;

    vector<Terrain *> terrains;
    vector<ContinuousTerrain *> continuous_terrains;

    ///system name
    string name;
    string filename;
    un_iter sigIter;

    ///to track the next given physics frame
    double time = 0;

    /// Everything to be drawn. Folded missiles in here oneday
    UnitCollection draw_list;
    UnitCollection gravitational_units;
    UnitCollection physics_buffer[SIM_QUEUE_SIZE + 1];
    unsigned int current_sim_location = 0;

    ///The moving, fading stars
    Stars *stars = nullptr;

    /// Objects subject to global gravitron physics (disabled)
    // TODO: investigate and remove
    unsigned char no_collision_time = 0;

    unsigned int zone = 0; //short fix - TODO: figure out for what
    int light_context;
    vector<class MissileEffect *> discharged_missiles;

    ///Starsystem XML Struct For use with XML loading
    Star_XML *xml;

    ///The background associated with this system
    Background *background = nullptr;
    ///The Light Map corresponding for the BP for spheremapping
    Texture *light_map[6];
public:
    // Constructors
    StarSystem(const string filename, const Vector &centroid = Vector(0, 0, 0), const float timeofyear = 0);
    virtual ~StarSystem();
    friend class Universe;

    // Methods
    void AddStarsystemToUniverse(const string &filename);
    void RemoveStarsystemFromUniverse();
    void LoadXML(const string, const Vector &centroid, const float timeofyear);

    void SetZone(unsigned int zonenum) {
        //short fix
        this->zone = zonenum;
    }

    unsigned int GetZone() {
        //short fix
        return this->zone;
    }

    virtual void AddMissileToQueue(class MissileEffect *);
    virtual void UpdateMissiles();
    void UpdateUnitsPhysics(bool firstframe);
    void UpdateUnitPhysics(bool firstframe, Unit *unit);

    ///Requeues the unit so that it is simulated ASAP.
    void RequestPhysics(Unit *un, unsigned int queue);

    /// update a simulation atom ExecuteDirector must be false if star system is just loaded before mission is loaded
    void Update(float priority, bool executeDirector);
    //This one is temporarly used on server side
    void Update(float priority);

    ///Gets the current simulation frame
    unsigned int getCurrentSimFrame() const {
        return current_sim_location;
    }

    void ExecuteUnitAI();

    static void beginElement(void *userData, const XML_Char *name, const XML_Char **atts);
    static void endElement(void *userData, const XML_Char *name);
    std::string getFileName() const;
    std::string getName();

    ///Loads the star system from an XML file
    UnitCollection &getUnitList() {
        return draw_list;
    }

    UnitCollection &gravitationalUnits() {
        return gravitational_units;
    }

    Unit *nextSignificantUnit();
    /// returns xy sorted bounding spheres of all units in current view
    ///Adds to draw list
    void AddUnit(Unit *unit);
    ///Removes from draw list
    bool RemoveUnit(Unit *unit);
    bool JumpTo(Unit *unit,
            Unit *jumppoint,
            const std::string &system,
            bool force = false,
            bool save_coordinates = false /*for intersystem transit the long way*/ );
    static void ProcessPendingJumps();

    Background *getBackground();

///activates the light map texture
    void activateLightMap(int stage = 1);
    Texture *getLightMap();
    static void DrawJumpStars();

    Terrain *getTerrain(unsigned int which);
    unsigned int numTerrain();
    ContinuousTerrain *getContTerrain(unsigned int which);
    unsigned int numContTerrain();

/// returns xy sorted bounding spheres of all units in current view
    ClickList *getClickList();
///Adds to draw list
///Draws a frame of action, interpolating between physics frames
    void Draw(bool DrawCockpit = true);

///re-enables the included lights and terrains
    void SwapIn();
///Disables included lights and terrains
    void SwapOut();

    friend class Atmosphere;
    void createBackground(Star_XML *xml);

    void VolitalizeJumpAnimation(const int ani);
    void DoJumpingComeSightAndSound(Unit *un);
    int DoJumpingLeaveSightAndSound(Unit *un);
};

#endif //VEGA_STRIKE_ENGINE_SYSTEM_H

