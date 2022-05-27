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


#include <assert.h>
#include "star_system.h"

#include "damageable.h"
#include "lin_time.h"
#include "audiolib.h"
#include "config_xml.h"
#include "vs_globals.h"
#include "vegastrike.h"
#include "universe.h"
#include "hashtable.h"
#include "galaxy_gen.h"
#include "in_kb.h"
#include "load_mission.h"
#include "vsfilesystem.h"
#include "vs_logging.h"
#include "drawable.h"
#include "options.h"
#include "configxml.h"
#include "vs_random.h"
#include "savegame.h"
#include "in_kb_data.h"
#include "universe_util.h" //get galaxy faction, dude

#include "cmd/planet.h"
#include "cmd/unit_collide.h"
#include "cmd/collection.h"
#include "cmd/click_list.h"
#include "cmd/cont_terrain.h"
#include "cmd/unit_collide.h"
#include "cmd/nebula.h"
#include "cmd/unit_find.h"
#include "cmd/script/flightgroup.h"
#include "cmd/script/mission.h"
#include "cmd/atmosphere.h"
#include "cmd/music.h"
#include "cmd/bolt.h"
#include "cmd/beam.h"
#include "cmd/planet.h"
#include "cmd/unit_generic.h"
#include "cmd/nebula.h"
#include "cmd/unit_util.h"
#include "cmd/missile.h"

#include "gfx/boltdrawmanager.h"
#include "gfx/sphere.h"
#include "gfx/particle.h"
#include "gfx/lerp.h"
#include "gfx/warptrail.h"
#include "gfx/env_map_gent.h"
#include "gfx/halo.h"
#include "gfx/background.h"
#include "gfx/animation.h"
#include "gfx/aux_texture.h"
#include "gfx/star.h"
#include "gfx/cockpit.h"
#include "gfx/occlusion.h"
#include "gfx/vec.h"
#include "gfx/cockpit_generic.h"

#include <boost/python/errors.hpp>

using std::endl;

extern void ClientServerSetLightContext(int lightcontext);
double calc_blend_factor(double frac,
        unsigned int priority,
        unsigned int when_it_will_be_simulated,
        unsigned int cur_simulation_frame);

StarSystem::StarSystem(const string filename, const Vector &centr, const float timeofyear) {
    collide_map[Unit::UNIT_ONLY] = new CollideMap(Unit::UNIT_ONLY);
    collide_map[Unit::UNIT_BOLT] = new CollideMap(Unit::UNIT_BOLT);

    // no_collision_time = (int)(1+2.000/SIMULATION_ATOM);

    sigIter = draw_list.createIterator();

    ///adds to jumping table;
    _Universe->pushActiveStarSystem(this);
    GFXCreateLightContext(light_context);
    collide_table = new CollideTable(this);

    LoadXML(filename, centr, timeofyear);
    if (name.empty()) {
        name = filename;
    }
    AddStarsystemToUniverse(filename);
    UpdateTime();

    Atmosphere::Parameters params;

    params.radius = 40000;

    params.low_color[0] = GFXColor(0, 0.5, 0.0);

    params.low_color[1] = GFXColor(0, 1.0, 0.0);

    params.low_ambient_color[0] = GFXColor(0.0 / 255.0, 0.0 / 255.0, 0.0 / 255.0);

    params.low_ambient_color[1] = GFXColor(0.0 / 255.0, 0.0 / 255.0, 0.0 / 255.0);

    params.high_color[0] = GFXColor(0.5, 0.0, 0.0);

    params.high_color[1] = GFXColor(1.0, 0.0, 0.0);

    params.high_ambient_color[0] = GFXColor(0, 0, 0);

    params.high_ambient_color[1] = GFXColor(0, 0, 0);

    params.scattering = 5;

    _Universe->popActiveStarSystem();
}

StarSystem::~StarSystem() {
    if (_Universe->getNumActiveStarSystem()) {
        _Universe->activeStarSystem()->SwapOut();
    }
    _Universe->pushActiveStarSystem(this);
    ClientServerSetLightContext(light_context);
    for (un_iter iter = draw_list.createIterator(); !iter.isDone(); ++iter) {
        (*iter)->Kill(false);
    }
    //if the next line goes ANYWHERE else Vega Strike will CRASH!!!!!
    //DO NOT MOVE THIS LINE! IT MUST STAY
    if (collide_table != nullptr) {
        delete collide_table;
        collide_table = nullptr;
    }
    _Universe->popActiveStarSystem();
    vector<StarSystem *> activ;
    while (_Universe->getNumActiveStarSystem()) {
        if (_Universe->activeStarSystem() != this) {
            activ.push_back(_Universe->activeStarSystem());
        } else {
            VS_LOG(error, (boost::format("Avoided fatal error in deleting star system %1%") % getFileName().c_str()));
        }
        _Universe->popActiveStarSystem();
    }
    while (activ.size()) {
        _Universe->pushActiveStarSystem(activ.back());
        activ.pop_back();
    }
    if (_Universe->getNumActiveStarSystem()) {
        _Universe->activeStarSystem()->SwapIn();
    }
    RemoveStarsystemFromUniverse();
    delete collide_map[Unit::UNIT_ONLY];
    delete collide_map[Unit::UNIT_BOLT];

#ifdef NV_CUBE_MAP
    delete light_map[0];
    delete light_map[1];
    delete light_map[2];
    delete light_map[3];
    delete light_map[4];
    delete light_map[5];
#else
    delete light_map[0];
#endif

    delete background;
    delete stars;
}

Texture *StarSystem::getLightMap() {
    return light_map[0];
}

void StarSystem::activateLightMap(int stage) {
    GFXActiveTexture(stage);
#ifdef NV_CUBE_MAP
    light_map[0]->MakeActive(stage);
#else
    light_map[0]->MakeActive( stage );
#endif
    GFXTextureEnv(stage, GFXADDTEXTURE);
#ifdef NV_CUBE_MAP
    GFXToggleTexture(true, stage, CUBEMAP);
    GFXTextureCoordGenMode(stage, CUBE_MAP_GEN, nullptr, nullptr);
#else
    const float tempo[4] = {1, 0, 0, 0};
    GFXToggleTexture( true, stage, TEXTURE2D );
    GFXTextureCoordGenMode( stage, SPHERE_MAP_GEN, tempo, tempo );
#endif
    GFXActiveTexture(0);
}

ClickList *StarSystem::getClickList() {
    return new ClickList(this, &draw_list);
}

void ConditionalCursorDraw(bool tf) {
    if (game_options()->hardware_cursor) {
        winsys_show_cursor(tf);
    }
}

void StarSystem::SwapIn() {
    GFXSetLightContext(light_context);
}

void StarSystem::SwapOut() {
}

extern double saved_interpolation_blend_factor;
extern double interpolation_blend_factor;
extern bool cam_setup_phase;

//Class for use of UnitWithinRangeLocator template
//Used to do distance based pre-culling for draw function based on sorted search structure
class UnitDrawer {
    struct empty {};
    vsUMap<void *, struct empty> gravunits;
public:
    Unit *parent;
    Unit *parenttarget;

    UnitDrawer() {
        parent = nullptr;
        parenttarget = nullptr;
    }

    // can't remove redundant distance parameter, as function acquire is overloaded
    // by template in UnitWithinRangeLocator
    bool acquire(Unit *unit, float distance) {
        if (gravunits.find(unit) == gravunits.end()) {
            return draw(unit);
        } else {
            return true;
        }
    }

    void drawParents() {
        if (parent && parent->isSubUnit()) {
            parent = UnitUtil::owner(parent);
        }
        if (parent) {
            draw(parent);
        }

        if (parenttarget && parenttarget->isSubUnit()) {
            parenttarget = UnitUtil::owner(parenttarget);
        }
        if (parenttarget) {
            draw(parenttarget);
        }
    }

    bool draw(Unit *unit) {
        if (parent == unit || (parent && parent->isSubUnit() && parent->owner == unit)) {
            parent = nullptr;
        }
        if (parenttarget == unit || (parenttarget && parenttarget->isSubUnit() && parenttarget->owner == unit)) {
            parenttarget = nullptr;
        }
        float backup = simulation_atom_var;
        //VS_LOG(trace, (boost::format("UnitDrawer::draw(): simulation_atom_var as backed up  = %1%") % simulation_atom_var));
        unsigned int cur_sim_frame = _Universe->activeStarSystem()->getCurrentSimFrame();
        interpolation_blend_factor = calc_blend_factor(saved_interpolation_blend_factor, unit->sim_atom_multiplier,
                unit->cur_sim_queue_slot,
                cur_sim_frame);
        simulation_atom_var = backup * unit->sim_atom_multiplier;
        //VS_LOG(trace, (boost::format("UnitDrawer::draw(): simulation_atom_var as multiplied = %1%") % simulation_atom_var));
        (/*(GameUnit*)*/ unit)->Draw();
        interpolation_blend_factor = saved_interpolation_blend_factor;
        simulation_atom_var = backup;
        //VS_LOG(trace, (boost::format("UnitDrawer::draw(): simulation_atom_var as restored   = %1%") % simulation_atom_var));
        return true;
    }

    bool grav_acquire(Unit *unit) {
        gravunits[unit] = empty();
        return draw(unit);
    }
};

#define UPDATEDEBUG  //for hard to track down bugs

void StarSystem::Draw(bool DrawCockpit) {
    GFXEnable(DEPTHTEST);
    GFXEnable(DEPTHWRITE);
    saved_interpolation_blend_factor = interpolation_blend_factor =
            (1. / PHY_NUM) * ((PHY_NUM * time) / static_cast<double>(simulation_atom_var) + current_stage);
    GFXColor4f(1, 1, 1, 1);
    if (DrawCockpit) {
        AnimatedTexture::UpdateAllFrame();
    }
    for (unsigned int i = 0; i < continuous_terrains.size(); ++i) {
        continuous_terrains[i]->AdjustTerrain(this);
    }
    Unit *par;
    if ((par = _Universe->AccessCockpit()->GetParent()) == nullptr) {
        _Universe->AccessCamera()->UpdateGFX(GFXTRUE);
    } else if (!par->isSubUnit()) {
        //now we can assume world is topps
        par->cumulative_transformation = linear_interpolate(par->prev_physical_state,
                par->curr_physical_state,
                interpolation_blend_factor);
        Unit *targ = par->Target();
        if (targ && !targ->isSubUnit()) {
            targ->cumulative_transformation = linear_interpolate(targ->prev_physical_state,
                    targ->curr_physical_state,
                    interpolation_blend_factor);
        }
        _Universe->AccessCockpit()->SetupViewPort(true);
    }
    double setupdrawtime = queryTime();
    {
        cam_setup_phase = true;

        Unit *saveparent = _Universe->AccessCockpit()->GetSaveParent();
        Unit *targ = nullptr;
        if (saveparent) {
            targ = saveparent->Target();
        }
        //Array containing the two interesting units, so as not to have to copy-paste code
        Unit *camunits[2] = {saveparent, targ};
        float backup = simulation_atom_var;
        //VS_LOG(trace, (boost::format("StarSystem::Draw(): simulation_atom_var as backed up  = %1%") % simulation_atom_var));
        unsigned int cur_sim_frame = _Universe->activeStarSystem()->getCurrentSimFrame();
        for (int i = 0; i < 2; ++i) {
            Unit *unit = camunits[i];
            //Make sure unit is not null;
            if (unit && !unit->isSubUnit()) {
                interpolation_blend_factor = calc_blend_factor(saved_interpolation_blend_factor,
                        unit->sim_atom_multiplier,
                        unit->cur_sim_queue_slot,
                        cur_sim_frame);
                // stephengtuggy 2020-07-25 - Should we just use the standard SIMULATION_ATOM here?
                simulation_atom_var = backup * unit->sim_atom_multiplier;
                //VS_LOG(trace, (boost::format("StarSystem::Draw(): simulation_atom_var as multiplied = %1%") % simulation_atom_var));
                //( (GameUnit*)unit )->GameUnit::Draw();
                unit->Draw();
            }
        }
        interpolation_blend_factor = saved_interpolation_blend_factor;
        simulation_atom_var = backup;
        //VS_LOG(trace, (boost::format("StarSystem::Draw(): simulation_atom_var as restored   = %1%") % simulation_atom_var));


        ///this is the final, smoothly calculated cam
        _Universe->AccessCockpit()->SetupViewPort(true);

        cam_setup_phase = false;
    }
    setupdrawtime = queryTime() - setupdrawtime;
    GFXDisable(LIGHTING);
    background->Draw();
    double drawtime = queryTime();

    // Initialize occluder system (we'll populate it during unit render)
    Occlusion::start();

    //Ballpark estimate of when an object of configurable size first becomes one pixel

    QVector drawstartpos = _Universe->AccessCamera()->GetPosition();

    Collidable key_iterator(0, 1, drawstartpos);
    UnitWithinRangeOfPosition<UnitDrawer> drawer(game_options()->precull_dist, 0, key_iterator);
    //Need to draw really big stuff (i.e. planets, deathstars, and other mind-bogglingly big things that shouldn't be culled despited extreme distance
    Unit *unit;
    if ((drawer.action.parent = _Universe->AccessCockpit()->GetParent()) != nullptr) {
        drawer.action.parenttarget = drawer.action.parent->Target();
    }
    for (un_iter iter = this->gravitational_units.createIterator(); (unit = *iter); ++iter) {
        float distance = (drawstartpos - unit->Position()).Magnitude() - unit->rSize();
        if (distance < game_options()->precull_dist) {
            drawer.action.grav_acquire(unit);
        } else {
            drawer.action.draw(unit);
        }
    }
    //Need to get iterator to approx camera position
    CollideMap::iterator parent = collide_map[Unit::UNIT_ONLY]->lower_bound(key_iterator);
    findObjectsFromPosition(this->collide_map[Unit::UNIT_ONLY], parent, &drawer, drawstartpos, 0, true);
    drawer.action.drawParents();     //draw units targeted by camera
    //FIXME  maybe we could do bolts & units instead of unit only--and avoid bolt drawing step

    drawtime = queryTime() - drawtime;
    WarpTrailDraw();

    GFXFogMode(FOG_OFF);

    // At this point, we've set all occluders
    // Mesh::ProcessXMeshes will query it

    GFXColor tmpcol(0, 0, 0, 1);
    GFXGetLightContextAmbient(tmpcol);
    double processmesh = queryTime();
    if (!game_options()->draw_near_stars_in_front_of_planets) {
        stars->Draw();
    }
    Mesh::ProcessZFarMeshes();
    if (game_options()->draw_near_stars_in_front_of_planets) {
        stars->Draw();
    }
    GFXEnable(DEPTHTEST);
    GFXEnable(DEPTHWRITE);
    //need to wait for lights to finish
    Planet::ProcessTerrains();
    Terrain::RenderAll();
    Mesh::ProcessUndrawnMeshes(true);
    processmesh = queryTime() - processmesh;
    Nebula *neb;

    Matrix ident;
    Identity(ident);

    GFXPopGlobalEffects();
    GFXLightContextAmbient(tmpcol);
    if ((neb = _Universe->AccessCamera()->GetNebula())) {
        neb->SetFogState();
    }
    Beam::ProcessDrawQueue();
    BoltDrawManager::Draw();

    GFXFogMode(FOG_OFF);
    Animation::ProcessDrawQueue();
    Halo::ProcessDrawQueue();
    particleTrail.DrawAndUpdate();
    StarSystem::DrawJumpStars();
    ConditionalCursorDraw(false);
    if (DrawCockpit) {
        _Universe->AccessCockpit()->Draw();
    }

    Drawable::UpdateFrames();

    // And now we're done with the occluder set
    Occlusion::end();
}

extern void update_ani_cache();

void UpdateAnimatedTexture() {
    AnimatedTexture::UpdateAllPhysics();
    update_ani_cache();
}

void TerrainCollide() {
    Terrain::CollideAll();
}

void UpdateTerrain() {
    Terrain::UpdateAll(64);
}

void UpdateCameraSnds() {
    _Universe->AccessCockpit(0)->AccessCamera()->UpdateCameraSounds();
}

void NebulaUpdate(StarSystem *ss) {
    if (_Universe->AccessCockpit()->activeStarSystem == ss) {
        Nebula *neb;
        if ((neb = _Universe->AccessCamera()->GetNebula())) {
            if (neb->getFade() <= 0) {
                //Update physics should set this
                _Universe->AccessCamera()->SetNebula(nullptr);
            }
        }
    }
}

void StarSystem::createBackground(Star_XML *xml) {
#ifdef NV_CUBE_MAP
    VS_LOG(info, "using NV_CUBE_MAP");
    light_map[0] = new Texture((xml->backgroundname + "_light.cube").c_str(), 1, TRILINEAR, CUBEMAP, CUBEMAP_POSITIVE_X,
            GFXFALSE, game_options()->max_cubemap_size);
    if (light_map[0]->LoadSuccess() && light_map[0]->isCube()) {
        light_map[1] = light_map[2] = light_map[3] = light_map[4] = light_map[5] = 0;
    } else {
        delete light_map[0];
        light_map[0] =
                new Texture((xml->backgroundname + "_right.image").c_str(), 1, TRILINEAR, CUBEMAP, CUBEMAP_POSITIVE_X,
                        GFXFALSE, game_options()->max_cubemap_size);
        light_map[1] =
                new Texture((xml->backgroundname + "_left.image").c_str(), 1, TRILINEAR, CUBEMAP, CUBEMAP_NEGATIVE_X,
                        GFXFALSE, game_options()->max_cubemap_size, GFXFALSE, GFXFALSE, DEFAULT_ADDRESS_MODE,
                        light_map[0]);
        light_map[2] =
                new Texture((xml->backgroundname + "_up.image").c_str(), 1, TRILINEAR, CUBEMAP, CUBEMAP_POSITIVE_Y,
                        GFXFALSE, game_options()->max_cubemap_size, GFXFALSE, GFXFALSE, DEFAULT_ADDRESS_MODE,
                        light_map[0]);
        light_map[3] =
                new Texture((xml->backgroundname + "_down.image").c_str(), 1, TRILINEAR, CUBEMAP, CUBEMAP_NEGATIVE_Y,
                        GFXFALSE, game_options()->max_cubemap_size, GFXFALSE, GFXFALSE, DEFAULT_ADDRESS_MODE,
                        light_map[0]);
        light_map[4] =
                new Texture((xml->backgroundname + "_front.image").c_str(), 1, TRILINEAR, CUBEMAP, CUBEMAP_POSITIVE_Z,
                        GFXFALSE, game_options()->max_cubemap_size, GFXFALSE, GFXFALSE, DEFAULT_ADDRESS_MODE,
                        light_map[0]);
        light_map[5] =
                new Texture((xml->backgroundname + "_back.image").c_str(), 1, TRILINEAR, CUBEMAP, CUBEMAP_NEGATIVE_Z,
                        GFXFALSE, game_options()->max_cubemap_size, GFXFALSE, GFXFALSE, DEFAULT_ADDRESS_MODE,
                        light_map[0]);
    }
#else
    string  bglight = xml->backgroundname+"_light.image";
    string  bgfile  = xml->backgroundname+"_light.image";
    VSFile  f;
    VSError err     = f.OpenReadOnly( bgfile, VSFileSystem::TextureFile );
    if (err > Ok)
        EnvironmentMapGeneratorMain( xml->backgroundname.c_str(), bglight.c_str(), 0, xml->reflectivity, 1 );
    else
        f.Close();
    light_map[0] = new Texture( bgfile.c_str(), 1, MIPMAP, TEXTURE2D, TEXTURE_2D, GFXTRUE );
#endif

    background = new Background(
            xml->backgroundname.c_str(),
            xml->numstars,
            g_game.zfar * .9,
            filename,
            xml->backgroundColor,
            xml->backgroundDegamma);
    stars = new Stars(xml->numnearstars, xml->starsp);
    stars->SetBlend(game_options()->starblend, game_options()->starblend);
}


/////////////////////////////////////////////////////////////////

using namespace XMLSupport;
vector<Vector> perplines;
extern vector<unorigdest *> pendingjump;

void TentativeJumpTo(StarSystem *ss, Unit *un, Unit *jumppoint, const std::string &system) {
    for (unsigned int i = 0; i < pendingjump.size(); ++i) {
        if (pendingjump[i]->un.GetUnit() == un) {
            return;
        }
    }
    ss->JumpTo(un, jumppoint, system);
}

float ScaleJumpRadius(float radius) {
    //need this because sys scale doesn't affect j-point size
    radius *= game_options()->jump_radius_scale * game_options()->game_speed;
    return radius;
}

/********* FROM STAR SYSTEM XML *********/
void setStaticFlightgroup(vector<Flightgroup *> &fg, const std::string &nam, int faction) {
    while (faction >= (int) fg.size()) {
        fg.push_back(new Flightgroup());
        fg.back()->nr_ships = 0;
    }
    if (fg[faction]->nr_ships == 0) {
        fg[faction]->flightgroup_nr = faction;
        fg[faction]->pos.i = fg[faction]->pos.j = fg[faction]->pos.k = 0;
        fg[faction]->nr_ships = 0;
        fg[faction]->ainame = "default";
        fg[faction]->faction = FactionUtil::GetFaction(faction);
        fg[faction]->type = "Base";
        fg[faction]->nr_waves_left = 0;
        fg[faction]->nr_ships_left = 0;
        fg[faction]->name = nam;
    }
    ++fg[faction]->nr_ships;
    ++fg[faction]->nr_ships_left;
}

Flightgroup *getStaticBaseFlightgroup(int faction) {
    //warning mem leak...not big O(num factions)
    static vector<Flightgroup *> fg;
    setStaticFlightgroup(fg, "Base", faction);
    return fg[faction];
}

Flightgroup *getStaticStarFlightgroup(int faction) {
    //warning mem leak...not big O(num factions)
    static vector<Flightgroup *> fg;
    setStaticFlightgroup(fg, "Base", faction);
    return fg[faction];
}

Flightgroup *getStaticNebulaFlightgroup(int faction) {
    static vector<Flightgroup *> fg;
    setStaticFlightgroup(fg, "Nebula", faction);
    return fg[faction];
}

Flightgroup *getStaticAsteroidFlightgroup(int faction) {
    static vector<Flightgroup *> fg;
    setStaticFlightgroup(fg, "Asteroid", faction);
    return fg[faction];
}

Flightgroup *getStaticUnknownFlightgroup(int faction) {
    static vector<Flightgroup *> fg;
    setStaticFlightgroup(fg, "Unknown", faction);
    return fg[faction];
}

extern string RemoveDotSystem(const char *input);

string StarSystem::getFileName() const {
    return getStarSystemSector(filename) + string("/") + RemoveDotSystem(getStarSystemName(filename).c_str());
}

string StarSystem::getName() {
    return name;
}

void StarSystem::AddUnit(Unit *unit) {
    if (stats.system_faction == FactionUtil::GetNeutralFaction()) {
        stats.CheckVitals(this);
    }
    if (unit->isPlanet() || unit->isJumppoint() || unit->isUnit() == Vega_UnitType::asteroid) {
        if (!gravitationalUnits().contains(unit)) {
            gravitationalUnits().prepend(unit);
        }
    }
    draw_list.prepend(unit);
    unit->activeStarSystem = this;     //otherwise set at next physics frame...
    unsigned int priority = UnitUtil::getPhysicsPriority(unit);
    //Do we need the +1 here or not - need to look at when current_sim_location is changed relative to this function
    //and relative to this function, when the bucket is processed...
    unsigned int tmp = 1 + ((unsigned int) vsrandom.genrand_int32()) % priority;
    this->physics_buffer[(this->current_sim_location + tmp) % SIM_QUEUE_SIZE].prepend(unit);
    stats.AddUnit(unit);
}

bool StarSystem::RemoveUnit(Unit *un) {
    for (unsigned int locind = 0; locind < Unit::NUM_COLLIDE_MAPS; ++locind) {
        if (!is_null(un->location[locind])) {
            collide_map[locind]->erase(un->location[locind]);
            set_null(un->location[locind]);
        }
    }

    if (draw_list.remove(un)) {
        // regardless of being drawn, it should be in physics list
        for (unsigned int i = 0; i <= SIM_QUEUE_SIZE; ++i) {
            if (physics_buffer[i].remove(un)) {
                i = SIM_QUEUE_SIZE + 1;
            }
        }
        stats.RemoveUnit(un);
        return (true);
    }
    return (false);
}

void StarSystem::ExecuteUnitAI() {
    try {
        Unit *unit = nullptr;
        for (un_iter iter = getUnitList().createIterator(); (unit = *iter); ++iter) {
            unit->ExecuteAI();
            unit->ResetThreatLevel();
        }
    }
    catch (const boost::python::error_already_set &) {
        if (PyErr_Occurred()) {
            VS_LOG_AND_FLUSH(fatal, "void StarSystem::ExecuteUnitAI(): Python error occurred");
            PyErr_Print();
            PyErr_Clear();
            VegaStrikeLogging::vega_logger()->FlushLogs();
        }
        throw;
    }
}


/////////////////////////////////////////////////////////////////















extern Unit *TheTopLevelUnit;

//sorry boyz...I'm just a tourist with a frag nav console--could you tell me where I am?
Unit *getTopLevelOwner() {
    return (TheTopLevelUnit);  // Now we return a pointer to a new game unit created in main(), outside of any lists
}

void CarSimUpdate(Unit *un, float height) {
    un->SetVelocity(Vector(un->GetVelocity().i, 0, un->GetVelocity().k));
    un->curr_physical_state.position = QVector(un->curr_physical_state.position.i,
            height,
            un->curr_physical_state.position.k);
}

Statistics::Statistics() {
    system_faction = FactionUtil::GetNeutralFaction();
    newfriendlycount = 0;
    newenemycount = 0;
    newcitizencount = 0;
    newneutralcount = 0;
    friendlycount = 0;
    enemycount = 0;
    neutralcount = 0;
    citizencount = 0;
    checkIter = 0;
    navCheckIter = 0;
}

void Statistics::CheckVitals(StarSystem *ss) {
    int faction = FactionUtil::GetFactionIndex(UniverseUtil::GetGalaxyFaction(ss->getFileName()));
    if (faction != system_faction) {
        *this = Statistics();                    //invoke copy constructor to clear it
        this->system_faction = faction;
        for (un_iter ui = ss->getUnitList().createIterator(); !ui.isDone(); ++ui) {
            this->AddUnit(*ui);                 //siege will take some time
        }
        return;                                          //no need to check vitals now, they're all set
    }
    size_t iter = navCheckIter;
    int k = 0;
    if (iter >= navs[0].size()) {
        iter -= navs[0].size();
        k = 1;
    }
    if (iter >= navs[1].size()) {
        iter -= navs[1].size();
        k = 2;
    }
    size_t totalnavchecking = 25;
    size_t totalsyschecking = 25;
    while (iter < totalnavchecking && iter < navs[k].size()) {
        if (navs[k][iter].GetUnit() == nullptr) {
            navs[k].erase(navs[k].begin() + iter);
            break;
        } else {
            ++iter;
            ++navCheckIter;
        }
    }
    if (k == 2 && iter >= navs[k].size()) {
        navCheckIter = 0;
    }                        //start over next time
    size_t sortedsize = ss->collide_map[Unit::UNIT_ONLY]->sorted.size();
    int sysfac = system_faction;
    size_t counter = checkIter + totalsyschecking;
    for (; checkIter < counter && checkIter < sortedsize; ++checkIter) {
        Collidable *collide = &ss->collide_map[Unit::UNIT_ONLY]->sorted[checkIter];
        if (collide->radius > 0) {
            Unit *un = collide->ref.unit;
            float rel = UnitUtil::getRelationFromFaction(un, sysfac);
            if (FactionUtil::isCitizenInt(un->faction)) {
                ++newcitizencount;
            } else {
                if (rel > 0.05) {
                    ++newfriendlycount;
                } else if (rel < 0.) {
                    ++newenemycount;
                } else {
                    ++newneutralcount;
                }
            }
        }
    }
    if (checkIter >= sortedsize && sortedsize
            > (unsigned int) (enemycount + neutralcount + friendlycount
                    + citizencount) / 4 /*suppose at least 1/4 survive a given frame*/) {
        citizencount = newcitizencount;
        newcitizencount = 0;
        enemycount = newenemycount;
        newenemycount = 0;
        neutralcount = newneutralcount;
        newneutralcount = 0;
        friendlycount = newfriendlycount;
        newfriendlycount = 0;
        checkIter = 0;                           //start over with list
    }
}

void Statistics::AddUnit(Unit *un) {
    float rel = UnitUtil::getRelationFromFaction(un, system_faction);
    if (FactionUtil::isCitizenInt(un->faction)) {
        ++citizencount;
    } else {
        if (rel > 0.05) {
            ++friendlycount;
        } else if (rel < 0.) {
            ++enemycount;
        } else {
            ++neutralcount;
        }
    }
    if (un->GetDestinations().size()) {
        jumpPoints[un->GetDestinations()[0]].SetUnit(un);
    }
    if (UnitUtil::isSignificant(un)) {
        int k = 0;
        if (rel > 0) {
            k = 1;
        }                      //base
        if (un->isPlanet() && !un->isJumppoint()) {
            k = 1;
        }                                       //friendly planet
        //asteroid field/debris field
        if (UnitUtil::isAsteroid(un)) {
            k = 2;
        }
        navs[k].push_back(UnitContainer(un));
    }
}

void Statistics::RemoveUnit(Unit *un) {
    float rel = UnitUtil::getRelationFromFaction(un, system_faction);
    if (FactionUtil::isCitizenInt(un->faction)) {
        --citizencount;
    } else {
        if (rel > 0.05) {
            --friendlycount;
        } else if (rel < 0.) {
            --enemycount;
        } else {
            --neutralcount;
        }
    }
    if (un->GetDestinations().size()) {
        //make sure it is there
        jumpPoints[(un->GetDestinations()[0])].SetUnit(nullptr);
        //kill it--stupid I know--but hardly time critical
        jumpPoints.erase(jumpPoints.find(un->GetDestinations()[0]));
    }
    bool temp_erased = false;
    if (UnitUtil::isSignificant(un)) {
        for (int k = 0; k < 3 && !temp_erased; ++k) {
            for (size_t i = 0; i < navs[k].size(); ++i) {
                if (navs[k][i].GetUnit() == un) {
                    //slow but who cares
                    navs[k].erase(navs[k].begin() + i);
                    temp_erased = true;
                    break; // would we exist in this array more than once?
                }
            }
        }
    }
}

//Variables for debugging purposes only - eliminate later
unsigned int physicsframecounter = 1;
unsigned int theunitcounter = 0;
unsigned int totalprocessed = 0;
unsigned int movingavgarray[128] = {0};
unsigned int movingtotal = 0;
double aggfire = 0;
int numprocessed = 0;
double targetpick = 0;

void StarSystem::RequestPhysics(Unit *un, unsigned int queue) {
    Unit *unit = nullptr;
    un_iter iter = this->physics_buffer[queue].createIterator();
    while ((unit = *iter) && *iter != un) {
        ++iter;
    }
    if (unit == un) {
        un->predicted_priority = 0;
        unsigned int newloc = (current_sim_location + 1) % SIM_QUEUE_SIZE;
        if (newloc != queue) {
            iter.moveBefore(this->physics_buffer[newloc]);
        }
    }
}

//BELOW COMMENTS ARE NO LONGER IN SYNCH
//NOTE: Randomization is necessary to preserve scattering - otherwise, whenever a
//unit goes from low-priority to high-priority and back to low-priority, they
//get synchronized and start producing peaks.
//NOTE2: But... randomization must come only on priority changes. Otherwise, it may
//interfere with subunit scheduling. Luckily, all units that make use of subunit
//scheduling also require a constant base priority, since otherwise priority changes
//will wreak havoc with subunit interpolation. Luckily again, we only need
//randomization on priority changes, so we're fine.
void StarSystem::UpdateUnitsPhysics(bool firstframe) {
    static int batchcount = SIM_QUEUE_SIZE - 1;
    double collidetime = 0.0;
    double bolttime = 0.0;
    targetpick = 0.0;
    aggfire = 0.0;
    numprocessed = 0;
    stats.CheckVitals(this);

    for (++batchcount; batchcount > 0; --batchcount) {
        try {
            UnitCollection col = physics_buffer[current_sim_location];
            un_iter iter = physics_buffer[current_sim_location].createIterator();
            Unit *unit = nullptr;
            for (; (unit = *iter); ++iter) {
                UpdateUnitPhysics(firstframe, unit);
            }
        } catch (const boost::python::error_already_set &) {
            if (PyErr_Occurred()) {
                VS_LOG_AND_FLUSH(fatal,
                        "void StarSystem::UpdateUnitPhysics( bool firstframe ): Msg D: Python error occurred");
                PyErr_Print();
                PyErr_Clear();
                VegaStrikeLogging::vega_logger()->FlushLogs();
            }
            throw;
        }
        double c0 = queryTime();
        Bolt::UpdatePhysics(this);
        double cc = queryTime();
        last_collisions.clear();
        collide_map[Unit::UNIT_BOLT]->flatten();
        if (Unit::NUM_COLLIDE_MAPS > 1) {
            collide_map[Unit::UNIT_ONLY]->flatten(*collide_map[Unit::UNIT_BOLT]);
        }
        Unit *unit;
        for (un_iter iter = physics_buffer[current_sim_location].createIterator(); (unit = *iter);) {
            unsigned int priority = unit->sim_atom_multiplier;
            float backup = simulation_atom_var;
            //VS_LOG(trace, (boost::format("void StarSystem::UpdateUnitPhysics( bool firstframe ): Msg E: simulation_atom_var as backed up:  %1%") % simulation_atom_var));
            simulation_atom_var *= priority;
            //VS_LOG(trace, (boost::format("void StarSystem::UpdateUnitPhysics( bool firstframe ): Msg F: simulation_atom_var as multiplied: %1%") % simulation_atom_var));
            unsigned int newloc = (current_sim_location + priority) % SIM_QUEUE_SIZE;
            unit->CollideAll();
            simulation_atom_var = backup;
            //VS_LOG(trace, (boost::format("void StarSystem::UpdateUnitPhysics( bool firstframe ): Msg G: simulation_atom_var as restored:   %1%") % simulation_atom_var));
            if (newloc == current_sim_location) {
                ++iter;
            } else {
                iter.moveBefore(physics_buffer[newloc]);
            }
        }
        double dd = queryTime();
        collidetime += dd - cc;
        bolttime += cc - c0;
        current_sim_location = (current_sim_location + 1) % SIM_QUEUE_SIZE;
        ++physicsframecounter;
        totalprocessed += theunitcounter;
        theunitcounter = 0;
    }
}

void StarSystem::UpdateUnitPhysics(bool firstframe, Unit *unit) {
    int priority = UnitUtil::getPhysicsPriority(unit);
    //Doing spreading here and only on priority changes, so as to make AI easier
    int predprior = unit->predicted_priority;
    //If the priority has really changed (not an initial scattering, because prediction doesn't match)
    if (priority != predprior) {
        if (predprior == 0) {
            //Validate snapshot of current interpolated state (this is a reschedule)
            unit->curr_physical_state = unit->cumulative_transformation;
        }
        //Save priority value as prediction for next scheduling, but don't overwrite yet.
        predprior = priority;
        //Scatter, so as to achieve uniform distribution
#if defined(RANDOMIZE_SIM_ATOMS)
        VS_LOG(trace, "void StarSystem::UpdateUnitPhysics( bool firstframe ): Randomizing unit priority");
        priority  = 1+( ( (unsigned int) vsrandom.genrand_int32() )%priority );
#endif
    }
    float backup = simulation_atom_var;
    //VS_LOG(trace, (boost::format("void StarSystem::UpdateUnitPhysics( bool firstframe ): Msg A: simulation_atom_var as backed up:  %1%") % simulation_atom_var));
    try {
        theunitcounter = theunitcounter + 1;
        simulation_atom_var *= priority;
        //VS_LOG(trace, (boost::format("void StarSystem::UpdateUnitPhysics( bool firstframe ): Msg B: simulation_atom_var as multiplied: %1%") % simulation_atom_var));
        unit->sim_atom_multiplier = priority;
        unit->ExecuteAI();
        unit->ResetThreatLevel();
        //FIXME "firstframe"-- assume no more than 2 physics updates per frame.
        unit->UpdatePhysics(identity_transformation,
                identity_matrix,
                Vector(0,
                        0,
                        0),
                priority
                        == 1 ? firstframe : true,
                &this->gravitationalUnits(),
                unit);
        simulation_atom_var = backup;
    } catch (...) {
        simulation_atom_var = backup;
        throw;
    }
    //VS_LOG(trace, (boost::format("void StarSystem::UpdateUnitPhysics( bool firstframe ): Msg C: simulation_atom_var as restored:   %1%") % simulation_atom_var));
    unit->predicted_priority = predprior;
}

extern void TerrainCollide();
extern void UpdateAnimatedTexture();
extern void UpdateCameraSnds();

extern float getTimeCompression();

//server
void ExecuteDirector() {
    unsigned int curcockpit = _Universe->CurrentCockpit();
    {
        for (unsigned int i = 0; i < active_missions.size(); ++i) {
            if (active_missions[i]) {
                _Universe->SetActiveCockpit(active_missions[i]->player_num);
                StarSystem *ss = _Universe->AccessCockpit()->activeStarSystem;
                if (ss) {
                    _Universe->pushActiveStarSystem(ss);
                }
                mission = active_missions[i];
                active_missions[i]->DirectorLoop();
                if (ss) {
                    _Universe->popActiveStarSystem();
                }
            }
        }
    }
    _Universe->SetActiveCockpit(curcockpit);
    mission = active_missions[0];
    processDelayedMissions();

    {
        for (unsigned int i = 1; i < active_missions.size();) {
            if (active_missions[i]) {
                if (active_missions[i]->runtime.pymissions) {
                    ++i;
                } else {
                    unsigned int w = active_missions.size();
                    active_missions[i]->terminateMission();
                    if (w == active_missions.size()) {
                        VS_LOG(warning, "MISSION NOT ERASED");
                        break;
                    }
                }
            } else {
                active_missions.Get()->erase(active_missions.Get()->begin() + i);
            }
        }
    }
}

Unit *StarSystem::nextSignificantUnit() {
    return (*sigIter);
}

void StarSystem::Update(float priority) {
    Unit *unit;
    bool firstframe = true;
    //No time compression here
    float normal_simulation_atom = SIMULATION_ATOM;
    time += GetElapsedTime();
    _Universe->pushActiveStarSystem(this);
    if (time > SIMULATION_ATOM * 2) {
        VS_LOG(trace,
                (boost::format(
                        "%1% %2%: time, %3$.6f, is more than twice simulation_atom_var, %4$.6f")
                        % __FILE__ % __LINE__ % time % simulation_atom_var));
        //Chew up all sim_atoms that have elapsed since last update
        while (time > SIMULATION_ATOM) {
            VS_LOG(trace, (boost::format("%1% %2%: Chewing up a sim atom") % __FILE__ % __LINE__));
            ExecuteDirector();
            TerrainCollide();
            Unit::ProcessDeleteQueue();
            current_stage = MISSION_SIMULATION;
            collide_table->Update();
            for (un_iter iter = draw_list.createIterator(); (unit = *iter); ++iter) {
                unit->SetNebula(nullptr);
            }
            UpdateMissiles();                    //do explosions
            UpdateUnitsPhysics(firstframe);

            firstframe = false;
            time -= SIMULATION_ATOM;
        }
    }
    assert(SIMULATION_ATOM == normal_simulation_atom);
    _Universe->popActiveStarSystem();
}

//client
void StarSystem::Update(float priority, bool executeDirector) {
    bool firstframe = true;
    ///this makes it so systems without players may be simulated less accurately
    for (unsigned int k = 0; k < _Universe->numPlayers(); ++k) {
        if (_Universe->AccessCockpit(k)->activeStarSystem == this) {
            priority = 1;
        }
    }
    float normal_simulation_atom = simulation_atom_var;
    //VS_LOG(trace, (boost::format("void StarSystem::Update( float priority, bool executeDirector ): Msg A: simulation_atom_var as backed up  = %1%") % simulation_atom_var));
    simulation_atom_var /= (priority / getTimeCompression());
    //VS_LOG(trace, (boost::format("void StarSystem::Update( float priority, bool executeDirector ): Msg B: simulation_atom_var as multiplied = %1%") % simulation_atom_var));
    ///just be sure to restore this at the end
    time += GetElapsedTime();
    _Universe->pushActiveStarSystem(this);
//    double bolttime = 0;
    if (time > simulation_atom_var) {
        if (time > simulation_atom_var * 2) {
            VS_LOG(trace,
                    (boost::format(
                            "%1% %2%: time, %3$.6f, is more than twice simulation_atom_var, %4$.6f")
                            % __FILE__ % __LINE__ % time % simulation_atom_var));
        }

//        double missionSimulationTimeSubtotal = 0.0;
//        double processUnitTimeSubtotal = 0.0;
//
//        double updateUnitsPhysicsTimeSubtotal = 0.0;
//        double updateMissilesTimeSubtotal = 0.0;
//        double collideTableUpdateTimeSubtotal = 0.0;
//        double updateCameraSoundsTimeSubtotal = 0.0;

        //Chew up all sim_atoms that have elapsed since last update
        // ** stephengtuggy 2020-07-23: We definitely need this block of code! **
        while (time > simulation_atom_var) {
            //VS_LOG(trace, "void StarSystem::Update( float priority, bool executeDirector ): Chewing up a sim atom");
            if (current_stage == MISSION_SIMULATION) {
//                double missionSimulationStageStartTime = realTime();
                TerrainCollide();
                UpdateAnimatedTexture();
                Unit::ProcessDeleteQueue();
                if ((run_only_player_starsystem
                        && _Universe->getActiveStarSystem(0) == this) || !run_only_player_starsystem) {
                    if (executeDirector) {
                        ExecuteDirector();
                    }
                }
                static int dothis = 0;
                if (this == _Universe->getActiveStarSystem(0)) {
                    if ((++dothis) % 2 == 0) {
                        AUDRefreshSounds();
                    }
                }
                for (unsigned int i = 0; i < active_missions.size(); ++i) {
                    //waste of frakkin time
                    active_missions[i]->BriefingUpdate();
                }
//                double missionSimulationStageEndTime = realTime();
//                missionSimulationTimeSubtotal += (missionSimulationStageEndTime - missionSimulationStageStartTime);
//                VS_LOG(trace, (boost::format("void StarSystem::Update( float priority, bool executeDirector ): Time taken by MISSION_SIMULATION stage: %1%") % (missionSimulationStageEndTime - missionSimulationStageStartTime)));
                current_stage = PROCESS_UNIT;
            } else if (current_stage == PROCESS_UNIT) {
//                double processUnitStageStartTime = realTime();
                UpdateUnitsPhysics(firstframe);
//                double updateUnitsPhysicsDoneTime = realTime();
//                updateUnitsPhysicsTimeSubtotal += (updateUnitsPhysicsDoneTime - processUnitStageStartTime);
                UpdateMissiles(); //do explosions
//                double updateMissilesDoneTime = realTime();
//                updateMissilesTimeSubtotal += (updateMissilesDoneTime - updateUnitsPhysicsDoneTime);
                collide_table->Update();
//                double collideTableUpdateDoneTime = realTime();
//                collideTableUpdateTimeSubtotal += (collideTableUpdateDoneTime - updateMissilesDoneTime);
                if (this == _Universe->getActiveStarSystem(0)) {
                    UpdateCameraSnds();
                }
//                bolttime = queryTime();
//                bolttime = queryTime() - bolttime;
//                double processUnitStageEndTime = realTime();
//                processUnitTimeSubtotal += (processUnitStageEndTime - processUnitStageStartTime);
//                updateCameraSoundsTimeSubtotal += (processUnitStageEndTime - collideTableUpdateDoneTime);
//                VS_LOG(trace, (boost::format("void StarSystem::Update( float priority, bool executeDirector ): Time taken by PROCESS_UNIT stage: %1%") % (processUnitStageEndTime - processUnitStageStartTime)));
                current_stage = MISSION_SIMULATION;
                firstframe = false;
            }
            time -= simulation_atom_var;
        }

//        VS_LOG(trace, (boost::format("%1% %2%: Subtotal of time taken by MISSION_SIMULATION: %3%") % __FILE__ % __LINE__ % missionSimulationTimeSubtotal));
//        VS_LOG(trace, (boost::format("%1% %2%: Subtotal of time taken by PROCESS_UNIT: %3%") % __FILE__ % __LINE__ % processUnitTimeSubtotal));
//
//        VS_LOG(trace, (boost::format("%1% %2%: Subtotal of time taken by updating units' physics: %3%") % __FILE__ % __LINE__ % updateUnitsPhysicsTimeSubtotal));
//        VS_LOG(trace, (boost::format("%1% %2%: Subtotal of time taken by updating missiles: %3%") % __FILE__ % __LINE__ % updateMissilesTimeSubtotal));
//        VS_LOG(trace, (boost::format("%1% %2%: Subtotal of time taken by updating collide tables: %3%") % __FILE__ % __LINE__ % collideTableUpdateTimeSubtotal));
//        VS_LOG(trace, (boost::format("%1% %2%: Subtotal of time taken by updating camera sounds: %3%") % __FILE__ % __LINE__ % updateCameraSoundsTimeSubtotal));

//        double cycleThroughPlayersStartTime = realTime();
        unsigned int i = _Universe->CurrentCockpit();
        for (unsigned int j = 0; j < _Universe->numPlayers(); ++j) {
            if (_Universe->AccessCockpit(j)->activeStarSystem == this) {
                _Universe->SetActiveCockpit(j);
                _Universe->AccessCockpit(j)->updateAttackers();
                if (_Universe->AccessCockpit(j)->Update()) {
                    simulation_atom_var = normal_simulation_atom;
                    //VS_LOG(trace, (boost::format("void StarSystem::Update( float priority, bool executeDirector ): Msg C: simulation_atom_var as restored   = %1%") % simulation_atom_var));
                    _Universe->SetActiveCockpit(i);
                    _Universe->popActiveStarSystem();
                    return;
                }
            }
        }
        _Universe->SetActiveCockpit(i);
//        double cycleThroughPlayersEndTime = realTime();
//        VS_LOG(trace, (boost::format("%1% %2%: Time taken by cycling through active players / cockpits: %3%") % __FILE__ % __LINE__ % (cycleThroughPlayersEndTime - cycleThroughPlayersStartTime)));
    }
    if (sigIter.isDone()) {
        sigIter = draw_list.createIterator();
    } else {
        ++sigIter;
    }
    while (!sigIter.isDone() && !UnitUtil::isSignificant(*sigIter)) {
        ++sigIter;
    }
    //If it is done, leave it nullptr for this frame then.
    //WARNING cockpit does not get here...
    simulation_atom_var = normal_simulation_atom;
    //VS_LOG(trace, (boost::format("void StarSystem::Update( float priority, bool executeDirector ): Msg D: simulation_atom_var as restored   = %1%") % simulation_atom_var));
    //WARNING cockpit does not get here...
    _Universe->popActiveStarSystem();
}

/*
 **************************************************************************************
 *** STAR SYSTEM JUMP STUFF                                                          **
 **************************************************************************************
 */

Hashtable<std::string, StarSystem, 127> star_system_table;

void StarSystem::AddStarsystemToUniverse(const string &mname) {
    star_system_table.Put(mname, this);
}

void StarSystem::RemoveStarsystemFromUniverse() {
    if (star_system_table.Get(filename)) {
        star_system_table.Delete(filename);
    }
}

StarSystem *GetLoadedStarSystem(const char *system) {
    StarSystem *ss = star_system_table.Get(string(system));
    std::string ssys(string(system) + string(".system"));
    if (!ss) {
        ss = star_system_table.Get(ssys);
    }
    return ss;
}

std::vector<unorigdest *> pendingjump;

bool PendingJumpsEmpty() {
    return pendingjump.empty();
}

void StarSystem::ProcessPendingJumps() {
    for (unsigned int kk = 0; kk < pendingjump.size(); ++kk) {
        Unit *un = pendingjump[kk]->un.GetUnit();
        if (pendingjump[kk]->delay >= 0) {
            Unit *jp = pendingjump[kk]->jumppoint.GetUnit();
            if (un && jp) {
                QVector delta = (jp->LocalPosition() - un->LocalPosition());
                float dist = delta.Magnitude();
                if (pendingjump[kk]->delay > 0) {
                    float speed = dist / pendingjump[kk]->delay;
                    bool player = (_Universe->isPlayerStarship(un) != nullptr);
                    if (dist > 10 && player) {
                        if (un->activeStarSystem == pendingjump[kk]->orig) {
                            un->SetCurPosition(un->LocalPosition() + simulation_atom_var * delta * (speed / dist));
                        }
                    } else if (!player) {
                        un->SetVelocity(Vector(0, 0, 0));
                    }
                    if (game_options()->jump_disables_shields) {
                        // Disable and then enable so they'll start recharging
                        un->shield->Disable();
                        un->shield->Enable();
                    }
                }
            }
            double time = GetElapsedTime();
            if (time > 1) {
                time = 1;
            }
            pendingjump[kk]->delay -= time;
            continue;
        } else {
#ifdef JUMP_DEBUG
            VS_LOG(trace, "Volitalizing pending jump animation.");
#endif
            _Universe->activeStarSystem()->VolitalizeJumpAnimation(pendingjump[kk]->animation);
        }
        // int playernum = _Universe->whichPlayerStarship( un );
        //In non-networking mode or in networking mode or a netplayer wants to jump and is ready or a non-player jump
        StarSystem *savedStarSystem = _Universe->activeStarSystem();
        if (un == nullptr || !_Universe->StillExists(pendingjump[kk]->dest)
                || !_Universe->StillExists(pendingjump[kk]->orig)) {
#ifdef JUMP_DEBUG
            VS_LOG(debug, "Adez Mon! Unit destroyed during jump!");
#endif
            delete pendingjump[kk];
            pendingjump.erase(pendingjump.begin() + kk);
            --kk;
            continue;
        }
        bool dosightandsound = ((pendingjump[kk]->dest == savedStarSystem) || _Universe->isPlayerStarship(un));
        _Universe->setActiveStarSystem(pendingjump[kk]->orig);
        if (un->TransferUnitToSystem(kk, savedStarSystem, dosightandsound)) {
            un->decreaseWarpEnergy(false, 1.0f);
        }
        if (dosightandsound) {
            _Universe->activeStarSystem()->DoJumpingComeSightAndSound(un);
        }
        _Universe->AccessCockpit()->OnJumpEnd(un);
        delete pendingjump[kk];
        pendingjump.erase(pendingjump.begin() + kk);
        --kk;
        _Universe->setActiveStarSystem(savedStarSystem);

    }
}

double calc_blend_factor(double frac,
        unsigned int priority,
        unsigned int when_it_will_be_simulated,
        unsigned int cur_simulation_frame) {
    if (when_it_will_be_simulated == SIM_QUEUE_SIZE) {
        return 1;
    }

    int signed_priority = static_cast<int>(priority);
    int signed_when_simulated = static_cast<int>(when_it_will_be_simulated);
    int signed_simulation_frame = static_cast<int>(cur_simulation_frame);

    int relwas = signed_when_simulated - signed_priority;
    if (relwas < 0) {
        relwas += SIM_QUEUE_SIZE;
    }

    int relcur = signed_simulation_frame - relwas - 1;
    if (relcur < 0) {
        relcur += SIM_QUEUE_SIZE;
    }

    return (relcur + frac) / static_cast<double>(priority);

}

void ActivateAnimation(Unit *jumppoint) {
    jumppoint->graphicOptions.Animating = 1;
    for (un_iter i = jumppoint->getSubUnits(); !i.isDone(); ++i) {
        ActivateAnimation(*i);
    }
}

static bool isJumping(const vector<unorigdest *> &pending, Unit *un) {
    for (size_t i = 0; i < pending.size(); ++i) {
        if (pending[i]->un == un) {
            return true;
        }
    }
    return false;
}

QVector SystemLocation(std::string system);
double howFarToJump();

QVector ComputeJumpPointArrival(QVector pos, std::string origin, std::string destination) {
    QVector finish = SystemLocation(destination);
    QVector start = SystemLocation(origin);
    QVector dir = finish - start;
    if (dir.MagnitudeSquared()) {
        dir.Normalize();
        dir = -dir;
        pos = -pos;
        pos.Normalize();
        if (pos.MagnitudeSquared()) {
            pos.Normalize();
        }
        return (dir * .5 + pos * .125) * howFarToJump();
    }
    return QVector(0, 0, 0);
}

bool StarSystem::JumpTo(Unit *un, Unit *jumppoint, const std::string &system, bool force, bool save_coordinates) {
    if ((un->DockedOrDocking() & (~Unit::DOCKING_UNITS)) != 0) {
        return false;
    }

    if (un->jump.drive >= 0) {
        un->jump.drive = -1;
    }
#ifdef JUMP_DEBUG
    VS_LOG(trace, (boost::format("jumping to %1%.  ") % system));
#endif
    StarSystem *ss = star_system_table.Get(system);
    std::string ssys(system + ".system");
    if (!ss) {
        ss = star_system_table.Get(ssys);
    }
    bool justloaded = false;
    if (!ss) {
        justloaded = true;
        ss = _Universe->GenerateStarSystem(ssys.c_str(), filename.c_str(), Vector(0, 0, 0));
    }
    if (ss && !isJumping(pendingjump, un)) {
#ifdef JUMP_DEBUG
        VS_LOG(debug, "Pushing back to pending queue!");
#endif
        bool dosightandsound = ((this == _Universe->getActiveStarSystem(0)) || _Universe->isPlayerStarship(un));
        int ani = -1;
        if (dosightandsound) {
            ani = _Universe->activeStarSystem()->DoJumpingLeaveSightAndSound(un);
        }
        _Universe->AccessCockpit()->OnJumpBegin(un);
        pendingjump.push_back(new unorigdest(un, jumppoint, this, ss, un->GetJumpStatus().delay, ani, justloaded,
                save_coordinates ? ComputeJumpPointArrival(un->Position(),
                        this->getFileName(),
                        system) : QVector(0, 0, 0)));
    } else {
#ifdef JUMP_DEBUG
        VS_LOG(debug, "Failed to retrieve!");
#endif
        return false;
    }
    if (jumppoint) {
        ActivateAnimation(jumppoint);
    }

    return true;
}

Background *StarSystem::getBackground() {
    return background;
}

Terrain *StarSystem::getTerrain(unsigned int which) {
    return terrains[which];
}

unsigned int StarSystem::numTerrain() {
    return terrains.size();
}

ContinuousTerrain *StarSystem::getContTerrain(unsigned int which) {
    return continuous_terrains[which];
}

unsigned int StarSystem::numContTerrain() {
    return continuous_terrains.size();
}

void StarSystem::UpdateMissiles() {
    //if false, missiles collide with rocks as units, but not harm them with explosions
    //FIXME that's how it's used now, but not really correct, as there could be separate AsteroidWeaponDamage for this
    static bool collideroids =
            XMLSupport::parse_bool(vs_config->getVariable("physics", "AsteroidWeaponCollision", "false"));

    //WARNING: This is a big performance problem...
    //...responsible for many hiccups.
    //TODO: Make it use the collide_map to only iterate through potential hits...
    //PROBLEM: The current collide_map does not allow this efficiently (no way of
    //taking the other unit's rSize() into account).
    if (!discharged_missiles.empty()) {
        if (discharged_missiles.back()->GetRadius()
                > 0) {           //we can avoid this iterated check for kinetic projectiles even if they "discharge" on hit
            Unit *un;
            for (un_iter ui = getUnitList().createIterator();
                    NULL != (un = (*ui));
                    ++ui) {
                enum Vega_UnitType type = un->isUnit();
                if (collideroids || type
                        != Vega_UnitType::asteroid) {           // could check for more, unless someone wants planet-killer missiles, but what it would change?
                    discharged_missiles.back()->ApplyDamage(un);
                }
            }
        }
        delete discharged_missiles.back();
        discharged_missiles.pop_back();
    }
}

void StarSystem::AddMissileToQueue(MissileEffect *me) {
    discharged_missiles.push_back(me);
}
