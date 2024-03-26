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

// -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil -*-

#include "unit_generic.h"

#include <set>
#include "configxml.h"
#include "audiolib.h"
#include "base.h"
#include "music.h"

#include "beam.h"
#include "lin_time.h"
#include "xml_serializer.h"
#include "vsfilesystem.h"
#include "vs_logging.h"
#include "file_main.h"
#include "universe_util.h"
#include "unit_util.h"
#include "script/mission.h"
#include "script/flightgroup.h"
#include "cmd/ai/fire.h"
#include "cmd/ai/turretai.h"
#include "cmd/ai/communication.h"
#include "cmd/ai/navigation.h"
#include "cmd/ai/script.h"
#include "cmd/ai/missionscript.h"
#include "cmd/ai/flybywire.h"
#include "cmd/ai/aggressive.h"
#include "python/python_class.h"
#include "missile.h"
#include "gfx/cockpit_generic.h"
#include "gfx/vsbox.h"
#include <algorithm>
#include "cmd/ai/ikarus.h"
#include "role_bitmask.h"
#include "unit_const_cache.h"
#include "gfx/warptrail.h"
#include "gfx/cockpit_generic.h"
#include "csv.h"
#include "vs_random.h"
#include "galaxy_xml.h"
#include "gfx/camera.h"
#include "options.h"
#include "star_system.h"
#include "universe.h"
#include "weapon_info.h"
#include "mount_size.h"
#include "turret.h"
#include "configuration/game_config.h"
#include "resource/resource.h"
#include "base_util.h"
#include "unit_csv_factory.h"

#include <math.h>
#include <list>
#include <cstdint>
#include <boost/format.hpp>

#ifdef _WIN32
#define strcasecmp stricmp
#endif

#include "unit_find.h"
#include "pilot.h"

#include <iostream>
#define DEBUG_MESH_ANI

using std::endl;
using std::list;

std::string getMasterPartListUnitName() {
    return configuration()->data_config.master_part_list;
}

Unit *_masterPartList = nullptr;

Unit *getMasterPartList() {
    if (_masterPartList == nullptr) {
        static bool making = true;
        if (making) {
            making = false;
            _masterPartList = Unit::makeMasterPartList();
            making = true;
        }
    }
    return _masterPartList;
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

bool Unit::InRange(const Unit *target, double &mm, bool cone, bool cap, bool lock) const {
    const float capship_size = configuration()->physics_config.capship_size;

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
    if (((mm - rSize() - target->rSize()) > radar.max_range)
            || target->rSize() < radar.GetMinTargetSize()) {
        Flightgroup *fg = target->getFlightgroup();
        if ((target->rSize() < capship_size || (!cap)) && (fg == NULL ? true : fg->name != "Base")) {
            return target->isUnit() == Vega_UnitType::planet;
        }
    }
    return true;
}

Unit *Unit::Target() {
    return computer.target.GetUnit();
}

const Unit *Unit::Target() const {
    return computer.target.GetConstUnit();
}

Unit *Unit::VelocityReference() {
    return computer.velocity_ref.GetUnit();
}

const Unit *Unit::VelocityReference() const {
    return computer.velocity_ref.GetConstUnit();
}

Unit *Unit::Threat() {
    return computer.threat.GetUnit();
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

#define INVERSEFORCEDISTANCE 5400
extern void abletodock(int dock);

bool CrashForceDock(Unit *thus, Unit *dockingUn, bool force) {
    Unit *un = dockingUn;
    int whichdockport = thus->CanDockWithMe(un, force);
    if (whichdockport != -1) {
        QVector place = UniverseUtil::SafeEntrancePoint(un->Position(), un->rSize() * 1.5);
        un->SetPosAndCumPos(place);
        if (un->ForceDock(thus, whichdockport) > 0) {
            abletodock(3);
            un->UpgradeInterface(thus);
            return true;
        }
    }
    return false;
}

float rand01() {
    return (float) rand() / (float) RAND_MAX;
}

/* UGLYNESS short fix */
unsigned int apply_float_to_unsigned_int(float tmp) {
    static unsigned long int seed = 2531011;
    seed += 214013;
    seed %= 4294967295u;
    unsigned int ans = (unsigned int) tmp;
    tmp -= ans;                                         //now we have decimal;
    if (seed < (unsigned long int) (4294967295u * tmp)) {
        ans += 1;
    }
    return ans;
}

static list<Unit *> Unitdeletequeue;
static Hashtable<uintmax_t, Unit, 2095> deletedUn;
int deathofvs = 1;

void CheckUnit(Unit *un) {
    if (deletedUn.Get((uintmax_t) un) != nullptr) {
        while (deathofvs) {
            VS_LOG(info, (boost::format("%1% died") % ((uintmax_t) un)));
        }
    }
}

void UncheckUnit(Unit *un) {
    if (deletedUn.Get((uintmax_t) un) != NULL) {
        deletedUn.Delete((uintmax_t) un);
    }
}

string GetUnitDir(string filename) {
    return filename.substr(0, filename.find("."));
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
    pImage->cockpit_damage = NULL;
    pilot = new Pilot(FactionUtil::GetNeutralFaction());
    // TODO: delete
    Init();
}


Unit::Unit() : Drawable(), Damageable(), Movable() //: cumulative_transformation_matrix( identity_matrix )
{
    pImage = (new UnitImages<void>);
    pImage->cockpit_damage = NULL;
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
    pImage->cockpit_damage = NULL;
    // TODO:
    Init();

    this->faction = fact;
    graphicOptions.SubUnit = SubU;
    meshdata = meshes;
    meshes.clear();
    meshdata.push_back(NULL);
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
    pImage->cockpit_damage = NULL;
    Init(filename, SubU, faction, unitModifications, flightgrp, fg_subnumber);
    pilot->SetComm(this);
}

Unit::~Unit() {
    free(pImage->cockpit_damage);
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
    VS_LOG_AND_FLUSH(trace, (boost::format("%1$d %2$x") % 6 % &mounts);
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
    for (size_t meshcount = 0; meshcount < meshdata.size(); ++meshcount) {
        if (meshdata[meshcount] != nullptr) {
            delete meshdata[meshcount];
            meshdata[meshcount] = nullptr;
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

    //No cockpit reference here
    if (!pImage->cockpit_damage) {
        unsigned int numg = (1 + MAXVDUS + UnitImages<void>::NUMGAUGES) * 2;
        pImage->cockpit_damage = (float *) malloc((numg) * sizeof(float));
        for (unsigned int damageiterator = 0; damageiterator < numg; ++damageiterator) {
            pImage->cockpit_damage[damageiterator] = 1;
        }
    }
}

using namespace VSFileSystem;
extern std::string GetReadPlayerSaveGame(int);

void Unit::Init(const char *filename,
        bool SubU,
        int faction,
        std::string unitModifications,
        Flightgroup *flightgrp,
        int fg_subnumber) {
    // Deprecated UNITTAB and configuration()->physics_config.unit_table options.
    // Game will always load units from the JSON or CSV files.
    // The other option was not implemented wholly. It simply opened the file
    // but didn't do anything with it. See VSFile f variable.

    // TODO: something with the following line
    this->Unit::Init();
    graphicOptions.SubUnit = SubU ? 1 : 0;
    graphicOptions.Animating = 1;
    graphicOptions.RecurseIntoSubUnitsOnCollision = !isSubUnit();
    this->faction = faction;
    SetFg(flightgrp, fg_subnumber);
    bool saved_game = false;
    bool modified = (unitModifications.length() != 0);
    if (modified) {
        string nonautosave = GetReadPlayerSaveGame(_Universe->CurrentCockpit());
        string filepath("");

        if (nonautosave.empty()) {
            VSFileSystem::CreateDirectoryHome(VSFileSystem::savedunitpath + "/" + unitModifications);
            filepath = unitModifications + "/" + string(filename);
        } else {
            VSFileSystem::CreateDirectoryHome(VSFileSystem::savedunitpath + "/" + nonautosave);
            filepath = nonautosave + "/" + string(filename);
        }

        //Try to open save
        if (filename[0]) {
            VSFile unitTab;
            VSError taberr = unitTab.OpenReadOnly(filepath + ".csv", UnitSaveFile);
            if (taberr <= Ok) {
                std::string data = unitTab.ReadFull();
                UnitCSVFactory::ParseCSV(data, unitTab.GetRoot(), true);
                unitTab.Close();
                saved_game = true;
            }
        }
    }

    this->filename = filename;
    this->name = filename;

    const std::string faction_name = FactionUtil::GetFactionName(faction);
    const std::string unit_key = GetUnitKeyFromNameAndFaction(filename, faction_name);

    if (unit_key == "") {
        // This is actually used for upgrade checks.
        bool istemplate = (string::npos != (string(filename).find(".template")));
        if (!istemplate || (istemplate && configuration()->data_config.using_templates)) {
            VS_LOG(trace, (boost::format("Unit file %1% not found") % filename));
        }
        meshdata.clear();
        meshdata.push_back(NULL);
        this->fullname = filename;
        this->name = string("LOAD_FAILED");
        calculate_extent(false);
        radial_size = 1;

        pilot->SetComm(this);
        return;
    }

    bool tmpbool;

    //load from table?
    //we have to set the root directory to where the saved unit would have come from.
    //saved only exists if taberr<=Ok && taberr!=Unspecified...that's why we pass in said boolean
    // Despite the check, has always taken the data folder, simplifying
    //VSFileSystem::current_path.push_back(taberr <= Ok && taberr
    //        != Unspecified ? GetUnitRow(filename, SubU, faction, false,
    //        tmpbool).getRoot() : unitRow.getRoot());
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

    bool initsucc = DrawableInit(filename, faction, flightgrp);
    if (initsucc) {
        SetAniSpeed(0.05);
        StartAnimation();
    }
}

vector<Mesh *> Unit::StealMeshes() {
    vector<Mesh *> ret;

    Mesh *shield = meshdata.empty() ? NULL : meshdata.back();
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
        tur->SetAngularVelocity(tur->DownCoordinateLevel(Vector(tur->GetComputerData().max_pitch_up,
                tur->GetComputerData().max_yaw_right,
                tur->GetComputerData().max_roll_right)));
    }
    return accessory;
}

void Unit::calculate_extent(bool update_collide_queue) {
    unsigned int a;
    corner_min = Vector(FLT_MAX, FLT_MAX, FLT_MAX);
    corner_max = Vector(-FLT_MAX, -FLT_MAX, -FLT_MAX);
    for (a = 0; a < nummesh(); ++a) {
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
    if (isUnit() == Vega_UnitType::planet) {
        radial_size = tmpmax(tmpmax(corner_max.i, corner_max.j), corner_max.k);
    }
}

const string Unit::getFgID() {
    if (flightgroup != NULL) {
        char buffer[32];
        sprintf(buffer, "-%d", flightgroup_subnumber);
        return flightgroup->name + buffer;
    } else {
        return fullname;
    }
}

void Unit::SetFaction(int faction) {
    this->faction = faction;
    for (un_iter ui = getSubUnits(); (*ui) != NULL; ++ui) {
        (*ui)->SetFaction(faction);
    }
}

void Unit::SetFg(Flightgroup *fg, int fg_subnumber) {
    flightgroup = fg;
    flightgroup_subnumber = fg_subnumber;
}

static float tmpsqr(float x) {
    return x * x;
}

float CloseEnoughCone(Unit *me) {
    return configuration()->physics_config.near_autotrack_cone;
}

bool CloseEnoughToAutotrack(Unit *me, Unit *targ, float &cone) {
    if (targ) {
        const float close_enough_to_autotrack =
                tmpsqr(configuration()->physics_config.close_enough_to_autotrack);
        float dissqr = (me->curr_physical_state.position.Cast()
                - targ->curr_physical_state.position.Cast()).MagnitudeSquared();
        float movesqr = close_enough_to_autotrack
                * (me->prev_physical_state.position.Cast()
                        - me->curr_physical_state.position.Cast()).MagnitudeSquared();
        if (dissqr < movesqr && movesqr > 0) {
            cone = CloseEnoughCone(me) * (movesqr - dissqr) / movesqr + 1 * dissqr / movesqr;
            return true;
        }
    }
    return false;
}

//Caps at +/- 1 so as to account for floating point inaccuracies.
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
    QVector totarget(targ->PositionITTS(cumulative_transformation.position, cumulative_velocity, speed, false));
    totarget = totarget - cumulative_transformation.position;
    dist = totarget.Magnitude();

    //Trial code
    float turnlimit =
            tmpmax(tmpmax(computer.max_yaw_left, computer.max_yaw_right),
                    tmpmax(computer.max_pitch_up, computer.max_pitch_down));
    float turnangle = simulation_atom_var
            * tmpmax(turnlimit,
                    tmpmax(simulation_atom_var * .5 * (limits.yaw + limits.pitch),
                            sqrtf(AngularVelocity.i * AngularVelocity.i + AngularVelocity.j * AngularVelocity.j)));
    float ittsangle = safeacos(Normal.Cast().Dot(totarget.Scale(1. / totarget.Magnitude())));
    QVector edgeLocation = (targ->cumulative_transformation_matrix.getP() * targ->rSize() + totarget);
    float radangle = safeacos(edgeLocation.Cast().Scale(1. / edgeLocation.Magnitude()).Dot(totarget.Normalize()));
    float rv = ittsangle - radangle - (turnmargin ? turnangle : 0);

    float rsize = targ->rSize() + rSize();
    if ((!targ->GetDestinations().empty() && jump.drive >= 0) || (targ->faction == faction)) {
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
    float tmpcos;
    Matrix mat;
    for (int i = 0; i < getNumMounts(); ++i) {
        float tmpdist = .001;
        Transformation finaltrans(mounts[i].GetMountOrientation(), mounts[i].GetMountLocation().Cast());
        finaltrans.Compose(cumulative_transformation, cumulative_transformation_matrix);
        finaltrans.to_matrix(mat);
        Vector Normal(mat.getR());

        QVector totarget(targ->PositionITTS(finaltrans.position, cumulative_velocity, mounts[i].type->speed, false));

        tmpcos = Normal.Dot(totarget.Cast());
        tmpdist = totarget.Magnitude();
        if (tmpcos > 0) {
            tmpcos = tmpdist * tmpdist - tmpcos * tmpcos;
            //one over distance perpendicular away from straight ahead times the size...high is good WARNING POTENTIAL DIV/0
            tmpcos = targ->rSize() / tmpcos;
        } else {
            tmpcos /= tmpdist;
        }
        //UNLIKELY DIV/0
        tmpdist /= mounts[i].type->range;
        if (tmpdist < 1 || tmpdist < dist) {
            if (tmpcos - tmpdist / 2 > retval - dist / 2) {
                dist = tmpdist;
                retval = tmpcos;
            }
        }
    }
    return retval;
}

#define PARANOIA (0.4f)

void Unit::Threaten(Unit *targ, float danger) {
    if (!targ) {
        computer.threatlevel = danger;
        computer.threat.SetUnit(NULL);
    } else if (targ->owner != this && this->owner != targ && danger > PARANOIA && danger > computer.threatlevel) {
        computer.threat.SetUnit(targ);
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
    for (un_iter i = uhn->getSubUnits(); (un = *i) != NULL; ++i) {
        disableSubUnits(un);
    }
    for (unsigned int j = 0; j < uhn->mounts.size(); ++j) {
        DestroyMount(&uhn->mounts[j]);
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
#if 0
                                                                                                                            StarSystem     *ssystem  = _Universe->activeStarSystem();
    UnitCollection *unitlist = ssystem->getUnitList();
    un_iter   uiter = unitlist->createIterator();

    GameUnit *found_target   = NULL;
    int found_attackers = 1000;
    for (GameUnit *other_unit = NULL; other_unit = *uiter; ++uiter) {
        string other_fgid = other_unit->getFgID();
        if ( other_unit->matchesFg( target_fgid[which_target] ) ) {
            //the other unit matches our primary target

            int num_attackers = other_unit->getNumAttackers();
            if (num_attackers < found_attackers) {
                //there's less ships attacking this target than the previous one
                found_target    = other_unit;
                found_attackers = num_attackers;
                setTarget( found_target );
            }
        }
    }
    if (found_target == NULL) {
        //we haven't found a target yet, search again
        if (which_target <= 1)
            ReTargetFg( which_target+1 );
        else
            //we can't find any target
            setTarget( NULL );
    }
#endif
}

/*
 **********************************************************************************
 **** UNIT_PHYSICS STUFF
 **********************************************************************************
 */

extern signed char ComputeAutoGuarantee(Unit *un);
extern float getAutoRSize(Unit *orig, Unit *un, bool ignore_friend = false);

double howFarToJump() {
    return configuration()->physics_config.distance_to_warp;
}

QVector SystemLocation(std::string system) {
    string xyz = _Universe->getGalaxyProperty(system, "xyz");
    QVector pos;
    if (xyz.size() && (sscanf(xyz.c_str(), "%lf %lf %lf", &pos.i, &pos.j, &pos.k) >= 3)) {
        return pos;
    } else {
        return QVector(0, 0, 0);
    }
}

static std::string NearestSystem(std::string currentsystem, QVector pos) {
    if (pos.i == 0 && pos.j == 0 && pos.k == 0) {
        return "";
    }
    QVector posnorm = pos.Normalize();
    posnorm.Normalize();
    QVector cur = SystemLocation(currentsystem);
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
            if (place.length()) {
                QVector pos2 = QVector(0, 0, 0);
                sscanf(place.c_str(), "%lf %lf %lf", &pos2.i, &pos2.j, &pos2.k);
                if ((pos2.i != 0 || pos2.j != 0 || pos2.k != 0)
                        && (pos2.i != cur.i || pos2.j != cur.j || pos2.k != cur.k)) {
                    QVector dir = pos2 - cur;
                    QVector norm = dir;
                    norm.Normalize();
                    double test = posnorm.Dot(norm);
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
                                    tmp /= configuration()->physics_config.target_distance_to_warp_bonus;
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
        float backup = simulation_atom_var;
        //VS_LOG(trace, (boost::format("Unit::UpdateSubunitPhysics(): simulation_atom_var as backed up  = %1%") % simulation_atom_var));
        float basesimatom = (this->sim_atom_multiplier ? backup / (float) this->sim_atom_multiplier : backup);
        unsigned int cur_sim_frame = _Universe->activeStarSystem()->getCurrentSimFrame();
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
                    simulation_atom_var = basesimatom * (float) su->sim_atom_multiplier;
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
        subunit->Target(NULL);
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

// Move to jump_enabled
void TurnJumpOKLightOn(Unit *un, Cockpit *cp) {
    if (cp) {
        if (un->energy_manager.GetLevel(EnergyType::FTL) >= un->jump.energy) {
            if (un->jump.drive > -2) {
                cp->jumpok = 1;
            }
        }
    }
}

bool Unit::jumpReactToCollision(Unit *smalle) {
    const bool ai_jump_cheat = configuration()->ai.jump_without_energy;
    const bool nojumpinSPEC = configuration()->physics_config.no_spec_jump;
    bool SPEC_interference = (nullptr != _Universe->isPlayerStarship(smalle)) ? smalle->graphicOptions.InWarp
            && nojumpinSPEC : (nullptr != _Universe->isPlayerStarship(this)) && graphicOptions.InWarp
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
        if ((!SPEC_interference && (smalle->jump.drive >= 0
                &&          //we have power
                        (smalle->energy_manager.GetLevel(EnergyType::FTL) >= smalle->jump.energy
                                //or we're being cheap
                                || (ai_jump_cheat && cp == nullptr)
                        )))
                || forcejump) {
            //or the jump is being forced?
            //NOW done in star_system_generic.cpp before TransferUnitToSystem smalle->warpenergy-=smalle->GetJumpStatus().energy;
            int dest = smalle->jump.drive;
            if (dest < 0) {
                dest = 0;
            }
            smalle->DeactivateJumpDrive();
            Unit *jumppoint = this;
            _Universe->activeStarSystem()
                    ->JumpTo(smalle, jumppoint, GetDestinations()[dest % GetDestinations().size()]);
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
        if ((!SPEC_interference && (jump.drive >= 0
                && (energy_manager.GetLevel(EnergyType::FTL) >= jump.energy || (ai_jump_cheat && cp == NULL))
        )) || smalle->forcejump) {
            jump_drive.Use();
            DeactivateJumpDrive();
            Unit *jumppoint = smalle;

            _Universe->activeStarSystem()->JumpTo(this, jumppoint,
                    smalle->GetDestinations()[jump.drive
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
        difficulty = pow(g_game.difficulty, configuration()->physics_config.difficulty_speed_exponent);
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
    for (unsigned int j = 0; j < mounts.size(); ++j) {
        DestroyMount(&mounts[j]);
        AUDDeleteSound(mounts[j].sound);
        if (mounts[j].ref.gun && mounts[j].type->type == WEAPON_TYPE::BEAM) {
            //hope we're not killin' em twice...they don't go in gunqueue
            delete mounts[j].ref.gun;
            mounts[j].ref.gun = NULL;
        }
    }
    mounts.clear();
    Unit *su;
    for (un_iter i = getSubUnits(); (su = *i) != NULL; ++i) {
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
    for (un_iter i = _Universe->activeStarSystem()->getUnitList().createIterator(); (un = *i) != NULL; ++i) {
        if (un == reinterpret_cast<const Unit *>(unitDoNotDereference)) {
            return un;
        }
    }
    return NULL;
}

//NUMGAUGES has been moved to pImages.h in UnitImages<void>
void Unit::DamageRandSys(float dam, const Vector &vec, float randnum, float degrees) {
    float deg = fabs(180 * atan2(vec.i, vec.k) / M_PI);
    randnum = rand01();
    const float inv_min_dam = 1.0F - configuration()->physics_config.min_damage;
    const float inv_max_dam = 1.0F - configuration()->physics_config.max_damage;
    if (dam < inv_max_dam) {
        dam = inv_max_dam;
    }
    if (dam > inv_min_dam) {
        dam = inv_min_dam;
    }
    degrees = deg;
    if (degrees > 180) {
        degrees = 360 - degrees;
    }
    if (degrees >= 0 && degrees < 20) {
        int which = rand() % (1 + UnitImages<void>::NUMGAUGES + MAXVDUS);
        pImage->cockpit_damage[which] *= dam;
        if (pImage->cockpit_damage[which] < .1) {
            pImage->cockpit_damage[which] = 0;
        }
        //DAMAGE COCKPIT
        if (randnum >= .85) {//do 25% damage to a gauge
            pImage->cockpit_damage[which] *= .75;
            if (pImage->cockpit_damage[which] < .1) {
                pImage->cockpit_damage[which] = 0;
            }
        } else if (randnum >= .775) {
            computer.itts = false;             //Set the computer to not have an itts
        } else if (randnum >= .7) {
            radar.Damage();
        } else if (randnum >= .5) {
            //THIS IS NOT YET SUPPORTED IN NETWORKING
            computer.target = nullptr;             //set the target to NULL
        } else if (randnum >= .4) {
            limits.retro *= dam;
        } else if (randnum >= .175) {
            radar.Damage();
        } else {
            int which = rand() % (1 + UnitImages<void>::NUMGAUGES + MAXVDUS);
            pImage->cockpit_damage[which] *= dam;
            if (pImage->cockpit_damage[which] < .1) {
                pImage->cockpit_damage[which] = 0;
            }
        }
        damages |= Damages::COMPUTER_DAMAGED;
        return;
    }
    if (rand01() < configuration()->physics_config.thruster_hit_chance) {
        //DAMAGE ROLL/YAW/PITCH/THRUST
        float orandnum = rand01() * .82 + .18;
        if (randnum >= .9) {
            computer.max_pitch_up *= orandnum;
        } else if (randnum >= .8) {
            computer.max_yaw_right *= orandnum;
        } else if (randnum >= .6) {
            computer.max_yaw_left *= orandnum;
        } else if (randnum >= .4) {
            computer.max_pitch_down *= orandnum;
        } else if (randnum >= .2) {
            computer.max_roll_right *= orandnum;
        } else if (randnum >= .18) {
            computer.max_roll_left *= orandnum;
        } else if (randnum >= .17) {
            limits.roll *= dam;
        } else if (randnum >= .10) {
            limits.yaw *= dam;
        } else if (randnum >= .03) {
            limits.pitch *= dam;
        } else {
            limits.lateral *= dam;
        }
        damages |= Damages::LIMITS_DAMAGED;
        return;
    }
    if (degrees >= 20 && degrees < 35) {
        //DAMAGE MOUNT
        if (randnum >= .65 && randnum < .9) {
            ecm *= float_to_int(dam);
        } else if (getNumMounts()) {
            unsigned int whichmount = rand() % getNumMounts();
            if (randnum >= .9) {
                DestroyMount(&mounts[whichmount]);
            } else if (mounts[whichmount].ammo > 0 && randnum >= .75) {
                mounts[whichmount].ammo *= float_to_int(dam);
            } else if (randnum >= .7) {
                mounts[whichmount].time_to_lock += (100 - (100 * dam));
            } else if (randnum >= .2) {
                mounts[whichmount].functionality *= dam;
            } else {
                mounts[whichmount].maxfunctionality *= dam;
            }
        }
        damages |= Damages::MOUNT_DAMAGED;
        return;
    }
    if (degrees >= 35 && degrees < 60) {
        //DAMAGE FUEL
        static float fuel_damage_prob = 1.f
                - XMLSupport::parse_float(vs_config->getVariable("physics", "fuel_damage_prob", ".25"));
        static float warpenergy_damage_prob = fuel_damage_prob
                - XMLSupport::parse_float(vs_config->getVariable("physics",
                        "warpenergy_damage_prob",
                        "0.05"));
        static float ab_damage_prob = warpenergy_damage_prob
                - XMLSupport::parse_float(vs_config->getVariable("physics", "ab_damage_prob", ".2"));
        static float cargovolume_damage_prob = ab_damage_prob
                - XMLSupport::parse_float(vs_config->getVariable("physics",
                        "cargovolume_damage_prob",
                        ".15"));
        static float upgradevolume_damage_prob = cargovolume_damage_prob
                - XMLSupport::parse_float(vs_config->getVariable("physics",
                        "upgradevolume_damage_prob",
                        ".1"));
        static float cargo_damage_prob = upgradevolume_damage_prob
                - XMLSupport::parse_float(vs_config->getVariable("physics", "cargo_damage_prob", "1"));
        // TODO: implement
        /*if (randnum >= fuel_damage_prob) {
            fuel *= dam;
        } else if (randnum >= warpenergy_damage_prob) {
            warpenergy *= dam;
        } else if (randnum >= ab_damage_prob) {
            this->afterburnenergy += ((1 - dam) * recharge);*/
        //} else 
        if (randnum >= cargovolume_damage_prob) {
            CargoVolume *= dam;
        } else if (randnum >= upgradevolume_damage_prob) {
            UpgradeVolume *= dam;
        } else if (randnum >= cargo_damage_prob) {
            //Do something NASTY to the cargo
            if (cargo.size() > 0) {
                unsigned int i = 0;
                unsigned int cargorand_o = rand();
                unsigned int cargorand;
                do {
                    cargorand = (cargorand_o + i) % cargo.size();
                } while ((cargo[cargorand].GetQuantity() == 0
                        || cargo[cargorand].GetMissionFlag()) && (++i) < cargo.size());
                cargo[cargorand].SetQuantity(cargo[cargorand].GetQuantity() * float_to_int(dam));
            }
        }
        damages |= Damages::CARGOFUEL_DAMAGED;
        return;
    }
    if (degrees >= 90 && degrees < 120) {
        //DAMAGE Shield
        //DAMAGE cloak
        if (randnum >= .7) {
            this->cloak.Damage();
            damages |= Damages::CLOAK_DAMAGED;
        }

        // TODO: lib_damage reenable
        shield_component.Damage();

        damages |= Damages::SHIELD_DAMAGED;
        return;
    }
    if (degrees >= 120 && degrees < 150) {
        //DAMAGE Reactor
        //DAMAGE JUMP
        if (randnum >= .9) {
            /*static char max_shield_leak =
                (char) std::max( 0.0,
                             std::min( 100.0, XMLSupport::parse_float( vs_config->getVariable( "physics", "max_shield_leak", "90" ) ) ) );
            static char min_shield_leak =
                (char) std::max( 0.0,
                             std::min( 100.0, XMLSupport::parse_float( vs_config->getVariable( "physics", "max_shield_leak", "0" ) ) ) );*/
            //char newleak = float_to_int( std::max( min_shield_leak, std::max( max_shield_leak, (char) ( (randnum-.9)*10.0*100.0 ) ) ) );
            // TODO: lib_damage if (shield.leak < newleak)
            //shield.leak = newleak;
        } else if (randnum >= .7) {
            // TODO: lib_damage shield.recharge *= dam;
        } else if (randnum >= .5) {
            static float mindam =
                    XMLSupport::parse_float(vs_config->getVariable("physics", "min_recharge_shot_damage", "0.5"));
            if (dam < mindam) {
                dam = mindam;
            }
            // TODO: this->recharge *= dam;
        } else if (randnum >= .2) {
            static float mindam =
                    XMLSupport::parse_float(vs_config->getVariable("physics", "min_maxenergy_shot_damage", "0.2"));
            if (dam < mindam) {
                dam = mindam;
            }
            // TODO: energy.DowngradeByPercent(dam);
        } else if (repair_droid > 0) {
            repair_droid--;
        }
        damages |= Damages::JUMP_DAMAGED;
        return;
    }
    if (degrees >= 150 && degrees <= 180) {
        //DAMAGE ENGINES
        if (randnum >= .8) {
            computer.max_combat_ab_speed *= dam;
        } else if (randnum >= .6) {
            computer.max_combat_speed *= dam;
        } else if (randnum >= .4) {
            limits.afterburn *= dam;
        } else if (randnum >= .2) {
            limits.vertical *= dam;
        } else {
            limits.forward *= dam;
        }
        damages |= Damages::LIMITS_DAMAGED;
        return;
    }
}

void Unit::Kill(bool erasefromsave, bool quitting) {
    if (this->colTrees) {
        this->colTrees->Dec();
    }           //might delete
    this->colTrees = NULL;
    killSounds();
    ClearMounts();

    if (docked & (DOCKING_UNITS)) {
        static float survival =
                XMLSupport::parse_float(vs_config->getVariable("physics", "survival_chance_on_base_death", "0.1"));
        static float player_survival =
                XMLSupport::parse_float(vs_config->getVariable("physics",
                        "player_survival_chance_on_base_death",
                        "1.0"));
        static int i_survival = float_to_int((RAND_MAX * survival));
        static int i_player_survival = float_to_int((RAND_MAX * player_survival));

        vector<Unit *> dockedun;
        unsigned int i;
        for (i = 0; i < pImage->dockedunits.size(); ++i) {
            Unit *un;
            if (NULL != (un = pImage->dockedunits[i]->uc.GetUnit())) {
                dockedun.push_back(un);
            }
        }
        while (!dockedun.empty()) {

            dockedun.back()->UnDock(this);

            if (rand() <= (UnitUtil::isPlayerStarship(dockedun.back()) ? i_player_survival : i_survival)) {
                dockedun.back()->Kill();
            }
            dockedun.pop_back();
        }
    }
    //eraticate everything. naturally (see previous line) we won't erraticate beams erraticated above
    if (!isSubUnit()) {
        RemoveFromSystem();
    }
    computer.target.SetUnit(NULL);

    //God I can't believe this next line cost me 1 GIG of memory until I added it
    computer.threat.SetUnit(NULL);
    computer.velocity_ref.SetUnit(NULL);
    computer.force_velocity_ref = true;
    if (aistate) {
        aistate->ClearMessages();
        aistate->Destroy();
    }
    aistate = NULL;

    // The following we don't want to do twice
    killed = true;
    Unit *un;
    for (un_iter iter = getSubUnits(); (un = *iter); ++iter) {
        un->Kill();
    }

    //if (isUnit() != Vega_UnitType::missile) {
    //    VS_LOG(info, (boost::format("UNIT HAS DIED: %1% %2% (file %3%)") % name.get() % fullname % filename.get()));
    //}

    if (ucref == 0) {
        VS_LOG(trace, (boost::format("UNIT DELETION QUEUED: %1$s %2$s (file %3$s, addr 0x%4$08x)")
                % name.get().c_str() % fullname.c_str() % filename.get().c_str() % this));
        Unitdeletequeue.push_back(this);
        if (flightgroup) {
            if (flightgroup->leader.GetUnit() == this) {
                flightgroup->leader.SetUnit(NULL);
            }
        }

//#ifdef DESTRUCTDEBUG
//        VS_LOG(trace, (boost::format("%s 0x%x - %d") % name.get().c_str() % this % Unitdeletequeue.size()));
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
        Unitdeletequeue.push_back(this);
#ifdef DESTRUCTDEBUG
        VS_LOG(trace, (boost::format("%1$s %2$x - %3$d") % name.get().c_str() % this % Unitdeletequeue.size()));
#endif
    }
}

float Unit::ExplosionRadius() {
    static float expsize = XMLSupport::parse_float(vs_config->getVariable("graphics", "explosion_size", "3"));
    return expsize * rSize();
}

void Unit::ProcessDeleteQueue() {
    while (!Unitdeletequeue.empty()) {
#ifdef DESTRUCTDEBUG
                                                                                                                                VS_LOG_AND_FLUSH(trace, (boost::format("Eliminatin' %1$x - %2$d") % Unitdeletequeue.back() % Unitdeletequeue.size()));
        VS_LOG_AND_FLUSH(trace, (boost::format("Eliminatin' %1$s") % Unitdeletequeue.back()->name.get().c_str()));
#endif
#ifdef DESTRUCTDEBUG
                                                                                                                                if ( Unitdeletequeue.back()->isSubUnit() ) {
            VS_LOG(debug, "Subunit Deleting (related to double dipping)");
        }
#endif
        Unit *mydeleter = Unitdeletequeue.back();
        Unitdeletequeue.pop_back();
        delete mydeleter;                        ///might modify unitdeletequeue

#ifdef DESTRUCTDEBUG
        VS_LOG_AND_FLUSH(trace, (boost::format("Completed %1$d") % Unitdeletequeue.size()));
#endif
    }
}

Unit *makeBlankUpgrade(string templnam, int faction) {
    Unit *bl = new Unit(templnam.c_str(), true, faction);
    for (int i = bl->numCargo() - 1; i >= 0; i--) {
        int q = bl->GetCargo(i).GetQuantity();
        bl->RemoveCargo(i, q);
    }
    bl->setMass(0);
    return bl;
}

static const string LOAD_FAILED = "LOAD_FAILED";

const Unit *makeFinalBlankUpgrade(string name, int faction) {
    char *unitdir = GetUnitDir(name.c_str());
    string limiternam = name;
    if (unitdir != name) {
        limiternam = string(unitdir) + string(".blank");
    }
    free(unitdir);
    const Unit *lim = UnitConstCache::getCachedConst(StringIntKey(limiternam, faction));
    if (!lim) {
        lim = UnitConstCache::setCachedConst(StringIntKey(limiternam, faction), makeBlankUpgrade(limiternam, faction));
    }
    if (lim->name == LOAD_FAILED) {
        lim = NULL;
    }
    return lim;
}

const Unit *makeTemplateUpgrade(string name, int faction) {
    char *unitdir = GetUnitDir(name.c_str());
    string limiternam = string(unitdir) + string(".template");
    free(unitdir);
    const Unit *lim = UnitConstCache::getCachedConst(StringIntKey(limiternam, faction));
    if (!lim) {
        lim =
                UnitConstCache::setCachedConst(StringIntKey(limiternam,
                        faction), new Unit(limiternam.c_str(), true, faction));
    }
    if (lim->name == LOAD_FAILED) {
        lim = NULL;
    }
    return lim;
}

const Unit *loadUnitByCache(std::string name, int faction) {
    const Unit *temprate = UnitConstCache::getCachedConst(StringIntKey(name, faction));
    if (!temprate) {
        temprate =
                UnitConstCache::setCachedConst(StringIntKey(name, faction), new Unit(name.c_str(), true, faction));
    }
    return temprate;
}

bool DestroySystem(float hull, float maxhull, float numhits) {
    static float damage_chance = XMLSupport::parse_float(vs_config->getVariable("physics", "damage_chance", ".005"));
    static float guaranteed_chance =
            XMLSupport::parse_float(vs_config->getVariable("physics", "definite_damage_chance", ".1"));
    float chance = 1 - (damage_chance * (guaranteed_chance + (maxhull - hull) / maxhull));
    if (numhits > 1) {
        chance = pow(chance, numhits);
    }
    return rand01() > chance;
}

bool DestroyPlayerSystem(float hull, float maxhull, float numhits) {
    static float
            damage_chance = XMLSupport::parse_float(vs_config->getVariable("physics", "damage_player_chance", ".5"));
    static float guaranteed_chance =
            XMLSupport::parse_float(vs_config->getVariable("physics", "definite_damage_chance", ".1"));
    float chance = 1 - (damage_chance * (guaranteed_chance + (maxhull - hull) / maxhull));
    if (numhits > 1) {
        chance = pow(chance, numhits);
    }
    bool ret = (rand01() > chance);
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
        Unit *su;
        bool inrange = (targ != NULL) ? InRange(targ) : true;
        if (inrange) {
            for (un_iter iter = getSubUnits(); (su = *iter); ++iter) {
                su->Target(targ);
                su->TargetTurret(targ);
            }
        }
    }
}

void WarpPursuit(Unit *un, StarSystem *sourcess, std::string destination) {
    static bool AINotUseJump = XMLSupport::parse_bool(vs_config->getVariable("physics", "no_ai_jump_points", "false"));
    if (AINotUseJump) {
        static float seconds_per_parsec =
                XMLSupport::parse_float(vs_config->getVariable("physics", "seconds_per_parsec", "10"));
        float ttime =
                (SystemLocation(sourcess->getFileName()) - SystemLocation(destination)).Magnitude()
                        * seconds_per_parsec;
        un->jump.delay += float_to_int(ttime);
        sourcess->JumpTo(un, NULL, destination, true, true);
        un->jump.delay -= float_to_int(ttime);
    }
}

//WARNING : WHEN TURRETS WE MAY NOT WANT TO ASK THE SERVER FOR INFOS ! ONLY FOR LOCAL PLAYERS (_Universe-isStarship())





void Unit::Target(Unit *targ) {
    if (targ == this) {
        return;
    }

    if (!(activeStarSystem == NULL || activeStarSystem == _Universe->activeStarSystem())) {
        computer.target.SetUnit(NULL);
        return;
    }
    if (targ) {
        if (targ->activeStarSystem == _Universe->activeStarSystem() || targ->activeStarSystem == NULL) {
            if (targ != Unit::Target()) {
                for (int i = 0; i < getNumMounts(); ++i) {
                    mounts[i].time_to_lock = mounts[i].type->lock_time;
                }

                computer.target.SetUnit(targ);
                radar.Unlock();
            }
        } else {
            if (jump.drive != -1) {
                bool found = false;
                Unit *u;
                for (un_iter i = _Universe->activeStarSystem()->getUnitList().createIterator(); (u = *i) != NULL; ++i) {
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
                computer.target.SetUnit(NULL);
            }
        }
    } else {
        computer.target.SetUnit(NULL);
    }
}

void Unit::VelocityReference(Unit *targ) {
    computer.force_velocity_ref = !!targ;
    computer.velocity_ref.SetUnit(targ);
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
    if (this->pImage->pExplosion == NULL && this->pImage->timeexplode == 0) {
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
            static Animation cache(game_options()->explosion_animation.c_str(), false, .1, BILINEAR, false);
            bleh = getRandomCachedAniString();
            if (bleh.size() == 0) {
                bleh = game_options()->explosion_animation;
            }
        }
        this->pImage->pExplosion = new Animation(bleh.c_str(), game_options()->explosion_face_player, .1, BILINEAR, true);
        this->pImage->pExplosion->SetDimensions(this->ExplosionRadius(), this->ExplosionRadius());
        Vector p, q, r;
        this->GetOrientation(p, q, r);
        this->pImage->pExplosion->SetOrientation(p, q, r);
        if (this->isUnit() != Vega_UnitType::missile) {
            _Universe->activeStarSystem()->AddMissileToQueue(new MissileEffect(this->Position(),
                    this->MaxShieldVal(),
                    0,
                    this->ExplosionRadius()
                            * game_options()->explosion_damage_center,
                    this->ExplosionRadius()
                            * game_options()->explosion_damage_center
                            * game_options()->explosion_damage_edge,
                    NULL));
        }
        QVector exploc = this->cumulative_transformation.position;
        bool sub = this->isSubUnit();
        Unit *un = NULL;
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
            if (this->isUnit() == Vega_UnitType::unit) {
                if (rand() < RAND_MAX * game_options()->percent_shockwave && (!this->isSubUnit())) {
                    static string shockani(game_options()->shockwave_animation);
                    static Animation *__shock__ani = new Animation(shockani.c_str(), true, .1, MIPMAP, false);

                    __shock__ani->SetFaceCam(false);
                    unsigned int which = AddAnimation(this->Position(),
                            this->ExplosionRadius(),
                            true,
                            shockani,
                            game_options()->shockwave_growth);
                    Animation *ani = GetVolatileAni(which);
                    if (ani) {
                        ani->SetFaceCam(false);
                        Vector p, q, r;
                        this->GetOrientation(p, q, r);
                        int tmp = rand();
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
                        static float lasttime = 0;
                        float newtime = getNewTime();
                        if (newtime - lasttime > game_options()->time_between_music
                                || (_Universe->isPlayerStarship(this) && this->isUnit() != Vega_UnitType::missile
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
            (this->pImage->timeexplode > game_options()->debris_time || this->isUnit() == Vega_UnitType::missile
                    || _Universe->AccessCockpit()->GetParent() == this || this->SubUnits.empty());
    if (this->pImage->pExplosion) {
        this->pImage->timeexplode += timeit;
        this->pImage->pExplosion->SetPosition(this->Position());
        Vector p, q, r;
        this->GetOrientation(p, q, r);
        this->pImage->pExplosion->SetOrientation(p, q, r);
        if (this->pImage->pExplosion->Done() && timealldone) {
            delete this->pImage->pExplosion;
            this->pImage->pExplosion = NULL;
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
    if ((game_options()->eject_cargo_on_blowup > 0) && (this->numCargo() > 0)) {
        unsigned int dropcount = floorf(this->numCargo() / game_options()->eject_cargo_on_blowup) + 1;
        if (dropcount > this->numCargo()) {
            dropcount = this->numCargo();
        }
        for (unsigned int i = 0; i < dropcount; i++) {
            this->EjectCargo(this->numCargo() - 1);
        }              //Ejecting the last one is somewhat faster
    }
    return alldone || (!timealldone);
}

float Unit::ExplodingProgress() const {
    static float debrisTime = XMLSupport::parse_float(vs_config->getVariable("physics", "debris_time", "500"));
    return std::min(pImage->timeexplode / debrisTime, 1.0f);
}

void Unit::SetCollisionParent(Unit *name) {
    assert(0);                                         //deprecated... many less collisions with subunits out of the table
#if 0
                                                                                                                            for (int i = 0; i < numsubunit; ++i) {
        subunits[i]->CollideInfo.object.u = name;
        subunits[i]->SetCollisionParent( name );
    }
#endif
}

//This function should not be used on server side
extern vector<Vector> perplines;
extern vector<int> turretcontrol;

float Unit::querySphereClickList(const QVector &st, const QVector &dir, float err) const {
    unsigned int i;
    float retval = 0;
    float adjretval = 0;
    const Matrix *tmpo = &cumulative_transformation_matrix;

    Vector TargetPoint(tmpo->getP());
    for (i = 0; i < nummesh(); ++i) {
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
    pImage->dockedunits[i]->uc.SetUnit(NULL);
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
    static bool changeddockedorient =
            (XMLSupport::parse_bool(vs_config->getVariable("physics", "change_docking_orientation", "false")));
    if (!changeddockedorient) {
        holder.orientation = bak;
    }
    return holder;
}

extern void ExecuteDirector();

void Unit::PerformDockingOperations() {
    for (unsigned int i = 0; i < pImage->dockedunits.size(); ++i) {
        Unit *un;
        if ((un = pImage->dockedunits[i]->uc.GetUnit()) == NULL) {
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

void UpdateMasterPartList(Unit *);

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
    UpdateMasterPartList(UniverseUtil::GetMasterPartList());
    unsigned int cockpit = UnitUtil::isPlayerStarship(this);

    // TODO: reimplement SPEC refueling (why?)
    /*static float MinimumCapacityToRefuelOnLand =
            XMLSupport::parse_float(vs_config->getVariable("physics",
                    "MinimumWarpCapToRefuelDockeesAutomatically",
                    "0"));
    float capdata = utdw->warpCapData();
    if ((capdata >= MinimumCapacityToRefuelOnLand) && (this->refillWarpEnergy())) {
        if (cockpit >= 0 && cockpit < _Universe->numPlayers()) {
            static float
                    docking_fee = XMLSupport::parse_float(vs_config->getVariable("general", "fuel_docking_fee", "0"));
            _Universe->AccessCockpit(cockpit)->credits -= docking_fee;
        }
    }*/

    // This code refuels from one ship to the other but does it for spec,
    // which doesn't make much sense. Probably more relevant for WC.
    // TODO: we can do better. delete!
    /*if ((capdata < MinimumCapacityToRefuelOnLand) && (this->faction == utdw->faction)) {
        if (utdw->warpEnergyData() > this->warpEnergyData() && utdw->warpEnergyData() > this->jump.energy) {
            this->increaseWarpEnergy(false, this->jump.energy);
            utdw->decreaseWarpEnergy(false, this->jump.energy);
        }
        if (utdw->warpEnergyData() < this->warpEnergyData() && this->warpEnergyData() > utdw->jump.energy) {
            utdw->increaseWarpEnergy(false, utdw->jump.energy);
            this->decreaseWarpEnergy(false, utdw->jump.energy);
        }
    }*/
    if (cockpit >= 0 && cockpit < _Universe->numPlayers()) {
        static float docking_fee = XMLSupport::parse_float(vs_config->getVariable("general", "docking_fee", "0"));
        if (_Universe->AccessCockpit(cockpit)->credits >= docking_fee) {
            _Universe->AccessCockpit(cockpit)->credits -= docking_fee;
        } else if (_Universe->AccessCockpit(cockpit)->credits >= 0) {
            _Universe->AccessCockpit(cockpit)->credits = 0;
        }
    }
    std::set<Unit *>::iterator arrested = arrested_list_do_not_dereference.find(this);
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
        if ((whichdockport = utdw->CanDockWithMe(this)) != -1) {
            utdw->pImage->clearedunits.erase(lookcleared);
            return ForceDock(utdw, whichdockport);
        }
    }
    return 0;
}

inline bool insideDock(const DockingPorts &dock, const QVector &pos, float radius) {
    if (dock.IsOccupied()) {
        return false;
    }
    return IsShorterThan(pos - dock.GetPosition(), double(radius + dock.GetRadius()));
}

int Unit::CanDockWithMe(Unit *un, bool force) {
    //don't need to check relation: already cleared.

    // If your unit has docking ports then we check if any of our docking
    // ports overlap with any of the station's docking ports.
    // Otherwise we simply check if our unit overlaps with any of the
    // station's docking ports.
    for (unsigned int i = 0; i < pImage->dockingports.size(); ++i) {
        if (!un->pImage->dockingports.empty()) {
            for (unsigned int j = 0; j < un->pImage->dockingports.size(); ++j) {
                if (insideDock(pImage->dockingports[i],
                        InvTransform(GetTransformation(),
                                Transform(un->GetTransformation(),
                                        un->pImage->dockingports[j].GetPosition().Cast())),
                        un->pImage->dockingports[j].GetRadius())) {
                    // We cannot dock if we are already docked
                    if (((un->docked & (DOCKED_INSIDE | DOCKED)) == 0) && (!(docked & DOCKED_INSIDE))) {
                        return i;
                    }
                }
            }
        } else if (insideDock(pImage->dockingports[i],
                InvTransform(GetTransformation(), un->Position()),
                un->rSize())) {
            return i;
        }
    }
    if (force) {
        for (unsigned int i = 0; i < pImage->dockingports.size(); ++i) {
            if (!pImage->dockingports[i].IsOccupied()) {
                return i;
            }
        }
    }
    return -1;
}

bool Unit::IsCleared(const Unit *DockingUnit) const {
    return std::find(pImage->clearedunits.begin(), pImage->clearedunits.end(), DockingUnit)
            != pImage->clearedunits.end();
}

bool Unit::hasPendingClearanceRequests() const {
    return pImage && (pImage->clearedunits.size() > 0);
}

bool Unit::isDocked(const Unit *d) const {
    if (!d) {
        return false;
    }
    if (!(d->docked & (DOCKED_INSIDE | DOCKED))) {
        return false;
    }
    for (unsigned int i = 0; i < pImage->dockedunits.size(); ++i) {
        Unit *un;
        if ((un = pImage->dockedunits[i]->uc.GetUnit()) != NULL) {
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
            this->owner = NULL;
        }
    }
    VS_LOG(trace, "Asking to undock");
    for (i = 0; i < utdw->pImage->dockedunits.size(); ++i) {
        if (utdw->pImage->dockedunits[i]->uc.GetUnit() == this) {
            utdw->FreeDockingPort(i);
            i--;
            SetVisible(true);
            docked &= (~(DOCKED_INSIDE | DOCKED));
            pImage->DockedTo.SetUnit(NULL);
            Velocity = utdw->Velocity;
            static float
                    launch_speed = XMLSupport::parse_float(vs_config->getVariable("physics", "launch_speed", "-1"));
            static bool auto_turn_towards =
                    XMLSupport::parse_bool(vs_config->getVariable("physics", "undock_turn_away", "true"));

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
    bool cancompletefully = true;
    int j;
    std::string turSize;
    un_iter ui;
    bool found = false;
    for (j = 0, ui = getSubUnits(); !ui.isDone() && j < subunitoffset; ++ui, ++j) {
    }     ///set the turrets to the offset
    un_kiter upturrets;
    Unit *giveAway;

    giveAway = *ui;
    if (giveAway == NULL) {
        return true;
    }
    bool hasAnyTurrets = false;
    turSize = getTurretSize(giveAway->name);
    //begin going through other unit's turrets
    for (upturrets = up->viewSubUnits(); ((*upturrets) != NULL) && ((*ui) != NULL); ++ui, ++upturrets) {
        hasAnyTurrets = true;
        const Unit *addtome;

        addtome = *upturrets;                    //set pointers

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
                    un->limits.yaw = 0;
                    un->limits.pitch = 0;
                    un->limits.roll = 0;
                    un->limits.lateral = un->limits.retro = un->limits.forward = un->limits.afterburn = 0.0;

                    un->name = turSize + "_blank";
                    if (un->pImage->unitwriter != NULL) {
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
    if ((*upturrets) != NULL) {
        return false;
    }
    return cancompletefully;
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

bool Unit::canDowngrade(const Unit *downgradeor,
        int mountoffset,
        int subunitoffset,
        double &percentage,
        const Unit *downgradelimit,
        bool gen_downgrade_list) {
    return UpAndDownGrade(downgradeor,
            NULL,
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
            NULL,
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

double ComputeMinDowngradePercent() {
    static float MyPercentMin =
            XMLSupport::parse_float(vs_config->getVariable("general", "remove_downgrades_less_than_percent", ".9"));
    return MyPercentMin;
}

class DoubleName {
public:
    string s;
    double d;

    DoubleName(string ss, double dd) {
        d = dd;
        s = ss;
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
    unsigned int cargonum;
    Cargo *cargo = GetCargo(file, cargonum);
    if (cargo) {
        cargo->SetInstalled(true);
    }
    char *unitdir = GetUnitDir(this->name.get().c_str());
    string templnam = string(unitdir) + ".template";
    const Unit *templ = UnitConstCache::getCachedConst(StringIntKey(templnam, this->faction));
    if (templ == NULL) {
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
                    ((templ->name == "LOAD_FAILED") ? NULL : templ),
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
    vsUMap<int, DoubleName>::iterator i = downgrademap.find(unitoffset);
    if (i != downgrademap.end()) {
        if ((*i).second.d <= value) {
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
    vsUMap<int, DoubleName>::iterator i = downgrademap.begin();
    std::set<std::string> retval;
    for (; i != downgrademap.end(); ++i) {
        retval.insert((*i).second.s);
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
            UnitHasRecursiveData::const_iterator iter2 = iter->second.find(key);
            if (iter2 != iter->second.end()) {
                return iter2->second;
            }
        }
    } else {
        has_recursive_data.resize(fac + 1);
    }
    bool retval = false;
    string faction = FactionUtil::GetFactionName(fac);
    string lkey = key;
    string::size_type lkstart = 0;
    string::size_type lkend = lkey.find('|');
    //Big short circuit - avoids recursion
    while (!retval && (lkstart != string::npos)) {
        string skey = lkey.substr(lkstart, (lkend == string::npos) ? string::npos : lkend - lkstart);
        string lus = UniverseUtil::LookupUnitStat(name, faction, skey);

        retval = (lus.length() != 0);

        lkstart = (lkend != string::npos) ? lkend + 1 : string::npos;
        lkend = lkey.find('|', lkstart);
    }
    if (!retval) {
        //Big short circuit - avoids recursion
        string::size_type when;
        string upgrades = UniverseUtil::LookupUnitStat(name, faction, upgradeString);
        string::size_type ofs = 0;
        while (!retval && ((when = upgrades.find('{', ofs)) != string::npos)) {
            string::size_type where = upgrades.find('}', when + 1);
            string upgrade = upgrades.substr(when + 1, ((where != string::npos) ? where - when - 1 : string::npos));
            retval = cell_has_recursive_data(upgrade, fac, key);
            ofs = where + 1;
        }
    }
    has_recursive_data[fac][name][key] = retval;
    return retval;
}

#define STDUPGRADE_SPECIFY_DEFAULTS(my, oth, temp, noth, dgradelimer, dgradelimerdefault, clamp, value_to_lookat) \
    do {                                                                                                            \
        retval =                                                                                                    \
            (                                                                                                       \
                UpgradeFloat(                                                                                       \
                    resultdoub,                                                                                     \
                    my,                                                                                             \
                    oth,                                                                                            \
                    (templ != NULL) ? temp : 0,                                                                     \
                    Adder, Comparer, noth, noth,                                                                    \
                    Percenter, temppercent,                                                                         \
                    forcetransaction,                                                                               \
                    templ != NULL,                                                                                  \
                    (downgradelimit != NULL) ? dgradelimer : dgradelimerdefault,                                    \
                    AGreaterB,                                                                                      \
                    clamp,                                                                                          \
                    force_change_on_nothing                                                                         \
                            )                                                                                       \
            );                                                                                                      \
        if (retval == UPGRADEOK)                                                                                    \
        {                                                                                                           \
            if (touchme)                                                                                            \
                my = resultdoub;                                                                                    \
            percentage += temppercent;                                                                              \
            ++numave;                                                                                               \
            can_be_redeemed = true;                                                                                 \
            if (gen_downgrade_list)                                                                                 \
                AddToDowngradeMap( up->name, oth, ( (char*) &value_to_lookat )-(char*) this, tempdownmap );         \
        }                                                                                                           \
        else if (retval != NOTTHERE)                                                                                \
        {                                                                                                           \
            if (retval == CAUSESDOWNGRADE)                                                                          \
                needs_redemption = true;                                                                            \
            else                                                                                                    \
                cancompletefully = false;                                                                           \
        }                                                                                                           \
    }                                                                                                               \
    while (0)

#define STDUPGRADE(my, oth, temp, noth)                \
    do {STDUPGRADE_SPECIFY_DEFAULTS( my,                 \
                                     oth,                \
                                     temp,               \
                                     noth,               \
                                     downgradelimit->my, \
                                     blankship->my,      \
                                     false,              \
                                     this->my ); }       \
    while (0)

#define STDUPGRADECLAMP(my, oth, temp, noth)                 \
    do {STDUPGRADE_SPECIFY_DEFAULTS( my,                       \
                                     oth,                      \
                                     temp,                     \
                                     noth,                     \
                                     downgradelimit->my,       \
                                     blankship->my,            \
                                     !force_change_on_nothing, \
                                     this->my ); }             \
    while (0)

// TODO: get rid of this
extern float accelStarHandler(float &input);
float speedStarHandler(float &input);

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

    static bool
            csv_cell_null_check = XMLSupport::parse_bool(vs_config->getVariable("data", "empty_cell_check", "true"));
    int numave = 0;
    bool cancompletefully = true;
    bool can_be_redeemed = false;
    bool needs_redemption = false;
    if (mountoffset >= 0) {
        cancompletefully = UpgradeMounts(up, mountoffset, touchme, downgrade, numave, percentage);
    }
    bool cancompletefully1 = true;
    if (subunitoffset >= 0) {
        cancompletefully1 = UpgradeSubUnits(up, subunitoffset, touchme, downgrade, numave, percentage);
    }
    cancompletefully = cancompletefully && cancompletefully1;
    adder Adder;
    comparer Comparer;
    percenter Percenter;
    vsUMap<int, DoubleName> tempdownmap;
    if (cancompletefully && cancompletefully1 && downgrade) {
        if (percentage > 0) {
            AddToDowngradeMap(up->name, 1, curdowngrademapoffset++, tempdownmap);
        }
    }
    float tmax_speed = up->computer.max_combat_speed;
    float tmax_ab_speed = up->computer.max_combat_ab_speed;
    float tmax_yaw_right = up->computer.max_yaw_right;
    float tmax_yaw_left = up->computer.max_yaw_left;
    float tmax_pitch_up = up->computer.max_pitch_up;
    float tmax_pitch_down = up->computer.max_pitch_down;
    float tmax_roll_right = up->computer.max_roll_right;
    float tmax_roll_left = up->computer.max_roll_left;
    float tlimits_yaw = up->limits.yaw;
    float tlimits_roll = up->limits.roll;
    float tlimits_pitch = up->limits.pitch;
    float tlimits_lateral = up->limits.lateral;
    float tlimits_vertical = up->limits.vertical;
    float tlimits_forward = up->limits.forward;
    float tlimits_retro = up->limits.retro;
    float tlimits_afterburn = up->limits.afterburn;
    if (downgrade) {
        Adder = &SubtractUp;
        Percenter = &computeDowngradePercent;
        Comparer = &GreaterZero;
    } else {
        if (additive == 1) {
            Adder = &AddUp;
            Percenter = &computeAdderPercent;
        } else if (additive == 2) {
            Adder = &MultUp;
            Percenter = &computeMultPercent;
            tmax_speed = speedStarHandler(tmax_speed);
            tmax_ab_speed = speedStarHandler(tmax_ab_speed);
            tmax_yaw_right = speedStarHandler(tmax_yaw_right);
            tmax_yaw_left = speedStarHandler(tmax_yaw_left);
            tmax_pitch_up = speedStarHandler(tmax_pitch_up);
            tmax_pitch_down = speedStarHandler(tmax_pitch_down);
            tmax_roll_right = speedStarHandler(tmax_roll_right);
            tmax_roll_left = speedStarHandler(tmax_roll_left);
            tlimits_yaw = speedStarHandler(tlimits_yaw);
            tlimits_pitch = speedStarHandler(tlimits_pitch);
            tlimits_roll = speedStarHandler(tlimits_roll);
            tlimits_forward = accelStarHandler(tlimits_forward);
            tlimits_retro = accelStarHandler(tlimits_retro);
            tlimits_lateral = accelStarHandler(tlimits_lateral);
            tlimits_vertical = accelStarHandler(tlimits_vertical);
            tlimits_afterburn = accelStarHandler(tlimits_afterburn);
        } else {
            Adder = &GetsB;
            Percenter = &computePercent;
        }
        Comparer = AGreaterB;
    }
    double resultdoub;
    int retval = 0; //"= 0" added by chuck_starchaser to shut off a warning about its possibly being used uninitialized
    double temppercent;
    static Unit *blankship = NULL;
    static bool initblankship = false;
    if (!initblankship) {
        blankship = this;
        initblankship = true;
        blankship = new Unit("upgrading_dummy_unit", true, FactionUtil::GetUpgradeFaction());
    }
    //set up vars for "LookupUnitStat" to check for empty cells
    string upgrade_name = up->name;
    //Check SPEC stuff
    if (!csv_cell_null_check || force_change_on_nothing
            || cell_has_recursive_data(upgrade_name, up->faction,
                    "Spec_Interdiction|Warp_Min_Multiplier|Warp_Max_Multiplier")) {
        if (!csv_cell_null_check || force_change_on_nothing
                || cell_has_recursive_data(upgrade_name, up->faction, "Spec_Interdiction")) {
            bool upneg = up->specInterdiction < 0;
            bool interdictionUnits = specInterdiction > 0;
            specInterdiction = fabs(specInterdiction);
            STDUPGRADE(specInterdiction, fabs(up->specInterdiction), upneg ? fabs(
                    templ->specInterdiction) : templ->specInterdiction, 0);
            if (upneg) {
                specInterdiction = -specInterdiction;
            }
            if (interdictionUnits != (specInterdiction > 0)) {
                StarSystem *ss = activeStarSystem;
                if (_Universe->getNumActiveStarSystem() && !ss) {
                    ss = _Universe->activeStarSystem();
                }
                if (ss) {
                    Unit *un;
                    for (un_iter i = ss->gravitationalUnits().createIterator(); (un = *i); ++i) {
                        if (un == this) {
                            i.remove();
                            //NOTE: I think we can only be in here once
                            break;
                        }
                    }
                    if (!interdictionUnits) {
                        //will interdict
                        ss->gravitationalUnits().prepend(this);
                    }
                }
            }
        }
        if (!csv_cell_null_check || force_change_on_nothing
                || cell_has_recursive_data(upgrade_name, up->faction, "Warp_Min_Multiplier")) {
            STDUPGRADE(graphicOptions.MinWarpMultiplier,
                    up->graphicOptions.MinWarpMultiplier,
                    templ->graphicOptions.MinWarpMultiplier,
                    1);
        }
        if (!csv_cell_null_check || force_change_on_nothing
                || cell_has_recursive_data(upgrade_name, up->faction, "Warp_Max_Multiplier")) {
            STDUPGRADE(graphicOptions.MaxWarpMultiplier,
                    up->graphicOptions.MaxWarpMultiplier,
                    templ->graphicOptions.MaxWarpMultiplier,
                    1);
        }
    }
    //Check jump and jump/SPEC stuff
    if (!csv_cell_null_check || force_change_on_nothing
            || cell_has_recursive_data(upgrade_name, up->faction,
                    "Warp_Capacitor|Warp_Usage_Cost")) {
        if (!csv_cell_null_check || force_change_on_nothing
                || cell_has_recursive_data(upgrade_name, up->faction, "Warp_Capacitor"))
            // TODO: STDUPGRADE(maxwarpenergy, up->maxwarpenergy, templ->maxwarpenergy, 0);
        if (!csv_cell_null_check || force_change_on_nothing
                || cell_has_recursive_data(upgrade_name, up->faction, "Warp_Usage_Cost"))
            STDUPGRADE(jump.insysenergy, up->jump.insysenergy, templ->jump.insysenergy, 0);

// for when we'll need more than one jump drive upgrade (Elite Strike?)
        if (!csv_cell_null_check || force_change_on_nothing
                || cell_has_recursive_data(upgrade_name, up->faction, "Outsystem_Jump_Cost"))
            STDUPGRADE(jump.energy, up->jump.energy, templ->jump.energy, 0);
        if (!csv_cell_null_check || force_change_on_nothing
                || cell_has_recursive_data(upgrade_name, up->faction, "Jump_Drive_Delay"))
            STDUPGRADE(jump.delay, up->jump.delay, templ->jump.delay, 0);

    }

    /*if (!csv_cell_null_check || force_change_on_nothing
            || cell_has_recursive_data(upgrade_name, up->faction, "Armor_Front_Top_Right")) {
        for (int i = 0; i < 8; i++) {
            STDUPGRADE(armor->facets[i].health,
                    up->armor->facets[i].health,
                    templ->armor->facets[i].health, 0);
            armor->facets[i].max_health = armor->facets[i].health;
        }
    }*/

    // TODO: lib_damage all of this should be implemented better elsewhere
    // Probably in DamageableFactory
    // Upgrade shield regeneration

    // Because of the complex macros taking partial expressions and building code from them,
    // this is the easiest way to refactor
    /*float previous = shield->GetRegeneration();

    if (!csv_cell_null_check || force_change_on_nothing
            || cell_has_recursive_data(upgrade_name, up->faction, "Shield_Recharge"))
        STDUPGRADE(shield_regeneration,
                up->shield_regeneration,
                templ->shield_regeneration, 0);
    bool upgradedrecharge = (previous != shield_regeneration);
    if (upgradedrecharge) {

        shield->UpdateRegeneration(shield_regeneration);
    }*/

    // Upgrade hull health
    upgrade_hull = *current_hull;

    if (up && up->current_hull) {
        const_cast<Unit *>(up)->upgrade_hull = *up->current_hull;
    }

    if (templ && templ->current_hull) {
        const_cast<Unit *>(templ)->upgrade_hull = *templ->current_hull;
    }

    if (!csv_cell_null_check || force_change_on_nothing || cell_has_recursive_data(upgrade_name, up->faction, "Hull")) {
        STDUPGRADE(upgrade_hull, up->upgrade_hull, templ->upgrade_hull, 0);
    }

    /*if ((hull->facets[0].max_health < hull->facets[0].health) && (!Destroyed())) {
        hull->facets[0].max_health = hull->facets[0].health;
    }*/

    //if (!csv_cell_null_check || force_change_on_nothing
      //      || cell_has_recursive_data(upgrade_name, up->faction, "Reactor_Recharge"))
        // TODO: STDUPGRADE(recharge, up->recharge, templ->recharge, 0);
    static bool unittable = XMLSupport::parse_bool(vs_config->getVariable("physics", "UnitTable", "false"));
    //Uncommon fields (capacities... rates... etc...)
    if (!csv_cell_null_check || force_change_on_nothing
            || cell_has_recursive_data(upgrade_name,
                    up->faction,
                    "Heat_Sink_Rating|Repair_Droid|Hold_Volume|Upgrade_Storage_Volume|Equipment_Space|Hidden_Hold_Volume|ECM_Rating|Primary_Capacitor|Warp_Capacitor")) {
        if (!csv_cell_null_check || force_change_on_nothing
                || cell_has_recursive_data(upgrade_name, up->faction, "Heat_Sink_Rating"))
            STDUPGRADE(HeatSink, up->HeatSink, templ->HeatSink, 0);
        if (!csv_cell_null_check || force_change_on_nothing
                || cell_has_recursive_data(upgrade_name, up->faction, "Repair_Droid"))
            STDUPGRADE(repair_droid, up->repair_droid, templ->repair_droid, 0);
        if (!csv_cell_null_check || force_change_on_nothing
                || cell_has_recursive_data(upgrade_name, up->faction, "Hold_Volume"))
            STDUPGRADE(CargoVolume, up->CargoVolume, templ->CargoVolume, 0);
        if (!csv_cell_null_check || force_change_on_nothing
                || cell_has_recursive_data(upgrade_name, up->faction, "Upgrade_Storage_Volume"))
            STDUPGRADE(UpgradeVolume, up->UpgradeVolume, templ->UpgradeVolume, 0);
        if (!csv_cell_null_check || force_change_on_nothing
                || cell_has_recursive_data(upgrade_name, up->faction, "Equipment_Space"))
            STDUPGRADE(equipment_volume, up->equipment_volume, templ->equipment_volume, 0);
        if (!csv_cell_null_check || force_change_on_nothing
                || cell_has_recursive_data(upgrade_name, up->faction, "Hidden_Hold_Volume"))
            STDUPGRADE(HiddenCargoVolume, up->HiddenCargoVolume, templ->HiddenCargoVolume, 0);
        if (!csv_cell_null_check || force_change_on_nothing
                || cell_has_recursive_data(upgrade_name, up->faction, "ECM_Rating"))
            STDUPGRADE(ecm, up->ecm, templ->ecm, 0); //ecm is unsigned --chuck_starchaser
        if (!csv_cell_null_check || force_change_on_nothing
                || cell_has_recursive_data(upgrade_name, up->faction, "Primary_Capacitor")) {
            //temporary_upgrade_float_variable = static_cast<float>(energy.MaxValue());
            // TODO: STDUPGRADE(temporary_upgrade_float_variable, up->energy.MaxValue(), templ->energy.MaxValue(), 0);
            // energy.SetMaxValue(temporary_upgrade_float_variable);
        }
    }
    //Maneuvering stuff
    if (!csv_cell_null_check || force_change_on_nothing
            || cell_has_recursive_data(upgrade_name,
                    up->faction,
                    "Maneuver_Yaw|Maneuver_Pitch|Maneuver_Roll|Left_Accel|Top_Accel|Retro_Accel|Forward_Accel|Afterburner_Accel|Default_Speed_Governor|Afterburner_Speed_Governor|Yaw_Governor|Pitch_Governor|Roll_Speed_Governor")) {
        if (!csv_cell_null_check || force_change_on_nothing
                || cell_has_recursive_data(upgrade_name, up->faction, "Maneuver_Yaw"))
            STDUPGRADE(limits.yaw, tlimits_yaw, templ->limits.yaw, 0);
        if (!csv_cell_null_check || force_change_on_nothing
                || cell_has_recursive_data(upgrade_name, up->faction, "Maneuver_Pitch"))
            STDUPGRADE(limits.pitch, tlimits_pitch, templ->limits.pitch, 0);
        if (!csv_cell_null_check || force_change_on_nothing
                || cell_has_recursive_data(upgrade_name, up->faction, "Maneuver_Roll"))
            STDUPGRADE(limits.roll, tlimits_roll, templ->limits.roll, 0);
        if (!csv_cell_null_check || force_change_on_nothing
                || cell_has_recursive_data(upgrade_name, up->faction, "Left_Accel"))
            STDUPGRADE(limits.lateral, tlimits_lateral, templ->limits.lateral, 0);
        if (!csv_cell_null_check || force_change_on_nothing
                || cell_has_recursive_data(upgrade_name, up->faction, "Top_Accel"))
            STDUPGRADE(limits.vertical, tlimits_vertical, templ->limits.vertical, 0);
        if (!csv_cell_null_check || force_change_on_nothing
                || cell_has_recursive_data(upgrade_name, up->faction, "Retro_Accel"))
            STDUPGRADE(limits.retro, tlimits_retro, templ->limits.retro, 0);
        if (!csv_cell_null_check || force_change_on_nothing
                || cell_has_recursive_data(upgrade_name, up->faction, "Forward_Accel"))
            STDUPGRADE(limits.forward, tlimits_forward, templ->limits.forward, 0);
        if (!csv_cell_null_check || force_change_on_nothing
                || cell_has_recursive_data(upgrade_name, up->faction, "Afterburner_Accel"))
            STDUPGRADE(limits.afterburn, tlimits_afterburn, templ->limits.afterburn, 0);
        //if (!csv_cell_null_check || force_change_on_nothing
        //        || cell_has_recursive_data(upgrade_name, up->faction, "Fuel_Capacity")) 
            // TODO: STDUPGRADE(fuel, up->fuel, templ->fuel, 0);
        if (!csv_cell_null_check || force_change_on_nothing
                || cell_has_recursive_data(upgrade_name, up->faction, "Default_Speed_Governor"))
            STDUPGRADE(computer.max_combat_speed, tmax_speed, templ->computer.max_combat_speed, 0);
        if (!csv_cell_null_check || force_change_on_nothing
                || cell_has_recursive_data(upgrade_name, up->faction, "Afterburner_Speed_Governor"))
            STDUPGRADE(computer.max_combat_ab_speed, tmax_ab_speed, templ->computer.max_combat_ab_speed, 0);
        if (!csv_cell_null_check || force_change_on_nothing
                || cell_has_recursive_data(upgrade_name, up->faction, "Yaw_Governor")) {
            STDUPGRADE(computer.max_yaw_right, tmax_yaw_right, templ->computer.max_yaw_right, 0);
            STDUPGRADE(computer.max_yaw_left, tmax_yaw_left, templ->computer.max_yaw_left, 0);
        }
        if (!csv_cell_null_check || force_change_on_nothing
                || cell_has_recursive_data(upgrade_name, up->faction, "Pitch_Governor")) {
            STDUPGRADE(computer.max_pitch_down, tmax_pitch_down, templ->computer.max_pitch_down, 0);
            STDUPGRADE(computer.max_pitch_up, tmax_pitch_up, templ->computer.max_pitch_up, 0);
        }
        if (!csv_cell_null_check || force_change_on_nothing
                || cell_has_recursive_data(upgrade_name, up->faction, "Roll_Speed_Governor")) {
            STDUPGRADE(computer.max_roll_left, tmax_roll_left, templ->computer.max_roll_left, 0);
            STDUPGRADE(computer.max_roll_right, tmax_roll_right, templ->computer.max_roll_right, 0);
        }
    }
    //FIXME - do cell lookup later here
    static bool UpgradeCockpitDamage =
            XMLSupport::parse_bool(vs_config->getVariable("physics", "upgrade_cockpit_damage", "false"));
    if (UpgradeCockpitDamage) {
        STDUPGRADE(fireControlFunctionality, up->fireControlFunctionality,
                templ->fireControlFunctionality,
                (unittable ? 0 : 1));
        STDUPGRADE(fireControlFunctionalityMax, up->fireControlFunctionalityMax,
                templ->fireControlFunctionalityMax,
                (unittable ? 0 : 1));
        STDUPGRADE(SPECDriveFunctionality, up->SPECDriveFunctionality, templ->SPECDriveFunctionality,
                (unittable ? 0 : 1));
        STDUPGRADE(SPECDriveFunctionalityMax, up->SPECDriveFunctionalityMax,
                templ->SPECDriveFunctionalityMax,
                (unittable ? 0 : 1));
        STDUPGRADE(CommFunctionality, up->CommFunctionality, templ->CommFunctionality,
                (unittable ? 0 : 1));
        STDUPGRADE(CommFunctionalityMax, up->CommFunctionalityMax, templ->CommFunctionalityMax,
                (unittable ? 0 : 1));
        STDUPGRADE(LifeSupportFunctionality, up->LifeSupportFunctionality,
                templ->LifeSupportFunctionality,
                (unittable ? 0 : 1));
        STDUPGRADE(LifeSupportFunctionalityMax, up->LifeSupportFunctionalityMax,
                templ->LifeSupportFunctionalityMax,
                (unittable ? 0 : 1));
        unsigned int upgrmax = (UnitImages<void>::NUMGAUGES + 1 + MAXVDUS) * 2;
        for (unsigned int upgr = 0; upgr < upgrmax; upgr++)
            STDUPGRADE(pImage->cockpit_damage[upgr],
                    up->pImage->cockpit_damage[upgr],
                    templ->pImage->cockpit_damage[upgr],
                    (unittable ? 0 : 1));
        for (unsigned int upgr = 0; upgr < upgrmax; ++upgr) {
            GCCBugCheckFloat(pImage->cockpit_damage, upgr);
        }
    }

    bool upgradedshield = false;

    /*if (!csv_cell_null_check || force_change_on_nothing
            || cell_has_recursive_data(upgrade_name, up->faction, "Shield_Front_Top_Right")) {
        if (shield->number_of_facets == up->shield->number_of_facets) {
            for (unsigned int i = 0; i < shield->number_of_facets; i++) {
                float previous_max = shield->facets[i].max_health;
                STDUPGRADE(shield->facets[i].max_health,
                        up->shield->facets[i].max_health,
                        templ->shield->facets[i].max_health, 0);

                if (shield->facets[i].max_health != previous_max) {
                    shield->facets[i].max_health = shield->facets[i].max_health;
                    shield->facets[i].adjusted_health = shield->facets[i].max_health;
                    shield->facets[i].health = shield->facets[i].max_health;
                }
            }

            if (touchme && retval == UPGRADEOK) {
                upgradedshield = true;
            }
        } else if (up->FShieldData() > 0 || up->RShieldData() > 0 || up->LShieldData() > 0 || up->BShieldData() > 0) {
            cancompletefully = false;
        }
    }*/

    // TODO: lib_damage. Disabled this until we restore efficiency and leak
    /*if (upgradedshield || upgradedrecharge) {
        if (up->shield.efficiency) {
            shield.efficiency = up->shield.efficiency;
            if (templ)
                if (shield.efficiency > templ->shield.efficiency)
                    shield.efficiency = templ->shield.efficiency;
        }
    }
    if ( !csv_cell_null_check || force_change_on_nothing
        || cell_has_recursive_data( upgrade_name, up->faction, "Shield_Leak" ) ) {
        double myleak   = 100-shield.leak;
        double upleak   = 100-up->shield.leak;
        double templeak = 100-(templ != NULL ? templ->shield.leak : 0);
        bool   ccf = cancompletefully;
        STDUPGRADE_SPECIFY_DEFAULTS( myleak, upleak, templeak, 0, 100, 100, false, shield.leak );
        if (touchme && myleak <= 100 && myleak >= 0) shield.leak = (char) 100-myleak;
        cancompletefully = ccf;
    }*/

    //DO NOT CHANGE see unit_customize.cpp
    static float lc = XMLSupport::parse_float(vs_config->getVariable("physics", "lock_cone", ".8"));
    //DO NOT CHANGE! see unit.cpp:258
    static float tc = XMLSupport::parse_float(vs_config->getVariable("physics", "autotracking", ".93"));
    static bool use_template_maxrange =
            XMLSupport::parse_bool(vs_config->getVariable("physics", "use_upgrade_template_maxrange", "true"));
    //Radar stuff
    if (!csv_cell_null_check || force_change_on_nothing
            || cell_has_recursive_data(upgrade_name, up->faction,
                    "Radar_Range|Radar_Color|ITTS|Can_Lock|Max_Cone|Lock_Cone|Tracking_Cone")) {
        if (!csv_cell_null_check || force_change_on_nothing
                || cell_has_recursive_data(upgrade_name, up->faction, "Radar_Range")) {
            STDUPGRADECLAMP(radar.max_range,
                    up->radar.max_range,
                    use_template_maxrange ? templ->radar.max_range : FLT_MAX,
                    0);
        }
        if (!csv_cell_null_check || force_change_on_nothing
                || cell_has_recursive_data(upgrade_name, up->faction, "Radar_Color"))
            STDUPGRADE(radar.capabilities, up->radar.capabilities, templ->radar.capabilities, 0);
        if (!csv_cell_null_check || force_change_on_nothing
                || cell_has_recursive_data(upgrade_name, up->faction, "ITTS")) {
            computer.itts = UpgradeBoolval(computer.itts,
                    up->computer.itts,
                    touchme,
                    downgrade,
                    numave,
                    percentage,
                    force_change_on_nothing);
        }
        if (!csv_cell_null_check || force_change_on_nothing
                || cell_has_recursive_data(upgrade_name, up->faction, "Can_Lock")) {
            radar.can_lock = UpgradeBoolval(radar.can_lock,
                    up->radar.can_lock,
                    touchme,
                    downgrade,
                    numave,
                    percentage,
                    force_change_on_nothing);
        }
        //Do the two reversed ones below
        bool ccf = cancompletefully;
        if (!csv_cell_null_check || force_change_on_nothing
                || cell_has_recursive_data(upgrade_name, up->faction, "Max_Cone")) {
            double myleak = 1 - radar.max_cone;
            double upleak = 1 - up->radar.max_cone;
            double templeak = 1 - (templ != NULL ? templ->radar.max_cone : -1);
            STDUPGRADE_SPECIFY_DEFAULTS(myleak, upleak, templeak, 0, 0, 0, false, radar.max_cone);
            if (touchme) {
                radar.max_cone = 1 - myleak;
            }
        }
        if (up->radar.lock_cone != lc) {
            double myleak = 1 - radar.lock_cone;
            double upleak = 1 - up->radar.lock_cone;
            double templeak = 1 - (templ != NULL ? templ->radar.lock_cone : -1);
            if (templeak == 1 - lc) {
                templeak = 2;
            }
            if (!csv_cell_null_check || force_change_on_nothing
                    || cell_has_recursive_data(upgrade_name, up->faction, "Lock_Cone")) {
                STDUPGRADE_SPECIFY_DEFAULTS(myleak, upleak, templeak, 0, 0, 0, false, radar.lock_cone);
                if (touchme) {
                    radar.lock_cone = 1 - myleak;
                }
            }
        }
        if (up->radar.tracking_cone != tc) {
            double myleak = 1 - radar.tracking_cone;
            double upleak = 1 - up->radar.tracking_cone;
            double templeak = 1 - (templ != NULL ? templ->radar.tracking_cone : -1);
            if (templeak == 1 - tc) {
                templeak = 2;
            }
            if (!csv_cell_null_check || force_change_on_nothing
                    || cell_has_recursive_data(upgrade_name, up->faction, "Tracking_Cone")) {
                STDUPGRADE_SPECIFY_DEFAULTS(myleak, upleak, templeak, 0, 0, 0, false, radar.tracking_cone);
                if (touchme) {
                    radar.tracking_cone = 1 - myleak;
                }
            }
        }
        cancompletefully = ccf;
    }
    //NO CLUE FOR BELOW
    if (downgrade) {
        if (jump.drive >= -1 && up->jump.drive >= -1) {
            if (touchme) {
                jump.drive = -2;
            }
            ++numave;
            percentage += .5 * ((float) (100 - jump.damage)) / (101 - up->jump.damage);
            if (gen_downgrade_list) {
                AddToDowngradeMap(up->name,
                        up->jump.drive,
                        ((char *) &this->jump.drive) - ((char *) this),
                        tempdownmap);
            }
        }
        if (cloak.Capable() && up->cloak.Capable()) {
            if (touchme) {
                cloak.Disable();
            }
            ++numave;
            ++percentage;
            if (gen_downgrade_list) {
                AddToDowngradeMap(up->name,
                                  up->cloak.current,
                                  ((char *) &this->cloak.current) - ((char *) this),
                                  tempdownmap);
            }
        }
        //NOTE: Afterburner type 2 (jmp)
        //NOTE: Afterburner type 1 (gas)
        //NOTE: Afterburner type 0 (pwr)
        /*TODO: if (afterburnenergy < 32767 && afterburnenergy <= up->afterburnenergy && up->afterburnenergy != 32767
                && up->afterburnenergy != 0) {
            if (touchme) {
                afterburnenergy = 32767, afterburntype = 0;
            }
            ++numave;
            ++percentage;
            if (gen_downgrade_list) {
                AddToDowngradeMap(up->name,
                        up->afterburntype,
                        ((char *) &this->afterburnenergy) - ((char *) this),
                        tempdownmap);
            }
        }*/
    } else {
        //we are upgrading!
        if (touchme) {
            for (unsigned int i = 0; i < up->cargo.size(); ++i) {
                if (CanAddCargo(up->cargo[i])) {
                    AddCargo(up->cargo[i], false);
                }
            }
        }
        if ((!cloak.Capable() && up->cloak.Capable()) || force_change_on_nothing) {
            if (touchme) {
                cloak.Enable();

                cloak.minimum = up->cloak.minimum;
                cloak.rate = up->cloak.rate;
                cloak.glass = up->cloak.glass;
                cloak.energy = up->cloak.energy;
            }
            ++numave;
        } else if (cloak.Capable() && up->cloak.Capable()) {
            cancompletefully = false;
        }
        //NOTE: Afterburner type 2 (jmp)
        //NOTE: Afterburner type 1 (gas)
        //NOTE: Afterburner type 0 (pwr)
        /*if (((afterburnenergy > up->afterburnenergy
                || (afterburntype != up->afterburntype && up->afterburnenergy != 32767))
                && up->afterburnenergy > 0) || force_change_on_nothing) {
            ++numave;
            if (touchme) {
                afterburnenergy = up->afterburnenergy, afterburntype = up->afterburntype;
            }
        } else if (afterburnenergy <= up->afterburnenergy && afterburnenergy >= 0 && up->afterburnenergy > 0
                && up->afterburnenergy < 32767) {
            cancompletefully = false;
        }*/
        if ((jump.drive == -2 && up->jump.drive >= -1) || force_change_on_nothing) {
            if (touchme) {
                jump.drive = up->jump.drive;
                jump.damage = 0;
            }
            ++numave;
        } else if (jump.drive >= -1 && up->jump.drive >= -1) {
            cancompletefully = false;
        }
    }
    if (needs_redemption) {
        if (!can_be_redeemed) {
            cancompletefully = false;
        }
    }
    if (0 == numave) {      //Doesn't upgrade anything -- JS_NUDGE -- may want to revisit this later
        percentage = 1.0;
    }
    if (numave) {
        percentage = percentage / numave;
    }
    if (0 && touchme && up->Mass && numave) {
        float multiplyer = ((downgrade) ? -1 : 1);
        Mass += multiplyer * percentage * up->Mass;
        if (Mass < (templ ? templ->Mass : .000000001)) {
            Mass = (templ ? templ->Mass : .000000001);
        }
        Momentofinertia += multiplyer * percentage * up->Momentofinertia;
        if (Momentofinertia < (templ ? templ->Momentofinertia : 0.00000001)) {
            Momentofinertia = (templ ? templ->Momentofinertia : 0.00000001);
        }
    }
    if (gen_downgrade_list) {
        float MyPercentMin = ComputeMinDowngradePercent();
        if (downgrade && percentage > MyPercentMin) {
            for (vsUMap<int, DoubleName>::iterator i = tempdownmap.begin(); i != tempdownmap.end(); ++i) {
                downgrademap[(*i).first] = (*i).second;
            }
        }
    }
    return cancompletefully;
}

#undef STDUPGRADECLAMP
#undef STDUPGRADE
#undef STDUPGRADE_SPECIFY_DEFAULTS

bool Unit::ReduceToTemplate() {
    vector<Cargo> savedCargo;
    savedCargo.swap(cargo);
    vector<Mount> savedWeap;
    savedWeap.swap(mounts);
    const Unit *temprate = makeFinalBlankUpgrade(name, faction);
    bool success = false;
    double pct = 0;
    if (temprate && temprate->name != string("LOAD_FAILED")) {
        success = Upgrade(temprate, -1, -1, 0, true, pct, NULL, true);
        if (pct > 0) {
            success = true;
        }
    }
    savedCargo.swap(cargo);
    savedWeap.swap(mounts);
    return success;
}

Vector Unit::MountPercentOperational(int whichmount) {
    if (whichmount < 0 || (unsigned int) whichmount >= mounts.size()) {
        return Vector(-1, -1, -1);
    }
    return Vector(mounts[whichmount].functionality,
            mounts[whichmount].maxfunctionality,
            ((mounts[whichmount].status == Mount::ACTIVE || mounts[whichmount].status
                    == Mount::INACTIVE) ? 0.0 : (mounts[whichmount].status == Mount::UNCHOSEN ? 2.0 : 1.0)));
}

int Unit::RepairCost() {
    int cost = 1;
    unsigned int i;
    for (i = 0; i < (1 + MAXVDUS + UnitImages<void>::NUMGAUGES) * 2; ++i) {
        if (pImage->cockpit_damage[i] < 1) {
            ++cost;
        }
    }
    if (fireControlFunctionality < 1) {
        ++cost;
    }
    if (fireControlFunctionalityMax < 1) {
        ++cost;
    }
    if (SPECDriveFunctionality < 1) {
        ++cost;
    }
    if (SPECDriveFunctionalityMax < 1) {
        ++cost;
    }
    if (CommFunctionality < 1) {
        ++cost;
    }
    if (CommFunctionalityMax < 1) {
        ++cost;
    }
    if (LifeSupportFunctionality < 1) {
        ++cost;
    }
    if (LifeSupportFunctionalityMax < 1) {
        ++cost;
    }
    for (i = 0; i < numCargo(); ++i) {
        if (GetCargo(i).GetCategory().find(DamagedCategory) == 0) {
            ++cost;
        }
    }
    return cost;
}

int Unit::RepairUpgrade() {
    vector<Cargo> savedCargo;
    savedCargo.swap(cargo);
    vector<Mount> savedWeap;
    savedWeap.swap(mounts);
    int upfac = FactionUtil::GetUpgradeFaction();
    const Unit *temprate = makeFinalBlankUpgrade(name, faction);
    int success = 0;
    double pct = 0;
    if (temprate && temprate->name != string("LOAD_FAILED")) {
        success = Upgrade(temprate, -1, -1, 0, false, pct, NULL, false) ? 1 : 0;
        if (pct > 0) {
            success = 1;
        }
    }
    savedCargo.swap(cargo);
    savedWeap.swap(mounts);
    UnitImages<void> *im = &GetImageInformation();
    for (int i = 0; i < (1 + MAXVDUS + UnitImages<void>::NUMGAUGES) * 2; ++i) {
        if (im->cockpit_damage[i] < 1) {
            im->cockpit_damage[i] = 1;
            success += 1;
            pct = 1;
        }
    }
    if (fireControlFunctionality < 1) {
        fireControlFunctionality = 1;
        pct = 1;
        success += 1;
    }
    if (fireControlFunctionalityMax < 1) {
        fireControlFunctionalityMax = 1;
        pct = 1;
        success += 1;
    }
    if (SPECDriveFunctionality < 1) {
        SPECDriveFunctionality = 1;
        pct = 1;
        success += 1;
    }
    if (SPECDriveFunctionalityMax < 1) {
        SPECDriveFunctionalityMax = 1;
        pct = 1;
        success += 1;
    }
    if (CommFunctionality < 1) {
        CommFunctionality = 1;
        pct = 1;
        success += 1;
    }
    if (CommFunctionalityMax < 1) {
        CommFunctionalityMax = 1;
        pct = 1;
        success += 1;
    }
    if (LifeSupportFunctionality < 1) {
        LifeSupportFunctionality = 1;
        pct = 1;
        success += 1;
    }
    if (LifeSupportFunctionalityMax < 1) {
        LifeSupportFunctionalityMax = 1;
        pct = 1;
        success += 1;
    }
    damages = Damages::NO_DAMAGE;
    bool ret = success && pct > 0;
    static bool ComponentBasedUpgrades =
            XMLSupport::parse_bool(vs_config->getVariable("physics", "component_based_upgrades", "false"));
    if (ComponentBasedUpgrades) {
        for (unsigned int i = 0; i < numCargo(); ++i) {
            if (GetCargo(i).GetCategory().find(DamagedCategory) == 0) {
                ++success;
                static int damlen = strlen(DamagedCategory);
                GetCargo(i).SetCategory("upgrades/" + GetCargo(i).GetCategory().substr(damlen));
            }
        }
    } else if (ret) {
        const Unit *maxrecharge = makeTemplateUpgrade(name.get() + ".template", faction);

        Unit *mpl = getMasterPartList();
        for (unsigned int i = 0; i < mpl->numCargo(); ++i) {
            if (mpl->GetCargo(i).GetCategory().find("upgrades") == 0) {
                const Unit *up = loadUnitByCache(mpl->GetCargo(i).GetName(), upfac);
                //now we analyzify up!
                // TODO: lib_damage
                /*if (up->MaxShieldVal() == MaxShieldVal() && up->shield.recharge > shield.recharge) {
                    shield.recharge = up->shield.recharge;
                    if (maxrecharge)
                        if (shield.recharge > maxrecharge->shield.recharge)
                            shield.recharge = maxrecharge->shield.recharge;
                }*/
                /*TODO: if (up->energy.MaxValue() == energy.MaxValue() && up->recharge > recharge) {
                    recharge = up->recharge;
                    if (recharge > maxrecharge->recharge) {
                        recharge = maxrecharge->recharge;
                    }
                }*/
            }
        }
    }
    return success;
}

float RepairPrice(float operational, float price) {
    return .5 * price * (1 - operational) * g_game.difficulty;
}

extern bool isWeapon(std::string name);

//item must be non-null... but baseUnit or credits may be NULL.
bool Unit::RepairUpgradeCargo(Cargo *item, Unit *baseUnit, float *credits) {
    assert((item != NULL) | !"Unit::RepairUpgradeCargo got a null item."); //added by chuck_starchaser
    double itemPrice = baseUnit ? baseUnit->PriceCargo(item->GetName()) : item->GetPrice();
    if (isWeapon(item->GetCategory())) {
        const Unit *upgrade = getUnitFromUpgradeName(item->GetName(), this->faction);
        if (upgrade->getNumMounts()) {
            double price = itemPrice; //RepairPrice probably won't work for mounts.
            if (!credits || price <= (*credits)) {
                if (credits) {
                    (*credits) -= price;
                }
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
                double percentage = UnitUtil::PercentOperational(this, item->GetName(), item->GetCategory(), false);
                double price = RepairPrice(percentage, itemPrice);
                if (!credits || price <= (*credits)) {
                    if (credits) {
                        (*credits) -= price;
                    }
                    if (notadditive) {
                        this->Upgrade(un, 0, 0, 0, true, percentage, makeTemplateUpgrade(this->name, this->faction));
                    }
                    if (item->GetCategory().find(DamagedCategory) == 0) {
                        unsigned int where;
                        Cargo *c = this->GetCargo(item->GetName(), where);
                        if (c) {
                            c->SetCategory("upgrades/" + c->GetCategory().substr(strlen(DamagedCategory)));
                        }
                    }
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
extern double ComputeMinDowngradePercent();

vector<CargoColor> &Unit::FilterDowngradeList(vector<CargoColor> &mylist, bool downgrade) {
    const Unit *templ = NULL;
    const Unit *downgradelimit = NULL;
    static bool staticrem =
            XMLSupport::parse_bool(vs_config->getVariable("general", "remove_impossible_downgrades", "true"));
    static float MyPercentMin = ComputeMinDowngradePercent();
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
                        UnitConstCache::getCachedConst(StringIntKey(mylist[i].cargo.GetName().c_str(), faction));
                if (!NewPart) {
                    NewPart = UnitConstCache::setCachedConst(StringIntKey(mylist[i].cargo.GetName(), faction),
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
                    if (templ == NULL) {
                        templ =
                                UnitConstCache::setCachedConst(StringIntKey(templnam,
                                                faction),
                                        new Unit(templnam.c_str(), true, this->faction));
                    }
                    if (templ->name == std::string("LOAD_FAILED")) {
                        templ = NULL;
                    }
                } else {
                    downgradelimit = UnitConstCache::getCachedConst(StringIntKey(limiternam, faction));
                    if (downgradelimit == NULL) {
                        downgradelimit = UnitConstCache::setCachedConst(StringIntKey(limiternam,
                                        faction),
                                new Unit(limiternam.c_str(), true,
                                        this->faction));
                    }
                    if (downgradelimit->name == std::string("LOAD_FAILED")) {
                        downgradelimit = NULL;
                    }
                }
                free(unitdir);
                for (int m = 0; m < maxmountcheck; ++m) {
                    int s = 0;
                    for (un_iter ui = getSubUnits(); s == 0 || ((*ui) != NULL); ++ui, ++s) {
                        double percent = 1;
                        if (downgrade) {
                            if (canDowngrade(NewPart, m, s, percent, downgradelimit)) {
                                if (percent > MyPercentMin) {
                                    removethis = false;
                                    break;
                                }
                            }
                        } else if (canUpgrade(NewPart, m, s, mode, false /*force*/, percent, templ)) {
                            removethis = false;
                            break;
                        }
                        if (*ui == NULL) {
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
    static bool filtercargoprice =
            XMLSupport::parse_bool(vs_config->getVariable("cargo", "filter_expensive_cargo", "false"));
    if (filtercargoprice) {
        Cockpit *cp = _Universe->isPlayerStarship(this);
        if (cp) {
            for (unsigned int i = 0; i < mylist.size(); ++i) {
                if (mylist[i].cargo.GetPrice() > cp->credits) {
                    mylist[i].color = disable;
                }
            }
        }
    }
    return FilterDowngradeList(mylist, false);
}

bool Unit::IsBase() const {
    return ((flightgroup != NULL) && (flightgroup->name == "Base"));
}

void Unit::TurretFAW() {
    turretstatus = 3;
    Unit *un;
    for (un_iter iter = getSubUnits(); (un = *iter); ++iter) {
        if (!CheckAccessory(un)) {
            un->EnqueueAIFirst(new Orders::FireAt(configuration()->ai.firing_config.aggressivity));
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

Cargo *GetMasterPartList(const char *input_buffer) {
    unsigned int i;
    return GetUnitMasterPartList().GetCargo(input_buffer, i);
}

void Unit::ImportPartList(const std::string &category, float price, float pricedev, float quantity, float quantdev) {
    unsigned int numcarg = GetUnitMasterPartList().numCargo();
    float minprice = FLT_MAX;
    float maxprice = 0;
    for (unsigned int j = 0; j < numcarg; ++j) {
        if (GetUnitMasterPartList().GetCargo(j).GetCategory() == category) {
            float price = GetUnitMasterPartList().GetCargo(j).GetPrice();
            if (price < minprice) {
                minprice = price;
            } else if (price > maxprice) {
                maxprice = price;
            }
        }
    }
    for (unsigned int i = 0; i < numcarg; ++i) {
        Cargo c = GetUnitMasterPartList().GetCargo(i);
        if (c.GetCategory() == category) {
            static float aveweight =
                    fabs(XMLSupport::parse_float(vs_config->getVariable("cargo", "price_recenter_factor", "0")));
            c.SetQuantity(float_to_int(quantity - quantdev));
            float baseprice = c.GetPrice();
            c.SetPrice(c.GetPrice() * (price - pricedev));

            //stupid way
            c.SetQuantity(c.GetQuantity() + float_to_int((quantdev * 2 + 1) * ((double) rand()) / (((double) RAND_MAX) + 1)));
            c.SetPrice(c.GetPrice() + pricedev * 2 * ((float) rand()) / RAND_MAX);
            c.SetPrice(fabs(c.GetPrice()));
            c.SetPrice((c.GetPrice() + (baseprice * aveweight)) / (aveweight + 1));
            if (c.GetQuantity() <= 0) {
                c.SetQuantity(0);
            }
                //quantity more than zero
            else if (maxprice > minprice + .01) {
                float renormprice = (baseprice - minprice) / (maxprice - minprice);
                static float maxpricequantadj =
                        XMLSupport::parse_float(vs_config->getVariable("cargo", "max_price_quant_adj", "5"));
                static float minpricequantadj =
                        XMLSupport::parse_float(vs_config->getVariable("cargo", "min_price_quant_adj", "1"));
                static float
                        powah = XMLSupport::parse_float(vs_config->getVariable("cargo", "price_quant_adj_power", "1"));
                renormprice = pow(renormprice, powah);
                renormprice *= (maxpricequantadj - minpricequantadj);
                renormprice += 1;
                if (renormprice > .001) {
                    c.SetQuantity(c.GetQuantity() / float_to_int(renormprice));
                    if (c.GetQuantity() < 1) {
                        c.SetQuantity(1);
                    }
                }
            }
            static float minprice = XMLSupport::parse_float(vs_config->getVariable("cargo", "min_cargo_price", "0.01"));
            if (c.GetPrice() < minprice) {
                c.SetPrice(minprice);
            }

            AddCargo(c, false);
        }
    }
}

std::string Unit::massSerializer(const XMLType &input, void *mythis) {
    Unit *un = (Unit *) mythis;
    float mass = un->Mass;
    static bool usemass = XMLSupport::parse_bool(vs_config->getVariable("physics", "use_cargo_mass", "true"));
    for (unsigned int i = 0; i < un->cargo.size(); ++i) {
        if (un->cargo[i].GetQuantity() > 0) {
            if (usemass) {
                mass -= un->cargo[i].GetMass() * un->cargo[i].GetQuantity();
            }
        }
    }
    return XMLSupport::tostring((float) mass);
}

std::string Unit::mountSerializer(const XMLType &input, void *mythis) {
    Unit *un = (Unit *) mythis;
    int i = input.w.hardint;
    if (un->getNumMounts() > i) {
        string result(getMountSizeString(un->mounts[i].size));
        if (un->mounts[i].status == Mount::INACTIVE || un->mounts[i].status == Mount::ACTIVE) {
            result += string("\" weapon=\"") + (un->mounts[i].type->name);
        }
        if (un->mounts[i].ammo != -1) {
            result += string("\" ammo=\"") + XMLSupport::tostring(un->mounts[i].ammo);
        }
        if (un->mounts[i].volume != -1) {
            result += string("\" volume=\"") + XMLSupport::tostring(un->mounts[i].volume);
        }
        result += string("\" xyscale=\"") + XMLSupport::tostring(un->mounts[i].xyscale) + string("\" zscale=\"")
                + XMLSupport::tostring(un->mounts[i].zscale);
        Matrix m;
        Transformation(un->mounts[i].GetMountOrientation(), un->mounts[i].GetMountLocation().Cast()).to_matrix(m);
        result += string("\" x=\"") + tostring((float) (m.p.i / parse_float(input.str)));
        result += string("\" y=\"") + tostring((float) (m.p.j / parse_float(input.str)));
        result += string("\" z=\"") + tostring((float) (m.p.k / parse_float(input.str)));

        result += string("\" qi=\"") + tostring(m.getQ().i);
        result += string("\" qj=\"") + tostring(m.getQ().j);
        result += string("\" qk=\"") + tostring(m.getQ().k);

        result += string("\" ri=\"") + tostring(m.getR().i);
        result += string("\" rj=\"") + tostring(m.getR().j);
        result += string("\" rk=\"") + tostring(m.getR().k);
        return result;
    } else {
        return string("");
    }
}

std::string Unit::subunitSerializer(const XMLType &input, void *mythis) {
    Unit *un = (Unit *) mythis;
    int index = input.w.hardint;
    Unit *su;
    int i = 0;
    for (un_iter ui = un->getSubUnits(); (su = *ui); ++ui, ++i) {
        if (i == index) {
            if (su->pImage->unitwriter) {
                return su->pImage->unitwriter->getName();
            }
            return su->name;
        }
    }
    return string("destroyed_blank");
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
extern float rand01();

bool isWeapon(std::string name) {
    if (name.find("Weapon") != std::string::npos) {
        return true;
    }
    if (name.find("SubUnit") != std::string::npos) {
        return true;
    }
    if (name.find("Ammunition") != std::string::npos) {
        return true;
    }
    return false;
}

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

void Unit::Repair() {
    static float repairtime = XMLSupport::parse_float(vs_config->getVariable("physics", "RepairDroidTime", "180"));
    static float checktime = XMLSupport::parse_float(vs_config->getVariable("physics", "RepairDroidCheckTime", "5"));
    if ((repairtime <= 0) || (checktime <= 0)) {
        return;
    }
    if (repair_droid > 0) {
        if (next_repair_time == -FLT_MAX || next_repair_time <= UniverseUtil::GetGameTime()) {
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
                                UniverseUtil::GetGameTime() + repairtime * (1 - percentoperational) / repair_droid;
                    } else {
                        //ACtually fix the cargo here
                        static int upfac = FactionUtil::GetUpgradeFaction();
                        const Unit *up = getUnitFromUpgradeName(carg->GetName(), upfac);
                        static std::string loadfailed("LOAD_FAILED");
                        if (up->name == loadfailed) {
                            VS_LOG(info,
                                    "Bug: Load failed cargo encountered: report on https://github.com/vegastrike/Vega-Strike-Engine-Source");
                        } else {
                            double percentage = 0;
                            //don't want to repair these things
                            if (up->SubUnits.empty() && up->getNumMounts() == 0) {
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
        }
        float ammt_repair = simulation_atom_var / repairtime * repair_droid;
        REPAIRINTEGRATED(LifeSupportFunctionality, LifeSupportFunctionalityMax);
        REPAIRINTEGRATED(fireControlFunctionality, fireControlFunctionalityMax);
        REPAIRINTEGRATED(SPECDriveFunctionality, SPECDriveFunctionalityMax);
        REPAIRINTEGRATED(CommFunctionality, CommFunctionalityMax);
        unsigned int numg = (1 + UnitImages<void>::NUMGAUGES + MAXVDUS);
        unsigned int which = vsrandom.genrand_int31() % numg;
        static float hud_repair_quantity =
                XMLSupport::parse_float(vs_config->getVariable("physics", "hud_repair_unit", ".25"));
        //total damage
        if (pImage->cockpit_damage[which] < pImage->cockpit_damage[which + numg]) {
            pImage->cockpit_damage[which] += hud_repair_quantity;
            if (pImage->cockpit_damage[which] > pImage->cockpit_damage[which + numg]) {
                //total damage
                pImage->cockpit_damage[which] = pImage->cockpit_damage[which + numg];
            }
        }
        if (mounts.size()) {
            static float mount_repair_quantity =
                    XMLSupport::parse_float(vs_config->getVariable("physics", "mount_repair_unit", ".25"));
            unsigned int i = vsrandom.genrand_int31() % mounts.size();
            if (mounts[i].functionality < mounts[i].maxfunctionality) {
                mounts[i].functionality += mount_repair_quantity;
                if (mounts[i].functionality > mounts[i].maxfunctionality) {
                    mounts[i].functionality = mounts[i].maxfunctionality;
                }
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
    if (_Universe->isPlayerStarship(this) != NULL) {
        tflags = tractorability_flags & tractorability_mask;
    } else {
        tflags = tractorability_flags;
    }
    return (Unit::tractorHow) (tflags);
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
        newtechnique = TechniquePtr(new Technique(*technique));
        for (int passno = 0; passno < technique->getNumPasses(); ++passno) {
            Pass &pass = technique->getPass(passno);
            for (size_t paramno = 0; paramno < pass.getNumShaderParams(); ++paramno) {
                Pass::ShaderParam &param = pass.getShaderParam(paramno);
                map<string, string>::const_iterator override = overrides.find(param.name);
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

    // Power
    energy_manager.Act();

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
    unsigned int i, n;
    if (lastframe) {
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
    // This should replace all the UpdatePhysics
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

    // Recharge energy and shields
    const bool apply_difficulty_shields = configuration()->physics_config.difficulty_based_shield_recharge;
    const bool energy_before_shield = configuration()->physics_config.engine_energy_takes_priority;

    // Difficulty settings
    float difficulty_shields = 1.0f;
    if (apply_difficulty_shields) {
        difficulty_shields = g_game.difficulty;
    }

    /*if (energy_before_shield) {
        rechargeEnergy();
    }*/

    bool is_player_ship = _Universe->isPlayerStarship(this);
    RegenerateShields(difficulty_shields, is_player_ship);
    //ExpendEnergy(is_player_ship);

    /*if (!energy_before_shield) {
        rechargeEnergy();
    }*/

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
    static float EXTRA_CARGO_SPACE_DRAG =
            XMLSupport::parse_float(vs_config->getVariable("physics", "extra_space_drag_for_cargo", "0.005"));
    if (EXTRA_CARGO_SPACE_DRAG > 0) {
        int upgfac = FactionUtil::GetUpgradeFaction();
        if ((this->faction == upgfac) || (this->name == "eject") || (this->name == "Pilot")) {
            Velocity = Velocity * (1 - EXTRA_CARGO_SPACE_DRAG);
        }
    }

    float dist_sqr_to_target = FLT_MAX;
    Unit *target = Unit::Target();
    bool increase_locking = false;
    if (target && !cloak.Cloaked()) {
        if (target->isUnit() != Vega_UnitType::planet) {
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
                if (computer.velocity_ref.GetUnit()) {
                    double dist = UnitUtil::getSignificantDistance(this, computer.velocity_ref.GetUnit());
                    double next_dist = UnitUtil::getSignificantDistance(this, nextVelRef);
                    if (next_dist < dist) {
                        computer.velocity_ref = nextVelRef;
                    }
                } else {
                    computer.velocity_ref = nextVelRef;
                }
            }
        }
    }
    static float SPACE_DRAG = XMLSupport::parse_float(vs_config->getVariable("physics", "unit_space_drag", "0.000000"));

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
        if (((false
                && mounts[i].status
                        == Mount::INACTIVE) || mounts[i].status == Mount::ACTIVE) && !cloak.Cloaked()
                && mounts[i].ammo != 0) {
            if (player_cockpit) {
                touched = true;
            }
            if (increase_locking && (dist_sqr_to_target < mounts[i].type->range * mounts[i].type->range)) {
                mounts[i].time_to_lock -= simulation_atom_var;
                static bool ai_lock_cheat =
                        XMLSupport::parse_bool(vs_config->getVariable("physics", "ai_lock_cheat", "true"));
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
                        (HeatSink ? HeatSink : 1.0f) * mounts[i].functionality,
                        this,
                        superunit);
            }
        } else {
            mounts[i].ref.refire += simulation_atom_var * (HeatSink ? HeatSink : 1.0f) * mounts[i].functionality;
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
                    (!isSubUnit() || owner == NULL) ? this : owner, target, autotrack,
                    tracking_cone,
                    hint)) {
                const WeaponInfo *typ = mounts[i].type;
                // TODO: energy += typ->energy_rate * (typ->type == WEAPON_TYPE::BEAM ? simulation_atom_var : 1);
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
    bool dead = true;

    UpdateSubunitPhysics(cumulative_transformation,
            cumulative_transformation_matrix,
            cumulative_velocity,
            lastframe,
            uc,
            superunit);
    //can a unit get to another system without jumping?.
    static bool warp_is_interstellar =
            XMLSupport::parse_bool(vs_config->getVariable("physics", "warp_is_interstellar", "false"));
    if (warp_is_interstellar
            && (curr_physical_state.position.MagnitudeSquared() > howFarToJump() * howFarToJump() && !isSubUnit())) {
        static bool direct =
                XMLSupport::parse_bool(vs_config->getVariable("physics", "direct_interstellar_journey", "true"));
        bool jumpDirect = false;
        if (direct) {
            Cockpit *cp = _Universe->isPlayerStarship(this);
            if (NULL != cp) {
                std::string sys = cp->GetNavSelectedSystem();
                if (!sys.empty()) {
                    jumpDirect = true;
                    _Universe->activeStarSystem()->JumpTo(this, NULL, sys, true, true);
                }
            }
        }
        if (!jumpDirect) {
            _Universe->activeStarSystem()->JumpTo(this, NULL,
                    NearestSystem(_Universe->activeStarSystem()->getFileName(),
                            curr_physical_state.position), true, true);
        }
    }


    // Destroyed means we just killed the unit and it is exploding
    if (Destroyed()) {
        this->Explode(true, 0);

        // Kill means it is done exploding and we can delete it
        dead &= (pImage->pExplosion == NULL);
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
    halos->AddHalo(filename, trans, size, col, halo_type, halo_speed);
}

/**** MOVED FROM BASE_INTERFACE.CPP ****/
extern string getCargoUnitName(const char *name);

void Unit::UpgradeInterface(Unit *baseun) {
    string basename = (::getCargoUnitName(baseun->getFullname().c_str()));
    if (baseun->isUnit() != Vega_UnitType::planet) {
        basename = baseun->name;
    }
    BaseUtil::LoadBaseInterfaceAtDock(basename, baseun, this);
}

//From star_system_jump.cpp
extern Hashtable<std::string, StarSystem, 127> star_system_table;
extern std::vector<unorigdest *> pendingjump;

//From star_system_jump.cpp
inline bool CompareDest(Unit *un, StarSystem *origin) {
    for (unsigned int i = 0; i < un->GetDestinations().size(); i++) {
        if (std::string(origin->getFileName()) == std::string(un->GetDestinations()[i])) {
            return true;
        }
    }
    return false;
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
    if (pendingjump[kk]->orig == this->activeStarSystem || this->activeStarSystem == NULL) {
        if (JumpCapable::TransferUnitToSystem(pendingjump[kk]->dest)) {
            ///eradicating from system, leaving no trace
            ret = true;

            Unit *unit;
            for (un_iter iter = pendingjump[kk]->orig->getUnitList().createIterator(); (unit = *iter); ++iter) {
                if (unit->Threat() == this) {
                    unit->Threaten(NULL, 0);
                }
                if (unit->VelocityReference() == this) {
                    unit->VelocityReference(NULL);
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
            Unit *primary;
            if (pendingjump[kk]->final_location.i == 0
                    && pendingjump[kk]->final_location.j == 0
                    && pendingjump[kk]->final_location.k == 0) {
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
                QVector pos = jumpnode->Position();

                this->SetCurPosition(pos);
                ActivateAnimation(jumpnode);
                if (jumpnode->isUnit() == Vega_UnitType::unit) {
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
                        (tester = *i) != NULL; ++i) {
                    if (tester->isUnit() == Vega_UnitType::unit && tester != this) {
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
            for (unsigned int i = 0; i < this->pImage->dockedunits.size(); i++) {
                Unit *unut;
                if (NULL != (unut = this->pImage->dockedunits[i]->uc.GetUnit())) {
                    unut->TransferUnitToSystem(kk, savedStarSystem, dosightandsound);
                }
            }
        }
        if (this->docked & (DOCKED | DOCKED_INSIDE)) {
            Unit *un = this->pImage->DockedTo.GetUnit();
            if (!un) {
                this->docked &= (~(DOCKED | DOCKED_INSIDE));
            } else {
                Unit *targ = NULL;
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
