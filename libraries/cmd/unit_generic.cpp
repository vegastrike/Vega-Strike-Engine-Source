/*
 * unit_generic.cpp
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

// -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil -*-

#define PY_SSIZE_T_CLEAN
#include <boost/python.hpp>
#include "cmd/unit_generic.h"

#include <set>
#include "root_generic/configxml.h"
#include "src/audiolib.h"
#include "cmd/base.h"
#include "cmd/music.h"

#include "cmd/beam.h"
#include "root_generic/lin_time.h"
#include "root_generic/xml_serializer.h"
#include "root_generic/vsfilesystem.h"
#include "src/vs_logging.h"
#include "src/file_main.h"
#include "src/universe_util.h"
#include "cmd/unit_util.h"
#include "cmd/script/mission.h"
#include "cmd/script/flightgroup.h"
#include "cmd/ai/fire.h"
#include "cmd/ai/turretai.h"
#include "cmd/ai/communication.h"
#include "cmd/ai/navigation.h"
#include "cmd/ai/script.h"
#include "cmd/ai/missionscript.h"
#include "cmd/ai/flybywire.h"
#include "cmd/ai/aggressive.h"
#include "src/python/python_class.h"
#include "cmd/missile.h"
#include "gfx_generic/cockpit_generic.h"
#include "gfx/vsbox.h"
#include <algorithm>
#include "cmd/ai/ikarus.h"
#include "cmd/role_bitmask.h"
#include "cmd/unit_const_cache.h"
#include "gfx/warptrail.h"
#include "gfx_generic/cockpit_generic.h"
#include "cmd/csv.h"
#include "src/vs_random.h"
#include "root_generic/galaxy_xml.h"
#include "gfx/camera.h"
#include "root_generic/options.h"
#include "src/star_system.h"
#include "src/universe.h"
#include "cmd/weapon_info.h"
#include "cmd/mount_size.h"
#include "cmd/turret.h"
#include "cmd/energetic.h"
#include "configuration/game_config.h"
#include "resource/resource.h"
#include "cmd/base_util.h"
#include "cmd/unit_csv_factory.h"
#include "cmd/unit_json_factory.h"
#include "root_generic/savegame.h"
#include "resource/manifest.h"
#include "cmd/dock_utils.h"
#include "vega_cast_utils.h"
#include "resource/random_utils.h"

#include <math.h>
#include <cmath>
#include <list>
#include <cstdint>
#include <boost/format.hpp>
#include <random>

#ifdef _WIN32
#define strcasecmp stricmp
#endif

#include "cmd/unit_find.h"
#include "cmd/pilot.h"

#include <iostream>
#define DEBUG_MESH_ANI

using std::endl;
using std::list;



// This is a left over kludge because I don't want to mess with python interfaces yet.
// TODO: remove
Unit *getMasterPartList() {
    static Unit ret;

    if(ret.cargo_hold.Empty()) {
        ret.name = "master_part_list";
        for(const Cargo& c : Manifest::MPL().GetItems()) {
            ret.cargo_hold.AddCargo(&ret, c);
        }
    }
    return &ret;
}

using namespace XMLSupport;

extern void DestroyMount(Mount *);

void Unit::setFaceCamera() {
    graphicOptions.FaceCamera = 1;
}

void Unit::SetNebula(Nebula *neb) {
    nebula = neb;
    if (!SubUnits.empty()) {
        un_fiter iter = SubUnits.fastIterator();
        Unit *un;
        while ((un = *iter)) {
            un->SetNebula(neb);
            ++iter;
        }
    }
}

// Move to radar
bool Unit::InRange(const Unit *target, double &mm, bool cone, bool cap, bool lock) const {
    const float capship_size = configuration().physics.capship_size_flt;

    if (this == target || target->cloak.Cloaked()) {
        return false;
    }
    if (cone && radar.max_cone > -.98) {
        QVector delta(target->Position() - Position());
        mm = delta.Magnitude();
        if ((!lock) || (!TargetLocked(target))) {
            double tempmm = mm - target->rSize();
            if (tempmm > 0.0001) {
                if ((ToLocalCoordinates(Vector(delta.i, delta.j, delta.k)).k / tempmm) < radar.max_cone
                        && cone) {
                    return false;
                }
            }
        }
    } else {
        mm = (target->Position() - Position()).Magnitude();
    }
    //owner==target?!
    if (((mm - rSize() - target->rSize()) > radar.max_range.Value())
            || target->rSize() < radar.GetMinTargetSize()) {
        Flightgroup *fg = target->getFlightgroup();
        if ((target->rSize() < capship_size || (!cap)) && (fg == nullptr ? true : fg->name != "Base")) {
            return target->getUnitType() == Vega_UnitType::planet;
        }
    }
    return true;
}



// two convenience functions
Unit* CheckNullAndCastToUnit(ComponentsManager* manager) {
    if (manager == nullptr) {
        return nullptr;
    }
    // VS_LOG_AND_FLUSH(trace, (boost::format("CheckNullAndCastToUnit: Runtime type is %1%") % typeid(*manager).name()));
    return vega_dynamic_cast_ptr<Unit>(manager);
}

const Unit* CheckNullAndConstCastToUnit(const ComponentsManager* manager)  {
    if (manager == nullptr) {
        return nullptr;
    }
    return vega_dynamic_const_cast_ptr<const Unit>(manager);
}

Vector Unit::GetNavPoint() {
    return Vector(computer.nav_point.i,
                  computer.nav_point.j,
                  computer.nav_point.k);
}

Unit *Unit::Target() {
    return CheckNullAndCastToUnit(computer.target);
}

const Unit *Unit::Target() const {
    return CheckNullAndConstCastToUnit(computer.target);
}

void Unit::SetTarget(Unit *target) {
    computer.target = target;
}


 Unit *Unit::Threat() {
    return CheckNullAndCastToUnit(computer.threat);
 }

const Unit *Unit::Threat() const {
    return CheckNullAndConstCastToUnit(computer.threat);
}

void Unit::SetThreat(Unit *target) {
    computer.threat = target;
}

Unit *Unit::VelocityReference() {
    return CheckNullAndCastToUnit(computer.velocity_reference);
}

const Unit *Unit::VelocityReference() const {
    return CheckNullAndConstCastToUnit(computer.velocity_reference);
}

void Unit::VelocityReference(Unit *target) {
    computer.force_velocity_ref = !!target;
    computer.velocity_reference = target;
}

void Unit::RestoreGodliness() {
    _Universe->AccessCockpit()->RestoreGodliness();
}

void Unit::Ref() {
#ifdef CONTAINER_DEBUG
    CheckUnit( this );
#endif
    ++ucref;
}

#define kInverseForceDistance 5400
extern void PlayDockingSound(int dock);

static list<Unit *> unit_delete_queue;
static Hashtable<uintmax_t, Unit, 2095> deletedUn;
int deathofvs = 1;

void CheckUnit(Unit *un) {
    if (deletedUn.Get(reinterpret_cast<uintmax_t>(un)) != nullptr) {
        while (deathofvs) {
            VS_LOG(info, (boost::format("%1% died") % reinterpret_cast<uintmax_t>(un)));
        }
    }
}

void UncheckUnit(Unit *un) {
    if (deletedUn.Get(reinterpret_cast<uintmax_t>(un)) != nullptr) {
        deletedUn.Delete(reinterpret_cast<uintmax_t>(un));
    }
}

string GetUnitDir(string filename) {
    return filename.substr(0, filename.find('.'));
}

char *GetUnitDir(const char *filename) {
    char *retval = strdup(filename);
    if (retval[0] == '\0') {
        return retval;
    }
    if (retval[1] == '\0') {
        return retval;
    }
    for (int i = 0; retval[i] != 0; ++i) {
        if (retval[i] == '.') {
            retval[i] = '\0';
            break;
        }
    }
    return retval;
}

/*
 **********************************************************************************
 **** UNIT STUFF
 **********************************************************************************
 */
// Called by Planet
Unit::Unit(int dummy) : Drawable(), Damageable(), Movable() {
    pImage = (new UnitImages<void>);
    pilot = new Pilot(FactionUtil::GetNeutralFaction());
    // TODO: delete
    Init();
}


Unit::Unit() : Drawable(), Damageable(), Movable() //: cumulative_transformation_matrix( identity_matrix )
{
    pImage = (new UnitImages<void>);
    pilot = new Pilot(FactionUtil::GetNeutralFaction());
    // TODO:
    Init();
}

// Called by Missile
Unit::Unit(std::vector<Mesh *> &meshes, bool SubU, int fact)
        : Drawable(), Damageable(), Movable() //: cumulative_transformation_matrix( identity_matrix )
{
    pImage = (new UnitImages<void>);
    pilot = new Pilot(fact);
    // TODO:
    Init();

    this->faction = fact;
    graphicOptions.SubUnit = SubU;
    meshdata = meshes;
    meshes.clear();
    meshdata.push_back(nullptr);
    calculate_extent(false);
    pilot->SetComm(this);
}

extern void update_ani_cache();

// Called by Carrier and Mount
Unit::Unit(const char *filename,
        bool SubU,
        int faction,
        std::string unitModifications,
        Flightgroup *flightgrp,
        int fg_subnumber)
        : Drawable(), Damageable(), Movable() //: cumulative_transformation_matrix( identity_matrix )
{
    pImage = (new UnitImages<void>);
    pilot = new Pilot(faction);
    Init(filename, SubU, faction, std::move(unitModifications), flightgrp, fg_subnumber);
    pilot->SetComm(this);
}

Unit::~Unit() {
    if ((!killed)) {
        // stephengtuggy 2020-07-27 - I think this message was mistakenly put in. This happens all the time when buying and selling cargo or ship upgrades.
        // stephengtuggy 2020-08-03 - Maybe not.
        VS_LOG(error, (boost::format("Assumed exit on unit %1%(if not quitting, report error)") % name));
    }
    if (ucref) {
        VS_LOG_AND_FLUSH(fatal, "DISASTER AREA!!!!");
    }
    VS_LOG(trace, (boost::format("Deallocating unit %1$s addr=%2$x refs=%3$d")
            % name.get().c_str() % this % ucref));
#ifdef DESTRUCTDEBUG
    VS_LOG_AND_FLUSH(trace, (boost::format("stage %1$d %2$x %3$d") % 0 % this % ucref));
#endif
#ifdef DESTRUCTDEBUG
    VS_LOG_AND_FLUSH(trace, (boost::format("%1$d %2$x") % 2 % pImage->pHudImage));
#endif
    if (pImage->unitwriter) {
        delete pImage->unitwriter;
        pImage->unitwriter = nullptr;
    }
    delete pImage;
#ifdef DESTRUCTDEBUG
    VS_LOG_AND_FLUSH(trace, (boost::format("%1$d %2$x") % 3 % pImage));
#endif
    pImage = nullptr;
    delete pilot;
#ifdef DESTRUCTDEBUG
    VS_LOG_AND_FLUSH(trace, (boost::format("%1$d") % 5));
#endif
#ifdef DESTRUCTDEBUG
    VS_LOG_AND_FLUSH(trace, (boost::format("%1$d %2$x") % 6 % &mounts));
#endif

#ifdef DESTRUCTDEBUG
    VS_LOG_AND_FLUSH(trace, (boost::format("%1$d %2$x") % 1 % &mounts));
#endif
#ifndef NO_MOUNT_STAR
                                                                                                                            for (vector< Mount* >::iterator jj = mounts.begin(); jj != mounts.end(); ++jj) {
        //Free all mounts elements
        if ( (*jj) != nullptr ) {
            delete (*jj);
            (*jj) = nullptr;
        }
    }
#endif
    mounts.clear();
#ifdef DESTRUCTDEBUG
    VS_LOG_AND_FLUSH(trace, (boost::format("%1$d") % 0));
#endif
    for (auto & mesh : meshdata) {
        if (mesh != nullptr) {
            delete mesh;
            mesh = nullptr;
        }
    }
    meshdata.clear();
}

// Delete this
void Unit::Init() {
    // What's left cannot be removed without breaking something.
    // TODO: need to figure out how to remove it safely.

    this->computer.combat_mode = true;
    // TODO: check if this is necessary and if it can be moved
#ifdef CONTAINER_DEBUG
    UncheckUnit( this );
#endif
    AddLayer(&hull);
    AddLayer(&armor);
    AddLayer(&shield);
}

using namespace VSFileSystem;
extern std::string GetReadPlayerSaveGame(int);

void Unit::Init(const char *filename,
        bool SubU,
        int faction,
        std::string unitModifications,
        Flightgroup *flightgrp,
        int fg_subnumber) {
    // TODO: something with the following line
    this->Unit::Init();
    graphicOptions.SubUnit = SubU ? 1 : 0;
    graphicOptions.Animating = 1;
    graphicOptions.RecurseIntoSubUnitsOnCollision = !isSubUnit();
    this->faction = faction;
    SetFg(flightgrp, fg_subnumber);
    bool saved_game = false;
    bool modified = !unitModifications.empty();
    if (modified) {
        string non_auto_save = GetReadPlayerSaveGame(_Universe->CurrentCockpit());
        string filepath("");

        if (non_auto_save.empty()) {
            VSFileSystem::CreateDirectoryHome(VSFileSystem::savedunitpath + "/" + unitModifications);
            filepath = unitModifications + "/" + string(filename);
        } else {
            VSFileSystem::CreateDirectoryHome(VSFileSystem::savedunitpath + "/" + non_auto_save);
            filepath = non_auto_save + "/" + string(filename);
        }

        //Try to open save
        if (filename[0]) {
            VSFile unitTab;
            VSError taberr = unitTab.OpenReadOnly(filepath + ".json", UnitSaveFile);
            if (taberr <= Ok) {
                UnitJSONFactory::ParseJSON(unitTab, true);
                unitTab.Close();
                saved_game = true;
            }
        }
    }

    this->filename = filename;
    this->name = filename;

    const std::string faction_name = FactionUtil::GetFactionName(faction);
    const std::string unit_key = GetUnitKeyFromNameAndFaction(filename, faction_name);

    if (unit_key.empty()) {
        // This is actually used for upgrade checks.
        bool is_template = (string::npos != (string(filename).find(".template")));
        if (!is_template || (is_template && configuration().data.using_templates)) {
            VS_LOG(trace, (boost::format("Unit file %1% not found") % filename));
        }
        meshdata.clear();
        meshdata.push_back(nullptr);
        this->fullname = filename;
        this->name = string("LOAD_FAILED");
        calculate_extent(false);
        radial_size = 1;

        pilot->SetComm(this);
        return;
    }

    //load from table?
    //we have to set the root directory to where the saved unit would have come from.
    //saved only exists if taberr<=Ok && taberr!=Unspecified...that's why we pass in said boolean
    // Despite the check, has always taken the data folder, simplifying
    //VSFileSystem::current_path.push_back(taberr <= Ok && taberr
    //        != Unspecified ? GetUnitRow(filename, SubU, faction, false,
    //        tmp_bool).getRoot() : unitRow.getRoot());
    std::string root = UnitCSVFactory::GetVariable(unit_key, "root", std::string());
    VSFileSystem::current_path.push_back(root);

    std::string directory = UnitCSVFactory::GetVariable(unit_key, "Directory", std::string());
    VSFileSystem::current_subdirectory.push_back("/" + directory);
    VSFileSystem::current_type.push_back(UnitFile);
    LoadRow(unit_key, unitModifications, saved_game);
    VSFileSystem::current_type.pop_back();
    VSFileSystem::current_subdirectory.pop_back();
    VSFileSystem::current_path.pop_back();

    calculate_extent(false);
    pilot->SetComm(this);

    bool init_succeeded = DrawableInit(filename, faction, flightgrp);
    if (init_succeeded) {
        SetAniSpeed(0.05);
        StartAnimation();
    }

    AddLayer(&hull);
    AddLayer(&armor);
    AddLayer(&shield);
}



bool Unit::GettingDestroyed() const {
    return hull.Destroyed();
}

vector<Mesh *> Unit::StealMeshes() {
    vector<Mesh *> ret;

    Mesh *shield = meshdata.empty() ? nullptr : meshdata.back();
    for (unsigned int i = 0; i <= nummesh(); ++i) {
        ret.push_back(meshdata[i]);
    }
    meshdata.clear();
    meshdata.push_back(shield);

    return ret;
}

static float tmpmax(float a, float b) {
    return a > b ? a : b;
}

bool CheckAccessory(Unit *tur) {
    bool accessory = tur->name.get().find("accessory") != string::npos;
    if (accessory) {
        tur->SetAngularVelocity(tur->DownCoordinateLevel(Vector(tur->drive.max_pitch_up,
                tur->drive.max_yaw_right,
                tur->drive.max_roll_right)));
    }
    return accessory;
}

void Unit::calculate_extent(bool update_collide_queue) {
    corner_min = Vector(FLT_MAX, FLT_MAX, FLT_MAX);
    corner_max = Vector(-FLT_MAX, -FLT_MAX, -FLT_MAX);
    for (unsigned int a = 0; a < nummesh(); ++a) {
        corner_min = corner_min.Min(meshdata[a]->corner_min());
        corner_max = corner_max.Max(meshdata[a]->corner_max());
    }                                                            /* have subunits now in table*/
    const Unit *un;
    for (un_kiter iter = SubUnits.constIterator(); (un = *iter); ++iter) {
        corner_min = corner_min.Min(un->LocalPosition().Cast() + un->corner_min);
        corner_max = corner_max.Max(un->LocalPosition().Cast() + un->corner_max);
    }
    if (corner_min.i == FLT_MAX || corner_max.i == -FLT_MAX || !FINITE(corner_min.i) || !FINITE(corner_max.i)) {
        radial_size = 0;
        corner_min.Set(0, 0, 0);
        corner_max.Set(0, 0, 0);
    } else {
        float tmp1 = corner_min.Magnitude();
        float tmp2 = corner_max.Magnitude();
        radial_size = tmp1 > tmp2 ? tmp1 : tmp2;
    }
    if (!isSubUnit() && update_collide_queue && (!Destroyed())) {
        //only do it in Unit::CollideAll UpdateCollideQueue();
    }
    if (getUnitType() == Vega_UnitType::planet) {
        radial_size = tmpmax(tmpmax(corner_max.i, corner_max.j), corner_max.k);
    }
}

const string Unit::getFgID() {
    if (flightgroup != nullptr) {
        char buffer[32];
        sprintf(buffer, "-%d", flightgroup_subnumber);
        return flightgroup->name + buffer;
    } else {
        return fullname;
    }
}

void Unit::SetFaction(int faction) {
    this->faction = faction;
    for (un_iter ui = getSubUnits(); (*ui) != nullptr; ++ui) {
        (*ui)->SetFaction(faction);
    }
}

void Unit::SetFg(Flightgroup *fg, int fg_subnumber) {
    flightgroup = fg;
    flightgroup_subnumber = fg_subnumber;
}




// TODO: Express everything in doubles, eliminating a lot of double-to-float conversions?
bool CloseEnoughToAutotrack(Unit *me, Unit *targ, float &cone) {
    if (targ) {
        const float close_enough_to_autotrack =
                std::pow(configuration().physics.close_enough_to_autotrack_flt,2);
        float dissqr = (me->curr_physical_state.position.Cast()
                - targ->curr_physical_state.position.Cast()).MagnitudeSquared();
        const float movesqr = close_enough_to_autotrack
                * (me->prev_physical_state.position.Cast()
                        - me->curr_physical_state.position.Cast()).MagnitudeSquared();
        if (dissqr < movesqr && movesqr > 0) {
            cone = configuration().physics.near_autotrack_cone_flt * (movesqr - dissqr) / movesqr + 1 * dissqr / movesqr;
            return true;
        }
    }
    return false;
}

//Caps at +/- 1 to account for floating point inaccuracies.
static inline float safeacos(float mycos) {
    if (mycos > 1.) {
        mycos = 1.;
    }
    if (mycos < -1.) {
        mycos = -1;
    }
    return acos(mycos);
}

float Unit::cosAngleTo(Unit *targ, float &dist, float speed, float range, bool turnmargin) const {
    Vector Normal(cumulative_transformation_matrix.getR());
    Normalize(Normal);
    QVector to_target(targ->PositionITTS(cumulative_transformation.position, cumulative_velocity, speed, false));
    to_target = to_target - cumulative_transformation.position;
    dist = to_target.Magnitude();

    //Trial code
    const float turn_limit =
            tmpmax(tmpmax(drive.max_yaw_left.Value(), drive.max_yaw_right.Value()),
                    tmpmax(drive.max_pitch_up.Value(), drive.max_pitch_down.Value()));
    const float turn_angle = simulation_atom_var
            * tmpmax(turn_limit,
                    tmpmax(simulation_atom_var * .5 * (drive.yaw.Value() + drive.pitch.Value()),
                            sqrtf(AngularVelocity.i * AngularVelocity.i + AngularVelocity.j * AngularVelocity.j)));
    const float itts_angle = safeacos(Normal.Cast().Dot(to_target.Scale(1. / to_target.Magnitude())));
    const QVector edgeLocation = (targ->cumulative_transformation_matrix.getP() * targ->rSize() + to_target);
    const float rad_angle = safeacos(edgeLocation.Cast().Scale(1. / edgeLocation.Magnitude()).Dot(to_target.Normalize()));
    const float rv = itts_angle - rad_angle - (turnmargin ? turn_angle : 0);

    float rsize = targ->rSize() + rSize();
    if ((!targ->GetDestinations().empty() && jump_drive.IsDestinationSet()) || (targ->faction == faction)) {
        rsize = 0;
    }                                       //HACK so missions work well
    if (range != 0) {
        dist = (dist - rsize) / range;
    } else {
        dist = 0;
    }
    if (!FINITE(dist) || dist < 0) {
        dist = 0;
    }
    return (rv < 0) ? 1 : cos(rv);
}

float Unit::cosAngleFromMountTo(Unit *targ, float &dist) const {
    float retval = -1;
    dist = FLT_MAX;
    Matrix mat;
    for (int i = 0; i < getNumMounts(); ++i) {
        float tmp_dist = .001;
        Transformation final_transformation(mounts[i].GetMountOrientation(), mounts[i].GetMountLocation().Cast());
        final_transformation.Compose(cumulative_transformation, cumulative_transformation_matrix);
        final_transformation.to_matrix(mat);
        Vector Normal(mat.getR());

        QVector to_target(targ->PositionITTS(final_transformation.position, cumulative_velocity, mounts[i].type->speed, false));

        float tmp_cos = Normal.Dot(to_target.Cast());
        tmp_dist = to_target.Magnitude();
        if (tmp_cos > 0) {
            tmp_cos = tmp_dist * tmp_dist - tmp_cos * tmp_cos;
            //one over distance perpendicular away from straight ahead times the size...high is good WARNING POTENTIAL DIV/0
            tmp_cos = targ->rSize() / tmp_cos;
        } else {
            tmp_cos /= tmp_dist;
        }
        //UNLIKELY DIV/0
        tmp_dist /= mounts[i].type->range;
        if (tmp_dist < 1 || tmp_dist < dist) {
            if (tmp_cos - tmp_dist / 2 > retval - dist / 2) {
                dist = tmp_dist;
                retval = tmp_cos;
            }
        }
    }
    return retval;
}

#define PARANOIA (0.4f)

void Unit::Threaten(Unit *targ, float danger) {
    if (!targ) {
        computer.threatlevel = danger;
        computer.threat = nullptr;
    } else if (targ->owner != this && this->owner != targ && danger > PARANOIA && danger > computer.threatlevel) {
        computer.threat = targ;
        computer.threatlevel = danger;
    }
}

const std::string &Unit::getCockpit() const {
    return pImage->cockpitImage.get();
}

void Unit::Select() {
    selected = true;
}

void Unit::Deselect() {
    selected = false;
}

void disableSubUnits(Unit *uhn) {
    Unit *un;
    for (un_iter i = uhn->getSubUnits(); (un = *i) != nullptr; ++i) {
        disableSubUnits(un);
    }
    for (auto & mount : uhn->mounts) {
        DestroyMount(&mount);
    }
}

un_iter Unit::getSubUnits() {
    return SubUnits.createIterator();
}

un_kiter Unit::viewSubUnits() const {
    return SubUnits.constIterator();
}

void Unit::SetVisible(bool vis) {
    if (vis) {
        invisible &= (~INVISCAMERA);
    } else {
        invisible |= INVISCAMERA;
    }
}

void Unit::SetAllVisible(bool vis) {
    if (vis) {
        invisible &= (~INVISUNIT);
    } else {
        invisible |= INVISUNIT;
    }
}

void Unit::SetGlowVisible(bool vis) {
    if (vis) {
        invisible &= (~INVISGLOW);
    } else {
        invisible |= INVISGLOW;
    }
}

void Unit::setTargetFg(string primary, string secondary, string tertiary) {
    target_fgid[0] = primary;
    target_fgid[1] = secondary;
    target_fgid[2] = tertiary;

    ReTargetFg(0);
}

void Unit::ReTargetFg(int which_target) {
}

/*
 **********************************************************************************
 **** UNIT_PHYSICS STUFF
 **********************************************************************************
 */

extern signed char ComputeAutoGuarantee(Unit *un);
extern float getAutoRSize(Unit *orig, Unit *un, bool ignore_friend = false);

QVector SystemLocation(std::string system) {
    string xyz = _Universe->getGalaxyProperty(system, "xyz");
    QVector pos;
    // TODO: Use strtod or similar, in order to catch conversion errors
    if (!xyz.empty() && (sscanf(xyz.c_str(), "%lf %lf %lf", &pos.i, &pos.j, &pos.k) >= 3)) {
        return pos;
    } else {
        return QVector(0, 0, 0);
    }
}

static std::string NearestSystem(std::string currentsystem, QVector pos) {
    if (pos.i == 0 && pos.j == 0 && pos.k == 0) {
        return "";
    }
    QVector normalized_position = pos.Normalize();
    normalized_position.Normalize();
    QVector cur = SystemLocation(std::move(currentsystem));
    if (cur.i == 0 && cur.j == 0 && cur.k == 0) {
        return "";
    }
    double closest_distance = 0.0;
    std::string closest_system;
    GalaxyXML::Galaxy *gal = _Universe->getGalaxy();
    GalaxyXML::SubHeirarchy *sectors = &gal->getHeirarchy();
    vsUMap<std::string, class GalaxyXML::SGalaxy>::iterator j, i = sectors->begin();
    for (; i != sectors->end(); ++i) {
        GalaxyXML::SubHeirarchy *systems = &i->second.getHeirarchy();
        for (j = systems->begin(); j != systems->end(); ++j) {
            std::string place = j->second["xyz"];
            if (!place.empty()) {
                QVector pos2 = QVector(0, 0, 0);
                // TODO: Use strtod or similar, in order to catch conversion errors
                sscanf(place.c_str(), "%lf %lf %lf", &pos2.i, &pos2.j, &pos2.k);
                if ((pos2.i != 0 || pos2.j != 0 || pos2.k != 0)
                        && (pos2.i != cur.i || pos2.j != cur.j || pos2.k != cur.k)) {
                    QVector dir = pos2 - cur;
                    QVector norm = dir;
                    norm.Normalize();
                    double test = normalized_position.Dot(norm);
                    if (test > .2) {
                        //test=1-test;
                        double tmp = dir.MagnitudeSquared() / test / test / test;
                        for (unsigned int cp = 0; cp < _Universe->numPlayers(); ++cp) {
                            std::string whereto = _Universe->AccessCockpit(cp)->GetNavSelectedSystem();
                            if (whereto.length() == 1 + i->first.length() + j->first.length()) {
                                if (whereto.substr(0,
                                        i->first.length()) == i->first
                                        && whereto.substr(i->first.length() + 1)
                                                == j->first) {
                                    tmp /= configuration().physics.target_distance_to_warp_bonus_dbl;
                                }
                            }
                        }
                        if (tmp < closest_distance || closest_distance == 0) {
                            closest_distance = tmp;
                            closest_system = i->first + "/" + j->first;
                        }
                    }
                }
            }
        }
    }
    return closest_system;
}

void Unit::UpdateSubunitPhysics(const Transformation &trans,
        const Matrix &transmat,
        const Vector &cum_vel,
        bool lastframe,
        UnitCollection *uc,
        Unit *superunit) {
    if (!SubUnits.empty()) {
        Unit *su;
        const float backup = simulation_atom_var;
        //VS_LOG(trace, (boost::format("Unit::UpdateSubunitPhysics(): simulation_atom_var as backed up  = %1%") % simulation_atom_var));
        const float base_sim_atom = (this->sim_atom_multiplier ? backup / static_cast<float>(this->sim_atom_multiplier) : backup);
        const unsigned int cur_sim_frame = _Universe->activeStarSystem()->getCurrentSimFrame();
        bool didSomeScattering = false;
        for (un_iter iter = getSubUnits(); (su = *iter); ++iter) {
            if (this->sim_atom_multiplier && su->sim_atom_multiplier) {
                //This ugly thing detects skipped frames.
                //This shouldn't happen during normal execution, as the interpolation will not be correct
                //when outside the expected range (that is, if the target queue slot is skipped).
                //BUT... this allows easy subunit simulation scattering by initializing cur_sim_frame
                //with random data.
                //Normal crossing
                if (((su->last_processed_sqs < su->cur_sim_queue_slot) && (cur_sim_frame >= su->cur_sim_queue_slot))
                        //Full round trip
                        || (su->last_processed_sqs == cur_sim_frame)
                                //Incomplete round trip - but including target frame
                        || ((su->last_processed_sqs > cur_sim_frame)
                                && ((su->cur_sim_queue_slot <= cur_sim_frame)
                                        || (su->last_processed_sqs < su->cur_sim_queue_slot)))
                        ) {
                    if (do_subunit_scheduling) {
                        #if defined(RANDOMIZE_SIM_ATOMS)
                                                                                                                                                int priority = UnitUtil::getPhysicsPriority( su );
                            //Add some scattering
                            priority = (priority+rand()%priority)/2;
                            if (priority < 1) priority = 1;
                            su->sim_atom_multiplier = this->sim_atom_multiplier*priority;
                            if (su->sim_atom_multiplier > SIM_QUEUE_SIZE)
                                su->sim_atom_multiplier = (SIM_QUEUE_SIZE/su->sim_atom_multiplier)*su->sim_atom_multiplier;
                            if (su->sim_atom_multiplier < this->sim_atom_multiplier)
                                su->sim_atom_multiplier = this->sim_atom_multiplier;
                            didSomeScattering = true;
                        #endif
                    } else {
                        su->sim_atom_multiplier = this->sim_atom_multiplier;
                    }
                    simulation_atom_var = base_sim_atom * static_cast<float>(su->sim_atom_multiplier);
                    //VS_LOG(trace, (boost::format("Unit::UpdateSubunitPhysics(): simulation_atom_var as multiplied = %1%") % simulation_atom_var));
                    Unit::UpdateSubunitPhysics(su,
                            cumulative_transformation,
                            cumulative_transformation_matrix,
                            cumulative_velocity,
                            lastframe,
                            uc,
                            superunit);
                }
            }
        }
        if (didSomeScattering) {
            VS_LOG(trace, "Unit::UpdateSubunitPhysics(): Did some random scattering inside skipped-frames handler");
        }
        simulation_atom_var = backup;
        //VS_LOG(trace, (boost::format("Unit::UpdateSubunitPhysics(): simulation_atom_var as restored   = %1%") % simulation_atom_var));
    }
}

void Unit::UpdateSubunitPhysics(Unit *subunit,
        const Transformation &trans,
        const Matrix &transmat,
        const Vector &CumulativeVelocity,
        bool lastframe,
        UnitCollection *uc,
        Unit *superunit) {
    subunit->UpdatePhysics(cumulative_transformation,
            cumulative_transformation_matrix,
            cumulative_velocity,
            lastframe,
            uc,
            superunit);

    // TODO: make the subunit->cloak a pointer to parent->cloak
    // Also, no reason why subunits should handle their own physics but that's
    // much harder to refactor
    subunit->cloak.status = cloak.status;
    if (Destroyed()) {
        subunit->Target(nullptr);
        UnFire();                                        //don't want to go off shooting while your body's splitting everywhere
    }
}

QVector Unit::realPosition() {
    if (isSubUnit()) {
        return Position();
    }
    return LocalPosition();
}

// TODO: need to move and join a class
QVector RealPosition(const Unit *un) {
    if (un->isSubUnit()) {
        return un->Position();
    }
    return un->LocalPosition();
}

float globQueryShell(QVector pos, QVector dir, float rad);

extern void ActivateAnimation(Unit *jp);

void TurnJumpOKLightOn(Unit *un, Cockpit *cp) {
    if(!cp) {
        return;
    }

    if(!un->jump_drive.Operational()) {
       return;
    }

    if (!un->jump_drive.CanConsume()) {
        return;
    }

    cp->jumpok = 1;
}

bool Unit::jumpReactToCollision(Unit *smalle) {
    const bool ai_jump_cheat = configuration().ai.jump_without_energy;
    const bool nojumpinSPEC = configuration().physics.no_spec_jump;
    bool SPEC_interference = (nullptr != _Universe->isPlayerStarship(smalle)) ? smalle->ftl_drive.Enabled()
            && nojumpinSPEC : (nullptr != _Universe->isPlayerStarship(this)) && ftl_drive.Enabled()
            && nojumpinSPEC;
    //only allow big with small
    if (!GetDestinations().empty()) {
        Cockpit *cp = _Universe->isPlayerStarship(smalle);
        if (!SPEC_interference || forcejump) {
            TurnJumpOKLightOn(smalle, cp);
        } else {
            return false;
        }
        //we have a drive
        if ((!SPEC_interference &&
                (smalle->jump_drive.IsDestinationSet() &&          //we have power
                    (smalle->jump_drive.CanConsume() ||
                    //or we're being cheap
                    (ai_jump_cheat && cp == nullptr))
                )
            ) || forcejump) {

            if(!smalle->jump_drive.IsDestinationSet()) {
                smalle->jump_drive.SetDestination(0);
            }
            int dest = smalle->jump_drive.Destination();

            smalle->jump_drive.UnsetDestination();
            Unit *jumppoint = this;
            vector<std::string> smalles_destinations = GetDestinations();
            _Universe->activeStarSystem()
                     ->JumpTo(smalle, jumppoint, smalles_destinations.at(dest % smalles_destinations.size()));
            return true;
        }
        return true;
    }
    if (!smalle->GetDestinations().empty()) {
        Cockpit *cp = _Universe->isPlayerStarship(this);
        if (!SPEC_interference || smalle->forcejump) {
            TurnJumpOKLightOn(this, cp);
        } else {
            return false;
        }
        if ((!SPEC_interference && (jump_drive.IsDestinationSet()
                && (jump_drive.CanConsume() || (ai_jump_cheat && cp == nullptr))
        )) || smalle->forcejump) {
            jump_drive.Consume();
            jump_drive.UnsetDestination();
            Unit *jumppoint = smalle;

            _Universe->activeStarSystem()->JumpTo(this, jumppoint,
                    smalle->GetDestinations()[jump_drive.Destination()
                            % smalle->GetDestinations().size()]);

            return true;
        }
        return true;
    }
    return false;
}

Cockpit *Unit::GetVelocityDifficultyMult(float &difficulty) const {
    difficulty = 1;
    Cockpit *player_cockpit = _Universe->isPlayerStarship(this);
    if ((player_cockpit) == nullptr) {
        difficulty = std::pow(g_game.difficulty, configuration().physics.difficulty_speed_exponent_flt);
    }
    return player_cockpit;
}



void Unit::SwitchCombatFlightMode() {
    if (computer.combat_mode) {
        computer.combat_mode = false;
    } else {
        computer.combat_mode = true;
    }
}

bool Unit::CombatMode() {
    return computer.combat_mode;
}



void Unit::ClearMounts() {
    for (auto & mount : mounts) {
        DestroyMount(&mount);
        AUDDeleteSound(mount.sound);
        if (mount.ref.gun && mount.type->type == WEAPON_TYPE::BEAM) {
            //hope we're not killin' em twice...they don't go in gun queue
            delete mount.ref.gun;
            mount.ref.gun = nullptr;
        }
    }
    mounts.clear();
    Unit *su;
    for (un_iter i = getSubUnits(); (su = *i) != nullptr; ++i) {
        su->ClearMounts();
    }
}



/*
 **********************************************************************************
 **** UNIT_DAMAGE STUFF
 **********************************************************************************
 */







Unit *findUnitInStarsystem(const void *unitDoNotDereference) {
    Unit *un;
    for (un_iter i = _Universe->activeStarSystem()->getUnitList().createIterator(); (un = *i) != nullptr; ++i) {
        if (un == static_cast<const Unit *>(unitDoNotDereference)) {
            return un;
        }
    }
    return nullptr;
}

extern std::string getDamageColor(double);

//NUMGAUGES has been moved to pImages.h in UnitImages<void>
void Unit::DamageRandSys(float dam, const Vector &vec) {
    // TODO: take actual damage into account when damaging components.
    const float deg = fabs(180 * atan2(vec.i, vec.k) / M_PI);
    const float random_number = randomDouble();
    const float inv_min_dam = 1.0F - configuration().physics.min_damage_flt;
    const float inv_max_dam = 1.0F - configuration().physics.max_damage_flt;
    if (dam < inv_max_dam) {
        dam = inv_max_dam;
    }
    if (dam > inv_min_dam) {
        dam = inv_min_dam;
    }
    float degrees = deg;
    if (degrees > 180) {
        degrees = 360 - degrees;
    }
    if (degrees >= 0 && degrees < 20) {

        //DAMAGE COCKPIT
        if (random_number >= .85) {//do 25% damage to a gauge
            ship_functions.Damage(Function::cockpit);
        } else if (random_number >= .775) {
            computer.itts = false;             //Set the computer to not have an itts
        } else if (random_number >= .7) {
            radar.Damage();
        } else if (random_number >= .5) {
            //THIS IS NOT YET SUPPORTED IN NETWORKING
            computer.target = nullptr;             //set the target to NULL
        } else if (random_number >= .4) {
            drive.retro.RandomDamage();
        } else if (random_number >= .25) {
            radar.Damage();
        } else if (random_number >= .175) {
            radar.Damage();
        } else {
            // Duplicate of above
            ship_functions.Damage(Function::cockpit);
        }

        GenerateHudText(getDamageColor);
        return;
    }
    if (randomDouble() < configuration().physics.thruster_hit_chance_dbl) {
        // This is fairly severe. One or two hits can disable the engine.
        // Note that retro can be damaged by both this and above.
        // Drive can also be damaged by code below - really computer.
        // TODO: figure out a better damage system that doesn't rely on where
        // the shots are coming from.
        drive.Damage();
        afterburner.Damage();
        GenerateHudText(getDamageColor);
        return;
    }
    if (degrees >= 20 && degrees < 35) {
        //DAMAGE MOUNT
        if (random_number >= .65 && random_number < .9) {
            ecm.Damage();
        } else if (getNumMounts()) {
            const unsigned int which_mount = randomInt(getNumMounts());
            if (random_number >= .9) {
                DestroyMount(&mounts[which_mount]);
            } else if (mounts[which_mount].ammo > 0 && random_number >= .75) {
                mounts[which_mount].ammo *= float_to_int(dam);
            } else if (random_number >= .7) {
                mounts[which_mount].time_to_lock += (100 - (100 * dam));
            } else if (random_number >= .2) {
                mounts[which_mount].functionality *= dam;
            } else {
                mounts[which_mount].maxfunctionality *= dam;
            }
        }
        GenerateHudText(getDamageColor);
        return;
    }
    if (degrees >= 35 && degrees < 60) {
        // This code potentially damages a whole bunch of components.
        // We generate a random int (0-19). 0-8 damages something.
        // 9-19 doesn't.
        // This is really a stopgap code until we refactor this better.
        std::random_device dev;
        std::mt19937 rng(dev());
        std::uniform_int_distribution<std::mt19937::result_type> dist20(0,19); // distribution in range [1, 6]

        switch (dist20(rng)) {
        case 0: fuel.Damage();
            break; // Fuel
        case 1: energy.Damage();
            break; // Energy
        case 2: ftl_energy.Damage();
            break;
        case 3: ftl_drive.Damage();
            break;
        case 4: jump_drive.Damage();
            break;
        case 5: afterburner.Damage();
            break;
        //case 6: CargoVolume *= dam;
            break;
        //case 7: UpgradeVolume *= dam;
            break;
        case 8:
            //Do something NASTY to the cargo
            if (!cargo_hold.Empty()) {
                unsigned int i = 0;
                const unsigned int cargo_rand_o = randomInt(INT_MAX);
                unsigned int cargo_rand;
                /*do {
                    cargo_rand = (cargo_rand_o + i) % cargo.size();
                }
                while ((cargo[cargo_rand].GetQuantity() == 0
                    || cargo[cargo_rand].IsMissionFlag()) && (++i) < cargo.size());
                cargo[cargo_rand].SetQuantity(cargo[cargo_rand].GetQuantity() * float_to_int(dam));*/
            }
            break;
        default:
            // No damage
            break;
        }

        GenerateHudText(getDamageColor);
        return;
    }
    if (degrees >= 90 && degrees < 120) {
        if (random_number >= .7) {
            this->cloak.Damage();
        }

        // TODO: lib_damage reenable
        shield.Damage();

        GenerateHudText(getDamageColor);
        return;
    }
    if (degrees >= 120 && degrees < 150) {
        //DAMAGE Reactor
        //DAMAGE JUMP
        if (random_number >= .9) {
            /*const char max_shield_leak =
                (char) std::max( 0.0,
                             std::min( 100.0, configuration().physics.max_shield_leak ) );
            const char min_shield_leak =
                (char) std::max( 0.0,
                             std::min( 100.0, configuration().physics.max_shield_leak ) );*/
            //char newleak = float_to_int( std::max( min_shield_leak, std::max( max_shield_leak, (char) ( (randnum-.9)*10.0*100.0 ) ) ) );
            // TODO: lib_damage if (shield.leak < newleak)
            //shield.leak = newleak;
        } else if (random_number >= .7) {
            // TODO: lib_damage shield.recharge *= dam;
        } else if (random_number >= .5) {
            /*const float mindam =
                    configuration().physics.min_recharge_shot_damage;
            if (dam < mindam) {
                dam = mindam;
            }
            this->recharge *= dam;*/
            // TODO: do the above
            reactor.Damage();
        } else if (random_number >= .2) {
            const float mindam = configuration().physics.min_maxenergy_shot_damage_flt;
            if (dam < mindam) {
                dam = mindam;
            }
            energy.DamageByPercent(dam);
        } else {
            repair_bot.Damage();
        }

        GenerateHudText(getDamageColor);
        return;
    }
    if (degrees >= 150 && degrees <= 180) {
        //DAMAGE ENGINES
        drive.Damage();
        GenerateHudText(getDamageColor);
        return;
    }
}

void Unit::Kill(bool erasefromsave, bool quitting) {
    if (this->colTrees) {
        this->colTrees->Dec();
    }           //might delete
    this->colTrees = nullptr;
    killSounds();
    ClearMounts();

    if (docked & (DOCKING_UNITS)) {
        const float survival = configuration().physics.survival_chance_on_base_death_flt;
        const float player_survival = configuration().physics.player_survival_chance_on_base_death_flt;
        const int i_survival = float_to_int((RAND_MAX * survival));
        const int i_player_survival = float_to_int((RAND_MAX * player_survival));

        vector<Unit *> dockedun;
        for (auto & docked_unit : pImage->dockedunits) {
            Unit *un;
            if (nullptr != (un = docked_unit->uc.GetUnit())) {
                dockedun.push_back(un);
            }
        }
        while (!dockedun.empty()) {

            dockedun.back()->UnDock(this);

            if (randomInt(INT_MAX) <= (UnitUtil::isPlayerStarship(dockedun.back()) ? i_player_survival : i_survival)) {
                dockedun.back()->Kill();
            }
            dockedun.pop_back();
        }
    }
    //eradicate everything. naturally (see previous line) we won't eradicate beams eradicated above
    if (!isSubUnit()) {
        RemoveFromSystem();
    }
    computer.target = nullptr;

    //God, I can't believe this next line cost me 1 GIG of memory until I added it
    computer.threat = nullptr;
    computer.velocity_reference = nullptr;
    computer.force_velocity_ref = true;
    if (aistate) {
        aistate->ClearMessages();
        aistate->Destroy();
    }
    aistate = nullptr;

    // The following we don't want to do twice
    killed = true;
    Unit *un;
    for (un_iter iter = getSubUnits(); (un = *iter); ++iter) {
        un->Kill();
    }

    //if (getUnitType() != Vega_UnitType::missile) {
    //    VS_LOG(info, (boost::format("UNIT HAS DIED: %1% %2% (file %3%)") % name.get() % fullname % filename.get()));
    //}

    if (ucref == 0) {
        VS_LOG(trace, (boost::format("UNIT DELETION QUEUED: %1$s %2$s (file %3$s, addr 0x%4$08x)")
                % name.get().c_str() % fullname.c_str() % filename.get().c_str() % this));
        unit_delete_queue.push_back(this);
        if (flightgroup) {
            if (flightgroup->leader.GetUnit() == this) {
                flightgroup->leader.SetUnit(nullptr);
            }
        }

        if (_Universe && _Universe->AccessCockpit() && _Universe->AccessCockpit()->GetParent() && _Universe->AccessCockpit()->GetParent()->Target() == this) {
            VS_LOG_AND_FLUSH(info, "Killing player's target");
            _Universe->AccessCockpit()->GetParent()->SetTarget(nullptr);
        }

//#ifdef DESTRUCTDEBUG
//        VS_LOG(trace, (boost::format("%s 0x%x - %d") % name.get().c_str() % this % Unit_delete_queue.size()));
//#endif
    }
}

void Unit::UnRef() {
#ifdef CONTAINER_DEBUG
    CheckUnit( this );
#endif
    ucref--;
    if (killed && ucref == 0) {
#ifdef CONTAINER_DEBUG
        deletedUn.Put( (uintmax_t) this, this );
#endif
        //delete
        unit_delete_queue.push_back(this);
#ifdef DESTRUCTDEBUG
        VS_LOG(trace, (boost::format("%1$s %2$x - %3$d") % name.get().c_str() % this % unit_delete_queue.size()));
#endif
    }
}

float Unit::ExplosionRadius() {
    const float expsize = configuration().graphics.explosion_size_flt;
    return expsize * rSize();
}

void Unit::ProcessDeleteQueue() {
    while (!unit_delete_queue.empty()) {
#ifdef DESTRUCTDEBUG
        VS_LOG_AND_FLUSH(trace, (boost::format("Eliminatin' %1$x - %2$d") % unit_delete_queue.back() % unit_delete_queue.size()));
        VS_LOG_AND_FLUSH(trace, (boost::format("Eliminatin' %1$s") % unit_delete_queue.back()->name.get().c_str()));
#endif
#ifdef DESTRUCTDEBUG
        if ( unit_delete_queue.back()->isSubUnit() ) {
            VS_LOG(debug, "Subunit Deleting (related to double dipping)");
        }
#endif
        Unit *mydeleter = unit_delete_queue.back();
        unit_delete_queue.pop_back();
        delete mydeleter;                        ///might modify unitdeletequeue

#ifdef DESTRUCTDEBUG
        VS_LOG_AND_FLUSH(trace, (boost::format("Completed %1$d") % unit_delete_queue.size()));
#endif
    }
}



static const string LOAD_FAILED = "LOAD_FAILED";



const Unit *loadUnitByCache(std::string name, int faction) {
    const Unit *temprate = UnitConstCache::getCachedConst(StringIntKey(name, faction));
    if (!temprate) {
        temprate =
                UnitConstCache::setCachedConst(StringIntKey(name, faction), new Unit(name.c_str(), true, faction));
    }
    return temprate;
}

bool DestroySystem(float hull_percent, float numhits) {
    const float damage_chance = configuration().physics.damage_chance_flt;
    const float guaranteed_chance = configuration().physics.definite_damage_chance_flt;
    float chance = 1 - (damage_chance * (guaranteed_chance + hull_percent));
    if (numhits > 1) {
        chance = std::pow(chance, numhits);
    }
    return randomDouble() > chance;
}

bool DestroyPlayerSystem(float hull_percent, float numhits) {
    const float damage_chance = configuration().physics.damage_player_chance_flt;
    const float guaranteed_chance = configuration().physics.definite_damage_chance_flt;
    float chance = 1 - (damage_chance * (guaranteed_chance + hull_percent));
    if (numhits > 1) {
        chance = std::pow(chance, numhits);
    }
    bool ret = (randomDouble() > chance);
    if (ret) {
        //VS_LOG(warning, "DAAAAAAMAGED!!!!");
    }
    return ret;
}

const char *DamagedCategory = "upgrades/Damaged/";

/*
 **********************************************************************************
 **** UNIT_WEAPON STUFF
 **********************************************************************************
 */

void Unit::TargetTurret(Unit *targ) {
    if (!SubUnits.empty()) {
        const bool in_range = (targ != nullptr) ? InRange(targ) : true;
        if (in_range) {
            Unit *su;
            for (un_iter iter = getSubUnits(); (su = *iter); ++iter) {
                su->Target(targ);
                su->TargetTurret(targ);
            }
        }
    }
}

void WarpPursuit(Unit *un, StarSystem *sourcess, std::string destination) {
    const bool AINotUseJump = configuration().physics.no_ai_jump_points;
    if (AINotUseJump) {
        const float seconds_per_parsec = configuration().physics.seconds_per_parsec_flt;
        float ttime =
                (SystemLocation(sourcess->getFileName()) - SystemLocation(destination)).Magnitude()
                        * seconds_per_parsec;
        un->jump_drive.SetDelay(float_to_int(ttime));
        sourcess->JumpTo(un, nullptr, destination, true, true);
        un->jump_drive.SetDelay(-float_to_int(ttime));
    }
}

//WARNING : WHEN TURRETS WE MAY NOT WANT TO ASK THE SERVER FOR INFOS ! ONLY FOR LOCAL PLAYERS (_Universe-isStarship())





void Unit::Target(Unit *targ) {
    if (targ == this) {
        return;
    }

    if (!(activeStarSystem == nullptr || activeStarSystem == _Universe->activeStarSystem())) {
        computer.target = nullptr;
        return;
    }
    if (targ) {
        if (targ->activeStarSystem == _Universe->activeStarSystem() || targ->activeStarSystem == nullptr) {
            if (targ != Target()) {
                for (int i = 0; i < getNumMounts(); ++i) {
                    mounts[i].time_to_lock = mounts[i].type->lock_time;
                }

                computer.target = targ;
                radar.Unlock();
            }
        } else {
            // TODO: this is unclear code. I translated it fully but
            // it doesn't really make sense. Maybe targets don't have destinations?!
            if (!jump_drive.Installed() || jump_drive.IsDestinationSet()) {
                bool found = false;
                Unit *u;
                for (un_iter i = _Universe->activeStarSystem()->getUnitList().createIterator(); (u = *i) != nullptr; ++i) {
                    if (!u->GetDestinations().empty()) {
                        if (std::find(u->GetDestinations().begin(), u->GetDestinations().end(),
                                targ->activeStarSystem->getFileName()) != u->GetDestinations().end()) {
                            Target(u);
                            ActivateJumpDrive(0);
                            found = true;
                        }
                    }
                }
                if (!found && !_Universe->isPlayerStarship(this)) {
                    WarpPursuit(this, _Universe->activeStarSystem(), targ->getStarSystem()->getFileName());
                }
            } else {
                computer.target = nullptr;
            }
        }
    } else {
        computer.target = nullptr;
    }
}

void Unit::SetOwner(Unit *target) {
    owner = target;
}

// Need this for python API. Do not delete.
void Unit::ActivateCloak(bool enable) {
    if(enable) {
        cloak.Activate();
    } else {
        cloak.Deactivate();
    }
}

void Unit::SetRecursiveOwner(Unit *target) {
    owner = target;
    Unit *su;
    for (un_iter iter = getSubUnits(); (su = *iter); ++iter) {
        su->SetRecursiveOwner(target);
    }
}

/*
 **********************************************************************************
 **** UNIT_COLLIDE STUFF
 **********************************************************************************
 */

extern unsigned int AddAnimation(const QVector &, const float, bool, const string &, float percentgrow);
extern string getRandomCachedAniString();
extern Animation *GetVolatileAni(unsigned int);

bool Unit::Explode(bool drawit, float timeit) {
    if (this->pImage->pExplosion == nullptr && this->pImage->timeexplode == 0) {
        //no explosion in unit data file && explosions haven't started yet

        //notify the director that a ship got destroyed
        mission->DirectorShipDestroyed(this);
        disableSubUnits(this);
        this->pImage->timeexplode = 0;

        string bleh = this->pImage->explosion_type;
        if (bleh.empty()) {
            FactionUtil::GetRandExplosionAnimation(this->faction, bleh);
        }
        if (bleh.empty()) {
            static Animation * cache = nullptr;
            static bool initialized = false;
            if (!initialized) {
                initialized = true;
                cache = new Animation(configuration().graphics.explosion_animation.c_str(), false, 0.1, BILINEAR, false);
            }
            bleh = getRandomCachedAniString();
            if (bleh.empty()) {
                bleh = configuration().graphics.explosion_animation;
            }
        }
        this->pImage->pExplosion = new Animation(bleh.c_str(), configuration().graphics.explosion_face_player, .1, BILINEAR, true);
        this->pImage->pExplosion->SetDimensions(this->ExplosionRadius(), this->ExplosionRadius());
        Vector p, q, r;
        this->GetOrientation(p, q, r);
        this->pImage->pExplosion->SetOrientation(p, q, r);
        if (this->getUnitType() != Vega_UnitType::missile) {
            _Universe->activeStarSystem()->AddMissileToQueue(new MissileEffect(this->Position(),
                    this->shield.AverageMaxLayerValue(),
                    0,
                    this->ExplosionRadius()
                            * configuration().physics.explosion_damage_center_flt,
                    this->ExplosionRadius()
                            * configuration().physics.explosion_damage_center_flt
                            * configuration().physics.explosion_damage_edge_flt,
                    NULL));
        }
        QVector exploc = this->cumulative_transformation.position;
        bool sub = this->isSubUnit();
        Unit *un = nullptr;
        if (!sub) {
            if ((un = _Universe->AccessCockpit(0)->GetParent())) {
                exploc = un->Position() * game_options()->explosion_closeness
                        + exploc * (1 - game_options()->explosion_closeness);
            }
        }
        //AUDPlay( this->sound->explode, exploc, this->Velocity, 1 );
        playExplosionDamageSound();

        if (!sub) {
            un = _Universe->AccessCockpit()->GetParent();
            if (this->getUnitType() == Vega_UnitType::unit) {
                if (randomInt(RAND_MAX) < RAND_MAX * configuration().graphics.percent_shockwave_dbl && (!this->isSubUnit())) {
                    const std::string shockani(configuration().graphics.shockwave_animation);
                    static Animation * shock_ani = nullptr;
                    static bool initialized = false;
                    if (!initialized) {
                        initialized = true;
                        shock_ani = new Animation(shockani.c_str(), true, 0.1, MIPMAP, false);
                    }

                    shock_ani->SetFaceCam(false);
                    unsigned int which = AddAnimation(this->Position(),
                            this->ExplosionRadius(),
                            true,
                            shockani,
                            configuration().graphics.shockwave_growth_flt);
                    Animation *ani = GetVolatileAni(which);
                    if (ani) {
                        ani->SetFaceCam(false);
                        Vector p, q, r;
                        this->GetOrientation(p, q, r);
                        int tmp = randomInt(RAND_MAX);
                        if (tmp < RAND_MAX / 24) {
                            ani->SetOrientation(Vector(0, 0, 1), Vector(1, 0, 0), Vector(0, 1, 0));
                        } else if (tmp < RAND_MAX / 16) {
                            ani->SetOrientation(Vector(0, 1, 0), Vector(0, 0, 1), Vector(1, 0, 0));
                        } else if (tmp < RAND_MAX / 8) {
                            ani->SetOrientation(Vector(1, 0, 0), Vector(0, 1, 0), Vector(0, 0, 1));
                        } else {
                            ani->SetOrientation(p, q, r);
                        }
                    }
                }
                if (un) {
                    int upgradesfaction = FactionUtil::GetUpgradeFaction();
                    float rel = un->getRelation(this);
                    if (!BaseInterface::CurrentBase) {
                        static double lasttime = 0;
                        double newtime = getNewTime();
                        if (newtime - lasttime > game_options()->time_between_music
                                || (_Universe->isPlayerStarship(this) && this->getUnitType() != Vega_UnitType::missile
                                        && this->faction
                                                != upgradesfaction)) {
                            //No victory for missiles or spawned explosions
                            if (rel > game_options()->victory_relationship) {
                                lasttime = newtime;
                                muzak->SkipRandSong(Music::LOSSLIST);
                            } else if (rel < game_options()->loss_relationship) {
                                lasttime = newtime;
                                muzak->SkipRandSong(Music::VICTORYLIST);
                            }
                        }
                    }
                }
            }
        }
    }
    bool timealldone =
            (this->pImage->timeexplode > configuration().physics.debris_time_flt || this->getUnitType() == Vega_UnitType::missile
                    || _Universe->AccessCockpit()->GetParent() == this || this->SubUnits.empty());
    if (this->pImage->pExplosion) {
        this->pImage->timeexplode += timeit;
        this->pImage->pExplosion->SetPosition(this->Position());
        Vector p, q, r;
        this->GetOrientation(p, q, r);
        this->pImage->pExplosion->SetOrientation(p, q, r);
        if (this->pImage->pExplosion->Done() && timealldone) {
            delete this->pImage->pExplosion;
            this->pImage->pExplosion = nullptr;
        }
        if (drawit && this->pImage->pExplosion) {
            this->pImage->pExplosion->Draw();
        }              //puts on draw queue... please don't delete
    }
    bool alldone = this->pImage->pExplosion ? !this->pImage->pExplosion->Done() : false;
    if (!this->SubUnits.empty()) {
        Unit *su;
        for (un_iter ui = this->getSubUnits(); (su = *ui); ++ui) {
            bool temp = su->Explode(drawit, timeit);
            if (su->GetImageInformation().pExplosion) {
                alldone |= temp;
            }
        }
    }
    if ((configuration().physics.eject_cargo_on_blowup > 0) && (this->numCargo() > 0)) {
        unsigned int dropcount = floorf(
                static_cast<float>(this->numCargo()) / static_cast<float>(configuration().physics.
                    eject_cargo_on_blowup)) +
            1;
        if (dropcount > this->numCargo()) {
            dropcount = this->numCargo();
        }
        for (unsigned int i = 0; i < dropcount; i++) {
            this->EjectCargo(this->numCargo() - 1);
        } //Ejecting the last one is somewhat faster
    }
    return alldone || (!timealldone);
}

float Unit::ExplodingProgress() const {
    const float debrisTime = configuration().physics.debris_time_flt;
    return std::min(pImage->timeexplode / debrisTime, 1.0f);
}

void Unit::SetCollisionParent(Unit *name) {
    assert(0);                                         //deprecated... many fewer collisions with subunits out of the table
}

//This function should not be used on server side
extern vector<Vector> perplines;
extern vector<int> turretcontrol;

float Unit::querySphereClickList(const QVector &st, const QVector &dir, float err) const {
    float retval = 0;
    float adjretval = 0;
    const Matrix *tmpo = &cumulative_transformation_matrix;

    Vector TargetPoint(tmpo->getP());
    for (unsigned int i = 0; i < nummesh(); ++i) {
        TargetPoint = Transform(*tmpo, meshdata[i]->Position());
        Vector origPoint = TargetPoint;

        perplines.push_back(TargetPoint);
        //find distance away from the line now :-)
        //find scale factor of end on start to get line.
        QVector tst = TargetPoint.Cast() - st;
        float k = tst.Dot(dir);
        TargetPoint = (tst - k * (dir)).Cast();
        perplines.push_back(origPoint - TargetPoint);
        if (TargetPoint.Dot(TargetPoint)
                < err * err
                        + meshdata[i]->rSize() * meshdata[i]->rSize() + 2 * err * meshdata[i]->rSize()
                ) {
            if (retval == 0) {
                retval = k;
                adjretval = k;
                if (adjretval < 0) {
                    adjretval += meshdata[i]->rSize();
                    if (adjretval > 0) {
                        adjretval = .001;
                    }
                }
            } else {
                if (retval > 0 && k < retval && k > -meshdata[i]->rSize()) {
                    retval = k;
                    adjretval = k;
                    if (adjretval < 0) {
                        adjretval += meshdata[i]->rSize();
                        if (adjretval > 0) {
                            adjretval = .001;
                        }
                    }
                }
                if (retval < 0 && k + meshdata[i]->rSize() > retval) {
                    retval = k;
                    adjretval = k + meshdata[i]->rSize();
                    if (adjretval > 0) {
                        //THRESHOLD;
                        adjretval = .001;
                    }
                }
            }
        }
    }
    for (un_kiter ui = viewSubUnits(); !ui.isDone(); ++ui) {
        float tmp = (*ui)->querySphereClickList(st, dir, err);
        if (tmp == 0) {
            continue;
        }
        if (retval == 0) {
            retval = tmp;
        } else {
            if (adjretval > 0 && tmp < adjretval) {
                retval = tmp;
                adjretval = tmp;
            }
            if (adjretval < 0 && tmp > adjretval) {
                retval = tmp;
                adjretval = tmp;
            }
        }
    }
    return adjretval;
}

/*
 **********************************************************************************
 **** UNIT_DOCK STUFF
 **********************************************************************************
 */

const std::vector<struct DockingPorts> &Unit::DockingPortLocations() const {
    return pImage->dockingports;
}

bool Unit::EndRequestClearance(Unit *targ) {
    std::vector<Unit *>::iterator lookcleared;
    if ((lookcleared =
            std::find(targ->pImage->clearedunits.begin(), targ->pImage->clearedunits.end(),
                    this)) != targ->pImage->clearedunits.end()) {
        targ->pImage->clearedunits.erase(lookcleared);
        return true;
    } else {
        return false;
    }
}

bool Unit::RequestClearance(Unit *dockingunit) {
    if (std::find(pImage->clearedunits.begin(), pImage->clearedunits.end(), dockingunit)
            == pImage->clearedunits.end()) {
        pImage->clearedunits.push_back(dockingunit);
    }
    return true;
}

void Unit::FreeDockingPort(unsigned int i) {
    if (pImage->dockedunits.size() == 1) {
        docked &= (~DOCKING_UNITS);
    }
    unsigned int whichdock = pImage->dockedunits[i]->whichdock;
    pImage->dockingports[whichdock].Occupy(false);
    pImage->dockedunits[i]->uc.SetUnit(nullptr);
    delete pImage->dockedunits[i];
    pImage->dockedunits.erase(pImage->dockedunits.begin() + i);
}

static Transformation HoldPositionWithRespectTo(Transformation holder,
        const Transformation &changeold,
        const Transformation &changenew) {
    Quaternion bak = holder.orientation;
    holder.position = holder.position - changeold.position;

    Quaternion invandrest = changeold.orientation.Conjugate();
    invandrest *= changenew.orientation;
    holder.orientation *= invandrest;
    Matrix m;

    invandrest.to_matrix(m);
    holder.position = TransformNormal(m, holder.position);

    holder.position = holder.position + changenew.position;
    const bool changeddockedorient = (configuration().physics.change_docking_orientation);
    if (!changeddockedorient) {
        holder.orientation = bak;
    }
    return holder;
}

extern void ExecuteDirector();

void Unit::PerformDockingOperations() {
    for (unsigned int i = 0; i < pImage->dockedunits.size(); ++i) {
        Unit *un;
        if ((un = pImage->dockedunits[i]->uc.GetUnit()) == nullptr) {
            FreeDockingPort(i);
            i--;
            continue;
        }
        un->prev_physical_state = un->curr_physical_state;
        un->curr_physical_state =
                HoldPositionWithRespectTo(un->curr_physical_state, prev_physical_state, curr_physical_state);
        un->NetForce = Vector(0, 0, 0);
        un->NetLocalForce = Vector(0, 0, 0);
        un->NetTorque = Vector(0, 0, 0);
        un->NetLocalTorque = Vector(0, 0, 0);
        un->AngularVelocity = Vector(0, 0, 0);
        un->Velocity = Vector(0, 0, 0);
        if (un == _Universe->AccessCockpit()->GetParent()) {
            ///CHOOSE NEW MISSION
            for (unsigned int i = 0; i < pImage->clearedunits.size(); ++i) {
                //this is a hack because we don't have an interface to say "I want to buy a ship"  this does it if you press shift-c in the base
                if (pImage->clearedunits[i] == un) {
                    pImage->clearedunits.erase(pImage->clearedunits.begin() + i);
                    un->UpgradeInterface(this);
                }
            }
        }
        //now we know the unit's still alive... what do we do to him *G*
        ///force him in a box...err where he is
    }
}

std::set<Unit *> arrested_list_do_not_dereference;

// A simple utility to recharge energy, ftl_energy and shields
// Also to charge for docking and refueling
void rechargeShip(Unit *unit, unsigned int cockpit) {
    unit->fuel.Refill();
    unit->energy.Refill();
    unit->ftl_energy.Refill();
    unit->shield.FullyCharge();

    if (cockpit < 0 || cockpit >= _Universe->numPlayers()) {
        return;
    }

    // Refueling fee
    // TODO: Use a more precise type for credits
    const float refueling_fee = configuration().general.fuel_docking_fee_flt;
    unit->credits -= refueling_fee;

    const float docking_fee = configuration().general.docking_fee_flt;
    unit->credits -= docking_fee;
}


// UTDW - unit to dock with
int Unit::ForceDock(Unit *utdw, unsigned int whichdockport) {
    if (utdw->pImage->dockingports.size() <= whichdockport) {
        return 0;
    }
    utdw->pImage->dockingports[whichdockport].Occupy(true);

    utdw->docked |= DOCKING_UNITS;
    utdw->pImage->dockedunits.push_back(new DockedUnits(this, whichdockport));
    //NETFIXME: Broken on server.
    if (utdw->pImage->dockingports[whichdockport].IsInside()) {
        RemoveFromSystem();
        SetVisible(false);
        docked |= DOCKED_INSIDE;
    } else {
        docked |= DOCKED;
    }
    pImage->DockedTo.SetUnit(utdw);
    computer.set_speed = 0;
    if (this == _Universe->AccessCockpit()->GetParent()) {
        this->RestoreGodliness();
    }

    unsigned int cockpit = UnitUtil::isPlayerStarship(this);

    // Refuel and recharge and charge docking/refueling fees
    rechargeShip(this, cockpit);

    const auto arrested = arrested_list_do_not_dereference.find(this);
    if (arrested != arrested_list_do_not_dereference.end()) {
        arrested_list_do_not_dereference.erase(arrested);
        //do this for jail time
        for (unsigned int j = 0; j < 100000; ++j) {
            for (unsigned int i = 0; i < active_missions.size(); ++i) {
                ExecuteDirector();
            }
        }
    }
    return whichdockport + 1;
}

int Unit::Dock(Unit *utdw) {

    if (docked & (DOCKED_INSIDE | DOCKED)) {
        return 0;
    }
    std::vector<Unit *>::iterator lookcleared;
    if ((lookcleared = std::find(utdw->pImage->clearedunits.begin(),
            utdw->pImage->clearedunits.end(), this)) != utdw->pImage->clearedunits.end()) {
        int whichdockport;
        if ((whichdockport = CanDock(utdw, this)) != -1) {
            utdw->pImage->clearedunits.erase(lookcleared);
            return ForceDock(utdw, whichdockport);
        }
    }
    return 0;
}


bool Unit::IsCleared(const Unit *DockingUnit) const {
    return std::find(pImage->clearedunits.begin(), pImage->clearedunits.end(), DockingUnit)
            != pImage->clearedunits.end();
}

bool Unit::hasPendingClearanceRequests() const {
    return pImage && !pImage->clearedunits.empty();
}

bool Unit::isDocked(const Unit *d) const {
    if (!d) {
        return false;
    }
    if (!(d->docked & (DOCKED_INSIDE | DOCKED))) {
        return false;
    }
    for (const auto & docked_unit : pImage->dockedunits) {
        Unit *un;
        if ((un = docked_unit->uc.GetUnit()) != nullptr) {
            if (un == d) {
                return true;
            }
        }
    }
    return false;
}

extern vector<int> switchunit;
extern vector<int> turretcontrol;

bool Unit::UnDock(Unit *utdw) {
    unsigned int i = 0;
    if (this->name == "return_to_cockpit") {
        if (this->faction == utdw->faction) {
            this->owner = utdw;
        } else {
            this->owner = nullptr;
        }
    }
    VS_LOG(trace, "Asking to undock");
    for (i = 0; i < utdw->pImage->dockedunits.size(); ++i) {
        if (utdw->pImage->dockedunits[i]->uc.GetUnit() == this) {
            utdw->FreeDockingPort(i);
            i--;
            SetVisible(true);
            docked &= (~(DOCKED_INSIDE | DOCKED));
            pImage->DockedTo.SetUnit(nullptr);
            Velocity = utdw->Velocity;
            const float launch_speed = configuration().physics.launch_speed_flt;
            const bool auto_turn_towards = configuration().physics.undock_turn_away;

            if (launch_speed > 0) {
                computer.set_speed = launch_speed;
            }
            if (auto_turn_towards) {
                for (int i = 0; i < 3; ++i) {
                    Vector methem(RealPosition(this) - RealPosition(utdw).Cast());
                    methem.Normalize();
                    Vector p, q, r;
                    GetOrientation(p, q, r);
                    p = methem.Cross(r);
                    float theta = p.Magnitude();
                    if (theta * theta > .00001) {
                        p *= (asin(theta) / theta);
                        Rotate(p);
                        GetOrientation(p, q, r);
                    }
                    if (r.Dot(methem) < 0) {
                        Rotate(p * (PI / theta));
                    }
                }
            }
            if (name == "return_to_cockpit" || this->name == "return_to_cockpit") {
                while (turretcontrol.size() <= _Universe->CurrentCockpit()) {
                    turretcontrol.push_back(0);
                }
                turretcontrol[_Universe->CurrentCockpit()] = 1;
            }
            // Send notification that a ship has undocked from a station
            _Universe->AccessCockpit()->OnDockEnd(utdw, this);
            return true;
        }
    }
    return false;
}

/*
 **********************************************************************************
 **** UNIT_CUSTOMIZE STUFF
 **********************************************************************************
 */
#define UPGRADEOK (1)
#define NOTTHERE (0)
#define CAUSESDOWNGRADE (-1)
#define LIMITEDBYTEMPLATE (-2)

const Unit *getUnitFromUpgradeName(const string &upgradeName, int myUnitFaction = 0);

typedef double (*adder)(double a, double b);
typedef double (*percenter)(double a, double b, double c);
typedef bool (*comparer)(double a, double b);

bool GreaterZero(double a, double b) {
    return a >= 0;
}

double AddUp(double a, double b) {
    return a + b;
}

double MultUp(double a, double b) {
    return a * b;
}

double GetsB(double a, double b) {
    return b;
}

bool AGreaterB(double a, double b) {
    return a > b;
}

double SubtractUp(double a, double b) {
    return a - b;
}

double SubtractClamp(double a, double b) {
    return (a - b < 0) ? 0 : a - b;
}

bool ALessB(double a, double b) {
    return a < b;
}

double computePercent(double old, double upgrade, double newb) {
    if (newb) {
        return old / newb;
    } else {
        return 0;
    }
}

double computeWorsePercent(double old, double upgrade, double isnew) {
    if (old) {
        return isnew / old;
    } else {
        return 1;
    }
}

double computeAdderPercent(double a, double b, double c) {
    return 0;
}

double computeMultPercent(double a, double b, double c) {
    return 0;
}

double computeDowngradePercent(double old, double upgrade, double isnew) {
    if (upgrade) {
        return (old - isnew) / upgrade;
    } else {
        return 0;
    }
}

static int UpgradeFloat(double &result,
        double tobeupgraded,
        double upgrador,
        double templatelimit,
        double (*myadd)(double,
                double),
        bool (*betterthan)(double a,
                double b),
        double nothing,
        double completeminimum,
        double (*computepercentage)(
                double oldvar,
                double upgrador,
                double newvar),
        double &percentage,
        bool forcedowngrade,
        bool usetemplate,
        double at_least_this,
        bool (*atLeastthiscompare)(
                double a,
                double b) = AGreaterB,
        bool clamp = false,
        bool force_nothing = false) {
    //if upgrador is better than nothing
    if (upgrador != nothing || force_nothing) {
        if (clamp) {
            if (tobeupgraded > upgrador) {
                upgrador = tobeupgraded;
            }
        }
        float newsum = (*myadd)(tobeupgraded, upgrador);
        //if we're downgrading
        if (!force_nothing && newsum < tobeupgraded && at_least_this >= upgrador && at_least_this > newsum
                && at_least_this
                        >= tobeupgraded) {
            return newsum == upgrador ? CAUSESDOWNGRADE : NOTTHERE;
        }
        if (newsum != tobeupgraded && (((*betterthan)(newsum, tobeupgraded) || forcedowngrade))) {
            if (((*betterthan)(newsum, templatelimit) && usetemplate) || newsum < completeminimum) {
                if (!forcedowngrade) {
                    return LIMITEDBYTEMPLATE;
                }
                if (newsum < completeminimum) {
                    newsum = completeminimum;
                } else {
                    newsum = templatelimit;
                }
            }
            ///we know we can replace result with newsum
            percentage = (*computepercentage)(tobeupgraded, upgrador, newsum);
            if ((*atLeastthiscompare)(at_least_this, newsum) && (!force_nothing)) {
                if ((*atLeastthiscompare)(at_least_this, tobeupgraded)) {
                    //no shift
                    newsum = tobeupgraded;
                } else {
                    //set it to its min
                    newsum = at_least_this;
                }
            }
            result = newsum;
            return UPGRADEOK;
        } else {
            return CAUSESDOWNGRADE;
        }
    } else {
        return NOTTHERE;
    }
}

int UpgradeBoolval(int a, int upga, bool touchme, bool downgrade, int &numave, double &percentage, bool force_nothing) {
    if (downgrade) {
        if (a && upga) {
            if (touchme) {
                (a = false);
            }
            ++numave;
            ++percentage;
        }
    } else {
        if (!a && upga) {
            if (touchme) {
                a = true;
            }
            ++numave;
            ++percentage;
        } else if (force_nothing && a && !upga) {
            if (touchme) {
                a = false;
            }
            ++numave;
            ++percentage;
        }
    }
    return a;
}

void YoinkNewlines(char *input_buffer) {
    for (int i = 0; input_buffer[i] != '\0'; ++i) {
        if (input_buffer[i] == '\n' || input_buffer[i] == '\r') {
            input_buffer[i] = '\0';
        }
    }
}

bool Quit(const char *input_buffer) {
    if (strcasecmp(input_buffer, "exit") == 0
            || strcasecmp(input_buffer, "quit") == 0) {
        return true;
    }
    return false;
}

using std::string;



bool Unit::UpgradeSubUnits(const Unit *up,
        int subunitoffset,
        bool touchme,
        bool downgrade,
        int &numave,
        double &percentage) {
    return UpgradeSubUnitsWithFactory(up, subunitoffset, touchme, downgrade, numave, percentage, &CreateGenericTurret);
}

bool Unit::UpgradeSubUnitsWithFactory(const Unit *up, int subunitoffset, bool touchme, bool downgrade, int &numave,
        double &percentage, Unit *(*createupgradesubunit)(std::string s,
        int
        faction)) {
    bool can_complete_fully = true;
    int j;
    un_iter ui;
    bool found = false;
    for (j = 0, ui = getSubUnits(); !ui.isDone() && j < subunitoffset; ++ui, ++j) {
    }     ///set the turrets to the offset
    un_kiter upturrets;

    Unit* giveAway = *ui;
    if (giveAway == nullptr) {
        return true;
    }
    bool hasAnyTurrets = false;
    const std::string turSize = getTurretSize(giveAway->name);
    //begin going through other unit's turrets
    for (upturrets = up->viewSubUnits(); ((*upturrets) != nullptr) && ((*ui) != nullptr); ++ui, ++upturrets) {
        hasAnyTurrets = true;

        const Unit* addtome = *upturrets;                    //set pointers

        bool foundthis = false;
        //if the new turret has any size at all
        if (turSize == getTurretSize(addtome->name) && addtome->rSize()
                && (turSize + "_blank" != addtome->name.get())) {
            if (!downgrade || addtome->name == giveAway->name) {
                found = true;
                foundthis = true;
                ++numave;                                //add it
                //add up percentage equal to ratio of sizes
                percentage += (giveAway->rSize() / addtome->rSize());
            }
        }
        if (foundthis) {
            if (touchme) {
                //if we wish to modify,
                Transformation addToMeCur = giveAway->curr_physical_state;
                Transformation addToMePrev = giveAway->prev_physical_state;
                giveAway->Kill();                 //risky??
                ui.remove();                     //remove the turret from the first unit
                //if we are upgrading swap them
                if (!downgrade) {
                    Unit *addToMeNew = (*createupgradesubunit)(addtome->name, addtome->faction);
                    addToMeNew->curr_physical_state = addToMeCur;
                    addToMeNew->SetFaction(faction);
                    addToMeNew->prev_physical_state = addToMePrev;
                    //add unit to your ship
                    ui.preinsert(addToMeNew);
                    //set recursive owner
                    addToMeNew->SetRecursiveOwner(this);
                } else {
                    Unit *un;                            //make garbage unit
                    //NOT 100% SURE A GENERIC UNIT CAN FIT (WAS GAME UNIT CREATION)
                    //give a default do-nothing unit
                    ui.preinsert(un = new Unit("upgrading_dummy_unit", true, faction));
                    un->SetFaction(faction);
                    un->curr_physical_state = addToMeCur;
                    un->prev_physical_state = addToMePrev;
                    un->drive.yaw = 0;
                    un->drive.pitch = 0;
                    un->drive.roll = 0;
                    un->drive.lateral = un->drive.retro = un->drive.forward = un->afterburner.thrust = 0.0;

                    un->name = turSize + "_blank";
                    if (un->pImage->unitwriter != nullptr) {
                        un->pImage->unitwriter->setName(un->name);
                    }
                    un->SetRecursiveOwner(this);
                }
            }
        }
    }
    if (!found) {
        return !hasAnyTurrets;
    }
    if ((*upturrets) != nullptr) {
        return false;
    }
    return can_complete_fully;
}

static void GCCBugCheckFloat(float *f, int offset) {
    if (f[offset] > 1) {
        f[offset] = 1;
    }          //keep it real
}

bool Unit::canUpgrade(const Unit *upgrador,
        int mountoffset,
        int subunitoffset,
        int additive,
        bool force,
        double &percentage,
        const Unit *templ,
        bool force_change_on_nothing,
        bool gen_downgrade_list) {
    return UpAndDownGrade(upgrador,
            templ,
            mountoffset,
            subunitoffset,
            false,
            false,
            additive,
            force,
            percentage,
            this,
            force_change_on_nothing,
            gen_downgrade_list);
}

bool Unit::Upgrade(const Unit *upgrador,
        int mountoffset,
        int subunitoffset,
        int additive,
        bool force,
        double &percentage,
        const Unit *templ,
        bool force_change_on_nothing,
        bool gen_downgrade_list) {
    return UpAndDownGrade(upgrador,
            templ,
            mountoffset,
            subunitoffset,
            true,
            false,
            additive,
            force,
            percentage,
            this,
            force_change_on_nothing,
            gen_downgrade_list);
}

bool Unit::canDowngrade(const Unit* downgradeor,
                        int mountoffset,
                        int subunitoffset,
                        double& percentage,
                        const Unit* downgradelimit,
                        bool gen_downgrade_list) {
    return UpAndDownGrade(downgradeor,
                          nullptr,
                          mountoffset,
                          subunitoffset,
                          false,
                          true,
                          false,
                          true,
                          percentage,
                          downgradelimit,
                          false,
                          gen_downgrade_list);
}

bool Unit::Downgrade(const Unit *downgradeor,
        int mountoffset,
        int subunitoffset,
        double &percentage,
        const Unit *downgradelimit,
        bool gen_downgrade_list) {
    return UpAndDownGrade(downgradeor,
                          nullptr,
                          mountoffset,
                          subunitoffset,
                          true,
                          true,
                          false,
                          true,
                          percentage,
                          downgradelimit,
                          false,
                          gen_downgrade_list);
}

class DoubleName {
public:
    string s;
    double d;

    DoubleName(string ss, double dd) {
        d = dd;
        s = std::move(ss);
    }

    DoubleName() {
        d = -FLT_MAX;
    }
};

extern int GetModeFromName(const char *);

extern Unit *CreateGameTurret(std::string tur, int faction);

extern char *GetUnitDir(const char *);

double Unit::Upgrade(const std::string &file,
        int mountoffset,
        int subunitoffset,
        bool force,
        bool loop_through_mounts) {
    int upgradefac = FactionUtil::GetUpgradeFaction();
    const Unit *up = UnitConstCache::getCachedConst(StringIntKey(file, upgradefac));
    if (!up) {
        up = UnitConstCache::setCachedConst(StringIntKey(file, upgradefac),
                new Unit(file.c_str(), true, upgradefac));
    }

    char *unitdir = GetUnitDir(this->name.get().c_str());
    string templnam = string(unitdir) + ".template";
    const Unit *templ = UnitConstCache::getCachedConst(StringIntKey(templnam, this->faction));
    if (templ == nullptr) {
        templ =
                UnitConstCache::setCachedConst(StringIntKey(templnam,
                                this->faction),
                        new Unit(templnam.c_str(), true, this->faction));
    }
    free(unitdir);
    double percentage = 0;
    if (up->name != "LOAD_FAILED") {
        for (int i = 0; percentage == 0; ++i) {
            if (!this->Unit::Upgrade(up, mountoffset + i, subunitoffset + i,
                    GetModeFromName(file.c_str()), force, percentage,
                    ((templ->name == "LOAD_FAILED") ? nullptr : templ),
                    false, false)) {
                percentage = 0;
            }
            if (!loop_through_mounts || (i + 1 >= this->getNumMounts()) || percentage > 0) {
                break;
            }
        }
    }

    return percentage;
}

vsUMap<int, DoubleName> downgrademap;
int curdowngrademapoffset = 5 * sizeof(Unit);

bool AddToDowngradeMap(std::string name, double value, int unitoffset, vsUMap<int, DoubleName> &tempdowngrademap) {
    using vsUMap;
    const auto i = downgrademap.find(unitoffset);
    if (i != downgrademap.end()) {
        if (i->second.d <= value) {
            tempdowngrademap[unitoffset] = DoubleName(name, value);
            return true;
        }
    } else {
        tempdowngrademap[unitoffset] = DoubleName(name, value);
        return true;
    }
    return false;
}

void ClearDowngradeMap() {
    downgrademap.clear();
}

std::set<std::string> GetListOfDowngrades() {
    using vsUMap;
    std::set<std::string> retval;
    for (auto i = downgrademap.begin(); i != downgrademap.end(); ++i) {
        retval.insert(i->second.s);
    }
    return retval;
}

typedef vsUMap<const char *, bool> UnitHasRecursiveData;
typedef vsUMap<std::string, UnitHasRecursiveData> FactionHasRecursiveData;
typedef std::vector<FactionHasRecursiveData> HasRecursiveData;

static HasRecursiveData has_recursive_data;
static std::string upgradeString("Upgrades");

static bool cell_has_recursive_data(const string &name, unsigned int fac, const char *key) {
    if (fac < has_recursive_data.size()) {
        FactionHasRecursiveData::const_iterator iter = has_recursive_data[fac].find(name);
        if (iter != has_recursive_data[fac].end()) {
            const auto iter2 = iter->second.find(key);
            if (iter2 != iter->second.end()) {
                return iter2->second;
            }
        }
    } else {
        has_recursive_data.resize(fac + 1);
    }
    bool retval = false;
    const string faction = FactionUtil::GetFactionName(fac);
    string lkey = key;
    string::size_type lkstart = 0;
    string::size_type lkend = lkey.find('|');
    //Big short circuit - avoids recursion
    while (!retval && (lkstart != string::npos)) {
        string skey = lkey.substr(lkstart, (lkend == string::npos) ? string::npos : lkend - lkstart);
        string lus = UniverseUtil::LookupUnitStat(name, faction, skey);

        retval = !lus.empty();

        lkstart = (lkend != string::npos) ? lkend + 1 : string::npos;
        lkend = lkey.find('|', lkstart);
    }
    if (!retval) {
        //Big short circuit - avoids recursion
        string::size_type when;
        string upgrades = UniverseUtil::LookupUnitStat(name, faction, upgradeString);
        string::size_type ofs = 0;
        while (!retval && ((when = upgrades.find('{', ofs)) != string::npos)) {
            const string::size_type where = upgrades.find('}', when + 1);
            string upgrade = upgrades.substr(when + 1, ((where != string::npos) ? where - when - 1 : string::npos));
            retval = cell_has_recursive_data(upgrade, fac, key);
            ofs = where + 1;
        }
    }
    has_recursive_data[fac][name][key] = retval;
    return retval;
}

// TODO: get rid of this
extern float accelStarHandler(float &input);
float speedStarHandler(float &input);

/* Stopgap measure - upgrade/downgrade cargo holds/upgrade spaces */
bool UpAndDownCargoHoldAndUpgradeSpace(Unit *unit, float cargo_volume, float hidden_volume,
                                       float upgrade_space, bool is_upgrade, bool do_upgrade) {
    float multiple = (is_upgrade ? 1.0f : -1.0f);

    // Check if this is actually a cargo/upgrade space upgrade
    if(cargo_volume == 0 && hidden_volume == 0 && upgrade_space == 0) {
        return false;
    }

    // Just checking if we can, and we can always upgrade/downgrade
    if(!do_upgrade) {
        return true;
    }

    // if(cargo_volume != 0) {
    //     unit->CargoVolume += cargo_volume * multiple;
    // }

    // if(hidden_volume != 0) {
    //     unit->HiddenCargoVolume += hidden_volume * multiple;
    // }

    // if(upgrade_space != 0) {
    //     unit->UpgradeVolume += upgrade_space * multiple;
    // }

    return true;
}


bool Unit::UpAndDownGrade(const Unit *up,
        const Unit *templ,
        int mountoffset,
        int subunitoffset,
        bool touchme,
        bool downgrade,
        int additive,
        bool forcetransaction,
        double &percentage,
        const Unit *downgradelimit,
        bool force_change_on_nothing,
        bool gen_downgrade_list) {
    // New Code
    UpgradeOperationResult result = UpgradeUnit(up->name, !downgrade, touchme);
    if(result.upgradeable) {
        percentage = result.percent;
        return result.success;
    }

    // Old Code
    percentage = 0;

    int numave = 0;
    bool cancompletefully = true;
    if (mountoffset >= 0) {
        cancompletefully = UpgradeMounts(up, mountoffset, touchme, downgrade, numave, percentage);
    }
    bool cancompletefully1 = true;
    if (subunitoffset >= 0) {
        cancompletefully1 = UpgradeSubUnits(up, subunitoffset, touchme, downgrade, numave, percentage);
    }
    cancompletefully = cancompletefully && cancompletefully1;
    vsUMap<int, DoubleName> tempdownmap;
    if (cancompletefully && cancompletefully1 && downgrade) {
        if (percentage > 0) {
            AddToDowngradeMap(up->name, 1, curdowngrademapoffset++, tempdownmap);
        }
    }

    //NO CLUE FOR BELOW
    if (downgrade) {
    } else {
        //we are upgrading!
        if (touchme) {
            for (const Cargo cargo : up->cargo_hold.GetItems()) {
                if (upgrade_space.CanAddCargo(cargo)) {
                    upgrade_space.AddCargo(this, cargo, false);
                }
            }
        }
    }

    if (0 == numave) {      //Doesn't upgrade anything -- JS_NUDGE -- may want to revisit this later
        percentage = 1.0;
    }
    if (numave) {
        percentage = percentage / numave;
    }

    // TODO: intertial dampener component - reduces mass

    if (gen_downgrade_list) {
        if (downgrade && percentage > configuration().general.remove_downgrades_less_than_percent_dbl) {
            for (auto each_downgrade = tempdownmap.begin(); each_downgrade != tempdownmap.end(); ++each_downgrade) {
                downgrademap[each_downgrade->first] = each_downgrade->second;
            }
        }
    }
    return cancompletefully;
}




Vector Unit::MountPercentOperational(int whichmount) {
    if (whichmount < 0 || static_cast<unsigned int>(whichmount) >= mounts.size()) {
        return Vector(-1, -1, -1);
    }
    return Vector(mounts[whichmount].functionality,
            mounts[whichmount].maxfunctionality,
            ((mounts[whichmount].status == Mount::ACTIVE || mounts[whichmount].status
                    == Mount::INACTIVE) ? 0.0 : (mounts[whichmount].status == Mount::UNCHOSEN ? 2.0 : 1.0)));
}

// TODO: remove function
// We no longer do repair through basic repair.
// Kept for compatibility with python API.
int Unit::RepairCost() {
    return 0;
}

// This was called when performing a BASIC_REPAIR
// This function doesn't do anything anymore
// Kept for compatibility with python API.
int Unit::RepairUpgrade() {
    // TODO: remove
    return 1;
}


//item must be non-null... but baseUnit or credits may be NULL.
bool Unit::RepairUpgradeCargo(Cargo *item, Unit *baseUnit) {
    assert((item != nullptr) | !"Unit::RepairUpgradeCargo got a null item."); //added by chuck_starchaser
    double itemPrice = 1; //baseUnit ? baseUnit->PriceCargo(item->GetName()) : item->GetPrice();

    // New repair
    if(RepairUnit(item->GetName())) {
        double repair_price = item->RepairPrice();
        ComponentsManager::credits -= repair_price;

        GenerateHudText(getDamageColor);
        return true;
    }

    if (item->IsWeapon()) {
        const Unit *upgrade = getUnitFromUpgradeName(item->GetName(), this->faction);
        if (upgrade->getNumMounts()) {
            double price = itemPrice; //RepairPrice probably won't work for mounts.
            if (price <= ComponentsManager::credits) {
                ComponentsManager::credits -= price;

                const Mount *mnt = &upgrade->mounts[0];
                unsigned int nummounts = this->getNumMounts();
                bool complete = false;
                for (unsigned int i = 0; i < nummounts; ++i) {
                    if (mnt->type->name == this->mounts[i].type->name) {
                        if (this->mounts[i].status == Mount::DESTROYED) {
                            this->mounts[i].status = Mount::INACTIVE;
                            complete = true;
                        }
                        if (this->mounts[i].functionality < 1.0f) {
                            this->mounts[i].functionality = 1.0f;
                            complete = true;
                        }
                        if (this->mounts[i].maxfunctionality < 1.0f) {
                            this->mounts[i].maxfunctionality = 1.0f;
                            complete = true;
                        }
                        if (complete) {
                            break;
                        }
                    }
                }
                return complete;
            }
        }
        return false;
    } else {
        Cargo sold;
        bool notadditive = (item->GetName().find("add_") != 0 && item->GetName().find("mult_") != 0);
        if (notadditive || item->GetCategory().find(DamagedCategory) == 0) {
            Cargo itemCopy = *item;                 //Copy this because we reload master list before we need it.
            const Unit *un = getUnitFromUpgradeName(item->GetName(), this->faction);
            if (un) {
                double percentage = UnitUtil::PercentOperational(*item, this, item->GetName(), item->GetCategory(), false);
                double price = item->RepairPrice();
                if (price <= ComponentsManager::credits) {
                    ComponentsManager::credits -= price;

                    // TODO: look closer at this
                    //if (notadditive) {
                    //    this->Upgrade(un, 0, 0, 0, true, percentage, makeTemplateUpgrade(this->name, this->faction));
                    //}

                    // This code changes the category of the item from "upgrades/Damaged/" to the original category.
                    // if (item->GetCategory().find(DamagedCategory) == 0) {
                    //     int index = this->upgrade_space.GetIndex(itemCopy);
                    //     Cargo c = this->upgrade_space.GetCargo(index);
                    //     if (!c.IsNullCargo()) {
                    //         c.SetCategory("upgrades/" + c.GetCategory().substr(strlen(DamagedCategory)));
                    //     }
                    // }
                    return true;
                }
            }
        }
    }
    return false;
}


// TODO: something with this comment.
/***************** UNCOMMENT GETMASTERPARTLIST WHEN MODIFIED FACTION STUFF !!!!!! */



static const GFXColor disable(1, 0, 0, 1);
extern int GetModeFromName(const char *);

vector<CargoColor> &Unit::FilterDowngradeList(vector<CargoColor> &mylist, bool downgrade) {
    const Unit *templ = nullptr;
    const Unit *downgradelimit = nullptr;
    const bool staticrem = configuration().general.remove_impossible_downgrades;
    int upgrfac = FactionUtil::GetUpgradeFaction();
    for (unsigned int i = 0; i < mylist.size(); ++i) {
        bool removethis = true /*staticrem*/;
        int mode = GetModeFromName(mylist[i].cargo.GetName().c_str());
        if (mode != 2 || (!downgrade)) {
            const Unit *NewPart =
                    UnitConstCache::getCachedConst(StringIntKey(mylist[i].cargo.GetName().c_str(), upgrfac));
            if (!NewPart) {
                NewPart = UnitConstCache::setCachedConst(StringIntKey(
                                mylist[i].cargo.GetName(),
                                upgrfac),
                        new Unit(mylist[i].cargo.GetName().c_str(), false,
                                upgrfac));
            }
            if (NewPart->name == string("LOAD_FAILED")) {
                const Unit *NewPart =
                        UnitConstCache::getCachedConst(StringIntKey(mylist[i].cargo.GetName(), faction));
                if (!NewPart) {
                    NewPart = UnitConstCache::setCachedConst(StringIntKey(mylist[i].cargo.GetName().c_str(), faction),
                            new Unit(mylist[i].cargo.GetName().c_str(),
                                    false, faction));
                }
            }
            if (NewPart->name != string("LOAD_FAILED")) {
                int maxmountcheck = NewPart->getNumMounts() ? getNumMounts() : 1;
                char *unitdir = GetUnitDir(name.get().c_str());
                string templnam = string(unitdir) + ".template";
                string limiternam = string(unitdir) + ".blank";
                if (!downgrade) {
                    templ = UnitConstCache::getCachedConst(StringIntKey(templnam, faction));
                    if (templ == nullptr) {
                        templ =
                                UnitConstCache::setCachedConst(StringIntKey(templnam,
                                                faction),
                                        new Unit(templnam.c_str(), true, this->faction));
                    }
                    if (templ->name == std::string("LOAD_FAILED")) {
                        templ = nullptr;
                    }
                } else {
                    downgradelimit = UnitConstCache::getCachedConst(StringIntKey(limiternam, faction));
                    if (downgradelimit == nullptr) {
                        downgradelimit = UnitConstCache::setCachedConst(StringIntKey(limiternam,
                                        faction),
                                new Unit(limiternam.c_str(), true,
                                        this->faction));
                    }
                    if (downgradelimit->name == std::string("LOAD_FAILED")) {
                        downgradelimit = nullptr;
                    }
                }
                free(unitdir);
                for (int m = 0; m < maxmountcheck; ++m) {
                    int s = 0;
                    for (un_iter ui = getSubUnits(); s == 0 || ((*ui) != nullptr); ++ui, ++s) {
                        double percent = 1;
                        if (downgrade) {
                            if (canDowngrade(NewPart, m, s, percent, downgradelimit)) {
                                if (percent > configuration().general.remove_downgrades_less_than_percent_dbl) {
                                    removethis = false;
                                    break;
                                }
                            }
                        } else if (canUpgrade(NewPart, m, s, mode, false /*force*/, percent, templ)) {
                            removethis = false;
                            break;
                        }
                        if (*ui == nullptr) {
                            break;
                        }
                    }
                }
            }
        } else {
            removethis = true;
        }
        if (removethis) {
            if (downgrade && staticrem) {
                mylist.erase(mylist.begin() + i);
                i--;
            } else {
                mylist[i].color = disable;
            }
        }
    }
    return mylist;
}

vector<CargoColor> &Unit::FilterUpgradeList(vector<CargoColor> &mylist) {
    const bool filtercargoprice = configuration().cargo.filter_expensive_cargo;
    if (filtercargoprice) {
        for (auto & cargo_color : mylist) {
            if (cargo_color.cargo.GetPrice() > credits) {
                cargo_color.color = disable;
            }
        }
    }
    return FilterDowngradeList(mylist, false);
}

bool Unit::IsBase() const {
    return ((flightgroup != nullptr) && (flightgroup->name == "Base"));
}

void Unit::TurretFAW() {
    turretstatus = 3;
    Unit *un;
    for (un_iter iter = getSubUnits(); (un = *iter); ++iter) {
        if (!CheckAccessory(un)) {
            un->EnqueueAIFirst(new Orders::FireAt(configuration().ai.firing.aggressivity_flt));
            un->EnqueueAIFirst(new Orders::FaceTarget(false, 3));
        }
        un->TurretFAW();
    }
}

UnitImages<void> &Unit::GetImageInformation() {
    return *pImage;
}

Unit &GetUnitMasterPartList() {
    return *getMasterPartList();
}

bool myless(const Cargo &a, const Cargo &b) {
    return a < b;
}


void Unit::ImportPartList(const std::string &category, float price, float pricedev, float quantity, float quantdev) {
    Manifest category_manifest = Manifest::MPL().GetCategoryManifest(category);
    std::vector<Cargo> cargo_list = category_manifest.GetItems();

    // Find the minimum and maximum prices in the cargo list
    // We start with extreme values but at the end, min < max
    float min_cargo_price = FLT_MAX;
    float max_cargo_price = 0.0f;
    for (const Cargo& c : cargo_list) {
        float price = c.GetPrice();
        if (price < min_cargo_price) {
            min_cargo_price = price;
        }
        if (price > max_cargo_price) {
            max_cargo_price = price;
        }
    }


    for (const Cargo& cargo : cargo_list) {
        Cargo c = cargo; // Copy the cargo item
        const float aveweight = fabs(configuration().cargo.price_recenter_factor_flt);
        c.SetQuantity(float_to_int(quantity - quantdev));
        float baseprice = c.GetPrice();
        c.SetPrice(c.GetPrice() * (price - pricedev));

        //stupid way
        c.SetQuantity(c.GetQuantity() + float_to_int((quantdev * 2 + 1) * static_cast<double>(rand()) / (static_cast<double>(RAND_MAX) + 1)));
        c.SetPrice(c.GetPrice() + pricedev * 2 * static_cast<float>(rand()) / RAND_MAX);
        c.SetPrice(fabs(c.GetPrice()));
        c.SetPrice((c.GetPrice() + (baseprice * aveweight)) / (aveweight + 1));
        if (c.GetQuantity() <= 0) {
            c.SetQuantity(0);
        }
            //quantity more than zero
        else if (max_cargo_price > min_cargo_price + .01) {
            float renormprice = (baseprice - min_cargo_price) / (max_cargo_price - min_cargo_price);
            const float maxpricequantadj = configuration().cargo.max_price_quant_adj_flt;
            const float minpricequantadj = configuration().cargo.min_price_quant_adj_flt;
            const float powah = configuration().cargo.price_quant_adj_power_flt;
            renormprice = std::pow(renormprice, powah);
            renormprice *= (maxpricequantadj - minpricequantadj);
            renormprice += 1;
            if (renormprice > .001) {
                c.SetQuantity(c.GetQuantity() / float_to_int(renormprice));
                if (c.GetQuantity() < 1) {
                    c.SetQuantity(1);
                }
            }
        }
        const float minprice = configuration().cargo.min_cargo_price_flt;
        if (c.GetPrice() < minprice) {
            c.SetPrice(minprice);
        }

        cargo_hold.AddCargo(this, c, false);

    }
}






void Unit::setUnitRole(const std::string &s) {
    unit_role = ROLES::getRole(s);
}

void Unit::setAttackPreference(const std::string &s) {
    attack_preference = ROLES::getRole(s);
}

const std::string &Unit::getUnitRole() const {
    return ROLES::getRole(unit_role);
}

const std::string &Unit::getAttackPreference() const {
    return ROLES::getRole(attack_preference);
}

//legacy function for python
void Unit::setCombatRole(const std::string &s) {
    unit_role = ROLES::getRole(s);
    attack_preference = ROLES::getRole(s);
}

//legacy function for python
const std::string &Unit::getCombatRole() const {
    static unsigned char inert = ROLES::getRole("INERT");
    unsigned char retA = unit_role;
    unsigned char retB = attack_preference;

    //often missions used this to render items either uninteresting or not attacking...so want to prioritize that behavior
    if (retA == inert || retB == inert) {
        static const std::string INERT("INERT");
        return INERT;
    }

    return ROLES::getRole(retA);
}

using XMLSupport::tostring;
using std::string;

/*
 **************************************************************************************
 *** UNIT_REPAIR STUFF                                                               **
 **************************************************************************************
 */



#define REPAIRINTEGRATED(functionality, max_functionality) \
    do {                                                     \
        if (functionality < max_functionality)               \
        {                                                    \
            (functionality) += ammt_repair;                  \
            if ( (functionality) > (max_functionality) )     \
                (functionality) = (max_functionality);       \
        }                                                    \
    }                                                        \
    while (0)

// This is called every cycle - repair in flight by droids
// TODO: move this to RepairBot
void Unit::Repair() {
    if(getUnitType() == Vega_UnitType::planet) {
        return;
    }

    // TODO: everything below here needs to go when we're done with lib_components
    const float repairtime = configuration().physics.repair_droid_time_flt;
    const float checktime = configuration().physics.repair_droid_check_time_flt;
    if ((repairtime <= 0) || (checktime <= 0)) {
        return;
    }

    if (repair_bot.Get() == 0) {
        return;
    }

    /*if (next_repair_time == -FLT_MAX || next_repair_time <= UniverseUtil::GetGameTime()) {
        unsigned int numcargo = numCargo();
        if (numcargo > 0) {
            if (next_repair_cargo >= numCargo()) {
                next_repair_cargo = 0;
            }
            Cargo *carg = &GetCargo(next_repair_cargo);
            float percentoperational = 1;
            if (carg->GetCategory().find("upgrades/") == 0
                    && carg->GetCategory().find(DamagedCategory) != 0
                    && carg->GetName().find("add_") != 0
                    && carg->GetName().find("mult_") != 0
                    && ((percentoperational =
                            UnitUtil::PercentOperational(this, carg->GetName(), carg->GetCategory(), true)) < 1.f)) {
                if (next_repair_time == -FLT_MAX) {
                    next_repair_time =
                            UniverseUtil::GetGameTime() + repairtime * (1 - percentoperational) / repair_bot.Get();
                } else {
                    //ACtually fix the cargo here
                    static int upfac = FactionUtil::GetUpgradeFaction();
                    const Unit *up = getUnitFromUpgradeName(carg->GetName(), upfac);
                    static std::string loadfailed("LOAD_FAILED");
                    if (up->name == loadfailed) {
                        VS_LOG(info,
                                "Bug: Load failed cargo encountered: report on https://github.com/vegastrike/Vega-Strike-Engine-Source");
                    } else {
                        //don't want to repair these things
                        if (up->SubUnits.empty() && up->getNumMounts() == 0) {
                            double percentage = 0;
                            this->Upgrade(up, 0, 0, 0, true, percentage, makeTemplateUpgrade(this->name,
                                                                                             this->faction), false,
                                          false);
                            if (percentage == 0) {
                                VS_LOG(error,
                                        (boost::format(
                                                "Failed repair for unit %1%, cargo item %2%: %3% (%4%) - please report error")
                                                % name.get().c_str()
                                                % next_repair_cargo
                                                % carg->GetName().c_str()
                                                % carg->GetCategory().c_str()));
                            }
                        }
                    }
                    next_repair_time = -FLT_MAX;
                    ++(next_repair_cargo);
                }
            } else {
                ++(next_repair_cargo);
            }
        }
    }*/

    // TODO: Figure out why these variables are unused, and either use them or delete them
    float ammt_repair = simulation_atom_var / repairtime * repair_bot.Get();

    unsigned int numg = (1 + UnitImages<void>::NUMGAUGES + MAXVDUS);
    unsigned int which = vsrandom.genrand_int31() % numg;
    const float hud_repair_quantity = configuration().physics.hud_repair_unit_flt;

    if (mounts.size()) {
        const float mount_repair_quantity = configuration().physics.mount_repair_unit_flt;
        unsigned int i = vsrandom.genrand_int31() % mounts.size();
        if (mounts[i].functionality < mounts[i].maxfunctionality) {
            mounts[i].functionality += mount_repair_quantity;
            if (mounts[i].functionality > mounts[i].maxfunctionality) {
                mounts[i].functionality = mounts[i].maxfunctionality;
            }
        }
    }
}

#undef REPAIRINTEGRATED

bool Unit::isTractorable(enum tractorHow how) const {
    if (how != tractorImmune) {
        return (getTractorability() & how) == how;
    } else {
        return getTractorability() == tractorImmune;
    }
}

void Unit::setTractorability(enum tractorHow how) {
    tractorability_flags = how;
}

enum Unit::tractorHow Unit::getTractorability() const {
    static bool tractorability_mask_init = false;
    static unsigned char tractorability_mask;
    if (!tractorability_mask_init) {
        std::string stractorability_mask = vs_config->getVariable("physics", "PlayerTractorabilityMask", "p");
        if (!stractorability_mask.empty()) {
            tractorability_mask = tractorImmune;
            if (stractorability_mask.find_first_of("pP") != string::npos) {
                tractorability_mask |= tractorPush;
            }
            if (stractorability_mask.find_first_of("iI") != string::npos) {
                tractorability_mask |= tractorIn;
            }
        } else {
            tractorability_mask = tractorPush;
        }
        tractorability_mask_init = true;
    }
    unsigned char tflags;
    if (_Universe->isPlayerStarship(this) != nullptr) {
        tflags = tractorability_flags & tractorability_mask;
    } else {
        tflags = tractorability_flags;
    }
    return static_cast<Unit::tractorHow>(tflags);
}

void Unit::RequestPhysics() {
    //Request ASAP physics
    if (getStarSystem()) {
        getStarSystem()->RequestPhysics(this, cur_sim_queue_slot);
    }
}

static float parseFloat(const std::string &s) {
    if (s.empty()) {
        return 0.0f;
    }

    return XMLSupport::parse_floatf(s);
}

static inline void parseFloat4(const std::string &s, float value[4]) {
    string::size_type ini = 0, end;
    int i = 0;
    while (i < 4 && ini != string::npos) {
        value[i++] = parseFloat(s.substr(ini, end = s.find_first_of(',', ini)));
        ini = ((end == string::npos) ? end : (end + 1));
    }
    //if (i >= 4 && ini != string::npos) {
    //VS_LOG(info, (boost::format("WARNING: invalid float4: %1%") % s));
    //}
    while (i < 4) {
        value[i++] = 0;
    }
}

void Unit::applyTechniqueOverrides(const std::map<std::string, std::string> &overrides) {
    //for (vector<Mesh*>::iterator mesh = this->meshdata.begin(); mesh != this->meshdata.end(); ++mesh) {
    for (Mesh *mesh : meshdata) {
        if (mesh == nullptr) {
            continue;
        }

        // First check to see if the technique holds any parameter being overridden
        TechniquePtr technique = mesh->getTechnique();

        if (technique.get() == nullptr) {
            continue;
        }

        // Can't be any lower, or goto won't work
        TechniquePtr newtechnique;

        for (int passno = 0; passno < technique->getNumPasses(); ++passno) {
            const Pass &pass = technique->getPass(passno);
            for (size_t paramno = 0; paramno < pass.getNumShaderParams(); ++paramno) {
                if (overrides.count(pass.getShaderParam(paramno).name) > 0) {
                    goto do_not_override;
                }
            }
        }

        // Prepare a new technique with the overrides
        // (make sure the technique has been compiled though -
        // parameter values don't really need recompilation)
        newtechnique = boost::make_shared<Technique>(*technique);
        for (int passno = 0; passno < technique->getNumPasses(); ++passno) {
            Pass &pass = technique->getPass(passno);
            for (size_t paramno = 0; paramno < pass.getNumShaderParams(); ++paramno) {
                Pass::ShaderParam &param = pass.getShaderParam(paramno);
                auto override = overrides.find(param.name);
                if (override != overrides.end()) {
                    parseFloat4(override->second, param.value);
                }
            }
        }

        mesh->setTechnique(newtechnique);

        do_not_override:;
    }
}

std::map<string, Unit *> Drawable::Units;



unsigned int Drawable::unitCount = 0;

void Unit::ActTurn() {
    // Dock
    if (docked & DOCKING_UNITS) {
        PerformDockingOperations();
    }

    // Repair Ship
    Repair();

}

void Unit::UpdatePhysics2(const Transformation &trans,
        const Transformation &old_physical_state,
        const Vector &accel,
        float difficulty,
        const Matrix &transmat,
        const Vector &cum_vel,
        bool lastframe,
        UnitCollection *uc) {
    Movable::UpdatePhysics2(trans, old_physical_state, accel, difficulty, transmat, cum_vel, lastframe, uc);

    this->AddVelocity(difficulty);

#ifdef DEPRECATEDPLANETSTUFF
                                                                                                                            if (planet) {
        Matrix basis;
        curr_physical_state.to_matrix( this->cumulative_transformation_matrix );
        Vector p, q, r, c;
        MatrixToVectors( this->cumulative_transformation_matrix, p, q, r, c );
        planet->trans->InvTransformBasis( this->cumulative_transformation_matrix, p, q, r, c );
        planet->cps = Transformation::from_matrix( this->cumulative_transformation_matrix );
    }
#endif
    this->cumulative_transformation = this->curr_physical_state;
    this->cumulative_transformation.Compose(trans, transmat);
    this->cumulative_transformation.to_matrix(this->cumulative_transformation_matrix);
    this->cumulative_velocity = TransformNormal(transmat, this->Velocity) + cum_vel;
    if (lastframe) {
        unsigned int n;
        unsigned int i;
        char tmp = 0;
        //double blah = queryTime();
        for (i = 0, n = this->meshdata.size(); i < n; i++) {
            if (!this->meshdata[i]) {
                continue;
            }
            if (!this->meshdata[i]->HasBeenDrawn()) {
                this->meshdata[i]->UpdateFX(simulation_atom_var /*SIMULATION_ATOM?*/ );
            } else {
                this->meshdata[i]->UnDraw();
                tmp = 1;
            }
        }
        //double blah1 = queryTime();
        if (!tmp && this->Destroyed()) {
            Explode(false, simulation_atom_var /*SIMULATION_ATOM?*/ );
        }
        //double blah2 = queryTime();
    }
}

/****************************** ONLY SOUND/GFX STUFF LEFT IN THOSE FUNCTIONS *********************************/




Vector Unit::ResolveForces(const Transformation &trans, const Matrix &transmat) {
#ifndef PERFRAMESOUND
    //AUDAdjustSound( this->sound->engine, this->cumulative_transformation.position, this->cumulative_velocity );
    adjustSound(SoundType::engine);
#endif
    return Movable::ResolveForces(trans, transmat);
}

void Unit::UpdatePhysics3(const Transformation &trans,
        const Matrix &transmat,
        bool lastframe,
        UnitCollection *uc,
        Unit *superunit) {
    ActTurn();

    static CloakingStatus previous_status = cloak.status;
    cloak.Update();

    // Play once per cloaking
    if(cloak.Cloaking() && previous_status != CloakingStatus::cloaking) {
        previous_status = cloak.status;
        playSound(SoundType::cloaking);
    } else if(cloak.Cloaked() && previous_status != CloakingStatus::cloaked) {
        previous_status = cloak.status;
        adjustSound(SoundType::cloaking, cumulative_transformation.position, cumulative_velocity);
    }



    bool is_player_ship = _Universe->isPlayerStarship(this);

    reactor.Generate();
    drive.Consume();

    shield.Regenerate(is_player_ship);
    ecm.Consume();
    DecreaseWarpEnergyInWarp();

    if (lastframe) {
        if (!(docked & (DOCKED | DOCKED_INSIDE))) {
            //the AIscript should take care
            prev_physical_state = curr_physical_state;
        }
#ifdef FIX_TERRAIN
                                                                                                                                if (planet) {
          if (!planet->dirty)
              SetPlanetOrbitData( NULL );
          else
              planet->pps = planet->cps;
      }
#endif
    }

    float difficulty;
    Cockpit *player_cockpit = GetVelocityDifficultyMult(difficulty);
    const float EXTRA_CARGO_SPACE_DRAG = configuration().physics.extra_space_drag_for_cargo_flt;
    if (EXTRA_CARGO_SPACE_DRAG > 0) {
        int upgfac = FactionUtil::GetUpgradeFaction();
        if ((this->faction == upgfac) || (this->name == "eject") || (this->name == "Pilot")) {
            Velocity = Velocity * (1 - EXTRA_CARGO_SPACE_DRAG);
        }
    }

    float dist_sqr_to_target = FLT_MAX;
    Unit *target = Target();
    bool increase_locking = false;
    if (target && !cloak.Cloaked()) {
        if (target->getUnitType() != Vega_UnitType::planet) {
            Vector TargetPos(InvTransform(cumulative_transformation_matrix, (target->Position())).Cast());
            dist_sqr_to_target = TargetPos.MagnitudeSquared();
            TargetPos.Normalize();
            if (TargetPos.k > radar.lock_cone) {
                increase_locking = true;
            }
        }
        /* Update the velocity reference to the nearer significant unit/planet. */
        if (!computer.force_velocity_ref && activeStarSystem) {
            Unit *nextVelRef = activeStarSystem->nextSignificantUnit();
            if (nextVelRef) {
                if (computer.velocity_reference) {
                    double dist = UnitUtil::getSignificantDistance(this, VelocityReference());
                    double next_dist = UnitUtil::getSignificantDistance(this, nextVelRef);
                    if (next_dist < dist) {
                        computer.velocity_reference = nextVelRef;
                    }
                } else {
                    computer.velocity_reference = nextVelRef;
                }
            }
        }
    }
    const float SPACE_DRAG = configuration().physics.unit_space_drag_flt;

    if (SPACE_DRAG > 0) {
        Velocity = Velocity * (1 - SPACE_DRAG);
    }

    static string LockingSoundName = vs_config->getVariable("unitaudio", "locking", "locking.wav");
    //enables spiffy wc2 torpedo music, default to normal though
    static string LockingSoundTorpName = vs_config->getVariable("unitaudio", "locking_torp", "locking.wav");
    static int LockingSound = AUDCreateSoundWAV(LockingSoundName, true);
    static int LockingSoundTorp = AUDCreateSoundWAV(LockingSoundTorpName, true);

    bool locking = false;
    bool touched = false;
    for (int i = 0; (int) i < getNumMounts(); ++i) {
        // TODO: simplify this if
        if ((mounts[i].status == Mount::ACTIVE) && !cloak.Cloaked()
                && mounts[i].ammo != 0) {
            if (player_cockpit) {
                touched = true;
            }
            if (increase_locking && (dist_sqr_to_target < mounts[i].type->range * mounts[i].type->range)) {
                mounts[i].time_to_lock -= simulation_atom_var;
                const bool ai_lock_cheat = configuration().physics.ai_lock_cheat;
                if (!player_cockpit) {
                    if (ai_lock_cheat) {
                        mounts[i].time_to_lock = -1;
                    }
                } else {
                    int LockingPlay = LockingSound;

                    //enables spiffy wc2 torpedo music, default to normal though
                    static bool LockTrumpsMusic =
                            XMLSupport::parse_bool(vs_config->getVariable("unitaudio",
                                    "locking_trumps_music",
                                    "false"));
                    //enables spiffy wc2 torpedo music, default to normal though
                    static bool TorpLockTrumpsMusic =
                            XMLSupport::parse_bool(vs_config->getVariable("unitaudio",
                                    "locking_torp_trumps_music",
                                    "false"));
                    if (mounts[i].type->lock_time > 0) {
                        static string LockedSoundName = vs_config->getVariable("unitaudio", "locked", "locked.wav");
                        static int LockedSound = AUDCreateSoundWAV(LockedSoundName, false);
                        if (mounts[i].type->size == MOUNT_SIZE::SPECIALMISSILE) {
                            LockingPlay = LockingSoundTorp;
                        } else {
                            LockingPlay = LockingSound;
                        }
                        if (mounts[i].time_to_lock > -SIMULATION_ATOM && mounts[i].time_to_lock <= 0) {
                            if (!AUDIsPlaying(LockedSound)) {
                                UniverseUtil::musicMute(false);
                                AUDStartPlaying(LockedSound);
                                AUDStopPlaying(LockingSound);
                                AUDStopPlaying(LockingSoundTorp);
                            }
                            AUDAdjustSound(LockedSound, Position(), GetVelocity());
                        } else if (mounts[i].time_to_lock > 0) {
                            locking = true;
                            if (!AUDIsPlaying(LockingPlay)) {
                                if (LockingPlay == LockingSoundTorp) {
                                    UniverseUtil::musicMute(TorpLockTrumpsMusic);
                                } else {
                                    UniverseUtil::musicMute(LockTrumpsMusic);
                                }
                                AUDStartPlaying(LockingSound);
                            }
                            AUDAdjustSound(LockingSound, Position(), GetVelocity());
                        }
                    }
                }
            } else if (mounts[i].ammo != 0) {
                mounts[i].time_to_lock = mounts[i].type->lock_time;
            }
        } else if (mounts[i].ammo != 0) {
            mounts[i].time_to_lock = mounts[i].type->lock_time;
        }
        if (mounts[i].type->type == WEAPON_TYPE::BEAM) {
            if (mounts[i].ref.gun) {
                bool autoTrack = isAutoTrackingMount(mounts[i].size);
                bool timeLocked = mounts[i].time_to_lock <= 0;
                bool tracked = TargetTracked();
                Unit *autotarg = (autoTrack && timeLocked && tracked) ? target : nullptr;

                float tracking_cone = radar.tracking_cone;
                // TODO: fix this or remove
                /*if (CloseEnoughToAutotrack(this, target, tracking_cone)) {
                    if (autotarg) {
                        if (radar.tracking_cone < tracking_cone) {
                            tracking_cone = radar.tracking_cone;
                        }
                    }
                    autotarg = target;
                }*/

                mounts[i].ref.gun->UpdatePhysics(cumulative_transformation,
                        cumulative_transformation_matrix,
                        autotarg,
                        tracking_cone,
                        target,
                        this,
                        superunit);
            }
        } else {
            mounts[i].ref.refire += simulation_atom_var * mounts[i].functionality;
        }
        if (mounts[i].processed == Mount::FIRED) {
            Transformation t1;
            Matrix m1;
            t1 = prev_physical_state;             //a hack that will not work on turrets
            t1.Compose(trans, transmat);
            t1.to_matrix(m1);
            int autotrack = 0;
            if ((isAutoTrackingMount(mounts[i].size))
                    && TargetTracked()) {
                autotrack = computer.itts ? 2 : 1;
            }
            float tracking_cone = radar.tracking_cone;
            if (CloseEnoughToAutotrack(this, target, tracking_cone)) {
                if (autotrack) {
                    if (tracking_cone > radar.tracking_cone) {
                        tracking_cone = radar.tracking_cone;
                    }
                }
                autotrack = 2;
            }
            CollideMap::iterator hint[Unit::NUM_COLLIDE_MAPS];
            for (unsigned int locind = 0; locind < Unit::NUM_COLLIDE_MAPS; ++locind) {
                hint[locind] =
                        (!is_null(superunit->location[locind])) ? superunit->location[locind]
                                : _Universe->activeStarSystem()->
                                collide_map[locind]->begin();
            }
            if (!mounts[i].PhysicsAlignedFire(this, t1, m1, cumulative_velocity,
                    (!isSubUnit() || owner == nullptr) ? this : owner, target, autotrack,
                    tracking_cone,
                    hint)) {
                const WeaponInfo *typ = mounts[i].type;
                energy.Charge(static_cast<double>(typ->energy_rate) * (typ->type == WEAPON_TYPE::BEAM ? simulation_atom_var : 1));
            }
        } else if (mounts[i].processed == Mount::UNFIRED || mounts[i].ref.refire > 2 * mounts[i].type->Refire()) {
            mounts[i].processed = Mount::UNFIRED;
            mounts[i].PhysicsAlignedUnfire();
        }
    }
    if (locking == false && touched == true) {
        if (AUDIsPlaying(LockingSound)) {
            UniverseUtil::musicMute(false);
            AUDStopPlaying(LockingSound);
        }
        if (AUDIsPlaying(LockingSoundTorp)) {
            UniverseUtil::musicMute(false);
            AUDStopPlaying(LockingSoundTorp);
        }
    }

    UpdateSubunitPhysics(cumulative_transformation,
                         cumulative_transformation_matrix,
                         cumulative_velocity,
                         lastframe,
                         uc,
                         superunit);
    //can a unit get to another system without jumping?.
    const bool warp_is_interstellar = configuration().physics.warp_is_interstellar;
    if (warp_is_interstellar
            && (curr_physical_state.position.MagnitudeSquared() > std::pow(configuration().physics.distance_to_warp_dbl, 2) && !isSubUnit())) {
        const bool direct = configuration().physics.direct_interstellar_journey;
        bool jumpDirect = false;
        if (direct) {
            Cockpit *cp = _Universe->isPlayerStarship(this);
            if (nullptr != cp) {
                std::string sys = cp->GetNavSelectedSystem();
                if (!sys.empty()) {
                    jumpDirect = true;
                    _Universe->activeStarSystem()->JumpTo(this, nullptr, sys, true, true);
                }
            }
        }
        if (!jumpDirect) {
            _Universe->activeStarSystem()->JumpTo(this, nullptr,
                    NearestSystem(_Universe->activeStarSystem()->getFileName(),
                            curr_physical_state.position), true, true);
        }
    }


    // Destroyed means we just killed the unit and it is exploding
    if (Destroyed()) {
        bool dead = true;
        this->Explode(true, 0);

        // Kill means it is done exploding and we can delete it
        dead &= (pImage->pExplosion == nullptr);
        if (dead) {
            Kill();
        }
        return;
    }

    // We stop processing for sub-units
    if (isSubUnit()) {
        return;
    }

    for (unsigned int locind = 0; locind < Unit::NUM_COLLIDE_MAPS; ++locind) {
        if (is_null(this->location[locind])) {
            this->getStarSystem()->collide_map[locind]->insert(Collidable(this));
        } else if (locind == Unit::UNIT_BOLT) {
            //that update will propagate with the flatten
            this->getStarSystem()->collide_map[Unit::UNIT_BOLT]->changeKey(this->location[locind], Collidable(this));
        }
    }
}


bool Unit::isPlayerShip() {
    return _Universe->isPlayerStarship(this) ? true : false;
}

///Updates the collide Queue with any possible change in sectors
///Queries if this unit is within a given frustum
bool Unit::queryFrustum(double frustum[6][4]) const {
    unsigned int i;
#ifdef VARIABLE_LENGTH_PQR
                                                                                                                            Vector TargetPoint( cumulative_transformation_matrix[0],
                        cumulative_transformation_matrix[1],
                        cumulative_transformation_matrix[2] );
    float  SizeScaleFactor = sqrtf( TargetPoint.Dot( TargetPoint ) );
#else
    Vector TargetPoint;
#endif
    for (i = 0; i < nummesh() && this->meshdata[i]; i++) {
        TargetPoint = Transform(this->cumulative_transformation_matrix, this->meshdata[i]->Position());
        if (GFXSphereInFrustum(frustum,
                TargetPoint,
                this->meshdata[i]->rSize()
#ifdef VARIABLE_LENGTH_PQR
                *SizeScaleFactor
#endif
        )) {
            return true;
        }
    }
    const Unit *un;
    for (un_fkiter iter = this->SubUnits.constFastIterator(); (un = *iter); ++iter) {
        if (un->queryFrustum(frustum)) {
            return true;
        }
    }
    return false;
}

void Unit::addHalo(const char *filename,
        const Matrix &trans,
        const Vector &size,
        const GFXColor &col,
        std::string halo_type,
        float halo_speed) {
    halos->AddHalo(filename, trans, size, col, std::move(halo_type), halo_speed);
}

/**** MOVED FROM BASE_INTERFACE.CPP ****/
extern string getCargoUnitName(const char *name);

void Unit::UpgradeInterface(Unit *baseun) {
    string basename = (::getCargoUnitName(baseun->getFullname().c_str()));
    if (baseun->getUnitType() != Vega_UnitType::planet) {
        basename = baseun->name;
    }
    BaseUtil::LoadBaseInterfaceAtDock(basename, baseun, this);
}

//From star_system_jump.cpp
extern Hashtable<std::string, StarSystem, 127> star_system_table;
extern std::vector<unorigdest *> pendingjump;

//From star_system_jump.cpp
inline bool CompareDest(Unit* un, StarSystem* origin) {
    return std::any_of(un->GetDestinations().cbegin(), un->GetDestinations().cend(),
                       [origin](const std::string& destination) {
                           return destination == origin->getFileName();
                       });
}

inline std::vector<Unit *> ComparePrimaries(Unit *primary, StarSystem *origin) {
    std::vector<Unit *> myvec;
    if (CompareDest(primary, origin)) {
        myvec.push_back(primary);
    }
    return myvec;
}

extern void DealPossibleJumpDamage(Unit *un);
extern void ActivateAnimation(Unit *);
void WarpPursuit(Unit *un, StarSystem *sourcess, std::string destination);

bool Unit::TransferUnitToSystem(unsigned int kk, StarSystem *&savedStarSystem, bool dosightandsound) {
    bool ret = false;
    if (pendingjump[kk]->orig == this->activeStarSystem || this->activeStarSystem == nullptr) {
        if (JumpCapable::TransferUnitToSystem(pendingjump[kk]->dest)) {
            ///eradicating from system, leaving no trace
            ret = true;

            Unit *unit;
            for (un_iter iter = pendingjump[kk]->orig->getUnitList().createIterator(); (unit = *iter); ++iter) {
                if (unit->Threat() == this) {
                    unit->Threaten(nullptr, 0);
                }
                if (unit->VelocityReference() == this) {
                    unit->VelocityReference(nullptr);
                }
                if (unit->Target() == this) {
                    if (pendingjump[kk]->jumppoint.GetUnit()) {
                        unit->Target(pendingjump[kk]->jumppoint.GetUnit());
                        unit->ActivateJumpDrive(0);
                    } else {
                        WarpPursuit(unit, pendingjump[kk]->orig, pendingjump[kk]->dest->getFileName());
                    }
                } else {
                    Flightgroup *ff = unit->getFlightgroup();
                    if (ff) {
                        if (this == ff->leader.GetUnit() && (ff->directive == "f" || ff->directive == "F")) {
                            unit->Target(pendingjump[kk]->jumppoint.GetUnit());
                            unit->getFlightgroup()->directive = "F";
                            unit->ActivateJumpDrive(0);
                        }
                    }
                }
            }
            if (this == _Universe->AccessCockpit()->GetParent()) {
                VS_LOG(info, "Unit is the active player character...changing scene graph\n");
                savedStarSystem->SwapOut();
                AUDStopAllSounds();
                savedStarSystem = pendingjump[kk]->dest;
                pendingjump[kk]->dest->SwapIn();
            }
            _Universe->setActiveStarSystem(pendingjump[kk]->dest);
            vector<Unit *> possibilities;
            if (pendingjump[kk]->final_location.i == 0
                    && pendingjump[kk]->final_location.j == 0
                    && pendingjump[kk]->final_location.k == 0) {
                Unit *primary;
                for (un_iter iter = pendingjump[kk]->dest->getUnitList().createIterator(); (primary = *iter); ++iter) {
                    vector<Unit *> tmp;
                    tmp = ComparePrimaries(primary, pendingjump[kk]->orig);
                    if (!tmp.empty()) {
                        possibilities.insert(possibilities.end(), tmp.begin(), tmp.end());
                    }
                }
            } else {
                this->SetCurPosition(pendingjump[kk]->final_location);
            }
            if (!possibilities.empty()) {
                static int jumpdest = 235034;
                Unit *jumpnode = possibilities[jumpdest % possibilities.size()];
                const QVector pos = jumpnode->Position();

                this->SetCurPosition(pos);
                ActivateAnimation(jumpnode);
                if (jumpnode->getUnitType() == Vega_UnitType::unit) {
                    QVector Offset(pos.i < 0 ? 1 : -1,
                            pos.j < 0 ? 1 : -1,
                            pos.k < 0 ? 1 : -1);
                    Offset *= jumpnode->rSize() * 2 + this->rSize() * 2;
                    this->SetPosAndCumPos(pos + Offset);
                    if (is_null(jumpnode->location[Unit::UNIT_ONLY]) == false
                            && is_null(jumpnode->location[Unit::UNIT_BOLT]) == false) {
                        this->UpdateCollideQueue(pendingjump[kk]->dest, jumpnode->location);
                    }
                }
                jumpdest += 23231;
            }
            Unit *tester;
            for (unsigned int jjj = 0; jjj < 2; ++jjj) {
                for (un_iter i = _Universe->activeStarSystem()->getUnitList().createIterator();
                        (tester = *i) != nullptr; ++i) {
                    if (tester->getUnitType() == Vega_UnitType::unit && tester != this) {
                        if ((this->LocalPosition() - tester->LocalPosition()).Magnitude()
                                < this->rSize() + tester->rSize()) {
                            this->SetCurPosition(this->LocalPosition() + this->cumulative_transformation_matrix.getR()
                                    * (4 * (this->rSize() + tester->rSize())));
                        }
                    }
                }
            }
            DealPossibleJumpDamage(this);
            static int
                    jumparrive = AUDCreateSound(vs_config->getVariable("unitaudio", "jumparrive", "sfx43.wav"), false);
            if (dosightandsound) {
                AUDPlay(jumparrive, this->LocalPosition(), this->GetVelocity(), 1);
            }
        } else {
#ifdef JUMP_DEBUG
            VS_LOG(debug, "Unit FAILED remove from star system\n");
#endif
        }
        if (this->docked & DOCKING_UNITS) {
            for (const auto & docked_unit : this->pImage->dockedunits) {
                Unit *unut;
                if (nullptr != (unut = docked_unit->uc.GetUnit())) {
                    unut->TransferUnitToSystem(kk, savedStarSystem, dosightandsound);
                }
            }
        }
        if (this->docked & (DOCKED | DOCKED_INSIDE)) {
            Unit *un = this->pImage->DockedTo.GetUnit();
            if (!un) {
                this->docked &= (~(DOCKED | DOCKED_INSIDE));
            } else {
                const Unit *targ = nullptr;
                for (un_iter i = pendingjump[kk]->dest->getUnitList().createIterator();
                        (targ = (*i));
                        ++i) {
                    if (targ == un) {
                        break;
                    }
                }
                if (targ != un) {
                    this->UnDock(un);
                }
            }
        }
    } else {
        VS_LOG(warning, "Already jumped\n");
    }
    return ret;
}
