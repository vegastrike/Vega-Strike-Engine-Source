/**
 * cockpit_generic.cpp
 *
 * Copyright (C) Daniel Horn
 * Copyright (C) 2020 pyramid3d, Stephen G. Tuggy, and other Vega Strike
 * contributors
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


#include "in.h"
#include "vsfilesystem.h"
#include "vs_globals.h"
#include "vegastrike.h"
#include "cockpit_generic.h"
#include "star_system_generic.h"
#include "cmd/unit_generic.h"
#include "unit.h"
#include "cmd/unit_util.h"
#include "cmd/collection.h"
#include "lin_time.h" //for fps
#include "configxml.h"
#include "cmd/images.h"
#include "cmd/script/mission.h"
#include "cmd/script/msgcenter.h"
//#include "cmd/ai/flyjoystick.h"
//#include "cmd/ai/firekeyboard.h"
#include "cmd/ai/aggressive.h"
//#include "main_loop.h"
#include <assert.h>     //needed for assert() calls
#include "savegame.h"
//#include "animation.h"
//#include "mesh.h"
#include "universe_util.h"
#include "cmd/ai/fire.h"
#include "background.h"
//#include "in_mouse.h"
//#include "gui/glut_support.h"
#include "save_util.h"
#include "star_system.h"
#include "universe.h"
#include "mount_size.h"
#include "weapon_info.h"

#include <algorithm>

#define SWITCH_CONST (0.9)

vector< int >respawnunit;
vector< int >switchunit;
vector< int >turretcontrol;

void Cockpit::beginElement( void *userData, const XML_Char *name, const XML_Char **atts )
{
    ( (Cockpit*) userData )->beginElement( name, AttributeList( atts ) );
}

void Cockpit::endElement( void *userData, const XML_Char *name )
{
    ( (Cockpit*) userData )->endElement( name );
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

using namespace VSFileSystem;
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
Unit* Cockpit::GetSaveParent()
{
    Unit *un = parentturret.GetUnit();
    if (!un)
        un = parent.GetUnit();
    return un;
}
void Cockpit::SetParent( Unit *unit, const char *filename, const char *unitmodname, const QVector &pos )
{
    if (unit->getFlightgroup() != NULL)
        fg = unit->getFlightgroup();
    activeStarSystem = _Universe->activeStarSystem();          //cannot switch to units in other star systems.
    parent.SetUnit( unit );
    savegame->SetPlayerLocation( pos );
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
}
void Cockpit::Delete()
{
    //int i;
    viewport_offset = cockpit_offset = 0;
}
void Cockpit::RestoreGodliness()
{
    static float maxgodliness = XMLSupport::parse_float( vs_config->getVariable( "physics", "player_godliness", "0" ) );
    godliness = maxgodliness;
    if (godliness > maxgodliness)
        godliness = maxgodliness;
}

void Cockpit::InitStatic()
{
    radar_time   = 0;
    cockpit_time = 0;
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
static float getInitialZoomFactor()
{
    static float inizoom = XMLSupport::parse_float( vs_config->getVariable( "graphics", "inital_zoom_factor", "2.25" ) );
    return inizoom;
}
Cockpit::Cockpit( const char *file, Unit *parent, const std::string &pilot_name ) : view( CP_FRONT )
    , parent( parent )
    , cockpit_offset( 0 )
    , viewport_offset( 0 )
    , zoomfactor( getInitialZoomFactor() )
    , savegame( new SaveGame( pilot_name ) )
{
    //static int headlag = XMLSupport::parse_int (vs_config->getVariable("graphics","head_lag","10"));
    //int i;
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
}
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

void Cockpit::recreate( const std::string &pilot_name )
{
    savegame->SetCallsign( pilot_name );
    Init( "" );
}
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
int Cockpit::Autopilot( Unit *target )
{
    int retauto = 0;
    if (target) {
        Unit *un = NULL;
        if ( ( un = GetParent() ) ) {
            if ( ( retauto = un->AutoPilotTo( un, false ) ) ) {
                //can he even start to autopilot
                //SetView (CP_PAN);
                un->AutoPilotTo( target, false );
                static bool face_target_on_auto =
                    XMLSupport::parse_bool( vs_config->getVariable( "physics", "face_on_auto", "false" ) );
                if (face_target_on_auto)
                    FaceTarget( un );
                static double averagetime = GetElapsedTime()/getTimeCompression();
                static double numave = 1.0;
                averagetime += GetElapsedTime()/getTimeCompression();
                //static float autospeed = XMLSupport::parse_float (vs_config->getVariable ("physics","autospeed",".020"));//10 seconds for auto to kick in;
                numave++;
                /*
                 *  AccessCamera(CP_PAN)->myPhysics.SetAngularVelocity(Vector(0,0,0));
                 *  AccessCamera(CP_PAN)->myPhysics.ApplyBalancedLocalTorque(_Universe->AccessCamera()->P,
                 *                                                     _Universe->AccessCamera()->R,
                 *                                                     averagetime*autospeed/(numave));
                 */
                static float initialzoom =
                    XMLSupport::parse_float( vs_config->getVariable( "graphics", "inital_zoom_factor", "2.25" ) );
                zoomfactor     = initialzoom;
                static float autotime    = XMLSupport::parse_float( vs_config->getVariable( "physics", "autotime", "10" ) );                 //10 seconds for auto to kick in;

                autopilot_time = autotime;
                autopilot_target.SetUnit( target );
            }
        }
    }
    return retauto;
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

void Cockpit::UpdAutoPilot()
{
    if (autopilot_time != 0) {
        autopilot_time -= SIMULATION_ATOM;
        if (autopilot_time <= 0) {
            autopilot_time = 0;
            Unit *par = GetParent();
            if (par) {
                Unit *autoun = autopilot_target.GetUnit();
                autopilot_target.SetUnit( NULL );
                if (autoun)
                    par->AutoPilotTo( autoun, false );
            }
        }
    }
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
void PowerDownShield( Shield *shield, float howmuch )
{
    switch (shield->number)
    {
    case 2:
        if (shield->shield2fb.front/howmuch > shield->shield2fb.frontmax)
            shield->shield2fb.front = shield->shield2fb.frontmax*howmuch;
        if (shield->shield2fb.back/howmuch > shield->shield2fb.backmax)
            shield->shield2fb.back = shield->shield2fb.backmax*howmuch;
        break;
    case 4:
        if (shield->shield4fbrl.front/howmuch > shield->shield4fbrl.frontmax)
            shield->shield4fbrl.front = shield->shield4fbrl.frontmax*howmuch;
        if (shield->shield4fbrl.right/howmuch > shield->shield4fbrl.rightmax)
            shield->shield4fbrl.right = shield->shield4fbrl.rightmax*howmuch;
        if (shield->shield4fbrl.left/howmuch > shield->shield4fbrl.leftmax)
            shield->shield4fbrl.left = shield->shield4fbrl.leftmax*howmuch;
        if (shield->shield4fbrl.back/howmuch > shield->shield4fbrl.backmax)
            shield->shield4fbrl.back = shield->shield4fbrl.backmax*howmuch;
        break;
    case 8:
        if (shield->shield8.frontlefttop/howmuch > shield->shield8.frontlefttopmax)
            shield->shield8.frontlefttop = shield->shield8.frontlefttopmax*howmuch;
        if (shield->shield8.frontleftbottom/howmuch > shield->shield8.frontleftbottommax)
            shield->shield8.frontleftbottom = shield->shield8.frontleftbottommax*howmuch;
        if (shield->shield8.frontrighttop/howmuch > shield->shield8.frontrighttopmax)
            shield->shield8.frontrighttop = shield->shield8.frontrighttopmax*howmuch;
        if (shield->shield8.frontrightbottom/howmuch > shield->shield8.frontrightbottommax)
            shield->shield8.frontrightbottom = shield->shield8.frontrightbottommax*howmuch;
        if (shield->shield8.backlefttop/howmuch > shield->shield8.backlefttopmax)
            shield->shield8.backlefttop = shield->shield8.backlefttopmax*howmuch;
        if (shield->shield8.backleftbottom/howmuch > shield->shield8.backleftbottommax)
            shield->shield8.backleftbottom = shield->shield8.backleftbottommax*howmuch;
        if (shield->shield8.backrighttop/howmuch > shield->shield8.backrighttopmax)
            shield->shield8.backrighttop = shield->shield8.backrighttopmax*howmuch;
        if (shield->shield8.backrightbottom/howmuch > shield->shield8.backrightbottommax)
            shield->shield8.backrightbottom = shield->shield8.backrightbottommax*howmuch;
        break;
    default:
        break;
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
                PowerDownShield( &par->shield, minEnergyShieldPercent );
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
                    Unit *un = new GameUnit(
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

void visitSystemHelp( Cockpit *cp, string systemname, float num )
{
    string key( string( "visited_" )+systemname );
    vector< float > *v = &cp->savegame->getMissionData( key );
    if ( v->empty() )
        v->push_back( num );
    else if ( (*v)[0] != 1.0 && num == 1 )
        (*v)[0] = num;
}
void Cockpit::visitSystem( string systemname )
{
    visitSystemHelp( this, systemname, 1.0 );
    int adj = UniverseUtil::GetNumAdjacentSystems( systemname );
    for (int i = 0; i < adj; ++i)
        visitSystemHelp( this, UniverseUtil::GetAdjacentSystem( systemname, i ), 0.0 );
}

Cockpit::~Cockpit()
{
    Delete();
    if (savegame != NULL)
        delete savegame;
}

void Cockpit::SetInsidePanYawSpeed( float )
{
    // No-op in generic cockpits
}

void Cockpit::SetInsidePanPitchSpeed( float )
{
    // No-op in generic cockpits
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

static const std::string emptystring;

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

SoundContainer* Cockpit::GetSoundForEvent(Cockpit::EVENTID eventId) const
{
    if (eventId < sounds.size())
        return sounds[eventId];
    else
        return NULL;
}

void Cockpit::SetSoundForEvent(Cockpit::EVENTID eventId, const SoundContainer &soundSpecs)
{
    while (eventId >= sounds.size())
        sounds.push_back(NULL);
    sounds[eventId] = soundImpl(soundSpecs);
}

SoundContainer* Cockpit::soundImpl(const SoundContainer &specs)
{
    return new SoundContainer(specs);
}

