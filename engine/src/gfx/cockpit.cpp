// -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil -*-

/*
 * cockpit.cpp
 *
 * Copyright (C) Daniel Horn
 * Copyright (C) 2020 pyramid3d, Stephen G. Tuggy, and other Vega Strike contributors
 * Copyright (C) 2021 Stephen G. Tuggy
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

#include <boost/version.hpp>
#if BOOST_VERSION != 102800
#include <boost/python/object.hpp>
#include <boost/python/dict.hpp>
#else
#include <boost/python/objects.hpp>
#endif

#include "vsfilesystem.h"
#include "vs_logging.h"
#include "vs_globals.h"
#include "vegastrike.h"
#include "gauge.h"
#include "cockpit.h"
#include "universe.h"
#include "star_system.h"
#include "cmd/unit_generic.h"
#include "cmd/movable.h"
#include "cmd/collection.h"
#include "cmd/unit_util.h"
#include "cmd/unit_find.h" //for radar iteration.
#include "cmd/base_util.h"
#include "hud.h"
#include "vdu.h"
#include "lin_time.h" //for fps
#include "cmd/beam.h"
#include "config_xml.h"
#include "lin_time.h"
#include "cmd/images.h"
#include "cmd/script/mission.h"
#include "cmd/script/msgcenter.h"
#include "cmd/ai/flyjoystick.h"
#include "cmd/ai/firekeyboard.h"
#include "cmd/ai/aggressive.h"
#include "cmd/ai/autodocking.h"
#include "main_loop.h"
#include <assert.h>     //needed for assert() calls
#include "savegame.h"
#include "animation.h"
#include "mesh.h"
#include "universe_util.h"
#include "in_mouse.h"
#include "gui/glut_support.h"
#include "audiolib.h"
#include "save_util.h"
#include "cmd/base.h"
#include "in_kb_data.h"
#include "main_loop.h"
#include <set>
#include <string>
#include "cmd/unit_const_cache.h"
#include "options.h"
#include "soundcontainer_aldrv.h"
#include "configxml.h"
#include "planet.h"
#include "mount_size.h"
#include "weapon_info.h"
#include "cockpit/cockpit_audio.h"
#include "cockpit/cockpit_gfx.h"
#include "gfx/background.h"

using std::min;
using std::max;

extern float rand01();
using VSFileSystem::SoundFile;
using namespace VSFileSystem;

#define SWITCH_CONST (.9)
/* The smaller VERYNEAR_CONST is, the worse Z-Buffer precision will be. So keep this above 0.004) */
#define VERYNEAR_CONST (0.004f)
/*so that znear/zfar are not too close to max/min values, and account for off-center cockpits */
#define COCKPITZ_HEADROOM (1.01f)

static soundContainer disableautosound;
static soundContainer enableautosound;

vector< int >respawnunit;
vector< int >switchunit;
vector< int >turretcontrol;

static float getInitialZoomFactor()
{
    static float inizoom = XMLSupport::parse_float( vs_config->getVariable( "graphics", "inital_zoom_factor", "2.25" ) );
    return inizoom;
}

void visitSystemHelp( Cockpit *cp, string systemname, float num )
{
    string key( string( "visited_" )+systemname );
    vector< float > *v = &cp->savegame->getMissionData( key );
    if ( v->empty() )
        v->push_back( num );
    else if ( (*v)[0] != 1.0 && num == 1 )
        (*v)[0] = num;
}

#define sqr( x ) ( ( (x) )*( (x) ) )
std::string Cockpit::GetNavSelectedSystem()
{
    return AccessNavSystem()->getSelectedSystem();
}



void Cockpit::ReceivedTargetInfo()
{
    for (size_t j = 0; j < vdu.size(); j++)
        vdu[j]->ReceivedTargetData();
}

void Cockpit::SetSoundFile( string sound )
{
    soundfile = AUDCreateSoundWAV( sound, false );
}























void Cockpit::Eject()
{
    ejecting = true;
    going_to_dock_screen = false;
}

void Cockpit::EjectDock()
{
    ejecting = true;
    going_to_dock_screen = true;
}

void Cockpit::DoAutoLanding(Unit *un, Unit *target)
{
    if (!un || !target)
        return;
    if (UnitUtil::isDockableUnit( target ) == false)
        return;
    static std::set< std::string >autoLandingExcludeList;
    static std::set< std::string >autoLandingExcludeWarningList;
    static bool autoLandingExcludeList_initialised = false;
    if (!autoLandingExcludeList_initialised) {
        autoLandingExcludeList_initialised = true;
        std::string excludes;

        excludes = vs_config->getVariable( "physics", "AutoLandingExcludeList", "" );
        if ( !excludes.empty() ) {
            std::string::size_type pos = 0, epos = 0;
            while (epos != std::string::npos) {
                std::string xx (excludes.substr( pos, epos = excludes.find( ' ', pos ) ));
                autoLandingExcludeList.insert( xx );
                pos = epos+1;
            }
        }
        excludes = vs_config->getVariable( "physics", "AutoLandingExcludeWarningList", "" );
        if ( !excludes.empty() ) {
            std::string::size_type pos = 0, epos = 0;
            while (epos != std::string::npos) {
                std::string yy(excludes.substr( pos, epos = excludes.find( ' ', pos ) ));
                autoLandingExcludeWarningList.insert( yy );
                pos = epos+1;
            }
        }
    }
    std::string tname=target->name;
    if (autoLandingExcludeList.find(tname) != autoLandingExcludeList.end())
        return;
    static float lessthan   = XMLSupport::parse_float( vs_config->getVariable( "physics", "AutoLandingDockDistance", "50" ) );
    static float warnless   = XMLSupport::parse_float( vs_config->getVariable( "physics", "AutoLandingWarningDistance", "350" ) );
    static float AutoLandingMoveDistance =
        XMLSupport::parse_float( vs_config->getVariable( "physics", "AutoLandingMoveDistance", "50" ) );
    static float moveout    = XMLSupport::parse_float( vs_config->getVariable( "physics", "AutoLandingDisplaceDistance", "50" ) );
    static float autorad    = XMLSupport::parse_float( vs_config->getVariable( "physics", "unit_default_autodock_radius", "0" ) );
    static bool  adjust_unit_radius =
        XMLSupport::parse_float( vs_config->getVariable( "physics", "use_unit_autodock_radius", "false" ) );
    float        rsize      = target->isPlanet() ? target->rSize() : ( autorad+(adjust_unit_radius ? target->rSize() : 0) );
    QVector      diffvec    = un->Position()-target->Position();
    float        dist       = diffvec.Magnitude()-un->rSize()-rsize;
    diffvec.Normalize();

    static bool  haswarned  = false;
    static void *lastwarned = NULL;
    static float docktime   = -FLT_MAX;
    if (dist < lessthan && haswarned && lastwarned == target) {
        //CrashForceDock(target,un,true);
        un->SetPosAndCumPos( target->Position()+diffvec.Scale( un->rSize()+rsize+AutoLandingMoveDistance ) );
        FireKeyboard::DockKey( KBData(), PRESS );
        haswarned  = false;
        lastwarned = target;
        docktime   = getNewTime();
    } else if (haswarned == false && lastwarned == target) {
        if (getNewTime()-docktime > SIMULATION_ATOM*2) {
            haswarned  = false;
            un->SetPosAndCumPos( UniverseUtil::SafeEntrancePoint( target->Position()+diffvec*( rsize+moveout+un->rSize() ),
                                                                  un->rSize()*1.1 ) );
            lastwarned = NULL;
        }
    } else if (dist < warnless) {
        if (lastwarned != target || !haswarned) {
            if (autoLandingExcludeWarningList.count( target->name ) == 0) {
                static string str  = vs_config->getVariable( "cockpitaudio", "automatic_landing_zone", "als" );
                static string str1 = vs_config->getVariable( "cockpitaudio", "automatic_landing_zone1", "als" );
                static string str2 = vs_config->getVariable( "cockpitaudio", "automatic_landing_zone2", "als" );
                static string autolandinga  = vs_config->getVariable( "graphics",
                                                                      "automatic_landing_zone_warning",
                                                                      "comm_docking.ani" );
                static string autolandinga1 = vs_config->getVariable( "graphics",
                                                                      "automatic_landing_zone_warning1",
                                                                      "comm_docking.ani" );
                static string autolandinga2 = vs_config->getVariable( "graphics",
                                                                      "automatic_landing_zone_warning2",
                                                                      "comm_docking.ani" );
                static string message = vs_config->getVariable( "graphics",
                                                                "automatic_landing_zone_warning_text",
                                                                "Now Entering an \"Automatic Landing Zone\"." );
                UniverseUtil::IOmessage( 0, "game", "all", message );
                static Animation     *ani0 = new Animation( autolandinga.c_str() );
                static Animation     *ani1 = new Animation( autolandinga1.c_str() );
                static Animation     *ani2 = new Animation( autolandinga2.c_str() );
                static soundContainer warnsound;
                static soundContainer warnsound1;
                static soundContainer warnsound2;
                int num = rand() < RAND_MAX/2 ? 0 : (rand() < RAND_MAX/2 ? 1 : 2);
                if (warnsound.sound < 0) {
                    warnsound.loadsound( str );
                    warnsound1.loadsound( str1 );
                    warnsound2.loadsound( str2 );
                }
                switch (num)
                {
                case 0:
                    warnsound.playsound();
                    SetCommAnimation( ani0, target );
                    break;
                case 1:
                    warnsound1.playsound();
                    SetCommAnimation( ani1, target );
                    break;
                default:
                    warnsound2.playsound();
                    SetCommAnimation( ani2, target );
                    break;
                }
            }
            haswarned  = true;
            lastwarned = target;
        }
    } else if (lastwarned == target) {
        haswarned  = false;
        lastwarned = NULL;
    }
}

void Cockpit::AutoLanding()
{
    static bool autolanding_enable =
        XMLSupport::parse_bool( vs_config->getVariable( "physics", "AutoLandingEnable", "false" ) );
    if (autolanding_enable)
    {
        Unit *player = GetParent();
        if (player == NULL)
            return;

        CollideMap *collideMap = _Universe->activeStarSystem()->collide_map[Unit::UNIT_ONLY];
        for (CollideMap::iterator it = collideMap->begin(); it != collideMap->end(); ++it)
        {
            if (it->radius <= 0)
                continue;

            Unit *target = it->ref.unit;
            if (target == NULL)
                continue;

            DoAutoLanding(player, target);
        }
    }
}





float Cockpit::LookupUnitStat( int stat, Unit *target )
{
    static float game_speed = XMLSupport::parse_float( vs_config->getVariable( "physics", "game_speed", "1" ) );
    static bool  display_in_meters = XMLSupport::parse_bool( vs_config->getVariable( "physics", "display_in_meters", "true" ) );
    static bool  lie = XMLSupport::parse_bool( vs_config->getVariable( "physics", "game_speed_lying", "true" ) );
    static float fpsval     = 0;
    const float  fpsmax     = 1;
    static float numtimes   = fpsmax;
    float armordat[8];     //short fix
    int   armori;
    Unit *tmpunit;

    // TODO: lib_damage
    // make sure the enums are in the right order as our
    // facet_configuration
    // Also, can't be defined within switch for some reason
    int shield_index = stat-UnitImages < void > ::SHIELDF;

    if (shield8) {
        switch (stat)
        {
        case UnitImages< void >::SHIELDF:
        case UnitImages< void >::SHIELDR:
        case UnitImages< void >::SHIELDL:
        case UnitImages< void >::SHIELDB:
        case UnitImages< void >::SHIELD4:
        case UnitImages< void >::SHIELD5:
        case UnitImages< void >::SHIELD6:
        case UnitImages< void >::SHIELD7:
            // TODO: lib_damage
            // Not really sure what this is supposed to return.
            // Probably a percent of the current/max shield values.
            // Subtracing enum SHIELDF (first shield gauge) converts the
            // stat parameter to the index of the shield.

            if (target->GetShieldLayer().facets[shield_index].max_health > 0) {
                return target->GetShieldLayer().facets[shield_index].Percent();
            } else {
                return 0;
            }
        default:
            break;
        }
    }
    switch (stat)
    {
    case UnitImages< void >::SHIELDF:
        return target->FShieldData();

    case UnitImages< void >::SHIELDR:
        return target->RShieldData();

    case UnitImages< void >::SHIELDL:
        return target->LShieldData();

    case UnitImages< void >::SHIELDB:
        return target->BShieldData();

    case UnitImages< void >::ARMORF:
    case UnitImages< void >::ARMORR:
    case UnitImages< void >::ARMORL:
    case UnitImages< void >::ARMORB:
    case UnitImages< void >::ARMOR4:
    case UnitImages< void >::ARMOR5:
    case UnitImages< void >::ARMOR6:
    case UnitImages< void >::ARMOR7:
        target->ArmorData( armordat );
        if (armor8) {
            return armordat[stat-UnitImages < void > ::ARMORF]/StartArmor[stat-UnitImages < void > ::ARMORF];
        } else {
            for (armori = 0; armori < 8; ++armori) {
                if (armordat[armori] > StartArmor[armori])
                    StartArmor[armori] = armordat[armori];
                armordat[armori] /= StartArmor[armori];
            }
        }
        switch (stat)
        {
        case UnitImages< void >::ARMORR:
            return .25*(armordat[0]+armordat[1]+armordat[4]+armordat[5]);

        case UnitImages< void >::ARMORL:
            return .25*(armordat[2]+armordat[3]+armordat[6]+armordat[7]);

        case UnitImages< void >::ARMORB:
            return .25*(armordat[1]+armordat[3]+armordat[5]+armordat[7]);

        case UnitImages< void >::ARMORF:
        default:
            return .25*(armordat[0]+armordat[2]+armordat[4]+armordat[6]);
        }
    case UnitImages< void >::FUEL:
        if (target->fuelData() > maxfuel)
            maxfuel = target->fuelData();
        if (maxfuel > 0) return target->fuelData()/maxfuel;
        return 0;

    case UnitImages< void >::ENERGY:
        return target->energyData();

    case UnitImages< void >::WARPENERGY:
        {
            static bool warpifnojump =
                XMLSupport::parse_bool( vs_config->getVariable( "graphics", "hud", "display_warp_energy_if_no_jump_drive",
                                                                "true" ) );
            return (warpifnojump || target->GetJumpStatus().drive != -2) ? target->warpEnergyData() : 0;
        }
    case UnitImages< void >::HULL:
        if ( maxhull < target->GetHull() )
            maxhull = target->GetHull();
        return target->GetHull()/maxhull;

    case UnitImages< void >::EJECT:
        {
            int go =
                ( ( (target->GetHull()/maxhull) < .25 )
                 && (target->BShieldData() < .25 || target->FShieldData() < .25) ) ? 1 : 0;
            static int overload = 0;
            if (overload != go) {
                if (go == 0) {
                    static soundContainer ejectstopsound;
                    if (ejectstopsound.sound < 0) {
                        static string str = vs_config->getVariable( "cockpitaudio", "overload_stopped", "overload_stopped" );
                        ejectstopsound.loadsound( str );
                    }
                    ejectstopsound.playsound();
                } else {
                    static soundContainer ejectsound;
                    if (ejectsound.sound < 0) {
                        static string str = vs_config->getVariable( "cockpitaudio", "overload", "overload" );
                        ejectsound.loadsound( str );
                    }
                    ejectsound.playsound();
                }
                overload = go;
            }
            return go;
        }
    case UnitImages< void >::LOCK:
        {
            static float locklight_time = XMLSupport::parse_float( vs_config->getVariable( "graphics", "locklight_time", "1" ) );
            bool res = false;
            if ( ( tmpunit = target->GetComputerData().threat.GetUnit() ) ) {
                res = tmpunit->cosAngleTo( target, *&armordat[0], FLT_MAX, FLT_MAX ) > .95;
                if (res) last_locktime = UniverseUtil::GetGameTime();
            }
            return ( res || ( (UniverseUtil::GetGameTime()-last_locktime) < locklight_time ) ) ? 1.0f : 0.0f;
        }
    case UnitImages< void >::MISSILELOCK:
        {
            static float locklight_time = XMLSupport::parse_float( vs_config->getVariable( "graphics", "locklight_time", "1" ) );
            bool res = target->graphicOptions.missilelock;
            if (res) last_mlocktime = UniverseUtil::GetGameTime();
            return ( res || ( (UniverseUtil::GetGameTime()-last_mlocktime) < locklight_time ) ) ? 1.0f : 0.0f;
        }
    case UnitImages< void >::COLLISION:
        {
            static double collidepanic =
                XMLSupport::parse_float( vs_config->getVariable( "physics", "collision_inertial_time", "1.25" ) );
            return (getNewTime()-TimeOfLastCollision) < collidepanic;
        }
    case UnitImages< void >::ECM:
        return (UnitUtil::getECM(target) > 0) ? 1 : 0;

    case UnitImages< void >::WARPFIELDSTRENGTH:
        return target->graphicOptions.WarpFieldStrength;

    case UnitImages< void >::MAXWARPFIELDSTRENGTH:
        return target->GetMaxWarpFieldStrength(1.f);

    case UnitImages< void >::JUMP:
        return jumpok ? 1 : 0;

    case UnitImages< void >::KPS:
    case UnitImages< void >::SETKPS:
    case UnitImages< void >::MAXKPS:
    case UnitImages< void >::MAXCOMBATKPS:
    case UnitImages< void >::MAXCOMBATABKPS:
        {
            static bool use_relative_velocity =
                XMLSupport::parse_bool( vs_config->getVariable( "graphics", "hud", "display_relative_velocity", "true" ) );
            float value;
            switch (stat)
            {
            case UnitImages< void >::KPS:
                if (target->graphicOptions.WarpFieldStrength != 1.0) {
                    if ( use_relative_velocity && target->computer.velocity_ref.GetUnit() ) {
                        if (target->computer.velocity_ref.GetUnit()->graphicOptions.WarpFieldStrength != 1.0) {
                            value =
                                ( target->GetWarpVelocity()
                                 -target->computer.velocity_ref.GetUnit()->GetWarpVelocity() ).Magnitude();
                        } else {
                            value =
                                (target->GetWarpVelocity()
                                 -target->computer.velocity_ref.GetUnit()->cumulative_velocity).Magnitude();
                        }
                    } else {
                        value = target->GetWarpVelocity().Magnitude();
                    }
                } else {
                    if ( use_relative_velocity && target->computer.velocity_ref.GetUnit() ) {
                        if (target->computer.velocity_ref.GetUnit()->graphicOptions.WarpFieldStrength != 1.0) {
                            value =
                                ( target->cumulative_velocity
                                 -target->computer.velocity_ref.GetUnit()->GetWarpVelocity() ).Magnitude();
                        } else {
                            value =
                                (target->cumulative_velocity
                                 -target->computer.velocity_ref.GetUnit()->cumulative_velocity).Magnitude();
                        }
                    } else {
                        value = target->cumulative_velocity.Magnitude();
                    }
                }
                break;
            case UnitImages< void >::SETKPS:
                value = target->GetComputerData().set_speed;
                break;
            case UnitImages< void >::MAXKPS:
                value = target->GetComputerData().max_speed();
                break;
            case UnitImages< void >::MAXCOMBATKPS:
                value = target->GetComputerData().max_combat_speed;
                break;
            case UnitImages< void >::MAXCOMBATABKPS:
                value = target->GetComputerData().max_combat_ab_speed;
                break;
            default:
                value = 0;
            }
            if (lie)
                return value/game_speed;
            else
                return display_in_meters ? value : value*3.6;          //JMS 6/28/05 - converted back to raw meters/second
        }
    case UnitImages< void >::MASSEFFECT:
        {
            float basemass = atof( UniverseUtil::LookupUnitStat( target->name, "", "Mass" ).c_str() );
            if (basemass > 0)
                return 100*target->getMass()/basemass;
            else
                return 0;
        }
    case UnitImages< void >::AUTOPILOT:
        {
            static int  wasautopilot = 0;
            int abletoautopilot = 0;
            static bool auto_valid   =
                XMLSupport::parse_bool( vs_config->getVariable( "physics", "insystem_jump_or_timeless_auto-pilot", "false" ) );
            if (target) {
                if (!auto_valid) {
                    abletoautopilot = (target->graphicOptions.InWarp);
                } else {
                    abletoautopilot = (target->AutoPilotTo( target, false ) ? 1 : 0);
                    static float no_auto_light_below =
                        XMLSupport::parse_float( vs_config->getVariable( "physics", "no_auto_light_below", "2000" ) );
                    Unit *targtarg = target->Target();
                    if (targtarg) {
                        if ( ( target->Position()-targtarg->Position() ).Magnitude()-targtarg->rSize()-target->rSize()
                            < no_auto_light_below )
                            abletoautopilot = false;
                    }
                }
            }
            if (abletoautopilot != wasautopilot) {
                if (abletoautopilot == 0) {
                    static soundContainer autostopsound;
                    if (autostopsound.sound < 0) {
                        static string str = vs_config->getVariable( "cockpitaudio",
                                                                    "autopilot_available",
                                                                    "autopilot_available" );
                        autostopsound.loadsound( str );
                    }
                    autostopsound.playsound();
                } else {
                    static soundContainer autosound;
                    if (autosound.sound < 0) {
                        static string str = vs_config->getVariable( "cockpitaudio",
                                                                    "autopilot_unavailable",
                                                                    "autopilot_unavailable" );
                        autosound.loadsound( str );
                    }
                    autosound.playsound();
                }
                wasautopilot = abletoautopilot;
            }
            return abletoautopilot;
        }
    case UnitImages< void >::COCKPIT_FPS:
        if (fpsval >= 0 && fpsval < .5*FLT_MAX)
            numtimes -= .1+fpsval;
        if (numtimes <= 0) {
            numtimes = fpsmax;
            fpsval   = GetElapsedTime();
        }
        if (fpsval)
            return 1./fpsval;
    case UnitImages< void >::AUTOPILOT_MODAL:
        if (target->autopilotactive)
            return (float) UnitImages< void >::ACTIVE;
        else
            return (float) UnitImages< void >::OFF;
    case UnitImages< void >::SPEC_MODAL:
        if (target->graphicOptions.WarpRamping)
            return (float) UnitImages< void >::SWITCHING;
        else if (target->graphicOptions.InWarp)
            return (float) UnitImages< void >::ACTIVE;
        else
            return (float) UnitImages< void >::OFF;
    case UnitImages< void >::FLIGHTCOMPUTER_MODAL:
        if (target->inertialmode)
            return (float) UnitImages< void >::OFF;
        else
            return (float) UnitImages< void >::ON;
    case UnitImages< void >::TURRETCONTROL_MODAL:
        if (0 == target->turretstatus)
            return (float) UnitImages< void >::NOTAPPLICABLE;
        else if (2 == target->turretstatus)          //FIXME -- need to check if turrets are active
            return (float) UnitImages< void >::ACTIVE;
        else if (3 == target->turretstatus)          //FIXME -- need to check if turrets are in FireAtWill state
            return (float) UnitImages< void >::FAW;
        else
            return (float) UnitImages< void >::OFF;
    case UnitImages< void >::ECM_MODAL:
        if (target->ecm > 0)
            return (target->GetComputerData().ecmactive ? (float) UnitImages< void >::ACTIVE : (float) UnitImages< void >::READY);
        else
            return (float) UnitImages< void >::NOTAPPLICABLE;
    case UnitImages< void >::CLOAK_MODAL:
        if (-1 == target->cloaking)
            return (float) UnitImages< void >::NOTAPPLICABLE;
        else if ( ( (int) (-2147483647)-1 ) == target->cloaking )
            return (float) UnitImages< void >::READY;
        else if (target->cloaking == target->cloakmin)
            return (float) UnitImages< void >::ACTIVE;
        else
            return (float) UnitImages< void >::SWITCHING;
    case UnitImages< void >::TRAVELMODE_MODAL:
        if ( target->CombatMode() )
            return (float) UnitImages< void >::MANEUVER;
        else
            return (float) UnitImages< void >::TRAVEL;
    case UnitImages< void >::RECIEVINGFIRE_MODAL:
        if (!target)          //FIXME
            return (float) UnitImages< void >::WARNING;
        else
            return (float) UnitImages< void >::NOMINAL;
    case UnitImages< void >::RECEIVINGMISSILES_MODAL:
        if (!target)          //FIXME
            return (float) UnitImages< void >::WARNING;
        else
            return (float) UnitImages< void >::NOMINAL;
    case UnitImages< void >::RECEIVINGMISSILELOCK_MODAL:
        if (!target)          //FIXME
            return (float) UnitImages< void >::WARNING;
        else
            return (float) UnitImages< void >::NOMINAL;
    case UnitImages< void >::RECEIVINGTARGETLOCK_MODAL:
        if (!target)          //FIXME
            return (float) UnitImages< void >::WARNING;
        else
            return (float) UnitImages< void >::NOMINAL;
    case UnitImages< void >::COLLISIONWARNING_MODAL:
        if (!target)          //FIXME
            return (float) UnitImages< void >::WARNING;
        else
            return (float) UnitImages< void >::NOMINAL;
    case UnitImages< void >::CANJUMP_MODAL:
        if (-2 == target->GetJumpStatus().drive)
            return (float) UnitImages< void >::NODRIVE;
        else if (target->getWarpEnergy() < target->GetJumpStatus().energy)
            return (float) UnitImages< void >::NOTENOUGHENERGY;
        else if (target->graphicOptions.InWarp)          //FIXME
            return (float) UnitImages< void >::OFF;
        else if (jumpok)
            return (float) UnitImages< void >::READY;
        else
            return (float) UnitImages< void >::TOOFAR;
    case UnitImages< void >::CANDOCK_MODAL:
        {
            Unit *station = target->Target();
            if (station)
            {
                if (station->CanDockWithMe(target, true) != -1)
                {
                    if (station->CanDockWithMe(target, false) != -1)
                    {
                        return (float) UnitImages<void>::READY;
                    }
                    if (Orders::AutoDocking::CanDock(target, station))
                    {
                        return (float) UnitImages<void>::AUTOREADY;
                    }
                    return (float) UnitImages<void>::TOOFAR;
                }
            }
            return (float) UnitImages< void >::NOMINAL;
        }
    }
    return 1.0f;
}



Cockpit::LastState::LastState()
{
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

void Cockpit::TriggerEvents( Unit *un ) {
    double curtime = UniverseUtil::GetGameTime();
    if ((curtime - AUDIO_ATOM) < last.processing_time)
        return;
    else
        last.processing_time = curtime;

    VS_LOG(trace, "Processing events");
    for (CockpitEvent event = EVENTID_FIRST; event < NUM_EVENTS; event = (CockpitEvent)(event+1)) {
        GameSoundContainer *sound = static_cast<GameSoundContainer*>(GetSoundForEvent(event));
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

        switch((int)event) {
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
            case ASAP_DOCKING_ENGAGED:
                {
                    float candock = LookupUnitStat(UnitImages< void >::CANDOCK_MODAL, un);
                    MODAL_TRIGGER("ASAP_DOCKING", true,
                                (un->autopilotactive && (   candock == UnitImages< void >::READY
                                                         || candock == UnitImages< void >::AUTOREADY)),
                                asap_dockon);
                }
                break;
            case ASAP_DOCKING_DISENGAGED:
                {
                    float candock = LookupUnitStat(UnitImages< void >::CANDOCK_MODAL, un);
                    MODAL_TRIGGER("ASAP_DOCKING", false,
                                (un->autopilotactive && (   candock == UnitImages< void >::READY
                                                         || candock == UnitImages< void >::AUTOREADY)),
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
            case WARP_LOOP0+1:
            case WARP_LOOP0+2:
            case WARP_LOOP0+3:
            case WARP_LOOP0+4:
            case WARP_LOOP0+5:
            case WARP_LOOP0+6:
            case WARP_LOOP0+7:
            case WARP_LOOP0+8:
            case WARP_LOOP0+9:
                {
                    float warpfieldstrength = LookupUnitStat(UnitImages< void >::WARPFIELDSTRENGTH, un);
                    int warpreflevel = event - WARP_LOOP0;
                    int warplevel = int(log(warpfieldstrength)/log(10.f));
                    MODAL_TRIGGER("WARP_LOOP", warpreflevel, warplevel, warplooplevel);
                }
                break;
            case WARP_SKIP0:
            case WARP_SKIP0+1:
            case WARP_SKIP0+2:
            case WARP_SKIP0+3:
            case WARP_SKIP0+4:
            case WARP_SKIP0+5:
            case WARP_SKIP0+6:
            case WARP_SKIP0+7:
            case WARP_SKIP0+8:
            case WARP_SKIP0+9:
                {
                    float warpfieldstrength = LookupUnitStat(UnitImages< void >::WARPFIELDSTRENGTH, un);
                    int warpreflevel = event - WARP_SKIP0;
                    int warplevel = int(log(warpfieldstrength)/log(10.0f));
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


void Cockpit::Init( const char *file, bool isDisabled )
{
    retry_dock     = 0;
    shakin         = 0;
    autopilot_time = 0;
    bool f404 = false;
    if (file == NULL || strlen( file ) == 0) {
        if (isDisabled)
            file = "cockpit.cpt";
        else
            file = "disabled-cockpit.cpt";
        f404 = true;
    }
    if (isDisabled == true) {
        std::string disname = std::string( "disabled-" )+file;
        Init( disname.c_str() );
        return;
    }
    VSFile  f;
    VSError err = f.OpenReadOnly( file, CockpitFile );
    if (err > Ok) {
        //File not found...
        if ( isDisabled == false && (string( file ).find( "disabled-" ) == string::npos) )
            Init( file, true );
        else if (!f404)
            Init( NULL );
        return;
    }
    Delete();
    LoadXML( f );
    f.Close();
}

void Cockpit::Init( const char *file )
{
    smooth_fov = g_game.fov;
    editingTextMessage = false;
    armor8     = false;
    shield8    = false;
    if (Panel.size() > 0) {
        float x, y;
        Panel.front()->GetPosition( x, y );
        Panel.front()->SetPosition( x, y+viewport_offset );
    }
}

void Cockpit::Delete() {
    viewport_offset = cockpit_offset = 0;

    int i;
    if (text) {
        delete text;
        text = NULL;
    }
    for (i = 0; i < (int) mesh.size(); ++i) {
        if (mesh[i])
            delete mesh[i];
        mesh[i] = NULL;
    }
    mesh.clear();
    if (soundfile >= 0) {
        AUDStopPlaying( soundfile );
        AUDDeleteSound( soundfile, false );
        soundfile = -1;
    }
    for (i = 0; i < 4; i++) {
        /*
         *  if (Pit[i]) {
         *  delete Pit[i];
         *  Pit[i] = NULL;
         *  }
         */
    }
    for (i = 0; i < UnitImages< void >::NUMGAUGES; i++)
        if (gauges[i]) {
            delete gauges[i];
            gauges[i] = NULL;
        }
    if (radarSprites[0]) {
        delete radarSprites[0];
        radarSprites[0] = NULL;
    }
    if (radarSprites[1]) {
        delete radarSprites[1];
        radarSprites[1] = NULL;
    }
    unsigned int j;
    for (j = 0; j < vdu.size(); j++)
        if (vdu[j]) {
            delete vdu[j];
            vdu[j] = NULL;
        }
    vdu.clear();
    for (j = 0; j < Panel.size(); j++) {
        assert( Panel[j] );
        delete Panel[j];
    }
    Panel.clear();
}

void Cockpit::InitStatic() {
    int i;
    for (i = 0; i < UnitImages< void >::NUMGAUGES; i++)
        gauge_time[i] = 0;
    for (i = 0; i < MAXVDUS; i++)
        vdu_time[i] = 0;
    radar_time   = 0;
    cockpit_time = 0;
}

/***** WARNING CHANGED ORDER *****/
Cockpit::Cockpit( const char *file, Unit *parent, const std::string &pilot_name )
    : insidePanYaw( 0 )
    , insidePanPitch( 0 )
    , insidePanYawSpeed( 0 )
    , insidePanPitchSpeed( 0 )
    , shake_time( 0 )
    , shake_type( 0 )
    , textcol( 1, 1, 1, 1 )
    , text( NULL )
    // cockpit class inits
    ,view( CP_FRONT )
    , parent( parent )
    , cockpit_offset( 0 )
    , viewport_offset( 0 )
    , zoomfactor( getInitialZoomFactor() )
    , savegame( new SaveGame( pilot_name ) ) {
    // Begin cockpit constructor
    // Previously, this was GameCockpit with a parent class of Cockpit
    partial_number_of_attackers = -1;
    number_of_attackers   = 0;
    secondsWithZeroEnergy = 0;
    fg = NULL;
    jumpok = 0;
    TimeOfLastCollision   = -200;
    /*
     *  for (i=0;i<headlag;i++) {
     *  headtrans.push_back (Matrix());
     *  Identity(headtrans.back());
     *  }
     *  for (i=0;i<UnitImages::NUMGAUGES;i++) {
     *  gauges[i]=NULL;
     *  }
     */
    activeStarSystem = NULL;
    InitStatic();
    //mesh=NULL;
    ejecting = false;
    currentcamera = 0;
    going_to_dock_screen = false;
    //Radar=Pit[0]=Pit[1]=Pit[2]=Pit[3]=NULL;
    RestoreGodliness();

    /*
     *  draw_all_boxes=XMLSupport::parse_bool(vs_config->getVariable("graphics","hud","drawAllTargetBoxes","false"));
     *  draw_line_to_target=XMLSupport::parse_bool(vs_config->getVariable("graphics","hud","drawLineToTarget","false"));
     *  draw_line_to_targets_target=XMLSupport::parse_bool(vs_config->getVariable("graphics","hud","drawLineToTargetsTarget","false"));
     *  draw_line_to_itts=XMLSupport::parse_bool(vs_config->getVariable("graphics","hud","drawLineToITTS","false"));
     *  always_itts=XMLSupport::parse_bool(vs_config->getVariable("graphics","hud","drawAlwaysITTS","false"));
     *  radar_type=vs_config->getVariable("graphics","hud","radarType","WC");
     *
     *  friendly=GFXColor(-1,-1,-1,-1);
     *  enemy=GFXColor(-1,-1,-1,-1);
     *  neutral=GFXColor(-1,-1,-1,-1);
     *  targeted=GFXColor(-1,-1,-1,-1);
     *  targetting=GFXColor(-1,-1,-1,-1);
     *  planet=GFXColor(-1,-1,-1,-1);
     *  if (friendly.r==-1) {
     *  vs_config->getColor ("enemy",&enemy.r);
     *  vs_config->getColor ("friend",&friendly.r);
     *  vs_config->getColor ("neutral",&neutral.r);
     *  vs_config->getColor("target",&targeted.r);
     *  vs_config->getColor("targetting_ship",&targetting.r);
     *  vs_config->getColor("planet",&planet.r);
     *  }
     */

    Init( file );
    // end cockpit constructor

    autoMessageTime    = 0;
    shield8 = armor8 = false;
    editingTextMessage = false;
    static int headlag = XMLSupport::parse_int( vs_config->getVariable( "graphics", "head_lag", "10" ) );
    int i;
    for (i = 0; i < headlag; i++) {
        headtrans.push_back( Matrix() );
        Identity( headtrans.back() );
    }
    for (i = 0; i < UnitImages< void >::NUMGAUGES; i++)
        gauges[i] = NULL;
    radarSprites[0] = radarSprites[1] = Pit[0] = Pit[1] = Pit[2] = Pit[3] = NULL;

    static bool st_draw_all_boxes =
        XMLSupport::parse_bool( vs_config->getVariable( "graphics", "hud", "drawAllTargetBoxes", "false" ) );
    static bool st_draw_line_to_target =
        XMLSupport::parse_bool( vs_config->getVariable( "graphics", "hud", "drawLineToTarget", "false" ) );
    static bool st_draw_line_to_targets_target =
        XMLSupport::parse_bool( vs_config->getVariable( "graphics", "hud", "drawLineToTargetsTarget", "false" ) );
    static bool st_draw_line_to_itts =
        XMLSupport::parse_bool( vs_config->getVariable( "graphics", "hud", "drawLineToITTS", "false" ) );
    static bool st_always_itts = XMLSupport::parse_bool( vs_config->getVariable( "graphics", "hud", "drawAlwaysITTS", "false" ) );
    static bool st_steady_itts = XMLSupport::parse_bool( vs_config->getVariable( "physics", "steady_itts", "false" ) );

    draw_all_boxes = st_draw_all_boxes;
    draw_line_to_target = st_draw_line_to_target;
    draw_line_to_targets_target = st_draw_line_to_targets_target;
    draw_line_to_itts   = st_draw_line_to_itts;
    always_itts   = st_always_itts;
    steady_itts   = st_steady_itts;
    last_locktime = last_mlocktime = -FLT_MAX;

    radarDisplay = Radar::Factory(Radar::Type::NullDisplay);

    //Compute the screen limits. Used to display the arrow pointing to the selected target.
    static float st_projection_limit_y = XMLSupport::parse_float( vs_config->getVariable( "graphics", "fov", "78" ) );
    smooth_fov = st_projection_limit_y;
    projection_limit_y = st_projection_limit_y;
    //The angle betwwen the center of the screen and the border is half the fov.
    projection_limit_y = tan( projection_limit_y*M_PI/(180*2) );
    projection_limit_x = projection_limit_y*g_game.aspect;
    //Precompute this division... performance.
    inv_screen_aspect_ratio = 1/g_game.aspect;

    oaccel     = Vector( 0, 0, 0 );

    enemy      = vs_config->getColor( "enemy",           GFXColor(1.0 ,0.0,0.0,1.0) ); // red
    friendly   = vs_config->getColor( "friend",          GFXColor(0.0 ,1.0,0.0,1.0) ); // green
    neutral    = vs_config->getColor( "neutral",         GFXColor(1.0 ,1.0,0.0,1.0) ); // yellow
    targeted   = vs_config->getColor( "target",          GFXColor(1.0 ,0.0,1.0,1.0) ); // violet
    targetting = vs_config->getColor( "targetting_ship", GFXColor( .68, .9,1.0,1.0) ); // light-blue
    planet     = vs_config->getColor( "planet",          GFXColor(1.0 ,1.0, .6,1.0) ); //
    soundfile  = -1;
    InitStatic();
    updateRadar(parent);
}

void Cockpit::SelectProperCamera()
{
    SelectCamera( view );
}

void DoCockpitKeys() {
    CockpitKeys::Pan( KBData(), PRESS );
    CockpitKeys::Inside( KBData(), PRESS );
}

void Cockpit::NavScreen( const KBData&, KBSTATE k ) {
    //scheherazade
    if (k == PRESS) {
        //UniverseUtil::IOmessage(0,"game","all","hit key");
        if ( ( _Universe->AccessCockpit() )->CanDrawNavSystem() ) {
            ( _Universe->AccessCockpit() )->SetDrawNavSystem( 0 );
            //UniverseUtil::IOmessage(0,"game","all","DRAWNAV - OFF");
            RestoreMouse();
        } else {
            ( _Universe->AccessCockpit() )->SetDrawNavSystem( 1 );
            //UniverseUtil::IOmessage(0,"game","all","DRAWNAV - ON");

            winsys_set_mouse_func( BaseInterface::ClickWin );
            winsys_set_motion_func( BaseInterface::ActiveMouseOverWin );
            winsys_set_passive_motion_func( BaseInterface::PassiveMouseOverWin );
        }
    }
}

bool Cockpit::SetDrawNavSystem( bool what ) {
    ThisNav.SetDraw( what );
    return what;
}

bool Cockpit::CanDrawNavSystem() {
    return ThisNav.CheckDraw();
}

void Cockpit::visitSystem( string systemname ) {
    visitSystemHelp( this, systemname, 1.0 );
    int adj = UniverseUtil::GetNumAdjacentSystems( systemname );
    for (int i = 0; i < adj; ++i)
        visitSystemHelp( this, UniverseUtil::GetAdjacentSystem( systemname, i ), 0.0 );

    if ( AccessNavSystem() ) {
        static bool AlwaysUpdateNavMap =
            XMLSupport::parse_bool( vs_config->getVariable( "graphics", "update_nav_after_jump", "false" ) );                              //causes occasional crash--only may have tracked it down
        if (AlwaysUpdateNavMap)
            AccessNavSystem()->pathman->updatePaths();
        AccessNavSystem()->setCurrentSystem( systemname );
    }
}



void RespawnNow( Cockpit *cp )
{
    while ( respawnunit.size() <= _Universe->numPlayers() )
        respawnunit.push_back( 0 );
    for (unsigned int i = 0; i < _Universe->numPlayers(); i++)
        if (_Universe->AccessCockpit( i ) == cp)
            respawnunit[i] = 2;
}

void Cockpit::SwitchControl( const KBData&, KBSTATE k )
{
    if (k == PRESS) {
        while ( switchunit.size() <= _Universe->CurrentCockpit() )
            switchunit.push_back( 0 );
        switchunit[_Universe->CurrentCockpit()] = 1;
    }
}

void Cockpit::ForceSwitchControl( const KBData&, KBSTATE k )
{
    while ( switchunit.size() <= _Universe->CurrentCockpit() )
        switchunit.push_back( 0 );
    switchunit[_Universe->CurrentCockpit()] = 1;
}

void SuicideKey( const KBData&, KBSTATE k )
{
    static int orig = 0;
    if (k == PRESS) {
        int newtime = time( NULL );
        if (newtime-orig > 8 || orig == 0) {
            orig = newtime;
            Unit *un = NULL;
            if ( ( un = _Universe->AccessCockpit()->GetParent() ) ) {
                float armor[8];                 //short fix
                un->Destroy();
            }
        }
    }
}

class UnivMap
{
    VSSprite *ul;
    VSSprite *ur;
    VSSprite *ll;
    VSSprite *lr;
public:
    bool isNull()
    {
        return ul == NULL;
    }
    UnivMap( VSSprite *ull, VSSprite *url, VSSprite *lll, VSSprite *lrl )
    {
        ul = ull;
        ur = url;
        ll = lll;
        lr = lrl;
    }
    void Draw()
    {
        if (ul || ur || ll || lr) {
            GFXBlendMode( SRCALPHA, INVSRCALPHA );
            GFXEnable( TEXTURE0 );
            GFXDisable( TEXTURE1 );
            GFXColor4f( 1, 1, 1, 1 );
        }
        if (ul)
            ul->Draw();
        if (ur)
            ur->Draw();
        if (ll)
            ll->Draw();
        if (lr)
            lr->Draw();
    }
};

std::vector< UnivMap >univmap;

void MapKey( const KBData&, KBSTATE k )
{
    if (k == PRESS) {
        static VSSprite ul( "upper-left-map.spr" );
        static VSSprite ur( "upper-right-map.spr" );
        static VSSprite ll( "lower-left-map.spr" );
        static VSSprite lr( "lower-right-map.spr" );
        while ( univmap.size() <= _Universe->CurrentCockpit() )
            univmap.push_back( UnivMap( NULL, NULL, NULL, NULL ) );
        if ( univmap[_Universe->CurrentCockpit()].isNull() )
            univmap[_Universe->CurrentCockpit()] = UnivMap( &ul, &ur, &ll, &lr );
        else
            univmap[_Universe->CurrentCockpit()] = UnivMap( NULL, NULL, NULL, NULL );
    }
}

void Cockpit::TurretControl( const KBData&, KBSTATE k )
{
    if (k == PRESS) {
        while ( turretcontrol.size() <= _Universe->CurrentCockpit() )
            turretcontrol.push_back( 0 );
        turretcontrol[_Universe->CurrentCockpit()] = 1;
    }
}

void Cockpit::Respawn( const KBData&, KBSTATE k )
{
    if (k == PRESS) {
        while ( respawnunit.size() <= _Universe->CurrentCockpit() )
            respawnunit.push_back( 0 );
        respawnunit[_Universe->CurrentCockpit()] = 1;
    }
}

//SAME AS IN COCKPIT BUT ADDS SETVIEW and ACCESSCAMERA -> ~ DUPLICATE CODE
int Cockpit::Autopilot( Unit *target )
{
    static bool autopan = XMLSupport::parse_bool( vs_config->getVariable( "graphics", "pan_on_auto", "true" ) );
    int retauto = 0;
    if (target) {
        if (enableautosound.sound < 0) {
            static string str = vs_config->getVariable( "cockpitaudio", "autopilot_enabled", "autopilot" );
            enableautosound.loadsound( str );
        }
        enableautosound.playsound();
        Unit *un = NULL;
        if ( ( un = GetParent() ) ) {
            autoMessage     = std::string();
            autoMessageTime = UniverseUtil::GetGameTime();
            QVector posA     = un->LocalPosition();
            bool    retautoA = false;
            if ( ( retauto = retautoA = un->AutoPilotToErrorMessage( un, false, autoMessage ) ) ) {
                //can he even start to autopilot
                retauto = un->AutoPilotToErrorMessage( target, false, autoMessage );
                QVector posB = un->LocalPosition();
                bool    movedatall = (posA-posB).Magnitude() > un->rSize();
                if (autoMessage.length() == 0 && !movedatall) {
                    autoMessage =
                        XMLSupport::escaped_string( vs_config->getVariable( "graphics", "hud", "AlreadyNearMessage",
                                                                            "#ff0000Already Near#000000" ) );
                    retauto     = false;
                } else if ( (retauto || retautoA) && movedatall ) {
                    if (autopan) {
                        SetView( CP_FIXEDPOS );
                        Vector P( 1, 0, 0 ), Q( 0, 1, 0 ), R( 0, 0, 1 );
                        Vector uP, uQ, uR;
                        un->GetOrientation( uP, uQ, uR );
                        static float auto_side_bias  =
                            XMLSupport::parse_float( vs_config->getVariable( "graphics", "autopilot_side_bias", "1.1" ) );
                        static float auto_front_bias =
                            XMLSupport::parse_float( vs_config->getVariable( "graphics", "autopilot_front_bias", "1.65" ) );
                        P += uP*auto_side_bias+uR*auto_front_bias;
                        P.Normalize();
                        R  = P.Cross( Q );
                        AccessCamera( CP_FIXED )->SetPosition( un->LocalPosition()+2*un->rSize()*P,
                                                              Vector( 0, 0, 0 ), Vector( 0, 0, 0 ), Vector( 0, 0, 0 ) );
                        AccessCamera( CP_FIXED )->SetOrientation( R, Q, -P );
                        AccessCamera( CP_FIXEDPOS )->SetPosition( un->LocalPosition()+2*un->rSize()*P, Vector( 0,
                                                                                                               0,
                                                                                                               0 ),
                                                                 Vector( 0, 0, 0 ), Vector( 0, 0, 0 ) );
                        AccessCamera( CP_FIXEDPOS )->SetOrientation( R, Q, -P );
                    }

                    static float  autotime = XMLSupport::parse_float( vs_config->getVariable( "physics", "autotime", "10" ) );                    //10 seconds for auto to kick in;

                    autopilot_time = autotime;
                    autopilot_target.SetUnit( target );
                }
            }
        }
    }
    return retauto;
}

extern void reset_time_compression( const KBData&, KBSTATE a );

void Cockpit::Shake( float amt, int dtype )
{
    static float shak     = XMLSupport::parse_float( vs_config->getVariable( "graphics", "cockpit_shake", "3" ) );
    static float shak_max = XMLSupport::parse_float( vs_config->getVariable( "graphics", "cockpit_shake_max", "20" ) );
    shakin += shak;
    if (shakin > shak_max)
        shakin = shak_max;
    this->shake_time = getNewTime();
    this->shake_type = dtype;
}

static void DrawDamageFlash( int dtype )
{
    const int     numtypes    = 3;
    static string shieldflash = vs_config->getVariable( "graphics", "shield_flash_animation", "" );
    static string armorflash  = vs_config->getVariable( "graphics", "armor_flash_animation", "armorflash.ani" );
    static string hullflash   = vs_config->getVariable( "graphics", "hull_flash_animation", "hullflash.ani" );
    string flashes[numtypes];
    flashes[0] = shieldflash;
    flashes[1] = armorflash;
    flashes[2] = hullflash;
    static GFXColor fallbackcolor[numtypes] = {
        vs_config->getColor( "shield_flash", GFXColor(0, 1, .5, .2 ) ),
        vs_config->getColor( "armor_flash" , GFXColor(1, 0, .2, .25) ),
        vs_config->getColor( "hull_flash"  , GFXColor(1, 0, 0 , .5 ) )
    };


    static bool init = false;
    static Animation *aflashes[numtypes];
    static bool doflash[numtypes];
    if (!init) {
        init = true;
        for (int i = 0; i < numtypes; ++i) {
            doflash[i] = (flashes[i].length() > 0);
            if (doflash[i])
                aflashes[i] = new Animation( flashes[i].c_str(), true, .1, BILINEAR, false, false );
            else
                aflashes[i] = NULL;
        }
    }
    if (dtype < numtypes) {
        int i = dtype;
        if (aflashes[i]) {
            GFXPushBlendMode();
            static bool damage_flash_alpha =
                XMLSupport::parse_bool( vs_config->getVariable( "graphics", "damage_flash_alpha", "true" ) );
            if (damage_flash_alpha)
                GFXBlendMode( SRCALPHA, INVSRCALPHA );
            else
                GFXBlendMode( ONE, ZERO );
            if ( aflashes[i]->LoadSuccess() ) {
                aflashes[i]->MakeActive();
                GFXColor4f( 1, 1, 1, 1 );

                static const float verts[4 * (3 + 2)] = {
                   -1.0f, -1.0f, 1.0f,  0.0f, 1.0f,    //lower left
                    1.0f, -1.0f, 1.0f,  1.0f, 1.0f,    //upper left
                    1.0f,  1.0f, 1.0f,  1.0f, 0.0f,    //upper right
                   -1.0f,  1.0f, 1.0f,  0.0f, 0.0f,    //lower right
                };
                GFXDraw( GFXQUAD, verts, 4, 3, 0, 2 );
            } else {
                GFXColor4f( fallbackcolor[i].r,
                            fallbackcolor[i].g,
                            fallbackcolor[i].b,
                            fallbackcolor[i].a );
                GFXDisable( TEXTURE0 );

                static const float verts[4 * 3] = {
                   -1.0f, -1.0f, 1.0f,
                   -1.0f,  1.0f, 1.0f,
                    1.0f,  1.0f, 1.0f,
                    1.0f, -1.0f, 1.0f,
                };
                GFXDraw( GFXQUAD, verts, 4 );

                GFXEnable( TEXTURE0 );
            }
            GFXPopBlendMode();
        }
    }
    GFXColor4f( 1, 1, 1, 1 );
}

static void DrawHeadingMarker( const Vector &p, const Vector &q, const Vector &pos, float size )
{
    static VertexBuilder<> verts;
    verts.clear();
    verts.insert( pos + (2.5*size)*p );
    verts.insert( pos + size*p );
    verts.insert( pos + (0.939*size)*p - (0.342*size)*q );
    verts.insert( pos + (0.776*size)*p - (0.643*size)*q );
    verts.insert( pos + (0.500*size)*p - (0.866*size)*q );
    verts.insert( pos + (0.174*size)*p - (0.985*size)*q );
    verts.insert( pos - (0.174*size)*p - (0.985*size)*q );
    verts.insert( pos - (0.500*size)*p - (0.866*size)*q );
    verts.insert( pos - (0.776*size)*p - (0.643*size)*q );
    verts.insert( pos - (0.939*size)*p - (0.342*size)*q );
    verts.insert( pos - size*p );
    verts.insert( pos - (2.5*size)*p );
    GFXDraw( GFXLINESTRIP, verts );
}

static void DrawHeadingMarker( Cockpit &cp, const GFXColor &col )
{
    const Unit * u = cp.GetParent();
    const Camera * cam = cp.AccessCamera();
    bool drawv = true;

    // heading direction (unit fwd direction)
    Vector d = u->GetTransformation().getR();

    // flight direcion (unit vel direction)
    Vector v = u->GetWarpVelocity();
    if (u->VelocityReference())
        v -= u->VelocityReference()->GetWarpVelocity();
    float v2 = v.MagnitudeSquared();
    if ( v2 > 0.25 ) // 1/2 m/s seems reasonable for a speed marker
        v *= 1.0f / sqrtf(v2);
    else
        drawv = false;

    // up and right dirs p, q
    Vector p, q, r;
    cam->GetPQR(p, q, r);

    // znear offset
    float offset = 2 * g_game.znear / cos(cam->GetFov() * M_PI / 180.0);
    v *= offset;
    d *= offset;

    // size scale and flight dir alpha
    float size = 0.175f;
    float alpha = std::min(0.60f, (v - d).MagnitudeSquared() / (size * size * 36));

    // draw
    GFXDisable( TEXTURE0 );
    GFXDisable( LIGHTING );
    GFXBlendMode( SRCALPHA, INVSRCALPHA );
    GFXEnable( SMOOTH );

    if (drawv) {
        GFXLineWidth( 1.35f );
        GFXColor4f( col.r, col.g, col.b, col.a * alpha );
        DrawHeadingMarker( p, q, v * 1.01, size );
    }

    GFXLineWidth( 1.25f );
    GFXColor4f( col.r, col.g, col.b, col.a );
    DrawHeadingMarker( p, q, d, size );

    GFXLineWidth( 1.0f );
    GFXEnable( TEXTURE0 );
}

static void DrawCrosshairs( float x, float y, float wid, float hei, const GFXColor &col )
{
    GFXColorf( col );
    GFXDisable( TEXTURE0 );
    GFXDisable( LIGHTING );
    GFXBlendMode( SRCALPHA, INVSRCALPHA );
    GFXEnable( SMOOTH );
    GFXCircle( x, y, wid/4, hei/4 );
    GFXCircle( x, y, wid/7, hei/7 );
    GFXDisable( SMOOTH );

    const float verts[12 * 3] = {
        x-(wid/2.f), y, 0,
        x-(wid/6.f), y, 0,
        x+(wid/2.f), y, 0,
        x+(wid/6.f), y, 0,
        x, y-(hei/2.f), 0,
        x, y-(hei/6.f), 0,
        x, y+(hei/2.f), 0,
        x, y+(hei/6.f), 0,
        x-.001f, y+.001f, 0,
        x+.001f, y-.001f, 0,
        x+.001f, y+.001f, 0,
        x-.001f, y-.001f, 0,
    };
    GFXDraw( GFXLINE, verts, 12 );

    GFXEnable( TEXTURE0 );
}

extern bool QuitAllow;
extern bool screenshotkey;
QVector SystemLocation( std::string system );
double howFarToJump();

void Cockpit::Draw()
{
    static bool drawHeadingMarker = parse_bool( vs_config->getVariable( "graphics", "draw_heading_marker", "false" ) );
    static bool     draw_star_destination_arrow =
        XMLSupport::parse_bool( vs_config->getVariable( "graphics", "hud", "draw_star_direction", "true" ) );
    static GFXColor destination_system_color    = vs_config->getColor( "destination_system_color" );
    Vector destination_system_location( 0, 0, 0 );
    cockpit_time += GetElapsedTime();
    if (cockpit_time >= 100000)
        InitStatic();
    _Universe->AccessCamera()->UpdateGFX( GFXFALSE, GFXFALSE, GFXTRUE );     //Preliminary frustum
    GFXDisable( TEXTURE0 );
    GFXDisable( TEXTURE1 );
    GFXLoadIdentity( MODEL );
    GFXDisable( LIGHTING );
    GFXDisable( DEPTHTEST );
    GFXDisable( DEPTHWRITE );
    GFXColor4f( 1, 1, 1, 1 );
    std::string nav_current = AccessNavSystem()->getCurrentSystem();
    std::string universe_current = _Universe->activeStarSystem()->getFileName();
    if (nav_current != universe_current)
        AccessNavSystem()->Setup();
    static bool draw_any_boxes   =
        XMLSupport::parse_bool( vs_config->getVariable( "graphics", "hud", "DrawTargettingBoxes", "true" ) );
    static bool draw_boxes_inside_only =
        XMLSupport::parse_bool( vs_config->getVariable( "graphics", "hud", "DrawTargettingBoxesInside", "true" ) );
    if ( draw_any_boxes && screenshotkey == false && (draw_boxes_inside_only == false || view < CP_CHASE) )
    {
        Unit *player = GetParent();
        if (player)
        {
            Radar::Sensor sensor(player);
            DrawTargetBox(sensor, draw_line_to_target, draw_line_to_targets_target,
                          always_itts, player->computeLockingPercent(), draw_line_to_itts, steady_itts);
            DrawTurretTargetBoxes(sensor);
            DrawTacticalTargetBox(sensor);
            DrawCommunicatingBoxes(vdu);
            if (draw_all_boxes)
                DrawTargetBoxes(sensor);
        }
        if (draw_star_destination_arrow) {
            std::string destination_system = AccessNavSystem()->getSelectedSystem();
            std::string current_system     = _Universe->activeStarSystem()->getFileName();
            if (destination_system != current_system) {
                QVector cur   = SystemLocation( current_system );
                QVector dest  = SystemLocation( destination_system );
                QVector delta = dest-cur;
                if (delta.i != 0 || dest.j != 0 || dest.k != 0) {
                    delta.Normalize();
                    Unit  *par = GetParent();
                    delta = delta*howFarToJump()*1.01-( par ? ( par->Position() ) : QVector( 0, 0, 0 ) );
                    destination_system_location = delta.Cast();
                    Vector P, Q, R;
                    static float nav_symbol_size =
                        XMLSupport::parse_float( vs_config->getVariable( "graphics", "nav_symbol_size", ".25" ) );
                    AccessCamera()->GetPQR( P, Q, R );

                    GFXColor4f( destination_system_color.r,
                                destination_system_color.g,
                                destination_system_color.b,
                                destination_system_color.a );

                    static GFXColor suncol = vs_config->getColor( "remote_star", GFXColor( 0, 1, 1, .8 ) );
                    GFXColorf( suncol );
                    DrawNavigationSymbol( delta.Cast(), P, Q, delta.Magnitude()*nav_symbol_size );

                    GFXColor4f( 1, 1, 1, 1 );
                }
            }
        }
    }
    if (drawHeadingMarker && view < CP_CHASE)
        DrawHeadingMarker( *this, textcol );
    GFXEnable( TEXTURE0 );
    GFXEnable( DEPTHTEST );
    GFXEnable( DEPTHWRITE );

    if (view < CP_CHASE) {
        if ( mesh.size() ) {
            Unit *par = GetParent();
            if (par) {
                //cockpit is unaffected by FOV WARP-Link
                float oldfov = AccessCamera()->GetFov();
                AccessCamera()->SetFov( g_game.fov );

                GFXLoadIdentity( MODEL );

                size_t i, j;
                float  cockpitradial = 1;                //LET IT NOT BE ZERO!
                for (i = 0; i < mesh.size(); ++i) {
                    float meshmaxdepth = mesh[i]->corner_min().Max( mesh[i]->corner_max() ).Magnitude();
                    if (meshmaxdepth > cockpitradial)
                        cockpitradial = meshmaxdepth;
                }
                cockpitradial *= COCKPITZ_HEADROOM;

                GFXEnable( DEPTHTEST );
                GFXEnable( DEPTHWRITE );
                GFXEnable( TEXTURE0 );
                GFXEnable( LIGHTING );
                Vector P, Q, R;
                AccessCamera( CP_FRONT )->GetPQR( P, Q, R );

                headtrans.clear();

                headtrans.push_back( Matrix() );
                VectorAndPositionToMatrix( headtrans.back(), -P, Q, R, QVector( 0, 0, 0 ) );
                static float theta = 0, wtheta = 0;
                static float shake_speed =
                    XMLSupport::parse_float( vs_config->getVariable( "graphics", "shake_speed", "50" ) );
                static float shake_reduction  =
                    XMLSupport::parse_float( vs_config->getVariable( "graphics", "shake_reduction", "8" ) );
                static float shake_limit      =
                    XMLSupport::parse_float( vs_config->getVariable( "graphics", "shake_limit", "25" ) );
                static float shake_mag = XMLSupport::parse_float( vs_config->getVariable( "graphics", "shake_magnitude", "0.3" ) );
                static float drift_limit      =
                    XMLSupport::parse_float( vs_config->getVariable( "graphics", "cockpit_drift_limit", "1.00" ) );
                static float drift_amount     =
                    XMLSupport::parse_float( vs_config->getVariable( "graphics", "cockpit_drift_amount", "0.15" ) );
                static float drift_ref_accel  =
                    XMLSupport::parse_float( vs_config->getVariable( "graphics", "cockpit_drift_ref_accel", "100" ) );

                static float warp_shake_mag   =
                    XMLSupport::parse_float( vs_config->getVariable( "graphics", "warp_shake_magnitude", "0.125" ) );
                static float warp_shake_speed =
                    XMLSupport::parse_float( vs_config->getVariable( "graphics", "warp_shake_speed", "70" ) );
                static float warp_shake_ref   =
                    XMLSupport::parse_float( vs_config->getVariable( "graphics", "warp_shake_ref", "2000" ) );
                if (warp_shake_ref <= 0) warp_shake_ref = 1;
                theta  += shake_speed*GetElapsedTime()*sqrt( fabs( shakin ) )/10;                 //For small shakes, slower shakes
                wtheta += warp_shake_speed*GetElapsedTime();                 //SPEC-related shaking

                float self_kps = ( (GetParent() != NULL) ? LookupUnitStat( UnitImages< void >::KPS, GetParent() ) : 0 );
                float self_setkps   =
                    max( 1.0f, ( (GetParent() != NULL) ? LookupUnitStat( UnitImages< void >::SETKPS, GetParent() ) : 0 ) );
                float warp_strength =
                    max( 0.0f,
                        min( max( 0.0f,
                                 min( 1.0f,
                                      self_kps/self_setkps ) ),
                             ( (GetParent() != NULL) ? LookupUnitStat( UnitImages< void >::WARPFIELDSTRENGTH,
                                                                      GetParent() ) : 0.0f )/warp_shake_ref ) );
                if (shakin > shake_limit) shakin = shake_limit;
                headtrans.back().p.i = shake_mag*shakin*cos( theta )*cockpitradial/100;          //AccessCamera()->GetPosition().i+shakin*cos(theta);
                headtrans.back().p.j = shake_mag*shakin*cos( 1.3731*theta )*cockpitradial/100;  //AccessCamera()->GetPosition().j+shakin*cos(theta);
                headtrans.back().p.k = 0;                 //AccessCamera()->GetPosition().k;
                headtrans.back().p.i += warp_shake_mag*cos( wtheta )*sqr( warp_strength )*cockpitradial/100;           //AccessCamera()->GetPosition().i+shakin*cos(theta);
                headtrans.back().p.j += warp_shake_mag*cos( 1.165864*wtheta )*sqr( warp_strength )*cockpitradial/100;  //AccessCamera()->GetPosition().j+shakin*cos(theta);
                if (shakin > 0) {
                    shakin -= GetElapsedTime()*shake_reduction*(shakin/5);                       //Fast convergence to 5% shaking, slow stabilization
                    if (shakin <= 0)
                        shakin = 0;
                }
                //Now, compute head drift
                Vector caccel = AccessCamera( CP_FRONT )->GetAcceleration();
                float  mag    = caccel.Magnitude();
                float  ref    = drift_ref_accel*drift_ref_accel;
                if ( (mag > 0) && (ref > 0) ) caccel *= -( drift_amount*min( drift_limit, (float) (mag*mag/ref) ) )/mag;

                else caccel = Vector( 0, 0, 0 );
                float driftphase     = pow( 0.25, GetElapsedTime() );
                oaccel = (1-driftphase)*caccel+driftphase*oaccel;
                headtrans.back().p += -cockpitradial*oaccel;
                float driftmag = cockpitradial*oaccel.Magnitude();

                //if (COCKPITZ_PARTITIONS>1) GFXClear(GFXFALSE,GFXFALSE,GFXTRUE);//only clear stencil buffer
                static size_t COCKPITZ_PARTITIONS =
                    XMLSupport::parse_int( vs_config->getVariable( "graphics", "cockpit_z_partitions", "1" ) );                                         //Should not be needed if VERYNEAR_CONST is propperly set, but would be useful with stenciled inverse order rendering.
                float zrange = cockpitradial*(1-VERYNEAR_CONST)+driftmag;
                float zfloor = cockpitradial*VERYNEAR_CONST;
                for (j = COCKPITZ_PARTITIONS; j > 0; j--) { //FIXME This is a program lockup!!! (actually, no; j is a size_t...)
                    AccessCamera()->UpdateGFX( GFXTRUE,
                        GFXTRUE,
                        GFXTRUE,
                        GFXTRUE,
                        zfloor+zrange*(j-1)/COCKPITZ_PARTITIONS,
                        zfloor+zrange*j/COCKPITZ_PARTITIONS );                                                                                       //cockpit-specific frustrum (with clipping, with frustrum update)
                    for (i = 0; i < mesh.size(); ++i)
                        mesh[i]->Draw( FLT_MAX, headtrans.back() );

                    Mesh::ProcessZFarMeshes( true );
                    //if (nocockpitcull) GFXDisable( CULLFACE );
                    Mesh::ProcessUndrawnMeshes( false, true );
                }
                headtrans.pop_back();
                //if (COCKPITZ_PARTITIONS>1) GFXDisable(STENCIL);
                GFXDisable( LIGHTING );
                GFXDisable( TEXTURE0 );
                GFXDisable( TEXTURE1 );
                AccessCamera()->SetFov( oldfov );
            }
        }
        _Universe->AccessCamera()->UpdateGFX( GFXFALSE, GFXFALSE, GFXTRUE, GFXFALSE, 0, 1000000 );         //Restore normal frustrum
    }
    GFXHudMode( true );
    static float damage_flash_length = XMLSupport::parse_float( vs_config->getVariable( "graphics", "damage_flash_length", ".1" ) );
    static bool  damage_flash_first  = XMLSupport::parse_bool( vs_config->getVariable( "graphics", "flash_behind_hud", "true" ) );
    if (view < CP_CHASE && damage_flash_first && getNewTime()-shake_time < damage_flash_length)
        DrawDamageFlash( shake_type );
    GFXColor4f( 1, 1, 1, 1 );
    GFXBlendMode( ONE, ONE );
    GFXDisable( DEPTHTEST );
    GFXDisable( DEPTHWRITE );

    Unit *un;
    float crosscenx = 0, crossceny = 0;
    static bool crosshairs_on_chasecam =
        parse_bool( vs_config->getVariable( "graphics", "hud", "crosshairs_on_chasecam", "false" ) );
    static bool crosshairs_on_padlock  =
        parse_bool( vs_config->getVariable( "graphics", "hud", "crosshairs_on_padlock", "false" ) );
    if ( (view == CP_FRONT)
        || (view == CP_CHASE && crosshairs_on_chasecam)
        || ((view == CP_VIEWTARGET || view == CP_PANINSIDE) && crosshairs_on_padlock) ) {
        if (Panel.size() > 0 && Panel.front() && screenshotkey == false) {
            static bool drawCrosshairs =
                parse_bool( vs_config->getVariable( "graphics", "hud", "draw_rendered_crosshairs",
                                                   vs_config->getVariable( "graphics", "draw_rendered_crosshairs", "true" ) ) );
            if (drawCrosshairs) {
                float x, y, wid, hei;
                Panel.front()->GetPosition( x, y );
                Panel.front()->GetSize( wid, hei );
                DrawCrosshairs( x, y, wid, hei, textcol );
            } else if (!drawHeadingMarker) {
                GFXBlendMode( SRCALPHA, INVSRCALPHA );
                GFXEnable( TEXTURE0 );
                Panel.front()->Draw();                 //draw crosshairs
            }
        }
    }
    GFXBlendMode( SRCALPHA, INVSRCALPHA );
    GFXEnable( TEXTURE0 );

    RestoreViewPort();

    static bool  blend_panels  = XMLSupport::parse_bool( vs_config->getVariable( "graphics", "blend_panels", "false" ) );
    static bool  blend_cockpit = XMLSupport::parse_bool( vs_config->getVariable( "graphics", "blend_cockpit", "false" ) );
    static bool  drawChaseVDU  =
        XMLSupport::parse_bool( vs_config->getVariable( "graphics", "draw_vdus_from_chase_cam", "false" ) );
    static bool  drawPanVDU    =
        XMLSupport::parse_bool( vs_config->getVariable( "graphics", "draw_vdus_from_panning_cam", "false" ) );
    static bool  drawTgtVDU    =
        XMLSupport::parse_bool( vs_config->getVariable( "graphics", "draw_vdus_from_target_cam", "false" ) );
    static bool  drawPadVDU    =
        XMLSupport::parse_bool( vs_config->getVariable( "graphics", "draw_vdus_from_padlock_cam", "false" ) );

    static bool  drawChasecp   =
        XMLSupport::parse_bool( vs_config->getVariable( "graphics", "draw_cockpit_from_chase_cam", "false" ) );
    static bool  drawPancp     =
        XMLSupport::parse_bool( vs_config->getVariable( "graphics", "draw_cockpit_from_panning_cam", "false" ) );
    static bool  drawTgtcp     =
        XMLSupport::parse_bool( vs_config->getVariable( "graphics", "draw_cockpit_from_target_cam", "false" ) );
    static bool  drawPadcp     =
        XMLSupport::parse_bool( vs_config->getVariable( "graphics", "draw_cockpit_from_padlock_cam", "false" ) );

    static float AlphaTestingCutoff = XMLSupport::parse_float( vs_config->getVariable( "graphics", "AlphaTestCutoff", ".8" ) );
    if (blend_cockpit) {
        GFXAlphaTest( ALWAYS, 0 );
        GFXBlendMode( SRCALPHA, INVSRCALPHA );
    } else {
        GFXBlendMode( ONE, ZERO );
        GFXAlphaTest( GREATER, AlphaTestingCutoff );
    }
    GFXColor4f( 1, 1, 1, 1 );
    if (view < CP_CHASE) {
        if (Pit[view])
            Pit[view]->Draw();
    } else if ( (view == CP_CHASE
                 && drawChasecp)
               || (view == CP_PAN && drawPancp) || (view == CP_TARGET && drawTgtcp) || ((view == CP_VIEWTARGET || view == CP_PANINSIDE) && drawPadcp) ) {
        if (Pit[0])
            Pit[0]->Draw();
    }
    if (blend_panels) {
        GFXAlphaTest( ALWAYS, 0 );
        GFXBlendMode( SRCALPHA, INVSRCALPHA );
    } else {
        GFXBlendMode( ONE, ZERO );
        GFXAlphaTest( GREATER, AlphaTestingCutoff );
    }
    GFXColor4f( 1, 1, 1, 1 );
    if ( view == CP_FRONT
        || (view == CP_CHASE
            && drawChaseVDU)
        || (view == CP_PAN && drawPanVDU) || (view == CP_TARGET && drawTgtVDU) || ((view == CP_VIEWTARGET || view == CP_PANINSIDE) && drawPadVDU) ) {
        for (unsigned int j = 1; j < Panel.size(); j++)
            if (Panel[j])
                Panel[j]->Draw();
    }
    GFXAlphaTest( ALWAYS, 0 );
    GFXBlendMode( SRCALPHA, INVSRCALPHA );
    GFXColor4f( 1, 1, 1, 1 );
    bool die = true;
    //draw target gauges
    for (unsigned int vd = 0; vd < vdu.size(); vd++)
        if (vdu[vd]->getMode() == VDU::TARGET) {
            if ( ( un = parent.GetUnit() ) ) {
                Unit *target = parent.GetUnit()->Target();
                if (target != NULL) {
                    if ( view == CP_FRONT
                        || (view == CP_CHASE
                            && drawChaseVDU)
                        || (view == CP_PAN
                            && drawPanVDU) || (view == CP_TARGET && drawTgtVDU) || ((view == CP_VIEWTARGET || view == CP_PANINSIDE) && drawPadVDU) )                                                                //{ //only draw crosshairs for front view
                        //if (!UnitUtil::isSignificant(target)&&!UnitUtil::isSun(target)||UnitUtil::isCapitalShip(target)) //{
                        DrawTargetGauges( target, gauges );
                }
            }
        }
    //draw unit gauges
    if ( ( un = parent.GetUnit() ) ) {
        switch (view) {
        case CP_FRONT:
        case CP_LEFT:
        case CP_RIGHT:
        case CP_BACK:
        case CP_VIEWTARGET:
        case CP_PANINSIDE:
            TriggerEvents( un );
            break;
        default:
            break;
        };
        if ( view == CP_FRONT
            || (view == CP_CHASE
                && drawChaseVDU)
            || (view == CP_PAN && drawPanVDU) || (view == CP_TARGET && drawTgtVDU) || ((view == CP_VIEWTARGET || view == CP_PANINSIDE) && drawPadVDU) ) {
            //only draw crosshairs for front view
            DrawGauges( this, un, gauges, gauge_time, cockpit_time, text, textcol );
            Radar::Sensor sensor(un);
            DrawRadar(sensor, cockpit_time, radar_time, radarSprites, radarDisplay.get());

            GFXColor4f( 1, 1, 1, 1 );
            for (unsigned int vd = 0; vd < vdu.size(); vd++)
                if (vdu[vd]) {
                    vdu[vd]->Draw( this, un, textcol );
                    GFXColor4f( 1, 1, 1, 1 );
                    float damage = un->GetImageInformation().cockpit_damage[(1+vd)%(MAXVDUS+1)];
                    if ( vdu[vd]->staticable() ) {
                        if (damage < .985) {
                            if (vdu_time[vd] >= 0) {
                                if ( damage > .001 && ( cockpit_time > ( vdu_time[vd]+(1-damage) ) ) )
                                    if (rand01() > SWITCH_CONST)
                                        vdu_time[vd] = -cockpit_time;
                                /*else {
                                 *  static string vdustatic=vs_config->getVariable("graphics","vdu_static","static.ani");
                                 *  static Animation vdu_ani(vdustatic.c_str(),true,.1,BILINEAR);
                                 *  static soundContainer ejectstopsound;
                                 *  if (ejectstopsound.sound<0) {
                                 *  static string str=vs_config->getVariable("cockpitaudio","vdu_static","vdu_static");
                                 *  ejectstopsound.loadsound(str);
                                 *  }
                                 *  if (!AUDIsPlaying(ejectstopsound.sound)) {
                                 *  ejectstopsound.playsound();
                                 *  }
                                 *
                                 *  GFXEnable(TEXTURE0);
                                 *  vdu_ani.DrawAsVSSprite(vdu[vd]);
                                 *
                                 *  }*/
                            } else if ( cockpit_time > ( ( 1-(-vdu_time[vd]) )+(damage) ) ) {
                                if (rand01() > SWITCH_CONST)
                                    vdu_time[vd] = cockpit_time;
                            }
                        }
                    }
                }
            //process VDU, damage VDU, targetting VDU
            //////////////////// DISPLAY CURRENT POSITION ////////////////////
            static bool debug_position =
                XMLSupport::parse_bool( vs_config->getVariable( "graphics", "hud", "debug_position", "false" ) );
            if (debug_position) {
                TextPlane tp;
                char str[400];                 //don't make the sprintf format too big... :-P
                Unit     *you = parent.GetUnit();
                if (you) {
                    sprintf( str, "Your Position: (%lf,%lf,%lf); Velocity: (%f,%f,%f); Frame: %lf\n",
                            you->curr_physical_state.position.i, you->curr_physical_state.position.j,
                            you->curr_physical_state.position.k, you->Velocity.i, you->Velocity.j, you->Velocity.k,
                            getNewTime() );
                    Unit *yourtarg = you->computer.target.GetUnit();
                    if (yourtarg) {
                        sprintf( str+strlen(
                                    str ), "Target Position: (%lf,%lf,%lf); Velocity: (%f,%f,%f); Now: %lf\n",
                                yourtarg->curr_physical_state.position.i, yourtarg->curr_physical_state.position.j,
                                yourtarg->curr_physical_state.position.k, yourtarg->Velocity.i, yourtarg->Velocity.j,
                                yourtarg->Velocity.k, queryTime() );
                    }
                }
                tp.SetPos( -0.8, -0.8 );
                tp.SetText( str );
                tp.Draw();
            }
            //////////////////////////////////////////////////////////////////////////
        }
        GFXColor4f( 1, 1, 1, 1 );
        if (un->GetHull() >= 0)
            die = false;
        if (un->Threat() != NULL) {
            if (0 && getTimeCompression() > 1)
                reset_time_compression( std::string(), PRESS );
            un->Threaten( NULL, 0 );
        }
        if ( _Universe->CurrentCockpit() < univmap.size() )
            univmap[_Universe->CurrentCockpit()].Draw();
        //Draw the arrow to the target.
        static bool drawarrow =
            XMLSupport::parse_bool( vs_config->getVariable( "graphics", "hud", "draw_arrow_to_target", "true" ) );
        static bool drawarrow_on_pancam   =
            XMLSupport::parse_bool( vs_config->getVariable( "graphics", "hud", "draw_arrow_on_pancam", "false" ) );
        static bool drawarrow_on_pantgt   =
            XMLSupport::parse_bool( vs_config->getVariable( "graphics", "hud", "draw_arrow_on_pantgt", "false" ) );
        static bool drawarrow_on_chasecam =
            XMLSupport::parse_bool( vs_config->getVariable( "graphics", "hud", "draw_arrow_on_chasecam", "true" ) );
        {
            Unit *parent = NULL;
            if ( drawarrow && ( parent = this->parent.GetUnit() ) ) {
                Radar::Sensor sensor(parent);
                if ( (view == CP_PAN
                      && !drawarrow_on_pancam)
                    || (view == CP_PANTARGET
                        && !drawarrow_on_pantgt) || (view == CP_CHASE && !drawarrow_on_chasecam) ) {} else {
                    DrawArrowToTarget(sensor, parent->Target(), projection_limit_x, projection_limit_y,
                                      inv_screen_aspect_ratio);
                    if ( draw_star_destination_arrow
                        && (destination_system_location.i || destination_system_location.j
                            || destination_system_location.k) ) {
                        GFXColorf( destination_system_color );
                        DrawArrowToTarget(sensor, parent->ToLocalCoordinates(destination_system_location),
                                          projection_limit_x, projection_limit_y,
                                          inv_screen_aspect_ratio);
                    }
                }
            }
        }         //end: draw arrow
    }
    AutoLanding();
    GFXColor4f( 1, 1, 1, 1 );
    if (QuitAllow || getTimeCompression() < .5) {
        if (QuitAllow) {
            if (!die) {
                static VSSprite QuitSprite( "quit.sprite", BILINEAR, GFXTRUE );
                static VSSprite QuitCompatSprite( "quit.spr", BILINEAR, GFXTRUE );

                GFXEnable( TEXTURE0 );
                if ( QuitSprite.LoadSuccess() )
                    QuitSprite.Draw();
                else
                    QuitCompatSprite.Draw();
            }
        } else {
            static VSSprite PauseSprite( "pause.sprite", BILINEAR, GFXTRUE );
            static VSSprite PauseCompatSprite( "pause.spr", BILINEAR, GFXTRUE );
            GFXEnable( TEXTURE0 );
            if ( PauseSprite.LoadSuccess() )
                PauseSprite.Draw();
            else
                PauseCompatSprite.Draw();
        }
    }
    static float dietime = 0;
    if (die) {
        if (un)
            if (un->GetHull() >= 0)
                die = false;
        if (die) {
            if (text) {
                GFXColor4f( 1, 1, 1, 1 );
                text->SetSize( 1, -1 );
                float x;
                float y;
                if (dietime == 0) {
                    editingTextMessage = false;
                    RestoreKB();
                    if ( respawnunit.size() > _Universe->CurrentCockpit() )
                        if (respawnunit[_Universe->CurrentCockpit()] == 1 && _Universe->numPlayers() == 1)
                            respawnunit[_Universe->CurrentCockpit()] = 0;
                    text->GetCharSize( x, y );
                    text->SetCharSize( x*4, y*4 );
                    text->SetPos( 0-(x*2*14), 0-(y*2) );
                }
                GFXColorf( textcol );
                static bool show_died_text =
                    XMLSupport::parse_bool( vs_config->getVariable( "graphics", "show_respawn_text", "false" ) );
                if (show_died_text)
                    text->Draw(
                        "#ff5555You Have Died!\n#000000Press #8080FF;#000000 (semicolon) to respawn\nOr Press #8080FFEsc and 'q'#000000 to quit" );
                GFXColor4f( 1, 1, 1, 1 );

                static float min_die_time =
                    XMLSupport::parse_float( vs_config->getVariable( "graphics", "death_scene_time", "4" ) );
                if (dietime > min_die_time) {
                    static std::string death_menu_script = vs_config->getVariable( "graphics", "death_menu_script", "" );
                    if ( death_menu_script.empty() ) {
                        static VSSprite DieSprite( "died.sprite", BILINEAR, GFXTRUE );
                        static VSSprite DieCompatSprite( "died.spr", BILINEAR, GFXTRUE );
                        GFXBlendMode( SRCALPHA, INVSRCALPHA );
                        GFXEnable( TEXTURE0 );
                        if ( DieSprite.LoadSuccess() )
                            DieSprite.Draw();
                        else
                            DieCompatSprite.Draw();
                    } else {
                        BaseUtil::LoadBaseInterface( death_menu_script );
                        dietime = 0;
                    }
                }
                dietime += GetElapsedTime();
                SetView( CP_PAN );
                CockpitKeys::YawLeft( std::string(), RELEASE );
                CockpitKeys::YawRight( std::string(), RELEASE );
                CockpitKeys::PitchUp( std::string(), RELEASE );
                CockpitKeys::PitchDown( std::string(), RELEASE );
                zoomfactor = dietime*10;
            }
            QuitAllow = true;
        } else {}
    } else if (dietime != 0) {
        QuitAllow = false;
        dietime   = 0;
    }
    //if(CommandInterpretor.console){
    //GFXColorf(textcol);
    //CommandInterpretor.renderconsole();
    //}
    GFXAlphaTest( ALWAYS, 0 );
    static bool mouseCursor = XMLSupport::parse_bool( vs_config->getVariable( "joystick", "mouse_cursor", "false" ) );
    static bool mousecursor_pancam   =
        XMLSupport::parse_bool( vs_config->getVariable( "joystick", "mouse_cursor_pancam", "false" ) );
    static bool mousecursor_pantgt   =
        XMLSupport::parse_bool( vs_config->getVariable( "joystick", "mouse_cursor_pantgt", "false" ) );
    static bool mousecursor_chasecam =
        XMLSupport::parse_bool( vs_config->getVariable( "joystick", "mouse_cursor_chasecam", "true" ) );
    if (mouseCursor && screenshotkey == false) {
        if ( (view == CP_PAN
              && !mousecursor_pancam)
            || (view == CP_PANTARGET && !mousecursor_pantgt) || (view == CP_CHASE && !mousecursor_chasecam) ) {} else {
            GFXBlendMode( SRCALPHA, INVSRCALPHA );
            GFXColor4f( 1, 1, 1, 1 );
            GFXEnable( TEXTURE0 );
            //GFXDisable (DEPTHTEST);
            //GFXDisable(TEXTURE1);
            static float    deadband = game_options.mouse_deadband;
            static int      revspr   =
                XMLSupport::parse_bool( vs_config->getVariable( "joystick", "reverse_mouse_spr", "true" ) ) ? 1 : -1;
            static string   blah     = vs_config->getVariable( "joystick", "mouse_crosshair", "crosshairs.spr" );
            static VSSprite MouseVSSprite( blah.c_str(), BILINEAR, GFXTRUE );
            float xcoord = ( -1+float(mousex)/(.5*g_game.x_resolution) );
            float ycoord = ( -revspr+float(revspr*mousey)/(.5*g_game.y_resolution) );
            MouseVSSprite.SetPosition( xcoord*( 1-fabs( crosscenx ) )+crosscenx, ycoord*( 1-fabs( crossceny ) )+crossceny );
            float xs, ys;
            MouseVSSprite.GetSize( xs, ys );
            if (xcoord < deadband && ycoord < deadband && xcoord > -deadband && ycoord > -deadband) {
                //The other option would be to place it in the center.
                //but it's sometimes useful to know where the mouse actually is.
                MouseVSSprite.SetSize( xs/2, ys/2 );
            } else if (xcoord < deadband && xcoord > -deadband) {
                MouseVSSprite.SetSize( xs/2, ys*5/6 );
            } else if (ycoord < deadband && ycoord > -deadband) {
                MouseVSSprite.SetSize( xs*5/6, ys/2 );
            }
            MouseVSSprite.Draw();
            MouseVSSprite.SetSize( xs, ys );
            //DrawGlutMouse(mousex,mousey,&MouseVSSprite);
            //DrawGlutMouse(mousex,mousey,&MouseVSSprite);
        }
    }
    if (view < CP_CHASE && damage_flash_first == false && getNewTime()-shake_time < damage_flash_length)
        DrawDamageFlash( shake_type );
    GFXHudMode( false );

    {
        //again, NAV computer is unaffected by FOV WARP-Link
        float oldfov = AccessCamera()->GetFov();
        AccessCamera()->SetFov( g_game.fov );
        AccessCamera()->UpdateGFXAgain();
        DrawNavSystem(&ThisNav, AccessCamera(), cockpit_offset);
        AccessCamera()->SetFov( oldfov );
        AccessCamera()->UpdateGFXAgain();
    }

    GFXEnable( DEPTHWRITE );
    GFXEnable( DEPTHTEST );
    GFXEnable( TEXTURE0 );
}

int Cockpit::getScrollOffset( unsigned int whichtype )
{
    for (unsigned int i = 0; i < vdu.size(); i++)
        if (vdu[i]->getMode()&whichtype)
            return vdu[i]->scrolloffset;
    return 0;
}



#include <algorithm>

void SetStartupView( Cockpit* );

void Cockpit::UpdAutoPilot()
{
    static bool autopan = XMLSupport::parse_bool( vs_config->getVariable( "graphics", "pan_on_auto", "true" ) );
    if (autopilot_time != 0) {
        autopilot_time -= SIMULATION_ATOM;
        {
            if (autopan) {
                Vector origR = Vector( 0, 0, 1 );
                Vector origP = Vector( 1, 0, 0 );

                static float rotspd  =
                    XMLSupport::parse_float( vs_config->getVariable( "graphics", "autopilot_rotation_speed", ".15" ) );

                static float curtime = 0;
                curtime += SIMULATION_ATOM;
                float  ang   = curtime*rotspd;
                origR.Yaw( ang );
                origP.Yaw( ang );
                Vector origQ = Vector( 0, 1, 0 );
                origP.Normalize();
                origQ.Normalize();
                origR.Normalize();
                AccessCamera( CP_FIXED )->myPhysics.SetAngularVelocity( Vector( 0, 0, 0 ) );                 //hack
                static float initialzoom =
                    XMLSupport::parse_float( vs_config->getVariable( "graphics", "inital_zoom_factor", "2.25" ) );
                zoomfactor = initialzoom;
            }
        }
        if (autopilot_time <= 0) {
            AccessCamera( CP_FIXED )->myPhysics.SetAngularVelocity( Vector( 0, 0, 0 ) );
            if (disableautosound.sound < 0) {
                static string str = vs_config->getVariable( "cockpitaudio", "autopilot_disabled", "autopilot_disabled" );
                disableautosound.loadsound( str );
            }
            disableautosound.playsound();
            if (autopan) {
                AccessCamera( CP_FIXED )->myPhysics.SetAngularVelocity( Vector( 0, 0, 0 ) );
                SetStartupView( this );
            }
            autopilot_time = 0;
            Unit *par = GetParent();
            if (par) {
                Unit *autoun = autopilot_target.GetUnit();
                autopilot_target.SetUnit( NULL );
                if (autoun && autopan)
                    par->AutoPilotTo( autoun, false );
            }
        }
    }
}

void SwitchUnits2( Unit *nw )
{
    if (nw) {
        nw->PrimeOrders();
        nw->EnqueueAI( new FireKeyboard( _Universe->CurrentCockpit(), _Universe->CurrentCockpit() ) );
        nw->EnqueueAI( new FlyByJoystick( _Universe->CurrentCockpit() ) );

        nw->SetTurretAI();
        nw->DisableTurretAI();

        static bool LoadNewCockpit =
            XMLSupport::parse_bool( vs_config->getVariable( "graphics", "UnitSwitchCockpitChange", "false" ) );
        static bool DisCockpit     =
            XMLSupport::parse_bool( vs_config->getVariable( "graphics", "SwitchCockpitToDefaultOnUnitSwitch", "false" ) );
        if (nw->getCockpit().length() > 0 || DisCockpit)
            _Universe->AccessCockpit()->Init( nw->getCockpit().c_str(), LoadNewCockpit == false );
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

Cockpit::~Cockpit()
{
    Delete();
    int i;
    for (i = 0; i < 4; i++)
        if (Pit[i]) {
            delete Pit[i];
            Pit[i] = NULL;
        }
    delete savegame;
}

int Cockpit::getVDUMode( int vdunum )
{
    if ( vdunum < (int) vdu.size() )
        if (vdu[vdunum])
            return vdu[vdunum]->getMode();
    return 0;
}

void Cockpit::VDUSwitch( int vdunum )
{
    if (soundfile >= 0)
        //AUDPlay (soundfile, AccessCamera()->GetPosition(), Vector (0,0,0), .5);
        AUDPlay( soundfile, QVector( 0, 0, 0 ), Vector( 0, 0, 0 ), 1 );
    if ( vdunum < (int) vdu.size() )
        if (vdu[vdunum])
            vdu[vdunum]->SwitchMode( this->parent.GetUnit() );
}

void Cockpit::ScrollVDU( int vdunum, int howmuch )
{
    if (soundfile >= 0)
        //AUDPlay (soundfile, AccessCamera()->GetPosition(), Vector (0,0,0),.5);
        AUDPlay( soundfile, QVector( 0, 0, 0 ), Vector( 0, 0, 0 ), 1 );
    if ( vdunum < (int) vdu.size() )
        if (vdu[vdunum])
            vdu[vdunum]->Scroll( howmuch );
}

void Cockpit::ScrollAllVDU( int howmuch )
{
    if ( ThisNav.CheckDraw() )
        ThisNav.scroll( howmuch );
    else
        for (unsigned int i = 0; i < vdu.size(); i++)
            ScrollVDU( i, howmuch );
}

void Cockpit::SetStaticAnimation()
{
    static string    comm_static = vs_config->getVariable( "graphics", "comm_static", "static.ani" );
    static Animation Statuc( comm_static.c_str() );
    for (unsigned int i = 0; i < vdu.size(); i++)
        if (vdu[i]->getMode() == VDU::COMM)
            vdu[i]->SetCommAnimation( &Statuc, NULL, true );
}

void Cockpit::SetCommAnimation( Animation *ani, Unit *un )
{
    bool seti = false;
    for (unsigned int i = 0; i < vdu.size(); i++)
        if ( vdu[i]->SetCommAnimation( ani, un, false ) ) {
            seti = true;
            break;
        }
    if (!seti) {
        for (unsigned int i = 0; i < vdu.size(); i++)
            if ( vdu[i]->SetCommAnimation( ani, un, true ) )
                break;
    }
}

string Cockpit::getTargetLabel()
{
    Unit *par = GetParent();
    if ( ( !targetLabel.empty() )
        && (!par || ( (void*) par->Target() ) != labeledTargetUnit) ) {
        targetLabel = string();
        if (par)
            labeledTargetUnit = par->Target();
    }
    return targetLabel;
}

void Cockpit::setTargetLabel( const string &newLabel )
{
    if ( GetParent() ) {
        targetLabel = newLabel;
        labeledTargetUnit = GetParent()->Target();
    }
}

void Cockpit::RestoreViewPort()
{
    _Universe->AccessCamera()->RestoreViewPort( 0, 0 );
}

static void FaceCamTarget( Cockpit *cp, int cam, Unit *un )
{
    QVector diff = un->Position()-cp->AccessCamera()->GetPosition();
    diff.Normalize();
    if (diff.i != 0 && diff.k != 0) {
        Vector z = diff.Cross( QVector( 0, 1, 0 ) ).Cast();
        cp->AccessCamera( cam )->SetOrientation( z, Vector( 0, 1, 0 ), diff.Cast() );
    }
}

static void ShoveCamBehindUnit( int cam, Unit *un, float zoomfactor )
{
    //commented out by chuck_starchaser; --never used
    QVector unpos = (/*un->GetPlanetOrbit() && !un->isSubUnit()*/ NULL) ? un->LocalPosition() : un->Position();
    _Universe->AccessCamera( cam )->SetPosition(
        unpos-_Universe->AccessCamera()->GetR().Cast()*(un->rSize()+g_game.znear*2)*zoomfactor,
        un->GetWarpVelocity(), un->GetAngularVelocity(), un->GetAcceleration() );
}

static void ShoveCamBelowUnit( int cam, Unit *un, float zoomfactor )
{
    //commented out by chuck_starchaser; --never used
    QVector unpos = (/*un->GetPlanetOrbit() && !un->isSubUnit()*/ NULL) ? un->LocalPosition() : un->Position();
    Vector  p, q, r;
    _Universe->AccessCamera( cam )->GetOrientation( p, q, r );
    static float ammttoshovecam = XMLSupport::parse_float( vs_config->getVariable( "graphics", "shove_camera_down", ".3" ) );
    _Universe->AccessCamera( cam )->SetPosition(
        unpos-(r-ammttoshovecam*q).Cast()*(un->rSize()+g_game.znear*2)*zoomfactor, un->GetWarpVelocity(),
        un->GetAngularVelocity(), un->GetAcceleration() );
}

static Vector lerp( const Vector &a, const Vector &b, float t )
{
    t = min( 1.0f, max( 0.0f, t ) );
    return a*(1-t)+b*t;
}

static void translate_as( Vector &p, Vector &q, Vector &r, Vector p1, Vector q1, Vector r1, Vector p2, Vector q2, Vector r2 )
{
    //Translate p,q,r to <p1,q1,r1> base
    p = Vector( p.Dot( p1 ), p.Dot( q1 ), p.Dot( r1 ) );
    q = Vector( q.Dot( p1 ), q.Dot( q1 ), q.Dot( r1 ) );
    r = Vector( r.Dot( p1 ), r.Dot( q1 ), r.Dot( r1 ) );
    //Interpret now as if it were in <p2,q2,r2> base
    p = p2*p.i+q2*p.j+r2*p.k;
    q = p2*q.i+q2*q.j+r2*q.k;
    r = p2*r.i+q2*r.j+r2*r.k;
}

void Cockpit::SetupViewPort( bool clip )
{
    _Universe->AccessCamera()->RestoreViewPort( 0, (view == CP_FRONT ? viewport_offset : 0) );
    GFXViewPort( 0,
                 (int) ( (view == CP_FRONT ? viewport_offset : 0)*g_game.y_resolution ),
                 g_game.x_resolution,
                 g_game.y_resolution );
    _Universe->AccessCamera()->setCockpitOffset( view < CP_CHASE ? cockpit_offset : 0 );
    Unit *un, *tgt;
    if ( ( un = parent.GetUnit() ) ) {
        //Previous frontal orientation - useful, sometimes...
        Vector prev_fp, prev_fq, prev_fr;
        _Universe->AccessCamera( CP_FRONT )->GetOrientation( prev_fp, prev_fq, prev_fr );

        un->UpdateHudMatrix( CP_FRONT );
        un->UpdateHudMatrix( CP_LEFT );
        un->UpdateHudMatrix( CP_RIGHT );
        un->UpdateHudMatrix( CP_BACK );
        un->UpdateHudMatrix( CP_CHASE );
        un->UpdateHudMatrix( CP_PANINSIDE );

        insidePanYaw += insidePanYawSpeed * GetElapsedTime();
        insidePanPitch += insidePanPitchSpeed * GetElapsedTime();

        Vector p, q, r, tmp;
        _Universe->AccessCamera( CP_FRONT )->GetOrientation( p, q, r );
        _Universe->AccessCamera( CP_LEFT )->SetOrientation( r, q, -p );
        _Universe->AccessCamera( CP_RIGHT )->SetOrientation( -r, q, p );
        _Universe->AccessCamera( CP_BACK )->SetOrientation( -p, q, -r );
#ifdef IWANTTOPVIEW
        _Universe->AccessCamera( CP_CHASE )->SetOrientation( p, r, -q );
#endif

        Matrix pitchMatrix, yawMatrix, panMatrix;
        RotateAxisAngle(pitchMatrix, Vector( 1, 0, 0 ), insidePanPitch);
        RotateAxisAngle(yawMatrix, Transform(pitchMatrix, Vector( 0, 1, 0 )), insidePanYaw);
        panMatrix = yawMatrix * pitchMatrix;
        _Universe->AccessCamera( CP_PANINSIDE )->SetOrientation(
            Transform(panMatrix, p),
            Transform(panMatrix, q),
            Transform(panMatrix, r) );

        tgt = un->Target();
        if (tgt) {
            un->GetOrientation( p, q, r );
            r = ( tgt->Position()-un->Position() ).Cast();
            r.Normalize();
            CrossProduct( r, q, tmp );
            CrossProduct( tmp, r, q );
            //Padlock block
            if (view == CP_VIEWTARGET) {
                static float PadlockViewLag =
                    XMLSupport::parse_float( vs_config->getVariable( "graphics", "hud", "PadlockViewLag", "1.5" ) );
                static float PadlockViewLag_inv    = 1.f/PadlockViewLag;
                static float PadlockViewLag_fix    =
                    XMLSupport::parse_float( vs_config->getVariable( "graphics", "hud", "PadlockViewLagFixZone", "0.0872" ) );                                            //~5 deg
                static float PadlockViewLag_fixcos = (float) cos( PadlockViewLag_fix );

                //pp,qq,rr <-- world-relative padlock target
                //p_p,p_q,p_r <-- previous head orientation translated to new front orientation
                Vector p_p, p_q, p_r, f_p, f_q, f_r, pp = tmp, qq = q, rr = r;
                _Universe->AccessCamera( CP_VIEWTARGET )->GetOrientation( p_p, p_q, p_r );
                _Universe->AccessCamera( CP_FRONT )->GetOrientation( f_p, f_q, f_r );
                translate_as( p_p, p_q, p_r, prev_fp, prev_fq, prev_fr, f_p, f_q, f_r );

                //Compute correction amount (vtphase), accounting for lag and fix-zone
                un->UpdateHudMatrix( CP_VIEWTARGET );
                bool  fixzone = (rr.Dot( p_r ) >= PadlockViewLag_fixcos) && (qq.Dot( p_q ) >= PadlockViewLag_fixcos);
                float vtphase = 1.0f-(float) pow( 0.1, GetElapsedTime()*PadlockViewLag_inv*(fixzone ? 0.1f : 1.0f) );

                //Apply correction
                _Universe->AccessCamera( CP_VIEWTARGET )->SetOrientation(
                    lerp( p_p, pp, vtphase ).Normalize(),
                    lerp( p_q, qq, vtphase ).Normalize(),
                    lerp( p_r, rr, vtphase ).Normalize() );
            } else {
                //Reset padlock matrix
                un->UpdateHudMatrix( CP_VIEWTARGET );
            }
            _Universe->AccessCamera( CP_TARGET )->SetOrientation( tmp, q, r );
            //_Universe->AccessCamera(CP_PANTARGET)->SetOrientation(tmp,q,r);
            ShoveCamBelowUnit( CP_TARGET, un, zoomfactor );
            ShoveCamBehindUnit( CP_PANTARGET, tgt, zoomfactor );
            FaceCamTarget( this, CP_FIXEDPOSTARGET, tgt );
        } else {
            un->UpdateHudMatrix( CP_VIEWTARGET );
            un->UpdateHudMatrix( CP_TARGET );
            un->UpdateHudMatrix( CP_PANTARGET );
        }
        static bool draw_unit_on_chasecam =
            XMLSupport::parse_bool( vs_config->getVariable( "graphics", "hud", "draw_unit_on_chasecam", "true" ) );
        if (view == CP_CHASE && !draw_unit_on_chasecam) {} else {
            ShoveCamBelowUnit( CP_CHASE, un, zoomfactor );
            //ShoveCamBehindUnit (CP_PANTARGET,un,zoomfactor);
        }
        FaceCamTarget( this, CP_FIXEDPOS, un );

        ShoveCamBehindUnit( CP_PAN, un, zoomfactor );
        un->SetVisible( view >= CP_CHASE );

        //WARP-FOV link
        {
            static float stable_lowarpref    =
                XMLSupport::parse_float( vs_config->getVariable( "graphics", "warp.fovlink.stable.loref", "1" ) );
            static float stable_hiwarpref    =
                XMLSupport::parse_float( vs_config->getVariable( "graphics", "warp.fovlink.stable.hiref", "100000" ) );
            static float stable_refexp       =
                XMLSupport::parse_float( vs_config->getVariable( "graphics", "warp.fovlink.stable.exp", "0.5" ) );
            static bool  stable_asymptotic   =
                XMLSupport::parse_bool( vs_config->getVariable( "graphics", "warp.fovlink.stable.asymptotic", "1" ) );
            static float stable_offset_f     =
                XMLSupport::parse_float( vs_config->getVariable( "graphics", "warp.fovlink.stable.offset.front", "0" ) );
            static float stable_offset_b     =
                XMLSupport::parse_float( vs_config->getVariable( "graphics", "warp.fovlink.stable.offset.back", "0" ) );
            static float stable_offset_p     =
                XMLSupport::parse_float( vs_config->getVariable( "graphics", "warp.fovlink.stable.offset.perpendicular", "0" ) );
            static float stable_multiplier_f =
                XMLSupport::parse_float( vs_config->getVariable( "graphics", "warp.fovlink.stable.multiplier.front", "0.85" ) );
            static float stable_multiplier_b =
                XMLSupport::parse_float( vs_config->getVariable( "graphics", "warp.fovlink.stable.multiplier.back", "1.5" ) );
            static float stable_multiplier_p =
                XMLSupport::parse_float( vs_config->getVariable( "graphics", "warp.fovlink.stable.multiplier.perpendicular",
                                                                 "1.25" ) );

            static float shake_lowarpref    =
                XMLSupport::parse_float( vs_config->getVariable( "graphics", "warp.fovlink.shake.loref", "10000" ) );
            static float shake_hiwarpref    =
                XMLSupport::parse_float( vs_config->getVariable( "graphics", "warp.fovlink.shake.hiref", "200000" ) );
            static float shake_refexp       =
                XMLSupport::parse_float( vs_config->getVariable( "graphics", "warp.fovlink.shake.exp", "1.5" ) );
            static bool  shake_asymptotic   =
                XMLSupport::parse_bool( vs_config->getVariable( "graphics", "warp.fovlink.shake.asymptotic", "1" ) );
            static float shake_speed        =
                XMLSupport::parse_float( vs_config->getVariable( "graphics", "warp.fovlink.shake.speed", "10" ) );
            static float shake_offset_f     =
                XMLSupport::parse_float( vs_config->getVariable( "graphics", "warp.fovlink.shake.offset.front", "0" ) );
            static float shake_offset_b     =
                XMLSupport::parse_float( vs_config->getVariable( "graphics", "warp.fovlink.shake.offset.back", "0" ) );
            static float shake_offset_p     =
                XMLSupport::parse_float( vs_config->getVariable( "graphics", "warp.fovlink.shake.offset.perpendicular", "0" ) );
            static float shake_multiplier_f =
                XMLSupport::parse_float( vs_config->getVariable( "graphics", "warp.fovlink.shake.multiplier.front", "0" ) );
            static float shake_multiplier_b =
                XMLSupport::parse_float( vs_config->getVariable( "graphics", "warp.fovlink.shake.multiplier.back", "0" ) );
            static float shake_multiplier_p =
                XMLSupport::parse_float( vs_config->getVariable( "graphics", "warp.fovlink.shake.multiplier.perpendicular", "0" ) );

            static float refkpsoverride     =
                XMLSupport::parse_float( vs_config->getVariable( "graphics", "warp.fovlink.referencekps", "0" ) );                                       //0 means automatic

            static float theta = 0;
            theta += shake_speed*GetElapsedTime();
            if (stable_lowarpref == stable_hiwarpref) stable_hiwarpref = stable_lowarpref+1;
            if (shake_lowarpref == shake_hiwarpref) shake_hiwarpref = shake_lowarpref+1;
            float warpfieldstrength    = LookupUnitStat( UnitImages< void >::WARPFIELDSTRENGTH, un );
            float refkps = (refkpsoverride > 0) ? refkpsoverride : LookupUnitStat( UnitImages< void >::MAXCOMBATABKPS, un );               //This one is stable, as opposed to SETKPS - for full stability, use the override (user override of governor settings will create weird behaviour if done under SPEC)
            float kps = LookupUnitStat( UnitImages< void >::KPS, un );
            float st_warpfieldstrength =
                pow( (max( stable_lowarpref,
                          min( stable_asymptotic ? FLT_MAX : stable_hiwarpref,
                               warpfieldstrength ) )-stable_lowarpref)/(stable_hiwarpref-stable_lowarpref), stable_refexp );
            float sh_warpfieldstrength =
                pow( (max( shake_lowarpref, min( shake_asymptotic ? FLT_MAX : shake_hiwarpref,
                                                 warpfieldstrength ) )-shake_lowarpref)/(shake_hiwarpref-shake_lowarpref),
                    shake_refexp );
            float costheta = cos( theta );
            if (stable_asymptotic) st_warpfieldstrength = atan( st_warpfieldstrength );
            if (shake_asymptotic) sh_warpfieldstrength = atan( sh_warpfieldstrength );
            if (refkps <= 1) refkps = 1;
            if (kps > refkps) kps = refkps;
            float unv = un->GetVelocity().Magnitude();
            float camv     = _Universe->AccessCamera()->GetR().Magnitude();
            if (unv <= 1) unv = 1;
            if (camv <= 1) camv = 1;
            float cosangle = ( un->GetVelocity()*_Universe->AccessCamera()->GetR() )/(unv*camv)*(kps/refkps);
            float st_offs, sh_offs, st_mult, sh_mult;
            if (cosangle > 0) {
                st_offs = stable_offset_f*cosangle+stable_offset_p*(1-cosangle);
                sh_offs = shake_offset_f*cosangle+shake_offset_p*(1-cosangle);
                st_mult = stable_multiplier_f*cosangle+stable_multiplier_p*(1-cosangle);
                sh_mult = shake_multiplier_f*cosangle+shake_multiplier_p*(1-cosangle);
            } else {
                st_offs = stable_offset_b* -cosangle+stable_offset_p*(1+cosangle);
                sh_offs = shake_offset_b* -cosangle+shake_offset_p*(1+cosangle);
                st_mult = stable_multiplier_b* -cosangle+stable_multiplier_p*(1+cosangle);
                sh_mult = shake_multiplier_b* -cosangle+shake_multiplier_p*(1+cosangle);
            }
            st_offs *= st_warpfieldstrength;
            sh_offs *= sh_warpfieldstrength*costheta;
            st_mult  = (1-st_warpfieldstrength)+st_mult*st_warpfieldstrength;
            sh_mult *= sh_warpfieldstrength*costheta;
            static float fov_smoothing =
                XMLSupport::parse_float( vs_config->getVariable( "graphics", "warp.fovlink.smoothing", ".4" ) );
            float fov_smoot = pow( double(fov_smoothing), GetElapsedTime() );
            smooth_fov =
                min( 170.0f, max( 5.0f, (1-fov_smoot)*smooth_fov+fov_smoot*(g_game.fov*(st_mult+sh_mult)+st_offs+sh_offs) ) );
            _Universe->AccessCamera()->SetFov( smooth_fov );
        }
    }
    _Universe->AccessCamera()->UpdateGFX( clip ? GFXTRUE : GFXFALSE );

    //parent->UpdateHudMatrix();
}

void Cockpit::SelectCamera( int cam )
{
    if (cam < NUM_CAM && cam >= 0)
        currentcamera = cam;
}

Camera* Cockpit::AccessCamera( int num )
{
    if (num < NUM_CAM && num >= 0)
        return &cam[num];
    else
        return NULL;
}



bool Cockpit::CheckCommAnimation( Unit *un )
{
    for (unsigned int i = 0; i < vdu.size(); ++i)
        if ( vdu[i]->CheckCommAnimation( un ) )
            return true;
    return false;
}

bool Cockpit::IsPaused() const
{
    // stephengtuggy 2020-07-21 FIXME - I don't think this is correct
    return (GetElapsedTime() <= 0.001);
}

void Cockpit::OnPauseBegin()
{
    radarDisplay->OnPauseBegin();
}

void Cockpit::OnPauseEnd()
{
    radarDisplay->OnPauseEnd();
}

void Cockpit::updateRadar(Unit*ship) {
    if (ship)
    {
        // We may have bought a new radar brand while docked, so the actual
        // radar display is instantiated when we undock.
        switch (ship->GetComputerData().radar.GetBrand())
        {
        case Computer::RADARLIM::Brand::BUBBLE:
            radarDisplay = Radar::Factory(Radar::Type::BubbleDisplay);
            break;

        case Computer::RADARLIM::Brand::PLANE:
            radarDisplay = Radar::Factory(Radar::Type::PlaneDisplay);
            break;

        default:
            radarDisplay = Radar::Factory(Radar::Type::SphereDisplay);
            break;
        }
        // Send notification that I have undocked
        radarDisplay->OnDockEnd();
    }

}
void Cockpit::SetParent( Unit *unit, const char *filename, const char *unitmodname, const QVector &startloc ){
    if (unit->getFlightgroup() != NULL)
        fg = unit->getFlightgroup();
    activeStarSystem = _Universe->activeStarSystem();          //cannot switch to units in other star systems.
    parent.SetUnit( unit );
    savegame->SetPlayerLocation( startloc );
    if (filename[0] != '\0') {
        this->GetUnitFileName() = std::string( filename );
        this->unitmodname       = std::string( unitmodname );
    }
    if (unit) {
        this->unitfaction = unit->faction;
        unit->ArmorData( StartArmor );
        if (StartArmor[0] == 0) StartArmor[0] = 1;
        if (StartArmor[1] == 0) StartArmor[1] = 1;
        if (StartArmor[2] == 0) StartArmor[2] = 1;
        if (StartArmor[3] == 0) StartArmor[3] = 1;
        if (StartArmor[4] == 0) StartArmor[4] = 1;
        if (StartArmor[5] == 0) StartArmor[5] = 1;
        if (StartArmor[6] == 0) StartArmor[6] = 1;
        if (StartArmor[7] == 0) StartArmor[7] = 1;
        maxfuel = unit->fuelData();
        maxhull = unit->GetHull();
    }
    updateRadar(unit);
}
void Cockpit::OnDockEnd(Unit *station, Unit *ship)
{
    if (_Universe->isPlayerStarship(ship))
        updateRadar(ship);
}

void Cockpit::OnJumpBegin(Unit *ship)
{
    if (_Universe->isPlayerStarship(ship))
    {
        radarDisplay->OnJumpBegin();
    }
}

void Cockpit::OnJumpEnd(Unit *ship)
{
    if (_Universe->isPlayerStarship(ship))
    {
        radarDisplay->OnJumpEnd();
    }
}

void Cockpit::SetInsidePanYawSpeed( float speed )
{
    insidePanYawSpeed = speed;
}

void Cockpit::SetInsidePanPitchSpeed( float speed )
{
    insidePanPitchSpeed = speed;
}

SoundContainer* Cockpit::soundImpl(const SoundContainer &specs)
{
    return new AldrvSoundContainer(specs);
}

float Unit::computeLockingPercent()
{
    float most = -1024;
    for (int i = 0; i < getNumMounts(); i++)
        if ( mounts[i].type->type == WEAPON_TYPE::PROJECTILE
            || ( as_integer(mounts[i].type->size)
                &(as_integer(MOUNT_SIZE::SPECIALMISSILE)|
                  as_integer(MOUNT_SIZE::LIGHTMISSILE)|
                  as_integer(MOUNT_SIZE::MEDIUMMISSILE)|
                  as_integer(MOUNT_SIZE::HEAVYMISSILE)|
                  as_integer(MOUNT_SIZE::CAPSHIPLIGHTMISSILE)|
                  as_integer(MOUNT_SIZE::CAPSHIPHEAVYMISSILE)|
                  as_integer(MOUNT_SIZE::SPECIAL)) ) ) {
            if (mounts[i].status == Mount::ACTIVE && mounts[i].type->lock_time > 0) {
                float rat = mounts[i].time_to_lock/mounts[i].type->lock_time;
                if (rat < .99)
                    if (rat > most)
                        most = rat;
            }
        }
    return (most == -1024) ? 1 : most;
}

/********************************************************************************
 * Functions from cockpit_generic
 * *****************************************************************************/









using namespace VSFileSystem;




/*
 *  static vector <int> respawnunit;
 *  static vector <int> switchunit;
 *  static vector <int> turretcontrol;
 *  static vector <int> suicide;
 *  void RespawnNow (Cockpit * cp) {
 *  while (respawnunit.size()<=_Universe->numPlayers())
 *   respawnunit.push_back(0);
 *  for (unsigned int i=0;i<_Universe->numPlayers();i++) {
 *   if (_Universe->AccessCockpit(i)==cp) {
 *     respawnunit[i]=2;
 *   }
 *  }
 *  }
 *  void Cockpit::SwitchControl (const KBData&,KBSTATE k) {
 *  if (k==PRESS) {
 *   while (switchunit.size()<=_Universe->CurrentCockpit())
 *     switchunit.push_back(0);
 *   switchunit[_Universe->CurrentCockpit()]=1;
 *  }
 *
 *  }
 *
 *  void Cockpit::Respawn (const KBData&,KBSTATE k) {
 *  if (k==PRESS) {
 *   while (respawnunit.size()<=_Universe->CurrentCockpit())
 *     respawnunit.push_back(0);
 *   respawnunit[_Universe->CurrentCockpit()]=1;
 *  }
 *  }
 */


static void FaceTarget( Unit *un )
{
    Unit *targ = un->Target();
    if (targ) {
        QVector dir( targ->Position()-un->Position() );
        dir.Normalize();
        Vector  p, q, r;
        un->GetOrientation( p, q, r );
        QVector qq( q.Cast() );
        qq = qq+QVector( .001, .001, .001 );
        un->SetOrientation( qq, dir );
    }
}


extern void SwitchUnits2( Unit *nw );
void SwitchUnits( Unit *ol, Unit *nw )
{
    bool pointingtool = false;
    for (unsigned int i = 0; i < _Universe->numPlayers(); ++i)
        if ( i != _Universe->CurrentCockpit() ) {
            if (_Universe->AccessCockpit( i )->GetParent() == ol)
                pointingtool = true;
        }
    if ( ol && (!pointingtool) ) {
        Unit *oltarg = ol->Target();
        if (oltarg)
            if (ol->getRelation( oltarg ) >= 0)
                ol->Target( NULL );
        ol->PrimeOrders();
        ol->SetAI( new Orders::AggressiveAI( "default.agg.xml" ) );
        ol->SetVisible( true );
    }
    SwitchUnits2( nw );
}
static void SwitchUnitsTurret( Unit *ol, Unit *nw )
{
    static bool FlyStraightInTurret =
        XMLSupport::parse_bool( vs_config->getVariable( "physics", "ai_pilot_when_in_turret", "true" ) );
    if (FlyStraightInTurret) {
        SwitchUnits( ol, nw );
    } else {
        ol->PrimeOrders();
        SwitchUnits( NULL, nw );
    }
}

Unit * GetFinalTurret( Unit *baseTurret )
{
    Unit *un = baseTurret;
    Unit *tur;
    for (un_iter uj = un->getSubUnits(); (tur = *uj); ++uj) {
        SwitchUnits( NULL, tur );
        un = GetFinalTurret( tur );
    }
    return un;
}



extern void DoCockpitKeys();
static float dockingdistance( Unit *port, Unit *un )
{
    vector< DockingPorts >::const_iterator i   = port->GetImageInformation().dockingports.begin();
    vector< DockingPorts >::const_iterator end = port->GetImageInformation().dockingports.end();
    QVector pos( InvTransform( port->cumulative_transformation_matrix, un->Position() ) );
    float   mag = FLT_MAX;
    for (; i != end; ++i) {
        float tmag = (pos.Cast()-(*i).GetPosition()).Magnitude()-un->rSize()-(*i).GetRadius();
        if (tmag < mag)
            mag = tmag;
    }
    if (mag == FLT_MAX)
        return UnitUtil::getDistance( port, un );
    return mag;
}






static const std::string emptystring;










Unit* Cockpit::GetSaveParent()
{
    Unit *un = parentturret.GetUnit();
    if (!un)
        un = parent.GetUnit();
    return un;
}


void Cockpit::RestoreGodliness()
{
    static float maxgodliness = XMLSupport::parse_float( vs_config->getVariable( "physics", "player_godliness", "0" ) );
    godliness = maxgodliness;
    if (godliness > maxgodliness)
        godliness = maxgodliness;
}


bool Cockpit::unitInAutoRegion( Unit *un )
{
    static float autopilot_term_distance =
        XMLSupport::parse_float( vs_config->getVariable( "physics", "auto_pilot_termination_distance", "6000" ) );
    Unit *targ = autopilot_target.GetUnit();
    if (targ)
        return UnitUtil::getSignificantDistance( un, targ ) < autopilot_term_distance*2.5;          //if both guys just auto'd in.
    else
        return false;
}


void Cockpit::recreate( const std::string &pilot_name )
{
    savegame->SetCallsign( pilot_name );
    Init( "" );
}

// Deleted instance in cockpit_generic.h (really script_call_unit_generic).
// Was supposed to be duplicate to the one above but isn't
//int Cockpit::Autopilot( Unit *target )

// Deleted instance in cockpit_generic.h (really script_call_unit_generic).
//void Cockpit::UpdAutoPilot()


static bool too_many_attackers = false;
bool Cockpit::tooManyAttackers()
{
    return too_many_attackers;
}

void Cockpit::updateAttackers()
{
    static int         max_attackers   = XMLSupport::parse_int( vs_config->getVariable( "AI", "max_player_attackers", "0" ) );
    if (max_attackers == 0) return;
    static un_iter     attack_iterator = _Universe->activeStarSystem()->getUnitList().createIterator();
    static StarSystem *ss = _Universe->activeStarSystem();
    if ( ss != _Universe->activeStarSystem() ) {
        attack_iterator = _Universe->activeStarSystem()->getUnitList().createIterator();
        ss = _Universe->activeStarSystem();
    }
    bool isDone = attack_iterator.isDone();
    if (_Universe->AccessCockpit( _Universe->numPlayers()-1 ) == this) {
        if (!isDone)
            ++attack_iterator;
        else
            attack_iterator = _Universe->activeStarSystem()->getUnitList().createIterator();
        //too_many_attackers=false;
    }
    if (isDone) {
        if (_Universe->AccessCockpit( 0 ) == this) {
            too_many_attackers = false;
        }
        number_of_attackers = partial_number_of_attackers;         //reupdate the count
        partial_number_of_attackers = 0;
        too_many_attackers  = max_attackers > 0 && (too_many_attackers || number_of_attackers > max_attackers);
    }
    Unit *un;
    if (attack_iterator.isDone() == false && (un = *attack_iterator) != NULL) {
        Unit *targ  = un->Target();
        float speed = 0, range = 0, mmrange = 0;
        if (parent == targ && targ != NULL) {
            un->getAverageGunSpeed( speed, range, mmrange );
            float dist = UnitUtil::getDistance( targ, un );
            if (dist <= range*2 || dist <= mmrange)
                partial_number_of_attackers += 1;
        }
    } else {
        //

        //partial_number_of_attackers=-1;
    }
}


bool Cockpit::Update()
{
    if (retry_dock) {
        QVector vec;
        DockToSavedBases( _Universe->CurrentCockpit(), vec );
    }
    if (jumpok)
        jumpok++;
    if (jumpok > 5)
        jumpok = 0;
    UpdAutoPilot();
    Unit *par = GetParent();
    if (par != NULL) {
        static float minEnergyForShieldDownpower =
            XMLSupport::parse_float( vs_config->getVariable( "physics", "shield_energy_downpower", "-.125" ) );
        static float minEnergyShieldTime    =
            XMLSupport::parse_float( vs_config->getVariable( "physics", "shield_energy_downpower_time", "5" ) );
        static float minEnergyShieldPercent =
            XMLSupport::parse_float( vs_config->getVariable( "physics", "shield_energy_downpower_percent", ".66666666666666" ) );

        bool toolittleenergy = (par->energyData() <= minEnergyForShieldDownpower);
        if (toolittleenergy) {
            secondsWithZeroEnergy += SIMULATION_ATOM;
            if (secondsWithZeroEnergy > minEnergyShieldTime) {
                secondsWithZeroEnergy = 0;

                // TODO: lib_damage
                // check the input is in the expected 0 to 1 values
                par->GetShieldLayer().AdjustPower(minEnergyShieldPercent);
            }
        } else {
            secondsWithZeroEnergy = 0;
        }
    }
    if ( turretcontrol.size() > _Universe->CurrentCockpit() ) {
        if (turretcontrol[_Universe->CurrentCockpit()]) {
            turretcontrol[_Universe->CurrentCockpit()] = 0;
            Unit *par = GetParent();
            //this being here, it will require poking the turret from the undock script
            if (par) {
                if (par->name == "return_to_cockpit") {
                    //if (par->owner->isUnit()==_UnitType::unit ) this->SetParent(par->owner,GetUnitFileName().c_str(),this->unitmodname.c_str(),savegame->GetPlayerLocation());     // this warps back to the parent unit if we're eject-docking. in this position it also causes badness upon loading a game.

                    Unit *temp = findUnitInStarsystem( par->owner );
                    if (temp) {
                        SwitchUnits( NULL, temp );
                        this->SetParent( temp, GetUnitFileName().c_str(), this->unitmodname.c_str(), temp->Position() );                         //this warps back to the parent unit if we're eject-docking. causes badness upon loading a game.
                    }
                    par->Kill();
                }
            }
            if (par) {
                static int index = 0;
                int  i      = 0;
                bool tmp    = false;
                bool tmpgot = false;
                if (parentturret.GetUnit() == NULL) {
                    tmpgot = true;
                    Unit *un;
                    for (un_iter ui = par->getSubUnits(); (un = *ui);) {
                        if ( _Universe->isPlayerStarship( un ) ) {
                            ++ui;
                            continue;
                        }
                        if (i++ == index) {
                            //NOTE : this may have been a correction to the conditional bug
                            ++index;
                            if (un->name.get().find( "accessory" ) == string::npos) {
                                tmp = true;
                                SwitchUnitsTurret( par, un );
                                parentturret.SetUnit( par );
                                Unit *finalunit = GetFinalTurret( un );
                                this->SetParent( finalunit, GetUnitFileName().c_str(),
                                                this->unitmodname.c_str(), savegame->GetPlayerLocation() );
                                break;
                            }
                        }
                        ++ui;
                    }
                }
                if (tmp == false) {
                    if (tmpgot) index = 0;
                    Unit *un = parentturret.GetUnit();
                    if ( un && ( !_Universe->isPlayerStarship( un ) ) ) {
                        SetParent( un, GetUnitFileName().c_str(), this->unitmodname.c_str(), savegame->GetPlayerLocation() );
                        SwitchUnits( NULL, un );
                        parentturret.SetUnit( NULL );
                        un->SetTurretAI();
                        un->DisableTurretAI();
                    }
                }
            }
        }
    }
    static bool autoclear = XMLSupport::parse_bool( vs_config->getVariable( "AI", "autodock", "false" ) );
    if (autoclear && par) {
        Unit *targ = par->Target();
        if (targ) {
            static float autopilot_term_distance =
                XMLSupport::parse_float( vs_config->getVariable( "physics", "auto_pilot_termination_distance", "6000" ) );
            float doubled = dockingdistance( targ, par );
            if ( ( (targ->isUnit() != _UnitType::planet
                    && doubled < autopilot_term_distance)
                  || (UnitUtil::getSignificantDistance( targ,
                                                        par ) <= 0) )
                && ( !( par->IsCleared( targ ) || targ->IsCleared( par ) || par->isDocked( targ )
                       || targ->isDocked( par ) ) ) && (par->getRelation( targ ) >= 0) && (targ->getRelation( par ) >= 0) ) {
                if ( targ->isUnit() != _UnitType::planet || targ->GetDestinations().empty() )
                    RequestClearence( par, targ, 0 );                      //sex is always 0... don't know how to	 get it.
            } else if ( ( par->IsCleared( targ )
                         || targ->IsCleared( par ) ) && ( !( par->isDocked( targ ) ) || targ->isDocked( par ) )
                       && ( (targ->isUnit() == _UnitType::planet && UnitUtil::getSignificantDistance( par, targ ) > 0)
                           || ( ( targ->isUnit() != _UnitType::planet
                                 && UnitUtil::getSignificantDistance( par, targ ) > ( targ->rSize()+par->rSize() ) )
                               && (doubled >= autopilot_term_distance) ) ) ) {
                if ( targ->isUnit() != _UnitType::planet || targ->GetDestinations().empty() ) {
                    par->EndRequestClearance( targ );
                    targ->EndRequestClearance( par );
                }
            }
        }
    }
    if ( switchunit.size() > _Universe->CurrentCockpit() ) {
        if (switchunit[_Universe->CurrentCockpit()]) {
            parentturret.SetUnit( NULL );

            static float initialzoom =
                XMLSupport::parse_float( vs_config->getVariable( "graphics", "inital_zoom_factor", "2.25" ) );
            zoomfactor = initialzoom;
            static int   index = 0;
            switchunit[_Universe->CurrentCockpit()] = 0;
            static bool  switch_nonowned_units =
                XMLSupport::parse_bool( vs_config->getVariable( "AI", "switch_nonowned_units", "true" ) );
//switch_nonowned_units = true;
            //static bool switch_to_fac=XMLSupport::parse_bool(vs_config->getVariable("AI","switch_to_whole_faction","true"));

            Unit *un;
            bool  found = false;
            int   i     = 0;
            for (un_iter ui = _Universe->activeStarSystem()->getUnitList().createIterator(); (un = *ui); ++ui)
                if (un->faction == this->unitfaction) {
//this switches units UNLESS we're an ejected pilot. Instead, if we are an ejected
//pilot, switch only if we're close enough.
//the trigger is to allow switching only between ships that are actually owned by you, this prevents
//stealing a ship from a hired wingman.
                    if ( ( ( (par != NULL)
                            && (i++) >= index )
                          || par == NULL ) && ( !_Universe->isPlayerStarship( un ) )
                        && ( switch_nonowned_units
                            || (par != NULL
                                && un->owner == par->owner)
                            || (par != NULL
                                && un == par->owner)
                            || (par != NULL
                                && un->owner == par)
                            || (par == NULL
                                && un->owner) ) && (un->name != "eject") && (un->name != "Pilot")
                        && (un->isUnit() != _UnitType::missile) ) {
                        found = true;
                        ++index;
                        Unit *k = GetParent();
                        bool  proceed = true;
                        if (k)
                            if (k->name == "eject" || k->name == "Pilot" || k->name == "return_to_cockpit")
                                proceed = false;
                        //we are an ejected pilot, so, if we can get close enough to the related unit, jump into it and remove the seat. This said, always allow
                        //switching from the "fake" ejection seat (ejectdock).
                        if ( !proceed && k && ( k->Position()-un->Position() ).Magnitude() < ( un->rSize()+5*k->rSize() ) ) {
                            if ( !(k->name == "return_to_cockpit") )
                                SwitchUnits( k, un );
                            //this refers to cockpit
                            if ( !(k->name == "return_to_cockpit") )
                                this->SetParent( un, GetUnitFileName().c_str(),
                                                this->unitmodname.c_str(), savegame->GetPlayerLocation() );
                            if ( !(k->name == "return_to_cockpit") )
                                k->Kill();
                            //un->SetAI(new FireKeyboard ())
                        }
                        if (proceed) {
                            //k->PrimeOrdersLaunched();
//k->SetAI (new Orders::AggressiveAI ("interceptor.agg.xml"));
//k->SetTurretAI();

//Flightgroup * fg = k->getFlightgroup();
//if (fg!=NULL) {
//
//un->SetFg (fg,fg->nr_ships++);
//fg->nr_ships_left++;
//fg->leader.SetUnit(un);
//fg->directive="b";
//}
                            SwitchUnits( k, un );
                            this->SetParent( un, GetUnitFileName().c_str(),
                                            this->unitmodname.c_str(), savegame->GetPlayerLocation() );
                            //un->SetAI(new FireKeyboard ())
                        }
                        break;
                    }
                }
            if (!found)
                index = 0;
        }
    }
    //this causes the physical ejecting. Check going_to_dock_screen in here, also.
    if (ejecting) {
        ejecting = false;
//going_to_dock_screen=true; // NO, clear this only after we've UNDOCKED that way we know we don't have issues.

        Unit *un = GetParent();
        if (un) {
            if (going_to_dock_screen == false)
                un->EjectCargo( (unsigned int) -1 );
            if (going_to_dock_screen == true) {
                un->EjectCargo( (unsigned int) -2 );
                going_to_dock_screen = false;
            }
        }
    }
    if (!par) {
        if ( respawnunit.size() > _Universe->CurrentCockpit() ) {
            if (respawnunit[_Universe->CurrentCockpit()]) {
                static float initialzoom =
                    XMLSupport::parse_float( vs_config->getVariable( "graphics", "inital_zoom_factor", "2.25" ) );
                zoomfactor = initialzoom;

                    parentturret.SetUnit( NULL );
                    respawnunit[_Universe->CurrentCockpit()] = 0;
                    std::string  savegamefile = mission->getVariable( "savegame", "" );
                    unsigned int k;
                    for (k = 0; k < _Universe->numPlayers(); ++k)
                        if (_Universe->AccessCockpit( k ) == this)
                            break;
                    if ( k == _Universe->numPlayers() ) k = 0;
                    if (active_missions.size() > 1) {
                        for (int i = active_missions.size()-1; i > 0; --i)                          //don't terminate zeroth mission
                            if (active_missions[i]->player_num == k)
                                active_missions[i]->terminateMission();
                    }
                    unsigned int whichcp = k;
                    string  newsystem;
                    QVector pos;
                    bool    setplayerXloc;
                    savegame->SetStarSystem( "" );
                    QVector tmpoldpos = savegame->GetPlayerLocation();
                    savegame->SetPlayerLocation( QVector( FLT_MAX, FLT_MAX, FLT_MAX ) );
                    vector< string > packedInfo;
                    savegame->ParseSaveGame( savegamefile,
                                             newsystem,
                                             newsystem,
                                             pos,
                                             setplayerXloc,
                                             this->credits,
                                             packedInfo,
                                             k );
                    UnpackUnitInfo(packedInfo);
                    if (pos.i == FLT_MAX && pos.j == FLT_MAX && pos.k == FLT_MAX)
                        pos = tmpoldpos;
                    savegame->SetPlayerLocation( pos );
                    CopySavedShips( savegame->GetCallsign(), whichcp, packedInfo, true );
                    bool actually_have_save = false;
                    static bool persistent_on_load =
                        XMLSupport::parse_bool( vs_config->getVariable( "physics", "persistent_on_load", "true" ) );
                    if (savegame->GetStarSystem() != "") {
                        actually_have_save = true;
                        newsystem = savegame->GetStarSystem()+".system";
                    } else {
                        newsystem = _Universe->activeStarSystem()->getFileName();
                        if (newsystem.find( ".system" ) == string::npos)
                            newsystem += ".system";
                    }
                    Background::BackgroundClone savedtextures = {
                        {NULL, NULL, NULL, NULL, NULL, NULL, NULL}
                    };
                    if (persistent_on_load) {
                        _Universe->getActiveStarSystem( 0 )->SwapOut();
                    } else {
                        Background *tmp = _Universe->activeStarSystem()->getBackground();
                        savedtextures = tmp->Cache();
                        _Universe->clearAllSystems();
                    }
                    StarSystem *ss = _Universe->GenerateStarSystem( newsystem.c_str(), "", Vector( 0, 0, 0 ) );
                    if (!persistent_on_load)
                        savedtextures.FreeClone();
                    this->activeStarSystem = ss;
                    _Universe->pushActiveStarSystem( ss );

                    vector< StarSystem* >saved;
                    while ( _Universe->getNumActiveStarSystem() ) {
                        saved.push_back( _Universe->activeStarSystem() );
                        _Universe->popActiveStarSystem();
                    }
                    if ( !saved.empty() )
                        saved.back() = ss;
                    unsigned int mysize = saved.size();
                    for (unsigned int i = 0; i < mysize; i++) {
                        _Universe->pushActiveStarSystem( saved.back() );
                        saved.pop_back();
                    }
                    ss->SwapIn();
                    int fgsnumber = 0;
                    if (fg) {
                        fgsnumber = fg->flightgroup_nr++;
                        fg->nr_ships++;
                        fg->nr_ships_left++;
                    }
                    Unit *un = new Unit(
                        GetUnitFileName().c_str(), false, this->unitfaction, unitmodname, fg, fgsnumber );
                    un->SetCurPosition( UniverseUtil::SafeEntrancePoint( savegame->GetPlayerLocation() ) );
                    ss->AddUnit( un );

                    this->SetParent( un, GetUnitFileName().c_str(), unitmodname.c_str(), savegame->GetPlayerLocation() );
                    SwitchUnits( NULL, un );
                    this->credits = savegame->GetSavedCredits();
                    DoCockpitKeys();
                    _Universe->popActiveStarSystem();
                    _Universe->pushActiveStarSystem( ss );
                    savegame->ReloadPickledData();
                    savegame->LoadSavedMissions();
                    if (actually_have_save) {
                        QVector vec;
                        DockToSavedBases( whichcp, vec );
                    }
                    UniverseUtil::hideSplashScreen();
                    _Universe->popActiveStarSystem();
                    if (!persistent_on_load)
                        _Universe->pushActiveStarSystem( ss );
                    return true;

            }
        }
    }
    return false;
}







void Cockpit::PackUnitInfo(vector< std::string > &info) const
{
    info.clear();

    // First entry, current ship
    if (GetNumUnits() > 0)
        info.push_back(GetUnitFileName());

    // Following entries, ship/location pairs
    for (size_t i=1,n=GetNumUnits(); i<n; ++i) {
        info.push_back(GetUnitFileName(i));
        info.push_back(GetUnitSystemName(i) + "@" + GetUnitBaseName(i));
    }
}

void Cockpit::UnpackUnitInfo(vector< std::string > &info)
{
    vector< string > filenames, systemnames, basenames;

    // First entry, current ship
    if (!info.empty()) {
        filenames.push_back( info[0] );
        systemnames.push_back( "" );
        basenames.push_back( "" );
    }

    // Following entries, ship/location pairs
    for (size_t i=1, n=info.size(); i < n; i += 2) {
        filenames.push_back( info[i] );

        string location = ((i+1) < n) ? info[i+1] : "";
        string::size_type atpos = location.find_first_of('@');

        systemnames.push_back(location.substr(0, atpos));
        basenames.push_back((atpos != string::npos) ? location.substr(atpos+1) : "");
    }

    unitfilename.swap(filenames);
    unitsystemname.swap(systemnames);
    unitbasename.swap(basenames);
}

const std::string& Cockpit::GetUnitFileName(unsigned int which) const
{
    if ( which >= unitfilename.size() )
        return emptystring;
    else
        return unitfilename[which];
}

const std::string& Cockpit::GetUnitSystemName(unsigned int which) const
{
    if ( which >= unitsystemname.size() )
        return emptystring;
    else
        return unitsystemname[which];
}

const std::string& Cockpit::GetUnitBaseName(unsigned int which) const
{
    if ( which >= unitbasename.size() )
        return emptystring;
    else
        return unitbasename[which];
}

void Cockpit::RemoveUnit(unsigned int which)
{
    if (which < unitfilename.size())
        unitfilename.erase(unitfilename.begin()+which);
    if (which < unitsystemname.size())
        unitsystemname.erase(unitsystemname.begin()+which);
    if (which < unitbasename.size())
        unitbasename.erase(unitbasename.begin()+which);
}

string Cockpit::MakeBaseName(const Unit *base)
{
    string name;
    if (base != NULL) {
        if (base->getFlightgroup() != NULL)
            name = base->getFlightgroup()->name + ':';
        name += base->getFullname();
        if (base->getFgSubnumber() > 0)
            name += ':' + XMLSupport::tostring(base->getFgSubnumber());
    }

    // remove all whitespace, it breaks savegames
    std::replace(name.begin(), name.end(), ' ', '_');

    return name;
}

SoundContainer* Cockpit::GetSoundForEvent(CockpitEvent eventId) const
{
    if (eventId < sounds.size())
        return sounds[eventId];
    else
        return NULL;
}

void Cockpit::SetSoundForEvent(CockpitEvent eventId, const SoundContainer &soundSpecs)
{
    while (eventId >= sounds.size())
        sounds.push_back(NULL);
    sounds[eventId] = soundImpl(soundSpecs);
}


