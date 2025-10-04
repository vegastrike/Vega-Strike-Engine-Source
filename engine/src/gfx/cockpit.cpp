// -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil -*-
/*
 * cockpit.cpp
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


/// Draws cockpit parts
/// Draws gauges, info strings, radar, ...

#define PY_SSIZE_T_CLEAN
#include <boost/version.hpp>
#include <boost/python.hpp>
#if BOOST_VERSION != 102800
#include <boost/python/object.hpp>
#include <boost/python/dict.hpp>
#else
#include <boost/python/objects.hpp>
#endif

#include "vegadisk/vsfilesystem.h"
#include "src/vs_logging.h"
#include "root_generic/vs_globals.h"
#include "src/vegastrike.h"
#include "gfx/gauge.h"
#include "gfx/cockpit.h"
#include "src/universe.h"
#include "src/star_system.h"
#include "cmd/unit_generic.h"
#include "cmd/movable.h"
#include "cmd/collection.h"
#include "cmd/unit_util.h"
#include "cmd/unit_find.h" //for radar iteration.
#include "cmd/base_util.h"
#include "gfx/hud.h"
#include "gfx/vdu.h"
#include "root_generic/lin_time.h" //for fps
#include "cmd/beam.h"
#include "src/config_xml.h"
#include "root_generic/lin_time.h"
#include "cmd/images.h"
#include "cmd/script/mission.h"
#include "cmd/script/msgcenter.h"
#include "cmd/ai/flyjoystick.h"
#include "cmd/ai/firekeyboard.h"
#include "cmd/ai/aggressive.h"
#include "cmd/ai/autodocking.h"
#include "src/main_loop.h"
#include <assert.h>     //needed for assert() calls
#include "vegadisk/savegame.h"
#include "gfx/animation.h"
#include "gfx_generic/mesh.h"
#include "src/universe_util.h"
#include "src/in_mouse.h"
#include "gui/glut_support.h"
#include "src/audiolib.h"
#include "src/save_util.h"
#include "cmd/base.h"
#include "src/in_kb_data.h"
#include "src/main_loop.h"
#include <set>
#include <string>
#include "cmd/unit_const_cache.h"
#include "root_generic/options.h"
#include "gfx/soundcontainer_aldrv.h"
#include "root_generic/configxml.h"
#include "cmd/planet.h"
#include "cmd/mount_size.h"
#include "cmd/weapon_info.h"
#include "gfx/cockpit_gfx.h"
#include "cmd/dock_utils.h"
#include "vega_cast_utils.h"
#include "resource/random_utils.h"
#include "resource/random_utils.h"

#include <cstddef>
#include <cfloat>

using std::min;
using std::max;

using VSFileSystem::SoundFile;
#define SWITCH_CONST (.9)
/* The smaller VERYNEAR_CONST is, the worse Z-Buffer precision will be. So keep this above 0.004) */
#define VERYNEAR_CONST (0.004f)
/*so that znear/zfar are not too close to max/min values, and account for off-center cockpits */
#define COCKPITZ_HEADROOM (1.01f)

static soundContainer disableautosound;
static soundContainer enableautosound;


std::string GameCockpit::GetNavSelectedSystem() {
    return AccessNavSystem()->getSelectedSystem();
}

void soundContainer::loadsound(string soundfile, bool looping) {
    if (this->sound == -2 && soundfile.size()) {
        string sound = GameCockpit::getsoundfile(soundfile);
        if (sound.size()) {
            this->sound = AUDCreateSoundWAV(sound, looping);
        } else {
            this->sound = -1;
        }
    }
}

void soundContainer::playsound() {
    if (sound >= 0) {
        AUDAdjustSound(sound, QVector(0, 0, 0), Vector(0, 0, 0));
        AUDPlay(sound, QVector(0, 0, 0), Vector(0, 0, 0), 1);
    }
}

soundContainer::~soundContainer() {
    if (sound >= 0) {
#if 0
                                                                                                                                AUDStopPlaying( sound );
        AUDDeleteSound( sound, false );
#endif
        sound = -2;
    }
}

void GameCockpit::ReceivedTargetInfo() {
    for (size_t j = 0; j < vdu.size(); j++) {
        vdu[j]->ReceivedTargetData();
    }
}

void GameCockpit::SetSoundFile(string sound) {
    soundfile = AUDCreateSoundWAV(sound, false);
}


void GameCockpit::Eject() {
    ejecting = true;
    going_to_dock_screen = false;
}

void GameCockpit::EjectDock() {
    ejecting = true;
    going_to_dock_screen = true;
}

void GameCockpit::DoAutoLanding(Unit *un, Unit *target) {
    if (!un || !target) {
        return;
    }
    if (UnitUtil::isDockableUnit(target) == false) {
        return;
    }
    static std::set<std::string> autoLandingExcludeList;
    static std::set<std::string> autoLandingExcludeWarningList;
    static bool autoLandingExcludeList_initialised = false;
    if (!autoLandingExcludeList_initialised) {
        autoLandingExcludeList_initialised = true;
        std::string excludes;

        excludes = configuration().physics.auto_landing_exclude_list;
        if (!excludes.empty()) {
            std::string::size_type pos = 0, epos = 0;
            while (epos != std::string::npos) {
                std::string xx(excludes.substr(pos, epos = excludes.find(' ', pos)));
                autoLandingExcludeList.insert(xx);
                pos = epos + 1;
            }
        }
        excludes = configuration().physics.auto_landing_exclude_warning_list;
        if (!excludes.empty()) {
            std::string::size_type pos = 0, epos = 0;
            while (epos != std::string::npos) {
                std::string yy(excludes.substr(pos, epos = excludes.find(' ', pos)));
                autoLandingExcludeWarningList.insert(yy);
                pos = epos + 1;
            }
        }
    }
    std::string tname = target->name;
    if (autoLandingExcludeList.find(tname) != autoLandingExcludeList.end()) {
        return;
    }
    const float lessthan = configuration().physics.auto_landing_dock_distance_flt;
    const float warnless = configuration().physics.auto_landing_warning_distance_flt;
    const float AutoLandingMoveDistance = configuration().physics.auto_landing_move_distance_flt;
    const float moveout = configuration().physics.auto_landing_displace_distance_flt;
    const float autorad = configuration().physics.unit_default_autodock_radius_flt;
    const bool adjust_unit_radius = configuration().physics.use_unit_autodock_radius;
    float rsize = target->isPlanet() ? target->rSize() : (autorad + (adjust_unit_radius ? target->rSize() : 0));
    QVector diffvec = un->Position() - target->Position();
    float dist = diffvec.Magnitude() - un->rSize() - rsize;
    diffvec.Normalize();

    static bool haswarned = false;
    static void *lastwarned = NULL;
    static float docktime = -FLT_MAX;
    if (dist < lessthan && haswarned && lastwarned == target) {
        //CrashForceDock(target,un,true);
        un->SetPosAndCumPos(target->Position() + diffvec.Scale(un->rSize() + rsize + AutoLandingMoveDistance));
        FireKeyboard::DockKey(KBData(), PRESS);
        haswarned = false;
        lastwarned = target;
        docktime = getNewTime();
    } else if (haswarned == false && lastwarned == target) {
        if (getNewTime() - docktime > SIMULATION_ATOM * 2) {
            haswarned = false;
            un->SetPosAndCumPos(UniverseUtil::SafeEntrancePoint(
                    target->Position() + diffvec * (rsize + moveout + un->rSize()),
                    un->rSize() * 1.1));
            lastwarned = NULL;
        }
    } else if (dist < warnless) {
        if (lastwarned != target || !haswarned) {
            if (autoLandingExcludeWarningList.count(target->name) == 0) {
                static string str = configuration().cockpit_audio.automatic_landing_zone;
                static string str1 = configuration().cockpit_audio.automatic_landing_zone1;
                static string str2 = configuration().cockpit_audio.automatic_landing_zone2;
                const string autolandinga = configuration().graphics.automatic_landing_zone_warning;
                const string autolandinga1 = configuration().graphics.automatic_landing_zone_warning1;
                const string autolandinga2 = configuration().graphics.automatic_landing_zone_warning2;
                static string message = configuration().graphics.automatic_landing_zone_warning_text;
                UniverseUtil::IOmessage(0, "game", "all", message);
                static Animation *ani0 = new Animation(autolandinga.c_str());
                static Animation *ani1 = new Animation(autolandinga1.c_str());
                static Animation *ani2 = new Animation(autolandinga2.c_str());
                static soundContainer warnsound;
                static soundContainer warnsound1;
                static soundContainer warnsound2;
                int num = rand() < RAND_MAX / 2 ? 0 : (rand() < RAND_MAX / 2 ? 1 : 2);
                if (warnsound.sound < 0) {
                    warnsound.loadsound(str);
                    warnsound1.loadsound(str1);
                    warnsound2.loadsound(str2);
                }
                switch (num) {
                    case 0:
                        warnsound.playsound();
                        SetCommAnimation(ani0, target);
                        break;
                    case 1:
                        warnsound1.playsound();
                        SetCommAnimation(ani1, target);
                        break;
                    default:
                        warnsound2.playsound();
                        SetCommAnimation(ani2, target);
                        break;
                }
            }
            haswarned = true;
            lastwarned = target;
        }
    } else if (lastwarned == target) {
        haswarned = false;
        lastwarned = NULL;
    }
}

void GameCockpit::AutoLanding() {
    const bool autolanding_enable = configuration().physics.auto_landing_enable;
    if (autolanding_enable) {
        Unit *player = GetParent();
        if (player == NULL) {
            return;
        }

        CollideMap *collideMap = _Universe->activeStarSystem()->collide_map[Unit::UNIT_ONLY];
        for (CollideMap::iterator it = collideMap->begin(); it != collideMap->end(); ++it) {
            if (it->radius <= 0) {
                continue;
            }

            Unit *target = it->ref.unit;
            if (target == NULL) {
                continue;
            }

            DoAutoLanding(player, target);
        }
    }
}




float GameCockpit::LookupUnitStat(int stat, Unit *target) {
    if (!target) {
        return 0.0f;
    }
    const float game_speed = configuration().physics.game_speed_flt;
    const bool display_in_meters = configuration().physics.display_in_meters;
    const bool lie = configuration().physics.game_speed_lying;
    static float fpsval = 0;
    const float fpsmax = 1;
    static float numtimes = fpsmax;
    Unit *tmpunit;

    // TODO: lib_damage
    // make sure the enums are in the right order as our
    // facet_configuration
    // Also, can't be defined within switch for some reason
    int shield_index = stat - UnitImages<void>::SHIELDF;

    switch (stat) {
        case UnitImages<void>::SHIELDF:
            return target->shield.Percent(Shield::front);

        case UnitImages<void>::SHIELDB:
            return target->shield.Percent(Shield::back);

        case UnitImages<void>::SHIELDL:
            return target->shield.Percent(Shield::left);

        case UnitImages<void>::SHIELDR:
            return target->shield.Percent(Shield::right);

        case UnitImages<void>::ARMORF:
            return target->armor.Percent(Armor::front);

        case UnitImages<void>::ARMORB:
            return target->armor.Percent(Armor::back);

        case UnitImages<void>::ARMORL:
            return target->armor.Percent(Armor::left);

        case UnitImages<void>::ARMORR:
            return target->armor.Percent(Armor::right);

        case UnitImages<void>::ARMOR4:
            return target->armor.Percent(Armor::left);
        case UnitImages<void>::ARMOR5:
            return target->armor.Percent(Armor::left);
        case UnitImages<void>::ARMOR6:
            return target->armor.Percent(Armor::right);
        case UnitImages<void>::ARMOR7:
            return target->armor.Percent(Armor::right);
        case UnitImages<void>::FUEL:
            return target->fuel.Percent();

        case UnitImages<void>::ENERGY:
            return target->energy.Percent();

        case UnitImages<void>::WARPENERGY: {
            const bool warpifnojump = configuration().graphics.hud.display_warp_energy_if_no_jump_drive;
            return (warpifnojump || target->jump_drive.Installed()) ? target->ftl_energy.Percent() : 0;
        }
        case UnitImages<void>::HULL:
            return target->hull.Percent();

        case UnitImages<void>::EJECT: {
            int go =
                    ((target->hull.Percent() < .25)
                            && (target->shield.Percent() < .25)) ? 1 : 0;
            static int overload = 0;
            if (overload != go) {
                if (go == 0) {
                    static soundContainer ejectstopsound;
                    if (ejectstopsound.sound < 0) {
                        const std::string str = configuration().cockpit_audio.overload_stopped;
                        ejectstopsound.loadsound(str);
                    }
                    ejectstopsound.playsound();
                } else {
                    static soundContainer ejectsound;
                    if (ejectsound.sound < 0) {
                        const std::string str = configuration().cockpit_audio.overload;
                        ejectsound.loadsound(str);
                    }
                    ejectsound.playsound();
                }
                overload = go;
            }
            return go;
        }
        case UnitImages<void>::LOCK: {
            float distance;
            const float locklight_time = configuration().graphics.locklight_time_flt;
            bool res = false;
            if ((tmpunit = target->Threat())) {
                res = tmpunit->cosAngleTo(target, distance, FLT_MAX, FLT_MAX) > .95;
                if (res) {
                    last_locktime = UniverseUtil::GetGameTime();
                }
            }
            return (res || ((UniverseUtil::GetGameTime() - last_locktime) < locklight_time)) ? 1.0f : 0.0f;
        }
        case UnitImages<void>::MISSILELOCK: {
            const float locklight_time = configuration().graphics.locklight_time_flt;
            const bool res = target->graphicOptions.missilelock;
            if (res) {
                last_mlocktime = UniverseUtil::GetGameTime();
            }
            return (res || ((UniverseUtil::GetGameTime() - last_mlocktime) < locklight_time)) ? 1.0f : 0.0f;
        }
        case UnitImages<void>::COLLISION: {
            const double collidepanic = configuration().physics.collision_inertial_time_dbl;
            return (getNewTime() - TimeOfLastCollision) < collidepanic;
        }
        case UnitImages<void>::ECM:
            return (UnitUtil::getECM(target) > 0) ? 1 : 0;

        case UnitImages<void>::WARPFIELDSTRENGTH:
            return target->graphicOptions.WarpFieldStrength;

        case UnitImages<void>::MAXWARPFIELDSTRENGTH:
            return target->GetMaxWarpFieldStrength(1.f);

        case UnitImages<void>::JUMP:
            return jumpok ? 1 : 0;

        case UnitImages<void>::KPS:
        case UnitImages<void>::SETKPS:
        case UnitImages<void>::MAXKPS:
        case UnitImages<void>::MAXCOMBATKPS:
        case UnitImages<void>::MAXCOMBATABKPS: {
            const bool use_relative_velocity = configuration().graphics.hud.display_relative_velocity;
            if (!target) {
                return 0.0f;
            }
            if (!target->VelocityReference()) {
                return 0.0f;
            }
            Unit *velocity_reference_unit = target->VelocityReference();
            float value;
            switch (stat) {
                case UnitImages<void>::KPS:
                    if (target->graphicOptions.WarpFieldStrength != 1.0) {
                        if (use_relative_velocity && target->VelocityReference()) {
                            if (velocity_reference_unit->graphicOptions.WarpFieldStrength != 1.0) {
                                value =
                                        (target->GetWarpVelocity()
                                                - velocity_reference_unit->GetWarpVelocity()).Magnitude();
                            } else {
                                value =
                                        (target->GetWarpVelocity()
                                                - velocity_reference_unit->cumulative_velocity).Magnitude();
                            }
                        } else {
                            value = target->GetWarpVelocity().Magnitude();
                        }
                    } else {
                        if (use_relative_velocity && target->VelocityReference()) {
                            if (velocity_reference_unit->graphicOptions.WarpFieldStrength != 1.0) {
                                value =
                                        (target->cumulative_velocity
                                                - velocity_reference_unit->GetWarpVelocity()).Magnitude();
                            } else {
                                value =
                                        (target->cumulative_velocity
                                                - velocity_reference_unit->cumulative_velocity).Magnitude();
                            }
                        } else {
                            value = target->cumulative_velocity.Magnitude();
                        }
                    }
                    break;
                case UnitImages<void>::SETKPS:
                    value = target->computer.set_speed;
                    break;
                case UnitImages<void>::MAXKPS:
                    value = target->MaxSpeed();
                    break;
                case UnitImages<void>::MAXCOMBATKPS:
                    value = target->drive.speed.Value();
                    break;
                case UnitImages<void>::MAXCOMBATABKPS:
                    value = target->afterburner.speed.Value();
                    break;
                default:
                    value = 0;
            }
            if (lie) {
                return value / game_speed;
            } else {
                return display_in_meters ? value : value
                        * 3.6;
            }          //JMS 6/28/05 - converted back to raw meters/second
        }
        case UnitImages<void>::MASSEFFECT: {
            float basemass = atof(UniverseUtil::LookupUnitStat(target->name, "", "Mass").c_str());
            if (basemass > 0) {
                return 100 * target->GetMass() / basemass;
            } else {
                return 0;
            }
        }
        case UnitImages<void>::AUTOPILOT: {
            static int wasautopilot = 0;
            int abletoautopilot = 0;
            const bool auto_valid = configuration().physics.in_system_jump_or_timeless_auto_pilot;
            if (target) {
                if (!auto_valid) {
                    abletoautopilot = (target->ftl_drive.Enabled());
                } else {
                    abletoautopilot = (target->AutoPilotTo(target, false) ? 1 : 0);
                    const float no_auto_light_below = configuration().physics.no_auto_light_below_flt;
                    Unit *targtarg = target->Target();
                    if (targtarg) {
                        if ((target->Position() - targtarg->Position()).Magnitude() - targtarg->rSize()
                                - target->rSize()
                                < no_auto_light_below) {
                            abletoautopilot = false;
                        }
                    }
                }
            }
            if (abletoautopilot != wasautopilot) {
                if (abletoautopilot == 0) {
                    static soundContainer autostopsound;
                    if (autostopsound.sound < 0) {
                        const std::string str = configuration().cockpit_audio.autopilot_available;
                        autostopsound.loadsound(str);
                    }
                    autostopsound.playsound();
                } else {
                    static soundContainer autosound;
                    if (autosound.sound < 0) {
                        const std::string str = configuration().cockpit_audio.autopilot_unavailable;
                        autosound.loadsound(str);
                    }
                    autosound.playsound();
                }
                wasautopilot = abletoautopilot;
            }
            return abletoautopilot;
        }
        case UnitImages<void>::COCKPIT_FPS:
            if (fpsval >= 0 && fpsval < .5 * FLT_MAX) {
                numtimes -= 0.1F + fpsval;
            }
            if (numtimes <= 0) {
                numtimes = fpsmax;
                fpsval = GetElapsedTime();
            }
            if (fpsval) {
                return 1.0F / fpsval;
            }
        case UnitImages<void>::AUTOPILOT_MODAL:
            if (target->autopilotactive) {
                return static_cast<float>(UnitImages<void>::ACTIVE);
            } else {
                return static_cast<float>(UnitImages<void>::OFF);
            }
        case UnitImages<void>::SPEC_MODAL:
            if (target->graphicOptions.WarpRamping) {
                return static_cast<float>(UnitImages<void>::SWITCHING);
            } else if (target->ftl_drive.Enabled()) {
                return static_cast<float>(UnitImages<void>::ACTIVE);
            } else {
                return static_cast<float>(UnitImages<void>::OFF);
            }
        case UnitImages<void>::FLIGHTCOMPUTER_MODAL:
            if (target->inertialmode) {
                return static_cast<float>(UnitImages<void>::OFF);
            } else {
                return static_cast<float>(UnitImages<void>::ON);
            }
        case UnitImages<void>::TURRETCONTROL_MODAL:
            if (0 == target->turretstatus) {
                return static_cast<float>(UnitImages<void>::NOTAPPLICABLE);
            } else if (2 == target->turretstatus) {          //FIXME -- need to check if turrets are active
                return static_cast<float>(UnitImages<void>::ACTIVE);
            } else if (3 == target->turretstatus) {          //FIXME -- need to check if turrets are in FireAtWill state
                return static_cast<float>(UnitImages<void>::FAW);
            } else {
                return static_cast<float>(UnitImages<void>::OFF);
            }
        case UnitImages<void>::ECM_MODAL:
            if (target->ecm.Get() > 0) {
                return (target->ecm.Active() ? static_cast<float>(UnitImages<void>::ACTIVE)
                        : static_cast<float>(UnitImages<void>::READY));
            } else {
                return static_cast<float>(UnitImages<void>::NOTAPPLICABLE);
            }
        case UnitImages<void>::CLOAK_MODAL:
            if (!target->cloak.Capable() || target->cloak.Damaged()) {
                return static_cast<float>(UnitImages<void>::NOTAPPLICABLE);
            } else if (target->cloak.Ready()) {
                return static_cast<float>(UnitImages<void>::READY);
            } else if (target->cloak.Cloaked()) {
                return static_cast<float>(UnitImages<void>::ACTIVE);
            } else {
                return static_cast<float>(UnitImages<void>::SWITCHING);
            }
        case UnitImages<void>::TRAVELMODE_MODAL:
            if (target->CombatMode()) {
                return static_cast<float>(UnitImages<void>::MANEUVER);
            } else {
                return static_cast<float>(UnitImages<void>::TRAVEL);
            }
        case UnitImages<void>::RECIEVINGFIRE_MODAL:
            if (!target) {          //FIXME
                return static_cast<float>(UnitImages<void>::WARNING);
            } else {
                return static_cast<float>(UnitImages<void>::NOMINAL);
            }
        case UnitImages<void>::RECEIVINGMISSILES_MODAL:
            if (!target) {          //FIXME
                return static_cast<float>(UnitImages<void>::WARNING);
            } else {
                return static_cast<float>(UnitImages<void>::NOMINAL);
            }
        case UnitImages<void>::RECEIVINGMISSILELOCK_MODAL:
            if (!target) {          //FIXME
                return static_cast<float>(UnitImages<void>::WARNING);
            } else {
                return static_cast<float>(UnitImages<void>::NOMINAL);
            }
        case UnitImages<void>::RECEIVINGTARGETLOCK_MODAL:
            if (!target) {          //FIXME
                return static_cast<float>(UnitImages<void>::WARNING);
            } else {
                return static_cast<float>(UnitImages<void>::NOMINAL);
            }
        case UnitImages<void>::COLLISIONWARNING_MODAL:
            if (!target) {          //FIXME
                return static_cast<float>(UnitImages<void>::WARNING);
            } else {
                return static_cast<float>(UnitImages<void>::NOMINAL);
            }
        case UnitImages<void>::CANJUMP_MODAL:
            if (!target->jump_drive.Installed() || !target->jump_drive.Operational()) {
                return static_cast<float>(UnitImages<void>::NODRIVE);
            } else if (!target->jump_drive.CanConsume()) {
                return static_cast<float>(UnitImages<void>::NOTENOUGHENERGY);
            } else if (target->ftl_drive.Enabled()) {          //FIXME
                return static_cast<float>(UnitImages<void>::OFF);
            } else if (jumpok) {
                return static_cast<float>(UnitImages<void>::READY);
            } else {
                return static_cast<float>(UnitImages<void>::TOOFAR);
            }
        case UnitImages<void>::CANDOCK_MODAL: {
            Unit *station = target->Target();
            if (station) {
                if(target->getUnitType() != Vega_UnitType::planet ) {
                    return static_cast<float>(UnitImages<void>::NOMINAL);
                }

                if (CanDock(station, target, true) != -1) {
                    if (CanDock(station, target, false) != -1) {
                        return static_cast<float>(UnitImages<void>::READY);
                    }
                    if (Orders::AutoDocking::CanDock(target, station)) {
                        return static_cast<float>(UnitImages<void>::AUTOREADY);
                    }
                    return static_cast<float>(UnitImages<void>::TOOFAR);
                }
            }
            return static_cast<float>(UnitImages<void>::NOMINAL);
        }
        default:
            return 1.0F;
    }
    return 1.0F;
}


GameCockpit::LastState::LastState() {
    processing_time = 0;

    jumpok = jumpnotok =
            specon = specoff =
                    asapon = asapoff =
                            asap_dockon = asap_dockoff =
                                    asap_dock_avail =
                                            dock =
                                                    dock_avail =
                                                            lock = missilelock =
                                                                    eject =
                                                                            flightcompon = flightcompoff = false;
}

void GameCockpit::TriggerEvents(Unit *un) {
    double curtime = UniverseUtil::GetGameTime();
    if ((curtime - AUDIO_ATOM) < last.processing_time) {
        return;
    } else {
        last.processing_time = curtime;
    }

    VS_LOG(trace, "Processing events");
    for (EVENTID event = EVENTID_FIRST; event < NUM_EVENTS; event = (EVENTID)(event + 1)) {
        GameSoundContainer *sound = static_cast<GameSoundContainer *>(GetSoundForEvent(event));
        if (sound != NULL) {

            #define MODAL_TRIGGER(name, _triggervalue, _curvalue, lastvar)                                                      \
                do {                                                                                                            \
                    bool triggervalue = _triggervalue;                                                                          \
                    bool curvalue = _curvalue;                                                                                  \
                    VS_LOG(trace, (boost::format("Processing event " name " (cur=%1% last=%2%)")                                \
                                                % int(curvalue) % int(last.lastvar)));                                          \
                                                                                                                                \
                    if (curvalue != last.lastvar) {                                                                             \
                        VS_LOG(debug, (boost::format("Triggering event edge " name " (cur=%1% last=%2% on=%3%)")                \
                                                % int(curvalue) % int(last.lastvar) % int(triggervalue)));                      \
                        last.lastvar = curvalue;                                                                                \
                        if (curvalue == triggervalue) {                                                                         \
                            sound->play();                                                                                      \
                        } else {                                                                                                \
                            sound->stop();                                                                                      \
                        }                                                                                                       \
                    }                                                                                                           \
                } while(0)

            #define MODAL_IMAGE_TRIGGER(image, itrigger, btrigger, lastvar) \
                MODAL_TRIGGER(#image, btrigger, LookupUnitStat(UnitImages< void >::image, un) == UnitImages< void >::itrigger, lastvar)

            #define MODAL_RAWIMAGE_TRIGGER(image, itrigger, btrigger, lastvar) \
                MODAL_TRIGGER(#image, btrigger, LookupUnitStat(UnitImages< void >::image, un) itrigger, lastvar)

            switch ((int) event) {
                case WARP_READY:
                    MODAL_RAWIMAGE_TRIGGER(MAXWARPFIELDSTRENGTH, >= 2, true, warpready);
                    break;
                case WARP_UNREADY:
                    MODAL_RAWIMAGE_TRIGGER(MAXWARPFIELDSTRENGTH, >= 2, false, warpunready);
                    break;
                case WARP_ENGAGED:
                    MODAL_IMAGE_TRIGGER(SPEC_MODAL, OFF, false, specon);
                    break;
                case WARP_DISENGAGED:
                    MODAL_IMAGE_TRIGGER(SPEC_MODAL, OFF, true, specoff);
                    break;
                case FLIGHT_COMPUTER_ENABLED:
                    MODAL_IMAGE_TRIGGER(FLIGHTCOMPUTER_MODAL, OFF, false, flightcompon);
                    break;
                case FLIGHT_COMPUTER_DISABLED:
                    MODAL_IMAGE_TRIGGER(FLIGHTCOMPUTER_MODAL, OFF, true, flightcompoff);
                    break;
                case ASAP_ENGAGED:
                    MODAL_TRIGGER("ASAP_ENGAGED", true, un->autopilotactive, asapon);
                    break;
                case ASAP_DISENGAGED:
                    MODAL_TRIGGER("ASAP_DISENGAGED", false, un->autopilotactive, asapoff);
                    break;
                case DOCK_AVAILABLE:
                    MODAL_IMAGE_TRIGGER(CANDOCK_MODAL, READY, true, dock_avail);
                    break;
                case ASAP_DOCKING_AVAILABLE:
                    MODAL_IMAGE_TRIGGER(CANDOCK_MODAL, AUTOREADY, true, asap_dock_avail);
                    break;
                case ASAP_DOCKING_ENGAGED: {
                    float candock = LookupUnitStat(UnitImages<void>::CANDOCK_MODAL, un);
                    MODAL_TRIGGER("ASAP_DOCKING", true,
                            (un->autopilotactive && (candock == UnitImages<void>::READY
                                    || candock == UnitImages<void>::AUTOREADY)),
                            asap_dockon);
                }
                    break;
                case ASAP_DOCKING_DISENGAGED: {
                    float candock = LookupUnitStat(UnitImages<void>::CANDOCK_MODAL, un);
                    MODAL_TRIGGER("ASAP_DOCKING", false,
                            (un->autopilotactive && (candock == UnitImages<void>::READY
                                    || candock == UnitImages<void>::AUTOREADY)),
                            asap_dockoff);
                }
                    break;
                case JUMP_AVAILABLE:
                    MODAL_TRIGGER("JUMP_AVAILABLE", true, ((jumpok) ? true : false), jumpok);
                    break;
                case JUMP_UNAVAILABLE:
                    MODAL_TRIGGER("JUMP_UNAVAILABLE", false, ((jumpok) ? true : false), jumpnotok);
                    break;
                case LOCK_WARNING:
                    MODAL_RAWIMAGE_TRIGGER(LOCK, >= 1, true, lock);
                    break;
                case MISSILELOCK_WARNING:
                    MODAL_RAWIMAGE_TRIGGER(MISSILELOCK, >= 1, true, missilelock);
                    break;
                case EJECT_WARNING:
                    MODAL_RAWIMAGE_TRIGGER(EJECT, >= 1, true, eject);
                    break;
                case WARP_LOOP0:
                case WARP_LOOP0 + 1:
                case WARP_LOOP0 + 2:
                case WARP_LOOP0 + 3:
                case WARP_LOOP0 + 4:
                case WARP_LOOP0 + 5:
                case WARP_LOOP0 + 6:
                case WARP_LOOP0 + 7:
                case WARP_LOOP0 + 8:
                case WARP_LOOP0 + 9: {
                    float warpfieldstrength = LookupUnitStat(UnitImages<void>::WARPFIELDSTRENGTH, un);
                    int warpreflevel = event - WARP_LOOP0;
                    int warplevel = int(log(warpfieldstrength) / log(10.f));
                    MODAL_TRIGGER("WARP_LOOP", warpreflevel, warplevel, warplooplevel);
                }
                    break;
                case WARP_SKIP0:
                case WARP_SKIP0 + 1:
                case WARP_SKIP0 + 2:
                case WARP_SKIP0 + 3:
                case WARP_SKIP0 + 4:
                case WARP_SKIP0 + 5:
                case WARP_SKIP0 + 6:
                case WARP_SKIP0 + 7:
                case WARP_SKIP0 + 8:
                case WARP_SKIP0 + 9: {
                    float warpfieldstrength = LookupUnitStat(UnitImages<void>::WARPFIELDSTRENGTH, un);
                    int warpreflevel = event - WARP_SKIP0;
                    int warplevel = int(log(warpfieldstrength) / log(10.0f));
                    MODAL_TRIGGER("WARP_SKIP", warpreflevel, warplevel, warpskiplevel);
                }
                    break;
                case JUMP_FAILED:
                case DOCK_FAILED:
                    // TODO
                    break;
                default:
                    break;
            } // switch
        } // if
    } // for
}


void GameCockpit::Init(const char *file) {
    smooth_fov = configuration().graphics.fov_flt;
    editingTextMessage = false;
    Cockpit::Init(file);
    if (Panel.size() > 0) {
        float x, y;
        Panel.front()->GetPosition(x, y);
        Panel.front()->SetPosition(x, y + viewport_offset);
    }
}

void GameCockpit::Delete() {
    Cockpit::Delete();

    if (text != nullptr) {
        delete text;
        text = nullptr;
    }
    for (size_t i = 0; i < mesh.size(); ++i) {
        if (mesh[i] != nullptr) {
            delete mesh[i];
            mesh[i] = nullptr;
        }
    }
    mesh.clear();
    if (soundfile >= 0) {
        AUDStopPlaying(soundfile);
        AUDDeleteSound(soundfile, false);
        soundfile = -1;
    }
    // for (i = 0; i < 4; i++) {
    //     /*
    //      *  if (Pit[i]) {
    //      *  delete Pit[i];
    //      *  Pit[i] = NULL;
    //      *  }
    //      */
    // }
    for (int i = 0; i < UnitImages<void>::NUMGAUGES; ++i) {
        if (gauges[i] != nullptr) {
            delete gauges[i];
            gauges[i] = nullptr;
        }
    }
    if (radarSprites[0] != nullptr) {
        delete radarSprites[0];
        radarSprites[0] = nullptr;
    }
    if (radarSprites[1] != nullptr) {
        delete radarSprites[1];
        radarSprites[1] = nullptr;
    }
    for (size_t j = 0; j < vdu.size(); ++j) {
        if (vdu[j] != nullptr) {
            delete vdu[j];
            vdu[j] = nullptr;
        }
    }
    vdu.clear();
    for (size_t j = 0; j < Panel.size(); ++j) {
        assert(Panel[j]);
        delete Panel[j];
        Panel[j] = nullptr;
    }
    Panel.clear();
}

void GameCockpit::InitStatic() {
    int i;
    for (i = 0; i < UnitImages<void>::NUMGAUGES; i++) {
        gauge_time[i] = 0;
    }
    for (i = 0; i < MAXVDUS; i++) {
        vdu_time[i] = 0;
    }
    radar_time = 0;
    cockpit_time = 0;
}

/***** WARNING CHANGED ORDER *****/
GameCockpit::GameCockpit(const char *file, Unit *parent, const std::string &pilot_name)
        : Cockpit(file, parent, pilot_name),
        insidePanYaw(0),
        insidePanPitch(0),
        insidePanYawSpeed(0),
        insidePanPitchSpeed(0),
        shake_time(0),
        shake_type(0),
        textcol(1, 1, 1, 1),
        text(NULL) {
    autoMessageTime = 0;
    editingTextMessage = false;
    const int headlag = configuration().graphics.head_lag;
    int i;
    for (i = 0; i < headlag; i++) {
        headtrans.push_back(Matrix());
        Identity(headtrans.back());
    }
    for (i = 0; i < UnitImages<void>::NUMGAUGES; i++) {
        gauges[i] = NULL;
    }
    radarSprites[0] = radarSprites[1] = Pit[0] = Pit[1] = Pit[2] = Pit[3] = NULL;

    draw_all_boxes = configuration().graphics.hud.draw_all_target_boxes;
    draw_line_to_target = configuration().graphics.hud.draw_line_to_target;
    draw_line_to_targets_target = configuration().graphics.hud.draw_line_to_targets_target;
    draw_line_to_itts = configuration().graphics.hud.draw_line_to_itts;
    always_itts = configuration().graphics.hud.draw_always_itts;
    steady_itts = configuration().physics.steady_itts;
    last_locktime = last_mlocktime = -FLT_MAX;

    radarDisplay = Radar::Factory(Radar::Type::NullDisplay);

    //Compute the screen limits. Used to display the arrow pointing to the selected target.
    const float limit_y = configuration().graphics.fov_flt;
    smooth_fov = limit_y;
    projection_limit_y = limit_y;
    //The angle between the center of the screen and the border is half the fov.
    projection_limit_y = tan(projection_limit_y * M_PI / (180 * 2));
    projection_limit_x = projection_limit_y * configuration().graphics.aspect_flt;
    //Precompute this division... performance.
    inv_screen_aspect_ratio = 1 / configuration().graphics.aspect_flt;

    oaccel = Vector(0, 0, 0);

    enemy = vs_config->getColor("enemy", GFXColor(1.0, 0.0, 0.0, 1.0)); // red
    friendly = vs_config->getColor("friend", GFXColor(0.0, 1.0, 0.0, 1.0)); // green
    neutral = vs_config->getColor("neutral", GFXColor(1.0, 1.0, 0.0, 1.0)); // yellow
    targeted = vs_config->getColor("target", GFXColor(1.0, 0.0, 1.0, 1.0)); // violet
    targetting = vs_config->getColor("targetting_ship", GFXColor(.68, .9, 1.0, 1.0)); // light-blue
    planet = vs_config->getColor("planet", GFXColor(1.0, 1.0, .6, 1.0)); //
    soundfile = -1;
    InitStatic();
    updateRadar(parent);
}

void GameCockpit::SelectProperCamera() {
    SelectCamera(view);
}

extern vector<int> respawnunit;
extern vector<int> switchunit;
extern vector<int> turretcontrol;

void DoCockpitKeys() {
    CockpitKeys::Pan(KBData(), PRESS);
    CockpitKeys::Inside(KBData(), PRESS);
}

void GameCockpit::NavScreen(const KBData &, KBSTATE k) {
    //scheherazade
    if (k == PRESS) {
        //UniverseUtil::IOmessage(0,"game","all","hit key");
        if ((_Universe->AccessCockpit())->CanDrawNavSystem()) {
            (_Universe->AccessCockpit())->SetDrawNavSystem(0);
            //UniverseUtil::IOmessage(0,"game","all","DRAWNAV - OFF");
            RestoreMouse();
        } else {
            (_Universe->AccessCockpit())->SetDrawNavSystem(1);
            //UniverseUtil::IOmessage(0,"game","all","DRAWNAV - ON");

            winsys_set_mouse_func(BaseInterface::ClickWin);
            winsys_set_motion_func(BaseInterface::ActiveMouseOverWin);
            winsys_set_passive_motion_func(BaseInterface::PassiveMouseOverWin);
        }
    }
}

bool GameCockpit::SetDrawNavSystem(bool what) {
    ThisNav.SetDraw(what);
    return what;
}

bool GameCockpit::CanDrawNavSystem() {
    return ThisNav.CheckDraw();
}

void GameCockpit::visitSystem(string systemname) {
    Cockpit::visitSystem(systemname);
    if (AccessNavSystem()) {
        //causes occasional crash--only may have tracked it down
        const bool AlwaysUpdateNavMap = configuration().graphics.update_nav_after_jump;
        if (AlwaysUpdateNavMap) {
            AccessNavSystem()->pathman->updatePaths();
        }
        AccessNavSystem()->setCurrentSystem(systemname);
    }
}


void RespawnNow(Cockpit *cp) {
    while (respawnunit.size() <= _Universe->numPlayers()) {
        respawnunit.push_back(0);
    }
    for (unsigned int i = 0; i < _Universe->numPlayers(); i++) {
        if (_Universe->AccessCockpit(i) == cp) {
            respawnunit[i] = 2;
        }
    }
}

void GameCockpit::SwitchControl(const KBData &, KBSTATE k) {
    if (k == PRESS) {
        while (switchunit.size() <= _Universe->CurrentCockpit()) {
            switchunit.push_back(0);
        }
        switchunit[_Universe->CurrentCockpit()] = 1;
    }
}

void GameCockpit::ForceSwitchControl(const KBData &, KBSTATE k) {
    while (switchunit.size() <= _Universe->CurrentCockpit()) {
        switchunit.push_back(0);
    }
    switchunit[_Universe->CurrentCockpit()] = 1;
}

void SuicideKey(const KBData &, KBSTATE k) {
    static int orig = 0;
    if (k == PRESS) {
        int newtime = time(NULL);
        if (newtime - orig > 8 || orig == 0) {
            orig = newtime;
            Unit *un = NULL;
            if ((un = _Universe->AccessCockpit()->GetParent())) {
//                float armor[8];                 //short fix
                un->Destroy();
            }
        }
    }
}

class UnivMap {
    VSSprite *ul;
    VSSprite *ur;
    VSSprite *ll;
    VSSprite *lr;
public:
    bool isNull() {
        return ul == NULL;
    }

    UnivMap(VSSprite *ull, VSSprite *url, VSSprite *lll, VSSprite *lrl) {
        ul = ull;
        ur = url;
        ll = lll;
        lr = lrl;
    }

    void Draw() {
        if (ul || ur || ll || lr) {
            GFXBlendMode(SRCALPHA, INVSRCALPHA);
            GFXEnable(TEXTURE0);
            GFXDisable(TEXTURE1);
            GFXColor4f(1, 1, 1, 1);
        }
        if (ul) {
            ul->Draw();
        }
        if (ur) {
            ur->Draw();
        }
        if (ll) {
            ll->Draw();
        }
        if (lr) {
            lr->Draw();
        }
    }
};

std::vector<UnivMap> univmap;

void MapKey(const KBData &, KBSTATE k) {
    if (k == PRESS) {
        static VSSprite ul("upper-left-map.spr");
        static VSSprite ur("upper-right-map.spr");
        static VSSprite ll("lower-left-map.spr");
        static VSSprite lr("lower-right-map.spr");
        while (univmap.size() <= _Universe->CurrentCockpit()) {
            univmap.push_back(UnivMap(NULL, NULL, NULL, NULL));
        }
        if (univmap[_Universe->CurrentCockpit()].isNull()) {
            univmap[_Universe->CurrentCockpit()] = UnivMap(&ul, &ur, &ll, &lr);
        } else {
            univmap[_Universe->CurrentCockpit()] = UnivMap(NULL, NULL, NULL, NULL);
        }
    }
}

void GameCockpit::TurretControl(const KBData &, KBSTATE k) {
    if (k == PRESS) {
        while (turretcontrol.size() <= _Universe->CurrentCockpit()) {
            turretcontrol.push_back(0);
        }
        turretcontrol[_Universe->CurrentCockpit()] = 1;
    }
}

void GameCockpit::Respawn(const KBData &, KBSTATE k) {
    if (k == PRESS) {
        while (respawnunit.size() <= _Universe->CurrentCockpit()) {
            respawnunit.push_back(0);
        }
        respawnunit[_Universe->CurrentCockpit()] = 1;
    }
}

//SAME AS IN COCKPIT BUT ADDS SETVIEW and ACCESSCAMERA -> ~ DUPLICATE CODE
int GameCockpit::Autopilot(Unit *target) {
    const bool autopan = configuration().graphics.pan_on_auto;
    int retauto = 0;
    if (target) {
        if (enableautosound.sound < 0) {
            static string str = configuration().cockpit_audio.autopilot_enabled;
            enableautosound.loadsound(str);
        }
        enableautosound.playsound();
        Unit *un = NULL;
        if ((un = GetParent())) {
            autoMessage = std::string();
            autoMessageTime = UniverseUtil::GetGameTime();
            QVector posA = un->LocalPosition();
            bool retautoA = false;
            if ((retauto = retautoA = un->AutoPilotToErrorMessage(un, false, autoMessage))) {
                //can he even start to autopilot
                retauto = un->AutoPilotToErrorMessage(target, false, autoMessage);
                QVector posB = un->LocalPosition();
                bool movedatall = (posA - posB).Magnitude() > un->rSize();
                if (autoMessage.length() == 0 && !movedatall) {
                    autoMessage = configuration().graphics.hud.already_near_message;
                    retauto = false;
                } else if ((retauto || retautoA) && movedatall) {
                    if (autopan) {
                        SetView(CP_FIXEDPOS);
                        Vector P(1, 0, 0), Q(0, 1, 0), R(0, 0, 1);
                        Vector uP, uQ, uR;
                        un->GetOrientation(uP, uQ, uR);
                        const float auto_side_bias = configuration().graphics.autopilot_side_bias_flt;
                        const float auto_front_bias = configuration().graphics.autopilot_front_bias_flt;
                        P += uP * auto_side_bias + uR * auto_front_bias;
                        P.Normalize();
                        R = P.Cross(Q);
                        AccessCamera(CP_FIXED)->SetPosition(un->LocalPosition() + 2 * un->rSize() * P,
                                Vector(0, 0, 0), Vector(0, 0, 0), Vector(0, 0, 0));
                        AccessCamera(CP_FIXED)->SetOrientation(R, Q, -P);
                        AccessCamera(CP_FIXEDPOS)->SetPosition(un->LocalPosition() + 2 * un->rSize() * P, Vector(0,
                                        0,
                                        0),
                                Vector(0, 0, 0), Vector(0, 0, 0));
                        AccessCamera(CP_FIXEDPOS)->SetOrientation(R, Q, -P);
                    }

                    const float autotime = configuration().physics.auto_time_in_seconds_flt;                    //10 seconds for auto to kick in;

                    autopilot_time = autotime;
                    autopilot_target.SetUnit(target);
                }
            }
        }
    }
    return retauto;
}

extern void reset_time_compression(const KBData &, KBSTATE a);

void GameCockpit::Shake(float amt, int dtype) {
    const float shak = configuration().graphics.cockpit_shake_flt;
    const float shak_max = configuration().graphics.cockpit_shake_max_flt;
    shakin += shak;
    if (shakin > shak_max) {
        shakin = shak_max;
    }
    this->shake_time = getNewTime();
    this->shake_type = dtype;
}

static void DrawDamageFlash(int dtype) {
    const int numtypes = 3;
    const string shieldflash = configuration().graphics.shield_flash_animation;
    const string armorflash = configuration().graphics.armor_flash_animation;
    const string hullflash = configuration().graphics.hull_flash_animation;
    string flashes[numtypes];
    flashes[0] = shieldflash;
    flashes[1] = armorflash;
    flashes[2] = hullflash;
    static GFXColor fallbackcolor[numtypes] = {
            vs_config->getColor("shield_flash", GFXColor(0, 1, .5, .2)),
            vs_config->getColor("armor_flash", GFXColor(1, 0, .2, .25)),
            vs_config->getColor("hull_flash", GFXColor(1, 0, 0, .5))
    };

    static bool init = false;
    static Animation *aflashes[numtypes];
    static bool doflash[numtypes];
    if (!init) {
        init = true;
        for (int i = 0; i < numtypes; ++i) {
            doflash[i] = (flashes[i].length() > 0);
            if (doflash[i]) {
                aflashes[i] = new Animation(flashes[i].c_str(), true, .1, BILINEAR, false, false);
            } else {
                aflashes[i] = NULL;
            }
        }
    }
    if (dtype < numtypes) {
        int i = dtype;
        if (aflashes[i]) {
            GFXPushBlendMode();
            const bool damage_flash_alpha = configuration().graphics.damage_flash_alpha;
            if (damage_flash_alpha) {
                GFXBlendMode(SRCALPHA, INVSRCALPHA);
            } else {
                GFXBlendMode(ONE, ZERO);
            }
            if (aflashes[i]->LoadSuccess()) {
                aflashes[i]->MakeActive();
                GFXColor4f(1, 1, 1, 1);

                static const float verts[4 * (3 + 2)] = {
                        -1.0f, -1.0f, 1.0f, 0.0f, 1.0f,    //lower left
                        1.0f, -1.0f, 1.0f, 1.0f, 1.0f,    //upper left
                        1.0f, 1.0f, 1.0f, 1.0f, 0.0f,    //upper right
                        -1.0f, 1.0f, 1.0f, 0.0f, 0.0f,    //lower right
                };
                GFXDraw(GFXQUAD, verts, 4, 3, 0, 2);
            } else {
                GFXColor4f(fallbackcolor[i].r,
                        fallbackcolor[i].g,
                        fallbackcolor[i].b,
                        fallbackcolor[i].a);
                GFXDisable(TEXTURE0);

                static const float verts[4 * 3] = {
                        -1.0f, -1.0f, 1.0f,
                        -1.0f, 1.0f, 1.0f,
                        1.0f, 1.0f, 1.0f,
                        1.0f, -1.0f, 1.0f,
                };
                GFXDraw(GFXQUAD, verts, 4);

                GFXEnable(TEXTURE0);
            }
            GFXPopBlendMode();
        }
    }
    GFXColor4f(1, 1, 1, 1);
}

static void DrawHeadingMarker(const Vector &p, const Vector &q, const Vector &pos, float size) {
    static VertexBuilder<> verts;
    verts.clear();
    verts.insert(pos + (2.5 * size) * p);
    verts.insert(pos + size * p);
    verts.insert(pos + (0.939 * size) * p - (0.342 * size) * q);
    verts.insert(pos + (0.776 * size) * p - (0.643 * size) * q);
    verts.insert(pos + (0.500 * size) * p - (0.866 * size) * q);
    verts.insert(pos + (0.174 * size) * p - (0.985 * size) * q);
    verts.insert(pos - (0.174 * size) * p - (0.985 * size) * q);
    verts.insert(pos - (0.500 * size) * p - (0.866 * size) * q);
    verts.insert(pos - (0.776 * size) * p - (0.643 * size) * q);
    verts.insert(pos - (0.939 * size) * p - (0.342 * size) * q);
    verts.insert(pos - size * p);
    verts.insert(pos - (2.5 * size) * p);
    GFXDraw(GFXLINESTRIP, verts);
}

static void DrawHeadingMarker(Cockpit &cp, const GFXColor &col) {
    const Unit *u = cp.GetParent();
    const Camera *cam = cp.AccessCamera();
    bool drawv = true;

    // heading direction (unit fwd direction)
    Vector d = u->GetTransformation().getR();

    // flight direcion (unit vel direction)
    Vector v = u->GetWarpVelocity();
    if (u->VelocityReference()) {
        v -= u->VelocityReference()->GetWarpVelocity();
    }
    float v2 = v.MagnitudeSquared();
    if (v2 > 0.25) { // 1/2 m/s seems reasonable for a speed marker
        v *= 1.0f / sqrtf(v2);
    } else {
        drawv = false;
    }

    // up and right dirs p, q
    Vector p, q, r;
    cam->GetPQR(p, q, r);

    // znear offset
    float offset = 2 * configuration().graphics.znear_flt / cos(cam->GetFov() * M_PI / 180.0);
    v *= offset;
    d *= offset;

    // size scale and flight dir alpha
    float size = 0.175f;
    float alpha = std::min(0.60f, (v - d).MagnitudeSquared() / (size * size * 36));

    // draw
    GFXDisable(TEXTURE0);
    GFXDisable(LIGHTING);
    GFXBlendMode(SRCALPHA, INVSRCALPHA);
    GFXEnable(SMOOTH);

    if (drawv) {
        GFXLineWidth(1.35f);
        GFXColor4f(col.r, col.g, col.b, col.a * alpha);
        DrawHeadingMarker(p, q, v * 1.01, size);
    }

    GFXLineWidth(1.25f);
    GFXColor4f(col.r, col.g, col.b, col.a);
    DrawHeadingMarker(p, q, d, size);

    GFXLineWidth(1.0f);
    GFXEnable(TEXTURE0);
}

static void DrawCrosshairs(float x, float y, float wid, float hei, const GFXColor &col) {
    GFXColorf(col);
    GFXDisable(TEXTURE0);
    GFXDisable(LIGHTING);
    GFXBlendMode(SRCALPHA, INVSRCALPHA);
    GFXEnable(SMOOTH);
    GFXCircle(x, y, wid / 4, hei / 4);
    GFXCircle(x, y, wid / 7, hei / 7);
    GFXDisable(SMOOTH);

    const float verts[12 * 3] = {
            x - (wid / 2.f), y, 0,
            x - (wid / 6.f), y, 0,
            x + (wid / 2.f), y, 0,
            x + (wid / 6.f), y, 0,
            x, y - (hei / 2.f), 0,
            x, y - (hei / 6.f), 0,
            x, y + (hei / 2.f), 0,
            x, y + (hei / 6.f), 0,
            x - .001f, y + .001f, 0,
            x + .001f, y - .001f, 0,
            x + .001f, y + .001f, 0,
            x - .001f, y - .001f, 0,
    };
    GFXDraw(GFXLINE, verts, 12);

    GFXEnable(TEXTURE0);
}

extern bool QuitAllow;
extern bool screenshotkey;
QVector SystemLocation(std::string system);

void GameCockpit::Draw() {
    const bool draw_heading_marker = configuration().graphics.draw_heading_marker;
    const bool draw_star_destination_arrow = configuration().graphics.hud.draw_star_direction;
    static GFXColor destination_system_color = vs_config->getColor("destination_system_color");
    Vector destination_system_location(0, 0, 0);
    cockpit_time += GetElapsedTime();
    if (cockpit_time >= 100000) {
        InitStatic();
    }
    _Universe->AccessCamera()->UpdateGFX(GFXFALSE, GFXFALSE, GFXTRUE);     //Preliminary frustum
    GFXDisable(TEXTURE0);
    GFXDisable(TEXTURE1);
    GFXLoadIdentity(MODEL);
    GFXDisable(LIGHTING);
    GFXDisable(DEPTHTEST);
    GFXDisable(DEPTHWRITE);
    GFXColor4f(1, 1, 1, 1);
    const std::string & nav_current = AccessNavSystem()->getCurrentSystem();
    const std::string & universe_current = _Universe->activeStarSystem()->getFileName();
    if (nav_current != universe_current) {
        AccessNavSystem()->Setup();
    }
    const bool draw_any_boxes = configuration().graphics.hud.draw_targetting_boxes;
    const bool draw_boxes_inside_only = configuration().graphics.hud.draw_targetting_boxes_inside;
    if (draw_any_boxes && screenshotkey == false && (draw_boxes_inside_only == false || view < CP_CHASE)) {
        Unit *player = GetParent();
        if (player) {
            Radar::Sensor sensor(player);
            DrawTargetBox(sensor, draw_line_to_target, draw_line_to_targets_target,
                          always_itts, player->computeLockingPercent(), draw_line_to_itts, steady_itts);
            DrawTurretTargetBoxes(sensor);
            DrawTacticalTargetBox(sensor);
            DrawCommunicatingBoxes(vdu);
            if (draw_all_boxes) {
                DrawTargetBoxes(sensor);
            }
        }
        if (draw_star_destination_arrow) {
            const std::string & destination_system = AccessNavSystem()->getSelectedSystem();
            const std::string & current_system = _Universe->activeStarSystem()->getFileName();
            if (destination_system != current_system) {
                QVector cur = SystemLocation(current_system);
                QVector dest = SystemLocation(destination_system);
                QVector delta = dest - cur;
                if (delta.i != 0 || dest.j != 0 || dest.k != 0) {
                    delta.Normalize();
                    Unit *par = GetParent();
                    delta = delta * configuration().physics.distance_to_warp_dbl * 1.01 - (par ? (par->Position()) : QVector(0, 0, 0));
                    destination_system_location = delta.Cast();
                    Vector P, Q, R;
                    const float nav_symbol_size = configuration().graphics.nav.symbol_size_flt;
                    AccessCamera()->GetPQR(P, Q, R);

                    GFXColor4f(destination_system_color.r,
                            destination_system_color.g,
                            destination_system_color.b,
                            destination_system_color.a);

                    static GFXColor suncol = vs_config->getColor("remote_star", GFXColor(0, 1, 1, .8));
                    GFXColorf(suncol);
                    DrawNavigationSymbol(delta.Cast(), P, Q, delta.Magnitude() * nav_symbol_size);

                    GFXColor4f(1, 1, 1, 1);
                }
            }
        }
    }
    if (draw_heading_marker && view < CP_CHASE) {
        DrawHeadingMarker(*this, textcol);
    }
    GFXEnable(TEXTURE0);
    GFXEnable(DEPTHTEST);
    GFXEnable(DEPTHWRITE);

    if (view < CP_CHASE) {
        if (mesh.size()) {
            Unit *par = GetParent();
            if (par) {
                //cockpit is unaffected by FOV WARP-Link
                float oldfov = AccessCamera()->GetFov();
                AccessCamera()->SetFov(configuration().graphics.fov_flt);

                GFXLoadIdentity(MODEL);

                size_t i, j;
                float cockpitradial = 1;                //LET IT NOT BE ZERO!
                for (i = 0; i < mesh.size(); ++i) {
                    float meshmaxdepth = mesh[i]->corner_min().Max(mesh[i]->corner_max()).Magnitude();
                    if (meshmaxdepth > cockpitradial) {
                        cockpitradial = meshmaxdepth;
                    }
                }
                cockpitradial *= COCKPITZ_HEADROOM;

                GFXEnable(DEPTHTEST);
                GFXEnable(DEPTHWRITE);
                GFXEnable(TEXTURE0);
                GFXEnable(LIGHTING);
                Vector P, Q, R;
                AccessCamera(CP_FRONT)->GetPQR(P, Q, R);

                headtrans.clear();

                headtrans.push_back(Matrix());
                VectorAndPositionToMatrix(headtrans.back(), -P, Q, R, QVector(0, 0, 0));
                static float theta = 0, wtheta = 0;
                const float shake_speed = configuration().graphics.shake_speed_flt;
                const float shake_reduction = configuration().graphics.shake_reduction_flt;
                const float shake_limit = configuration().graphics.shake_limit_flt;
                const float shake_mag = configuration().graphics.shake_magnitude_flt;
                const float drift_limit = configuration().graphics.cockpit_drift_limit_flt;
                const float drift_amount = configuration().graphics.cockpit_drift_amount_flt;
                const float drift_ref_accel = configuration().graphics.cockpit_drift_ref_accel_flt;

                const float warp_shake_mag = configuration().graphics.warp_shake_magnitude_flt;
                const float warp_shake_speed = configuration().graphics.warp_shake_speed_flt;
                float warp_shake_ref = configuration().graphics.warp_shake_ref_flt;
                if (warp_shake_ref <= 0) {
                    warp_shake_ref = 1;
                }
                theta += shake_speed * GetElapsedTime() * sqrt(fabs(shakin))
                        / 10;                 //For small shakes, slower shakes
                wtheta += warp_shake_speed * GetElapsedTime();                 //SPEC-related shaking

                float self_kps = ((GetParent() != NULL) ? LookupUnitStat(UnitImages<void>::KPS, GetParent()) : 0);
                float self_setkps =
                        max(1.0f, ((GetParent() != NULL) ? LookupUnitStat(UnitImages<void>::SETKPS, GetParent()) : 0));
                float warp_strength =
                        max(0.0f,
                                min(max(0.0f,
                                                min(1.0f,
                                                        self_kps / self_setkps)),
                                        ((GetParent() != NULL) ? LookupUnitStat(UnitImages<void>::WARPFIELDSTRENGTH,
                                                GetParent()) : 0.0f) / warp_shake_ref));
                if (shakin > shake_limit) {
                    shakin = shake_limit;
                }
                headtrans.back().p.i = shake_mag * shakin * cos(theta) * cockpitradial
                        / 100;          //AccessCamera()->GetPosition().i+shakin*cos(theta);
                headtrans.back().p.j = shake_mag * shakin * cos(1.3731 * theta) * cockpitradial
                        / 100;  //AccessCamera()->GetPosition().j+shakin*cos(theta);
                headtrans.back().p.k = 0;                 //AccessCamera()->GetPosition().k;
                headtrans.back().p.i += warp_shake_mag * cos(wtheta) * sqrtf(warp_strength) * cockpitradial
                        / 100;           //AccessCamera()->GetPosition().i+shakin*cos(theta);
                headtrans.back().p.j += warp_shake_mag * cos(1.165864 * wtheta) * sqrtf(warp_strength) * cockpitradial
                        / 100;  //AccessCamera()->GetPosition().j+shakin*cos(theta);
                if (shakin > 0) {
                    shakin -= GetElapsedTime() * shake_reduction
                            * (shakin / 5);                       //Fast convergence to 5% shaking, slow stabilization
                    if (shakin <= 0) {
                        shakin = 0;
                    }
                }
                //Now, compute head drift
                Vector caccel = AccessCamera(CP_FRONT)->GetAcceleration();
                float mag = caccel.Magnitude();
                float ref = drift_ref_accel * drift_ref_accel;
                if ((mag > 0) && (ref > 0)) {
                    caccel *= -(drift_amount * min(drift_limit, (float) (mag * mag / ref))) / mag;
                } else {
                    caccel = Vector(0, 0, 0);
                }
                float driftphase = std::pow(0.25, GetElapsedTime());
                oaccel = (1 - driftphase) * caccel + driftphase * oaccel;
                headtrans.back().p += -cockpitradial * oaccel;
                float driftmag = cockpitradial * oaccel.Magnitude();

                //if (COCKPITZ_PARTITIONS>1) GFXClear(GFXFALSE,GFXFALSE,GFXTRUE);//only clear stencil buffer
                //Should not be needed if VERYNEAR_CONST is propperly set, but would be useful with stenciled inverse order rendering.
                const size_t COCKPITZ_PARTITIONS = configuration().graphics.cockpit_z_partitions;
                float zrange = cockpitradial * (1 - VERYNEAR_CONST) + driftmag;
                float zfloor = cockpitradial * VERYNEAR_CONST;
                for (j = COCKPITZ_PARTITIONS; j > 0;
                        j--) { //FIXME This is a program lockup!!! (actually, no; j is a size_t...)
                    AccessCamera()->UpdateGFX(GFXTRUE,
                            GFXTRUE,
                            GFXTRUE,
                            GFXTRUE,
                            zfloor + zrange * (j - 1) / COCKPITZ_PARTITIONS,
                            zfloor + zrange * j
                                    / COCKPITZ_PARTITIONS);                                                                                       //cockpit-specific frustrum (with clipping, with frustrum update)
                    for (i = 0; i < mesh.size(); ++i) {
                        mesh[i]->Draw(FLT_MAX, headtrans.back());
                    }

                    Mesh::ProcessZFarMeshes(true);
                    //if (nocockpitcull) GFXDisable( CULLFACE );
                    Mesh::ProcessUndrawnMeshes(false, true);
                }
                headtrans.pop_back();
                //if (COCKPITZ_PARTITIONS>1) GFXDisable(STENCIL);
                GFXDisable(LIGHTING);
                GFXDisable(TEXTURE0);
                GFXDisable(TEXTURE1);
                AccessCamera()->SetFov(oldfov);
            }
        }
        _Universe->AccessCamera()
                ->UpdateGFX(GFXFALSE, GFXFALSE, GFXTRUE, GFXFALSE, 0, 1000000);         //Restore normal frustrum
    }
    GFXHudMode(true);
    const float damage_flash_length = configuration().graphics.damage_flash_length_flt;
    const bool damage_flash_first = configuration().graphics.flash_behind_hud;
    if (view < CP_CHASE && damage_flash_first && getNewTime() - shake_time < damage_flash_length) {
        DrawDamageFlash(shake_type);
    }
    GFXColor4f(1, 1, 1, 1);
    GFXBlendMode(ONE, ONE);
    GFXDisable(DEPTHTEST);
    GFXDisable(DEPTHWRITE);

    Unit *un;
    float crosscenx = 0, crossceny = 0;
    const bool crosshairs_on_chasecam = configuration().graphics.hud.crosshairs_on_chase_cam;
    const bool crosshairs_on_padlock = configuration().graphics.hud.crosshairs_on_padlock;
    if ((view == CP_FRONT)
            || (view == CP_CHASE && crosshairs_on_chasecam)
            || ((view == CP_VIEWTARGET || view == CP_PANINSIDE) && crosshairs_on_padlock)) {
        if (Panel.size() > 0 && Panel.front() && screenshotkey == false) {
            const bool drawCrosshairs = configuration().graphics.hud.draw_rendered_crosshairs;
            if (drawCrosshairs) {
                float x, y, wid, hei;
                Panel.front()->GetPosition(x, y);
                Panel.front()->GetSize(wid, hei);
                DrawCrosshairs(x, y, wid, hei, textcol);
            } else if (!draw_heading_marker) {
                GFXBlendMode(SRCALPHA, INVSRCALPHA);
                GFXEnable(TEXTURE0);
                Panel.front()->Draw();                 //draw crosshairs
            }
        }
    }
    GFXBlendMode(SRCALPHA, INVSRCALPHA);
    GFXEnable(TEXTURE0);

    RestoreViewPort();

    const bool blend_panels = configuration().graphics.blend_panels;
    const bool blend_cockpit = configuration().graphics.blend_cockpit;
    const bool drawChaseVDU = configuration().graphics.draw_vdus_from_chase_cam;
    const bool drawPanVDU = configuration().graphics.draw_vdus_from_panning_cam;
    const bool drawTgtVDU = configuration().graphics.draw_vdus_from_target_cam;
    const bool drawPadVDU = configuration().graphics.draw_vdus_from_padlock_cam;

    const bool drawChasecp = configuration().graphics.draw_cockpit_from_chase_cam;
    const bool drawPancp = configuration().graphics.draw_cockpit_from_panning_cam;
    const bool drawTgtcp = configuration().graphics.draw_cockpit_from_target_cam;
    const bool drawPadcp = configuration().graphics.draw_cockpit_from_padlock_cam;

    const float AlphaTestingCutoff = configuration().graphics.stars_alpha_test_cutoff_flt;
    if (blend_cockpit) {
        GFXAlphaTest(ALWAYS, 0);
        GFXBlendMode(SRCALPHA, INVSRCALPHA);
    } else {
        GFXBlendMode(ONE, ZERO);
        GFXAlphaTest(GREATER, AlphaTestingCutoff);
    }
    GFXColor4f(1, 1, 1, 1);
    if (view < CP_CHASE) {
        if (Pit[view]) {
            Pit[view]->Draw();
        }
    } else if ((view == CP_CHASE
            && drawChasecp)
            || (view == CP_PAN && drawPancp) || (view == CP_TARGET && drawTgtcp)
            || ((view == CP_VIEWTARGET || view == CP_PANINSIDE) && drawPadcp)) {
        if (Pit[0]) {
            Pit[0]->Draw();
        }
    }
    if (blend_panels) {
        GFXAlphaTest(ALWAYS, 0);
        GFXBlendMode(SRCALPHA, INVSRCALPHA);
    } else {
        GFXBlendMode(ONE, ZERO);
        GFXAlphaTest(GREATER, AlphaTestingCutoff);
    }
    GFXColor4f(1, 1, 1, 1);
    if (view == CP_FRONT
            || (view == CP_CHASE
                    && drawChaseVDU)
            || (view == CP_PAN && drawPanVDU) || (view == CP_TARGET && drawTgtVDU)
            || ((view == CP_VIEWTARGET || view == CP_PANINSIDE) && drawPadVDU)) {
        for (unsigned int j = 1; j < Panel.size(); j++) {
            if (Panel[j]) {
                Panel[j]->Draw();
            }
        }
    }
    GFXAlphaTest(ALWAYS, 0);
    GFXBlendMode(SRCALPHA, INVSRCALPHA);
    GFXColor4f(1, 1, 1, 1);
    bool die = true;
    //draw target gauges
    for (unsigned int vd = 0; vd < vdu.size(); vd++) {
        if (vdu[vd]->getMode() == VDU::TARGET) {
            if ((un = parent.GetUnit())) {
                Unit *target = parent.GetUnit()->Target();
                if (target != nullptr) {
                    if (view == CP_FRONT
                            || (view == CP_CHASE
                                    && drawChaseVDU)
                            || (view == CP_PAN
                                    && drawPanVDU) || (view == CP_TARGET && drawTgtVDU)
                            || ((view == CP_VIEWTARGET || view == CP_PANINSIDE)
                                    && drawPadVDU)) {                                                                //{ //only draw crosshairs for front view
                        //if (!UnitUtil::isSignificant(target)&&!UnitUtil::isSun(target)||UnitUtil::isCapitalShip(target)) //{
                        DrawTargetGauges(target, gauges);
                    }
                }
            }
        }
    }
    //draw unit gauges
    if ((un = parent.GetUnit())) {
        switch (view) {
            case CP_FRONT:
            case CP_LEFT:
            case CP_RIGHT:
            case CP_BACK:
            case CP_VIEWTARGET:
            case CP_PANINSIDE:
                TriggerEvents(un);
                break;
            default:
                break;
        };
        if (view == CP_FRONT
                || (view == CP_CHASE
                        && drawChaseVDU)
                || (view == CP_PAN && drawPanVDU) || (view == CP_TARGET && drawTgtVDU)
                || ((view == CP_VIEWTARGET || view == CP_PANINSIDE) && drawPadVDU)) {
            //only draw crosshairs for front view
            DrawGauges(this, un, gauges, gauge_time, cockpit_time, text, textcol);
            Radar::Sensor sensor(un);
            DrawRadar(sensor, cockpit_time, radar_time, radarSprites, radarDisplay.get());

            GFXColor4f(1, 1, 1, 1);
            for (unsigned int vd = 0; vd < vdu.size(); vd++) {
                if (vdu[vd]) {
                    vdu[vd]->Draw(this, un, textcol);
                    GFXColor4f(1, 1, 1, 1);
                    float damage = un->ship_functions.Value(Function::cockpit);
                    if (vdu[vd]->staticable()) {
                        if (damage < .985) {
                            if (vdu_time[vd] >= 0) {
                                if (damage > .001 && (cockpit_time > (vdu_time[vd] + (1 - damage)))) {
                                    if (randomDouble() > SWITCH_CONST) {
                                        vdu_time[vd] = -cockpit_time;
                                    }
                                }
                            } else if (cockpit_time > ((1 - (-vdu_time[vd])) + (damage))) {
                                if (randomDouble() > SWITCH_CONST) {
                                    vdu_time[vd] = cockpit_time;
                                }
                            }
                        }
                    }
                }
            }
            //process VDU, damage VDU, targetting VDU
            //////////////////// DISPLAY CURRENT POSITION ////////////////////
            if (configuration().graphics.hud.debug_position) {
                TextPlane tp;
                std::string str;
                Unit *you = parent.GetUnit();
                if (you) {
                    str = (boost::format("Your Position: (%1%,%2%,%3%); Velocity: (%4%,%5%,%6%); Frame: %7%\n")
                            % you->curr_physical_state.position.i
                            % you->curr_physical_state.position.j
                            % you->curr_physical_state.position.k
                            % you->Velocity.i
                            % you->Velocity.j
                            % you->Velocity.k
                            % getNewTime()).str();
                    Unit *yourtarg = you->Target();
                    if (yourtarg) {
                        str += (boost::format("Target Position: (%1%,%2%,%3%); Velocity: (%4%,%5%,%6%); Now: %7%\n")
                                % yourtarg->curr_physical_state.position.i
                                % yourtarg->curr_physical_state.position.j
                                % yourtarg->curr_physical_state.position.k
                                % yourtarg->Velocity.i
                                % yourtarg->Velocity.j
                                % yourtarg->Velocity.k
                                % queryTime()).str();
                    }
                }
                tp.SetPos(-0.8, -0.8);
                tp.SetText(str);
                tp.Draw();
            }
            //////////////////////////////////////////////////////////////////////////
        }
        GFXColor4f(1, 1, 1, 1);
        if (!un->Destroyed()) {
            die = false;
        }
        if (un->Threat() != nullptr) {
            if (0 && getTimeCompression() > 1) {
                reset_time_compression(std::string(), PRESS);
            }
            un->Threaten(nullptr, 0);
        }
        if (_Universe->CurrentCockpit() < univmap.size()) {
            univmap[_Universe->CurrentCockpit()].Draw();
        }
        //Draw the arrow to the target.
        {
            Unit *parent = nullptr;
            if (configuration().graphics.hud.draw_arrow_to_target && ((parent = this->parent.GetUnit()))) {
                Radar::Sensor sensor(parent);
                if ((view == CP_PAN
                        && !configuration().graphics.hud.draw_arrow_on_pan_cam)
                        || (view == CP_PANTARGET
                                && !configuration().graphics.hud.draw_arrow_on_pan_target) || (view == CP_CHASE && !configuration()
                        .graphics
                        .hud
                        .draw_arrow_on_chase_cam)) {
                } else {
                    DrawArrowToTarget(sensor, parent->Target(), projection_limit_x, projection_limit_y,
                                      inv_screen_aspect_ratio);
                    if (draw_star_destination_arrow
                            && (destination_system_location.i || destination_system_location.j
                                    || destination_system_location.k)) {
                        GFXColorf(destination_system_color);
                        DrawArrowToTarget(sensor, parent->ToLocalCoordinates(destination_system_location),
                                          projection_limit_x, projection_limit_y,
                                          inv_screen_aspect_ratio);
                    }
                }
            }
        }         //end: draw arrow
    }
    AutoLanding();
    GFXColor4f(1, 1, 1, 1);
    if (QuitAllow || getTimeCompression() < .5) {
        if (QuitAllow) {
            if (!die) {
                static VSSprite QuitSprite("quit.sprite", BILINEAR, GFXTRUE);
                static VSSprite QuitCompatSprite("quit.spr", BILINEAR, GFXTRUE);

                GFXEnable(TEXTURE0);
                if (QuitSprite.LoadSuccess()) {
                    QuitSprite.Draw();
                } else {
                    QuitCompatSprite.Draw();
                }
            }
        } else {
            static VSSprite PauseSprite("pause.sprite", BILINEAR, GFXTRUE);
            static VSSprite PauseCompatSprite("pause.spr", BILINEAR, GFXTRUE);
            GFXEnable(TEXTURE0);
            if (PauseSprite.LoadSuccess()) {
                PauseSprite.Draw();
            } else {
                PauseCompatSprite.Draw();
            }
        }
    }
    static float dietime = 0;
    if (die) {
        if (un) {
            if (!un->Destroyed()) {
                die = false;
            }
        }
        if (die) {
            if (text) {
                GFXColor4f(1, 1, 1, 1);
                text->SetSize(1, -1);
                float x;
                float y;
                if (dietime == 0) {
                    editingTextMessage = false;
                    RestoreKB();
                    if (respawnunit.size() > _Universe->CurrentCockpit()) {
                        if (respawnunit[_Universe->CurrentCockpit()] == 1 && _Universe->numPlayers() == 1) {
                            respawnunit[_Universe->CurrentCockpit()] = 0;
                        }
                    }
                    text->GetCharSize(x, y);
                    text->SetCharSize(x * 4, y * 4);
                    text->SetPos(0 - (x * 2 * 14), 0 - (y * 2));
                }
                GFXColorf(textcol);
                const bool show_died_text = configuration().graphics.show_respawn_text;
                if (show_died_text) {
                    text->Draw(
                            "#ff5555You Have Died!\n#000000Press #8080FF;#000000 (semicolon) to respawn\nOr Press #8080FFEsc and 'q'#000000 to quit");
                }
                GFXColor4f(1, 1, 1, 1);

                const float min_die_time = configuration().graphics.death_scene_time_flt;
                if (dietime > min_die_time) {
                    const std::string& death_menu_script = configuration().graphics.death_menu_script;
                    if (death_menu_script.empty()) {
                        static VSSprite DieSprite("died.sprite", BILINEAR, GFXTRUE);
                        static VSSprite DieCompatSprite("died.spr", BILINEAR, GFXTRUE);
                        GFXBlendMode(SRCALPHA, INVSRCALPHA);
                        GFXEnable(TEXTURE0);
                        if (DieSprite.LoadSuccess()) {
                            DieSprite.Draw();
                        } else {
                            DieCompatSprite.Draw();
                        }
                    } else {
                        BaseUtil::LoadBaseInterface(death_menu_script);
                        dietime = 0;
                    }
                }
                dietime += GetElapsedTime();
                SetView(CP_PAN);
                CockpitKeys::YawLeft(std::string(), RELEASE);
                CockpitKeys::YawRight(std::string(), RELEASE);
                CockpitKeys::PitchUp(std::string(), RELEASE);
                CockpitKeys::PitchDown(std::string(), RELEASE);
                zoomfactor = dietime * 10;
            }
            QuitAllow = true;
        } else {
        }
    } else if (dietime != 0) {
        QuitAllow = false;
        dietime = 0;
    }
    //if(CommandInterpretor.console){
    //GFXColorf(textcol);
    //CommandInterpretor.renderconsole();
    //}
    GFXAlphaTest(ALWAYS, 0);
    const bool mouseCursor = configuration().joystick.mouse_cursor;
    const bool mousecursor_pancam = configuration().joystick.mouse_cursor_pancam;
    const bool mousecursor_pantgt = configuration().joystick.mouse_cursor_pantgt;
    const bool mousecursor_chasecam = configuration().joystick.mouse_cursor_chasecam;
    if (mouseCursor && screenshotkey == false) {
        if ((view == CP_PAN
                && !mousecursor_pancam)
                || (view == CP_PANTARGET && !mousecursor_pantgt) || (view == CP_CHASE && !mousecursor_chasecam)) {
        } 
    }
    if (view < CP_CHASE && damage_flash_first == false && getNewTime() - shake_time < damage_flash_length) {
        DrawDamageFlash(shake_type);
    }
    GFXHudMode(false);

    {
        //again, NAV computer is unaffected by FOV WARP-Link
        float oldfov = AccessCamera()->GetFov();
        AccessCamera()->SetFov(configuration().graphics.fov_flt);
        AccessCamera()->UpdateGFXAgain();
        DrawNavSystem(&ThisNav, AccessCamera(), cockpit_offset);
        AccessCamera()->SetFov(oldfov);
        AccessCamera()->UpdateGFXAgain();
    }

    GFXEnable(DEPTHWRITE);
    GFXEnable(DEPTHTEST);
    GFXEnable(TEXTURE0);
}

int GameCockpit::getScrollOffset(unsigned int whichtype) {
    for (unsigned int i = 0; i < vdu.size(); i++) {
        if (vdu[i]->getMode() & whichtype) {
            return vdu[i]->scrolloffset;
        }
    }
    return 0;
}

string GameCockpit::getsoundending(int which) {
    static bool gotten = false;
    static string strs[9];
    if (gotten == false) {
        char tmpstr[2] = {'\0'};
        for (int i = 0; i < 9; i++) {
            tmpstr[0] = i + '1';
            string vsconfigvar = string("sounds_extension_") + tmpstr;
            strs[i] = vs_config->getVariable("cockpitaudio", vsconfigvar, "\n");
            if (strs[i] == "\n") {
                strs[i] = "";
                break;
            }
        }
        gotten = true;
    }
    return strs[which];
}

#include <algorithm>

string GameCockpit::getsoundfile(string sound) {
    bool ok = false;
    int i;
    string lastsound = "";
    string anothertmpstr = "";
    for (i = 0; i < 9 && !ok; i++) {
        anothertmpstr = getsoundending(i);
        bool foundyet = false;
        while (1) {
            std::string::iterator found = std::find(anothertmpstr.begin(), anothertmpstr.end(), '*');
            if (found != anothertmpstr.end()) {
                anothertmpstr.erase(found);
                anothertmpstr.insert((found - anothertmpstr.begin()), sound);
                foundyet = true;
            } else {
                if (!foundyet) {
                    anothertmpstr = sound + anothertmpstr;
                }
                break;
            }
        }
        if (VSFileSystem::LookForFile(anothertmpstr, SoundFile) < VSFileSystem::Ok) {
            ok = true;
        }
    }
    if (ok) {
        //return lastsound;
        return anothertmpstr;
    } else {
        return "";
    }
}

void SetStartupView(Cockpit *);

void GameCockpit::UpdAutoPilot() {
    const bool autopan = configuration().graphics.pan_on_auto;
    if (autopilot_time != 0) {
        autopilot_time -= SIMULATION_ATOM;
        {
            if (autopan) {
                Vector origR = Vector(0, 0, 1);
                Vector origP = Vector(1, 0, 0);

                const float rotspd = configuration().graphics.autopilot_rotation_speed_flt;

                static float curtime = 0;
                curtime += SIMULATION_ATOM;
                float ang = curtime * rotspd;
                origR.Yaw(ang);
                origP.Yaw(ang);
                Vector origQ = Vector(0, 1, 0);
                origP.Normalize();
                origQ.Normalize();
                origR.Normalize();
                AccessCamera(CP_FIXED)->myPhysics.SetAngularVelocity(Vector(0, 0, 0));                 //hack
                const float initialzoom = configuration().graphics.initial_zoom_factor_flt;
                zoomfactor = initialzoom;
            }
        }
        if (autopilot_time <= 0) {
            AccessCamera(CP_FIXED)->myPhysics.SetAngularVelocity(Vector(0, 0, 0));
            if (disableautosound.sound < 0) {
                static string str = configuration().cockpit_audio.autopilot_disabled;
                disableautosound.loadsound(str);
            }
            disableautosound.playsound();
            if (autopan) {
                AccessCamera(CP_FIXED)->myPhysics.SetAngularVelocity(Vector(0, 0, 0));
                SetStartupView(this);
            }
            autopilot_time = 0;
            Unit *par = GetParent();
            if (par) {
                Unit *autoun = autopilot_target.GetUnit();
                autopilot_target.SetUnit(NULL);
                if (autoun && autopan) {
                    par->AutoPilotTo(autoun, false);
                }
            }
        }
    }
}

void SwitchUnits2(Unit *nw) {
    if (nw) {
        nw->PrimeOrders();
        nw->EnqueueAI(new FireKeyboard(_Universe->CurrentCockpit(), _Universe->CurrentCockpit()));
        nw->EnqueueAI(new FlyByJoystick(_Universe->CurrentCockpit()));

        nw->SetTurretAI();
        nw->DisableTurretAI();

        const bool LoadNewCockpit = configuration().graphics.unit_switch_cockpit_change;
        const bool DisCockpit = configuration().graphics.switch_cockpit_to_default_on_unit_switch;
        if (nw->getCockpit().length() > 0 || DisCockpit) {
            _Universe->AccessCockpit()->Init(nw->getCockpit().c_str(), LoadNewCockpit == false);
        }
        /* Here is the old code:
         *
         *  if (nw->getCockpit().length()>0&&LoadNewCockpit) {
         *  _Universe->AccessCockpit()->Init (nw->getCockpit().c_str());
         *  }else {
         *  if (DisCockpit) {
         *  _Universe->AccessCockpit()->Init ("disabled-cockpit.cpt");
         *  }
         *  }
         *
         */
    }
}

GameCockpit::~GameCockpit() {
    GameCockpit::Delete();
    for (int i = 0; i < 4; i++) {
        if (Pit[i]) {
            delete Pit[i];
            Pit[i] = nullptr;
        }
    }
    delete savegame;
}

int GameCockpit::getVDUMode(int vdunum) {
    if (vdunum < (int) vdu.size()) {
        if (vdu[vdunum]) {
            return vdu[vdunum]->getMode();
        }
    }
    return 0;
}

void GameCockpit::VDUSwitch(int vdunum) {
    if (soundfile >= 0) {
        //AUDPlay (soundfile, AccessCamera()->GetPosition(), Vector (0,0,0), .5);
        AUDPlay(soundfile, QVector(0, 0, 0), Vector(0, 0, 0), 1);
    }
    if (vdunum < (int) vdu.size()) {
        if (vdu[vdunum]) {
            vdu[vdunum]->SwitchMode(this->parent.GetUnit());
        }
    }
}

void GameCockpit::ScrollVDU(int vdunum, int howmuch) {
    if (soundfile >= 0) {
        //AUDPlay (soundfile, AccessCamera()->GetPosition(), Vector (0,0,0),.5);
        AUDPlay(soundfile, QVector(0, 0, 0), Vector(0, 0, 0), 1);
    }
    if (vdunum < (int) vdu.size()) {
        if (vdu[vdunum]) {
            vdu[vdunum]->Scroll(howmuch);
        }
    }
}

void GameCockpit::ScrollAllVDU(int howmuch) {
    if (ThisNav.CheckDraw()) {
        ThisNav.scroll(howmuch);
    } else {
        for (unsigned int i = 0; i < vdu.size(); i++) {
            ScrollVDU(i, howmuch);
        }
    }
}

void GameCockpit::SetStaticAnimation() {
    const string comm_static = configuration().graphics.comm_static;
    static Animation Statuc(comm_static.c_str());
    for (unsigned int i = 0; i < vdu.size(); i++) {
        if (vdu[i]->getMode() == VDU::COMM) {
            vdu[i]->SetCommAnimation(&Statuc, NULL, true);
        }
    }
}

void GameCockpit::SetCommAnimation(Animation *ani, Unit *un) {
    bool seti = false;
    for (unsigned int i = 0; i < vdu.size(); i++) {
        if (vdu[i]->SetCommAnimation(ani, un, false)) {
            seti = true;
            break;
        }
    }
    if (!seti) {
        for (unsigned int i = 0; i < vdu.size(); i++) {
            if (vdu[i]->SetCommAnimation(ani, un, true)) {
                break;
            }
        }
    }
}

string GameCockpit::getTargetLabel() {
    Unit *par = GetParent();
    if ((!targetLabel.empty())
            && (!par || ((void *) par->Target()) != labeledTargetUnit)) {
        targetLabel = string();
        if (par) {
            labeledTargetUnit = par->Target();
        }
    }
    return targetLabel;
}

void GameCockpit::setTargetLabel(const string &newLabel) {
    if (GetParent()) {
        targetLabel = newLabel;
        labeledTargetUnit = GetParent()->Target();
    }
}

void GameCockpit::RestoreViewPort() {
    _Universe->AccessCamera()->RestoreViewPort(0, 0);
}

static void FaceCamTarget(Cockpit *cp, int cam, Unit *un) {
    QVector diff = un->Position() - cp->AccessCamera()->GetPosition();
    diff.Normalize();
    if (diff.i != 0 && diff.k != 0) {
        Vector z = diff.Cross(QVector(0, 1, 0)).Cast();
        cp->AccessCamera(cam)->SetOrientation(z, Vector(0, 1, 0), diff.Cast());
    }
}

static void ShoveCamBehindUnit(int cam, Unit *un, float zoomfactor) {
    //commented out by chuck_starchaser; --never used
    QVector unpos = (/*un->GetPlanetOrbit() && !un->isSubUnit()*/ NULL) ? un->LocalPosition() : un->Position();
    _Universe->AccessCamera(cam)->SetPosition(
            unpos - _Universe->AccessCamera()->GetR().Cast() * (un->rSize() + configuration().graphics.znear_dbl * 2) * zoomfactor,
            un->GetWarpVelocity(), un->GetAngularVelocity(), un->GetAcceleration());
}

static void ShoveCamBelowUnit(int cam, Unit *un, float zoomfactor) {
    //commented out by chuck_starchaser; --never used
    QVector unpos = un->Position();
    Vector p, q, r;
    _Universe->AccessCamera(cam)->GetOrientation(p, q, r);
    const float ammttoshovecam = configuration().graphics.shove_camera_down_flt;
    _Universe->AccessCamera(cam)->SetPosition(
            unpos - (r - ammttoshovecam * q).Cast() * (un->rSize() + configuration().graphics.znear_dbl * 2) * zoomfactor,
            un->GetWarpVelocity(),
            un->GetAngularVelocity(),
            un->GetAcceleration());
}

static Vector lerp(const Vector &a, const Vector &b, float t) {
    t = min(1.0f, max(0.0f, t));
    return a * (1 - t) + b * t;
}

static void translate_as(Vector &p,
        Vector &q,
        Vector &r,
        Vector p1,
        Vector q1,
        Vector r1,
        Vector p2,
        Vector q2,
        Vector r2) {
    //Translate p,q,r to <p1,q1,r1> base
    p = Vector(p.Dot(p1), p.Dot(q1), p.Dot(r1));
    q = Vector(q.Dot(p1), q.Dot(q1), q.Dot(r1));
    r = Vector(r.Dot(p1), r.Dot(q1), r.Dot(r1));
    //Interpret now as if it were in <p2,q2,r2> base
    p = p2 * p.i + q2 * p.j + r2 * p.k;
    q = p2 * q.i + q2 * q.j + r2 * q.k;
    r = p2 * r.i + q2 * r.j + r2 * r.k;
}

void GameCockpit::SetupViewPort(bool clip) {
    _Universe->AccessCamera()->RestoreViewPort(0, (view == CP_FRONT ? viewport_offset : 0));
    GFXViewPort(0,
            (int) ((view == CP_FRONT ? viewport_offset : 0) * configuration().graphics.resolution_y),
            configuration().graphics.resolution_x,
            configuration().graphics.resolution_y);
    _Universe->AccessCamera()->setCockpitOffset(view < CP_CHASE ? cockpit_offset : 0);
    Unit *un, *tgt;
    if ((un = parent.GetUnit())) {
        //Previous frontal orientation - useful, sometimes...
        Vector prev_fp, prev_fq, prev_fr;
        _Universe->AccessCamera(CP_FRONT)->GetOrientation(prev_fp, prev_fq, prev_fr);

        un->UpdateHudMatrix(CP_FRONT);
        un->UpdateHudMatrix(CP_LEFT);
        un->UpdateHudMatrix(CP_RIGHT);
        un->UpdateHudMatrix(CP_BACK);
        un->UpdateHudMatrix(CP_CHASE);
        un->UpdateHudMatrix(CP_PANINSIDE);

        insidePanYaw += insidePanYawSpeed * GetElapsedTime();
        insidePanPitch += insidePanPitchSpeed * GetElapsedTime();

        Vector p, q, r, tmp;
        _Universe->AccessCamera(CP_FRONT)->GetOrientation(p, q, r);
        _Universe->AccessCamera(CP_LEFT)->SetOrientation(r, q, -p);
        _Universe->AccessCamera(CP_RIGHT)->SetOrientation(-r, q, p);
        _Universe->AccessCamera(CP_BACK)->SetOrientation(-p, q, -r);
#ifdef IWANTTOPVIEW
        _Universe->AccessCamera( CP_CHASE )->SetOrientation( p, r, -q );
#endif

        Matrix pitchMatrix, yawMatrix, panMatrix;
        RotateAxisAngle(pitchMatrix, Vector(1, 0, 0), insidePanPitch);
        RotateAxisAngle(yawMatrix, Transform(pitchMatrix, Vector(0, 1, 0)), insidePanYaw);
        panMatrix = yawMatrix * pitchMatrix;
        _Universe->AccessCamera(CP_PANINSIDE)->SetOrientation(
                Transform(panMatrix, p),
                Transform(panMatrix, q),
                Transform(panMatrix, r));

        tgt = un->Target();
        if (tgt) {
            un->GetOrientation(p, q, r);
            r = (tgt->Position() - un->Position()).Cast();
            r.Normalize();
            CrossProduct(r, q, tmp);
            CrossProduct(tmp, r, q);
            //Padlock block
            if (view == CP_VIEWTARGET) {
                const float padlock_view_lag = configuration().graphics.hud.padlock_view_lag_flt;
                const float padlock_view_lag_inv = 1.0F / padlock_view_lag;
                const float padlock_view_lag_fix = configuration().graphics.hud.padlock_view_lag_fix_zone_flt;
                const float padlock_view_lag_fix_cos = (float) cos(padlock_view_lag_fix);

                //pp,qq,rr <-- world-relative padlock target
                //p_p,p_q,p_r <-- previous head orientation translated to new front orientation
                Vector p_p, p_q, p_r, f_p, f_q, f_r, pp = tmp, qq = q, rr = r;
                _Universe->AccessCamera(CP_VIEWTARGET)->GetOrientation(p_p, p_q, p_r);
                _Universe->AccessCamera(CP_FRONT)->GetOrientation(f_p, f_q, f_r);
                translate_as(p_p, p_q, p_r, prev_fp, prev_fq, prev_fr, f_p, f_q, f_r);

                //Compute correction amount (vtphase), accounting for lag and fix-zone
                un->UpdateHudMatrix(CP_VIEWTARGET);
                bool fixzone = (rr.Dot(p_r) >= padlock_view_lag_fix_cos) && (qq.Dot(p_q) >= padlock_view_lag_fix_cos);
                float vtphase =
                        1.0f - (float) std::pow(0.1, GetElapsedTime() * padlock_view_lag_inv * (fixzone ? 0.1f : 1.0f));

                //Apply correction
                _Universe->AccessCamera(CP_VIEWTARGET)->SetOrientation(
                        lerp(p_p, pp, vtphase).Normalize(),
                        lerp(p_q, qq, vtphase).Normalize(),
                        lerp(p_r, rr, vtphase).Normalize());
            } else {
                //Reset padlock matrix
                un->UpdateHudMatrix(CP_VIEWTARGET);
            }
            _Universe->AccessCamera(CP_TARGET)->SetOrientation(tmp, q, r);
            //_Universe->AccessCamera(CP_PANTARGET)->SetOrientation(tmp,q,r);
            ShoveCamBelowUnit(CP_TARGET, un, zoomfactor);
            ShoveCamBehindUnit(CP_PANTARGET, tgt, zoomfactor);
            FaceCamTarget(this, CP_FIXEDPOSTARGET, tgt);
        } else {
            un->UpdateHudMatrix(CP_VIEWTARGET);
            un->UpdateHudMatrix(CP_TARGET);
            un->UpdateHudMatrix(CP_PANTARGET);
        }
        if (view == CP_CHASE && !configuration().graphics.hud.draw_unit_on_chase_cam) {
        } else {
            ShoveCamBelowUnit(CP_CHASE, un, zoomfactor);
            //ShoveCamBehindUnit (CP_PANTARGET,un,zoomfactor);
        }
        FaceCamTarget(this, CP_FIXEDPOS, un);

        ShoveCamBehindUnit(CP_PAN, un, zoomfactor);
        un->SetVisible(view >= CP_CHASE);

        //WARP-FOV link
        {
            const float stable_lowarpref = configuration().warp.fov_link.stable.low_ref_flt; /* default: 1 */
            float stable_hiwarpref = configuration().warp.fov_link.stable.high_ref_flt; /* default: 100000 */
            const float stable_refexp = configuration().warp.fov_link.stable.exp_flt; /* default: 0.5 */
            const bool stable_asymptotic = configuration().warp.fov_link.stable.asymptotic; /* default: 1 */
            const float stable_offset_f = configuration().warp.fov_link.stable.offset.front_flt; /* default: 0 */
            const float stable_offset_b = configuration().warp.fov_link.stable.offset.back_flt; /* default: 0 */
            const float stable_offset_p = configuration().warp.fov_link.stable.offset.perpendicular_flt; /* default: 0 */
            const float stable_multiplier_f = configuration().warp.fov_link.stable.multiplier.front_flt; /* default: 0.85 */
            const float stable_multiplier_b = configuration().warp.fov_link.stable.multiplier.back_flt; /* default: 1.5 */
            const float stable_multiplier_p = configuration().warp.fov_link.stable.multiplier.perpendicular_flt; /* default: 1.25 */

            const float shake_lowarpref = configuration().warp.fov_link.shake.low_ref_flt; /* default: 10000 */
            float shake_hiwarpref = configuration().warp.fov_link.shake.high_ref_flt; /* default: 200000 */
            const float shake_refexp = configuration().warp.fov_link.shake.exp_flt; /* default: 1.5 */
            const bool shake_asymptotic = configuration().warp.fov_link.shake.asymptotic; /* default: 1 */
            const float shake_speed = configuration().warp.fov_link.shake.speed_flt; /* default: 10 */
            const float shake_offset_f = configuration().warp.fov_link.shake.offset.front_flt; /* default: 0 */
            const float shake_offset_b = configuration().warp.fov_link.shake.offset.back_flt; /* default: 0 */
            const float shake_offset_p = configuration().warp.fov_link.shake.offset.perpendicular_flt; /* default: 0 */
            const float shake_multiplier_f = configuration().warp.fov_link.shake.multiplier.front_flt; /* default: 0 */
            const float shake_multiplier_b = configuration().warp.fov_link.shake.multiplier.back_flt; /* default: 0 */
            const float shake_multiplier_p = configuration().warp.fov_link.shake.multiplier.perpendicular_flt; /* default: 0 */

            //0 means automatic
            const float refkpsoverride = configuration().warp.fov_link.reference_kps_flt; /* default: 0 */

            static float theta = 0;
            theta += shake_speed * GetElapsedTime();
            if (stable_lowarpref == stable_hiwarpref) {
                stable_hiwarpref = stable_lowarpref + 1;
            }
            if (shake_lowarpref == shake_hiwarpref) {
                shake_hiwarpref = shake_lowarpref + 1;
            }
            float warpfieldstrength = LookupUnitStat(UnitImages<void>::WARPFIELDSTRENGTH, un);
            float refkps = (refkpsoverride > 0) ? refkpsoverride : LookupUnitStat(UnitImages<void>::MAXCOMBATABKPS,
                    un);               //This one is stable, as opposed to SETKPS - for full stability, use the override (user override of governor settings will create weird behaviour if done under SPEC)
            float kps = LookupUnitStat(UnitImages<void>::KPS, un);
            float st_warpfieldstrength =
                    std::pow((max(stable_lowarpref,
                                    min(stable_asymptotic ? FLT_MAX : stable_hiwarpref,
                                            warpfieldstrength)) - stable_lowarpref) / (stable_hiwarpref - stable_lowarpref),
                            stable_refexp);
            float sh_warpfieldstrength =
                    std::pow((max(shake_lowarpref, min(shake_asymptotic ? FLT_MAX : shake_hiwarpref,
                                    warpfieldstrength)) - shake_lowarpref)
                                    / (shake_hiwarpref - shake_lowarpref),
                            shake_refexp);
            float costheta = cos(theta);
            if (stable_asymptotic) {
                st_warpfieldstrength = atan(st_warpfieldstrength);
            }
            if (shake_asymptotic) {
                sh_warpfieldstrength = atan(sh_warpfieldstrength);
            }
            if (refkps <= 1) {
                refkps = 1;
            }
            if (kps > refkps) {
                kps = refkps;
            }
            float unv = un->GetVelocity().Magnitude();
            float camv = _Universe->AccessCamera()->GetR().Magnitude();
            if (unv <= 1) {
                unv = 1;
            }
            if (camv <= 1) {
                camv = 1;
            }
            float cosangle = (un->GetVelocity() * _Universe->AccessCamera()->GetR()) / (unv * camv) * (kps / refkps);
            float st_offs, sh_offs, st_mult, sh_mult;
            if (cosangle > 0) {
                st_offs = stable_offset_f * cosangle + stable_offset_p * (1 - cosangle);
                sh_offs = shake_offset_f * cosangle + shake_offset_p * (1 - cosangle);
                st_mult = stable_multiplier_f * cosangle + stable_multiplier_p * (1 - cosangle);
                sh_mult = shake_multiplier_f * cosangle + shake_multiplier_p * (1 - cosangle);
            } else {
                st_offs = stable_offset_b * -cosangle + stable_offset_p * (1 + cosangle);
                sh_offs = shake_offset_b * -cosangle + shake_offset_p * (1 + cosangle);
                st_mult = stable_multiplier_b * -cosangle + stable_multiplier_p * (1 + cosangle);
                sh_mult = shake_multiplier_b * -cosangle + shake_multiplier_p * (1 + cosangle);
            }
            st_offs *= st_warpfieldstrength;
            sh_offs *= sh_warpfieldstrength * costheta;
            st_mult = (1 - st_warpfieldstrength) + st_mult * st_warpfieldstrength;
            sh_mult *= sh_warpfieldstrength * costheta;
            const float fov_smoothing = configuration().warp.fov_link.smoothing_flt;
            float fov_smoot = std::pow(double(fov_smoothing), GetElapsedTime());
            smooth_fov =
                    min(170.0,
                            max(5.0,
                                    (1 - fov_smoot) * smooth_fov
                                            + fov_smoot * (configuration().graphics.fov_dbl * (st_mult + sh_mult) + st_offs + sh_offs)));
            _Universe->AccessCamera()->SetFov(smooth_fov);
        }
    }
    _Universe->AccessCamera()->UpdateGFX(clip ? GFXTRUE : GFXFALSE);

    //parent->UpdateHudMatrix();
}

void GameCockpit::SelectCamera(int cam) {
    if (cam < NUM_CAM && cam >= 0) {
        currentcamera = cam;
    }
}

Camera *GameCockpit::AccessCamera(int num) {
    if (num < NUM_CAM && num >= 0) {
        return &cam[num];
    } else {
        return NULL;
    }
}


bool GameCockpit::CheckCommAnimation(Unit *un) {
    for (unsigned int i = 0; i < vdu.size(); ++i) {
        if (vdu[i]->CheckCommAnimation(un)) {
            return true;
        }
    }
    return false;
}

bool GameCockpit::IsPaused() const {
    // stephengtuggy 2020-07-21 FIXME - I don't think this is correct
    return (GetElapsedTime() <= 0.001);
}

void GameCockpit::OnPauseBegin() {
    radarDisplay->OnPauseBegin();
}

void GameCockpit::OnPauseEnd() {
    radarDisplay->OnPauseEnd();
}

void GameCockpit::updateRadar(Unit *ship) {
    if (!ship) {
        return;
    }

    // We may have bought a new radar brand while docked, so the actual
    // radar display is instantiated when we undock.
    RadarType type = ship->radar.GetType();
    Radar::Type::Value displayType = Radar::Type::Value::NullDisplay;

    if(type == RadarType::BUBBLE) {
        displayType = Radar::Type::BubbleDisplay;
    } else if(type == RadarType::PLANE) {
        displayType = Radar::Type::PlaneDisplay;
    } else if(type == RadarType::SPHERE) {
        displayType = Radar::Type::SphereDisplay;
    }

    if(displayType != Radar::Type::Value::NullDisplay) {
        radarDisplay = Radar::Factory(displayType);
    }

    // Send notification that I have undocked
    radarDisplay->OnDockEnd();
}

void GameCockpit::SetParent(Unit *unit, const char *filename, const char *unitmodname, const QVector &startloc) {
    this->Cockpit::SetParent(unit, filename, unitmodname, startloc);
    updateRadar(unit);
}

void GameCockpit::OnDockEnd(Unit *station, Unit *ship) {
    if (ship->IsPlayerShip()) {
        updateRadar(ship);
    }
}

void GameCockpit::OnJumpBegin(Unit *ship) {
    if (ship->IsPlayerShip()) {
        radarDisplay->OnJumpBegin();
    }
}

void GameCockpit::OnJumpEnd(Unit *ship) {
    if (ship->IsPlayerShip()) {
        radarDisplay->OnJumpEnd();
    }
}

void GameCockpit::SetInsidePanYawSpeed(float speed) {
    insidePanYawSpeed = speed;
}

void GameCockpit::SetInsidePanPitchSpeed(float speed) {
    insidePanPitchSpeed = speed;
}

SoundContainer *GameCockpit::soundImpl(const SoundContainer &specs) {
    return new AldrvSoundContainer(specs);
}
