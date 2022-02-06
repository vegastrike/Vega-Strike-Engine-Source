/*
 * Copyright (C) 2001-2022 Daniel Horn, pyramid3d, Stephen G. Tuggy,
 * and other Vega Strike contributors.
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
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef _WrapGFX_H
#define _WrapGFX_H

#include "gfx/camera.h"
#include "star_system.h"
#include "gfx/cockpit.h"
#include "faction_generic.h"
#include "galaxy_xml.h"
#include "stardate.h"

/**
 * Class Universe Deals with universal constants. It is a global,
 * accessed from anywhere as _Universe-> Universe may be queried for
 * Relationships, the current star system rendering is taking place in
 * etc.  It acts as a wrapper to the active Star System.
 * Additionally it handles beginning and ending the main loop.
 * And starting and ending graphics. (incl the task of wiping temp lights)
 * Deprecated: loaded dynamic gldrv module
 */
class Universe {
    // Fields
public:
    StarDate current_stardate;
    vector<StarSystem *> star_system;

protected:
    std::unique_ptr<GalaxyXML::Galaxy> galaxy;
    Camera hud_camera; // a generic camera facing the HUD

    // Constructors
public:
    Universe(int argc, char **argv, const char *galaxy);
    Universe();
    void Init(int argc, char **argv, const char *galaxy);
    ~Universe();
    class StarSystem *Init(string systemfile,
                           const Vector &centroid = Vector(0, 0, 0),
                           const string planetname = string()); // Should load the Universe data file. Now just inits system with test.xml
    friend void bootstrap_main_loop(); //so it can get all cockpits
    friend class Faction;

    // Methods
public:
// Gameplay Methods
    void Loop(void main_loop());
    void WriteSaveGame(bool auto_save);
    // Graphic Methods
    void StartDraw(); // Begin a scene
    void StartGFX(); // Loads Defaults in Graphics Drivers
    void StartGL(); // init proc
    void SetViewport()
    {
        AccessCockpit()->SetViewport();
    }

    //Update starsystems (for server side)
    void Update();

// Camera
    Camera *AccessCamera(int num);
    Camera *AccessCamera();
    Camera *AccessHudCamera();
    void SelectCamera(int cam);

// Cockpits
    void SetupCockpits(vector<string> players);
    Cockpit *AccessCockpit();
    Cockpit *AccessCockpit(int i);
    unsigned int CurrentCockpit();
    void SetActiveCockpit(int whichcockpit);
    void SetActiveCockpit(Cockpit *which);
    Cockpit *createCockpit(std::string player);

// Galaxy
    void getJumpPath(const string &from, const string &to, vector<string> &path) const;
    const vector<string> &getAdjacentStarSystems(const string &ss) const;
    string getGalaxyProperty(const string &sys, const string &prop);
    string getGalaxyPropertyDefault(const string &sys, const string &prop, const string def = "");
    GalaxyXML::Galaxy *getGalaxy();

// Light Map
    void activateLightMap(int stage = 1);
    Texture *getLightMap();

// Player Ship
    Cockpit *isPlayerStarship(const Unit *fighter);

    Cockpit *isPlayerStarshipVoid(const void *pointercompare)
    {
        return isPlayerStarship((const Unit *) pointercompare);
    }

    int whichPlayerStarship(const Unit *fighter);

// Script System - whatever that is
    StarSystem *scriptStarSystem();
    bool setScriptSystem(string name);

// Star Systems
    StarSystem *activeStarSystem();
    bool StillExists(StarSystem *ss);
    void setActiveStarSystem(StarSystem *ss);
    void pushActiveStarSystem(StarSystem *ss);
    void popActiveStarSystem();
    StarSystem *GenerateStarSystem(const char *file, const char *jumpback, Vector origin);
    void LoadStarSystem(StarSystem *ss);
    void UnloadStarSystem(StarSystem *ss);
    void Generate1(const char *file, const char *jumpback);
    void Generate2(StarSystem *ss);
    void clearAllSystems();
    StarSystem *getActiveStarSystem(unsigned int size);
    unsigned int getNumActiveStarSystem();
    StarSystem *getStarSystem(string name);
    int StarSystemIndex(StarSystem *ss);

// Misc. Methods
    void LoadFactionXML(const char *factfile);
    UnitCollection &getActiveStarSystemUnitList();
    unsigned int numPlayers();
};
#endif

