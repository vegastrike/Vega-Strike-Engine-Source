/**
* cockpit_xml.cpp
*
* Copyright (c) 2001-2002 Daniel Horn
* Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
* Copyright (c) 2019-2021 Stephen G. Tuggy, and other Vega Strike Contributors
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

#include "cockpit.h"
#include "xml_support.h"
#include "gauge.h"
#include <float.h>
#include "hud.h"
#include "vdu.h"
#include "mesh.h"
#include "configxml.h"

using XMLSupport::EnumMap;
using XMLSupport::Attribute;
using XMLSupport::AttributeList;
using XMLSupport::parse_float;
using XMLSupport::parse_bool;
using XMLSupport::parse_int;

namespace CockpitXML
{
//

enum Names
{
    UNKNOWN=UnitImages< void >::NUMGAUGES,
    COCKPIT,
    MESH,
    CROSSHAIRS,
    RADAR,
    REARRADAR,
    LVDU,
    RVDU,
    AVDU,
    VDUTYPE,
    PANEL,
    ROWS,
    COLS,
    XFILE,
    SOUNDFILE,
    XCENT,
    YCENT,
    TOPY,
    BOTY,
    XSIZE,
    YSIZE,
    MYFONT,
    RED,
    GREEN,
    BLUE,
    COCKPITOFFSET,
    VIEWOFFSET,
    FRONT,
    BACK,
    LEFT,
    RIGHT,
    NETWORK,
//use the UnitImages<void> enum for the gauge values instead!
/*    KARMORF,
 *     KARMORB,
 *     KARMORR,
 *     KARMORL,
 *     KFUEL,
 *     KSHIELDF,
 *     KSHIELDR,
 *     KSHIELDL,
 *     KSHIELDB,
 *     KENERGY,
 *     KAUTO,
 *     KEJECT,
 *     KLOCK,
 *     KHULL,
 *     KWARPENERGY,
 *     KKPS,
 *     KSETKPS,
 *     KFPS,
 *         COCKPIT_LAG,
 */
    G_UP,
    G_DOWN,
    G_LEFT,
    G_RIGHT,
    G_TIME,
    ALPH,
    EVENT,
    LOOPING,
    GAIN
};

const EnumMap::Pair element_names[] = {
    EnumMap::Pair( "UNKNOWN",           UNKNOWN ),
    EnumMap::Pair( "Cockpit",           COCKPIT ),
    EnumMap::Pair( "Radar",             RADAR ),
    EnumMap::Pair( "RearRadar",         REARRADAR ),
    EnumMap::Pair( "LeftVDU",           LVDU ),
    EnumMap::Pair( "VDU",               AVDU ),
    EnumMap::Pair( "RightVDU",          RVDU ),
    EnumMap::Pair( "Panel",             PANEL ),
    EnumMap::Pair( "Crosshairs",        CROSSHAIRS ),
    EnumMap::Pair( "Event",             EVENT ),
    EnumMap::Pair( "ArmorF",            UnitImages< void >::ARMORF ),
    EnumMap::Pair( "ArmorR",            UnitImages< void >::ARMORR ),
    EnumMap::Pair( "ArmorL",            UnitImages< void >::ARMORL ),
    EnumMap::Pair( "ArmorB",            UnitImages< void >::ARMORB ),
    EnumMap::Pair( "ArmorFRT",          UnitImages< void >::ARMORF ),
    EnumMap::Pair( "ArmorBRT",          UnitImages< void >::ARMORR ),
    EnumMap::Pair( "ArmorFLT",          UnitImages< void >::ARMORL ),
    EnumMap::Pair( "ArmorBLT",          UnitImages< void >::ARMORB ),
    EnumMap::Pair( "ArmorFRB",          UnitImages< void >::ARMOR4 ),
    EnumMap::Pair( "ArmorBRB",          UnitImages< void >::ARMOR5 ),
    EnumMap::Pair( "ArmorFLB",          UnitImages< void >::ARMOR6 ),
    EnumMap::Pair( "ArmorBLB",          UnitImages< void >::ARMOR7 ),
    EnumMap::Pair( "ShieldF",           UnitImages< void >::SHIELDF ),
    EnumMap::Pair( "ShieldR",           UnitImages< void >::SHIELDR ),
    EnumMap::Pair( "ShieldL",           UnitImages< void >::SHIELDL ),
    EnumMap::Pair( "ShieldB",           UnitImages< void >::SHIELDB ),
    EnumMap::Pair( "ShieldFRT",         UnitImages< void >::SHIELDF ),
    EnumMap::Pair( "ShieldBRT",         UnitImages< void >::SHIELDR ),
    EnumMap::Pair( "ShieldFLT",         UnitImages< void >::SHIELDL ),
    EnumMap::Pair( "ShieldBLT",         UnitImages< void >::SHIELDB ),
    EnumMap::Pair( "ShieldFRB",         UnitImages< void >::SHIELD4 ),
    EnumMap::Pair( "ShieldBRB",         UnitImages< void >::SHIELD5 ),
    EnumMap::Pair( "ShieldFLB",         UnitImages< void >::SHIELD6 ),
    EnumMap::Pair( "ShieldBLB",         UnitImages< void >::SHIELD7 ),
    EnumMap::Pair( "Fuel",              UnitImages< void >::FUEL ),
    EnumMap::Pair( "Energy",            UnitImages< void >::ENERGY ),
    EnumMap::Pair( "Eject",             UnitImages< void >::EJECT ),
    EnumMap::Pair( "Lock",              UnitImages< void >::LOCK ),
    EnumMap::Pair( "MissileLock",       UnitImages< void >::MISSILELOCK ),
    EnumMap::Pair( "Jump",              UnitImages< void >::JUMP ),
    EnumMap::Pair( "ECM",               UnitImages< void >::ECM ),
    EnumMap::Pair( "Hull",              UnitImages< void >::HULL ),
    EnumMap::Pair( "WarpEnergy",        UnitImages< void >::WARPENERGY ),
    EnumMap::Pair( "Speed",             UnitImages< void >::KPS ),
    EnumMap::Pair( "SetSpeed",          UnitImages< void >::SETKPS ),
    EnumMap::Pair( "Auto",              UnitImages< void >::AUTOPILOT ),
    EnumMap::Pair( "Collision",         UnitImages< void >::COLLISION ),
    EnumMap::Pair( "FPS",               UnitImages< void >::COCKPIT_FPS ),
    EnumMap::Pair( "WarpFieldStrength", UnitImages< void >::WARPFIELDSTRENGTH ),
    EnumMap::Pair( "MASSEFFECT",        UnitImages< void >::MASSEFFECT ),
    EnumMap::Pair( "MAUTO",             UnitImages< void >::AUTOPILOT_MODAL ),
    EnumMap::Pair( "MSPEC",             UnitImages< void >::SPEC_MODAL ),
    EnumMap::Pair( "MFLIGHTCOMP",       UnitImages< void >::FLIGHTCOMPUTER_MODAL ),
    EnumMap::Pair( "MTURRET",           UnitImages< void >::TURRETCONTROL_MODAL ),
    EnumMap::Pair( "MECM",              UnitImages< void >::ECM_MODAL ),
    EnumMap::Pair( "MCLOAK",            UnitImages< void >::CLOAK_MODAL ),
    EnumMap::Pair( "MTRAVEL",           UnitImages< void >::TRAVELMODE_MODAL ),
    EnumMap::Pair( "MRECFIRE",          UnitImages< void >::RECIEVINGFIRE_MODAL ),
    EnumMap::Pair( "MRECMISSILE",       UnitImages< void >::RECEIVINGMISSILES_MODAL ),
    EnumMap::Pair( "MRECMLOCK",         UnitImages< void >::RECEIVINGMISSILELOCK_MODAL ),
    EnumMap::Pair( "MRECTLOCK",         UnitImages< void >::RECEIVINGTARGETLOCK_MODAL ),
    EnumMap::Pair( "MCOLLISIONWARNING", UnitImages< void >::COLLISIONWARNING_MODAL ),
    EnumMap::Pair( "MJUMP",             UnitImages< void >::CANJUMP_MODAL ),
    EnumMap::Pair( "MDOCK",             UnitImages< void >::CANDOCK_MODAL ),
    EnumMap::Pair( "TargetShieldF",     UnitImages< void >::TARGETSHIELDF ),
    EnumMap::Pair( "TargetShieldB",     UnitImages< void >::TARGETSHIELDB ),
    EnumMap::Pair( "TargetShieldR",     UnitImages< void >::TARGETSHIELDR ),
    EnumMap::Pair( "TargetShieldL",     UnitImages< void >::TARGETSHIELDL )
};
const EnumMap::Pair attribute_names[] = {
    EnumMap::Pair( "UNKNOWN",       UNKNOWN ),
    EnumMap::Pair( "mesh",          MESH ),
    EnumMap::Pair( "file",          XFILE ),
    EnumMap::Pair( "soundfile",     SOUNDFILE ),
    EnumMap::Pair( "font",          MYFONT ),
    EnumMap::Pair( "front",         FRONT ),
    EnumMap::Pair( "left",          LEFT ),
    EnumMap::Pair( "right",         RIGHT ),
    EnumMap::Pair( "back",          BACK ),
    EnumMap::Pair( "xcent",         XCENT ),
    EnumMap::Pair( "ycent",         YCENT ),
    EnumMap::Pair( "width",         XSIZE ),
    EnumMap::Pair( "height",        YSIZE ),
    EnumMap::Pair( "Top",           TOPY ),
    EnumMap::Pair( "Bottom",        BOTY ),
    EnumMap::Pair( "ViewOffset",    VIEWOFFSET ),
    EnumMap::Pair( "CockpitOffset", COCKPITOFFSET ),
    EnumMap::Pair( "network",       NETWORK ),
    EnumMap::Pair( "GaugeUp",       G_UP ),
    EnumMap::Pair( "GaugeDown",     G_DOWN ),
    EnumMap::Pair( "GaugeLeft",     G_LEFT ),
    EnumMap::Pair( "GaugeRight",    G_RIGHT ),
    EnumMap::Pair( "GaugeTime",     G_TIME ),
    EnumMap::Pair( "TextRows",      ROWS ),
    EnumMap::Pair( "TextCols",      COLS ),
    EnumMap::Pair( "r",             RED ),
    EnumMap::Pair( "g",             GREEN ),
    EnumMap::Pair( "b",             BLUE ),
    EnumMap::Pair( "type",          VDUTYPE ),
    EnumMap::Pair( "a",             ALPH ),
    EnumMap::Pair( "event",         EVENT ),
    EnumMap::Pair( "looping",       LOOPING ),
    EnumMap::Pair( "gain",          GAIN ),
    
    
    // Cockpit events
    EnumMap::Pair( "WarpReady",     Cockpit::WARP_READY ),
    EnumMap::Pair( "WarpUnready",   Cockpit::WARP_UNREADY ),
    EnumMap::Pair( "WarpEngaged",   Cockpit::WARP_ENGAGED ),
    EnumMap::Pair( "WarpDisengaged",Cockpit::WARP_DISENGAGED ),
    EnumMap::Pair( "WarpLoop0",     Cockpit::WARP_LOOP0 ),
    EnumMap::Pair( "WarpLoop1",     Cockpit::WARP_LOOP0+1 ),
    EnumMap::Pair( "WarpLoop2",     Cockpit::WARP_LOOP0+2 ),
    EnumMap::Pair( "WarpLoop3",     Cockpit::WARP_LOOP0+3 ),
    EnumMap::Pair( "WarpLoop4",     Cockpit::WARP_LOOP0+4 ),
    EnumMap::Pair( "WarpLoop5",     Cockpit::WARP_LOOP0+5 ),
    EnumMap::Pair( "WarpLoop6",     Cockpit::WARP_LOOP0+6 ),
    EnumMap::Pair( "WarpLoop7",     Cockpit::WARP_LOOP0+7 ),
    EnumMap::Pair( "WarpLoop8",     Cockpit::WARP_LOOP0+8 ),
    EnumMap::Pair( "WarpLoop9",     Cockpit::WARP_LOOP0+9 ),
    EnumMap::Pair( "WarpSkip0",     Cockpit::WARP_LOOP0 ),
    EnumMap::Pair( "WarpSkip1",     Cockpit::WARP_LOOP0+1 ),
    EnumMap::Pair( "WarpSkip2",     Cockpit::WARP_LOOP0+2 ),
    EnumMap::Pair( "WarpSkip3",     Cockpit::WARP_LOOP0+3 ),
    EnumMap::Pair( "WarpSkip4",     Cockpit::WARP_LOOP0+4 ),
    EnumMap::Pair( "WarpSkip5",     Cockpit::WARP_LOOP0+5 ),
    EnumMap::Pair( "WarpSkip6",     Cockpit::WARP_LOOP0+6 ),
    EnumMap::Pair( "WarpSkip7",     Cockpit::WARP_LOOP0+7 ),
    EnumMap::Pair( "WarpSkip8",     Cockpit::WARP_LOOP0+8 ),
    EnumMap::Pair( "WarpSkip9",     Cockpit::WARP_LOOP0+9 ),
    
    EnumMap::Pair( "ASAPEngaged",   Cockpit::ASAP_ENGAGED ),
    EnumMap::Pair( "ASAPDisengaged",Cockpit::ASAP_DISENGAGED ),
    EnumMap::Pair( "ASAPDockingAvailable",   Cockpit::ASAP_DOCKING_AVAILABLE ),
    EnumMap::Pair( "ASAPDockingEngaged",     Cockpit::ASAP_DOCKING_ENGAGED ),
    EnumMap::Pair( "ASAPDockingDisengaged",  Cockpit::ASAP_DOCKING_DISENGAGED ),
    EnumMap::Pair( "FlightComputerEnabled",  Cockpit::FLIGHT_COMPUTER_ENABLED ),
    EnumMap::Pair( "FlightComputerDisabled", Cockpit::FLIGHT_COMPUTER_DISABLED ),

    EnumMap::Pair( "DockAvailable",     Cockpit::DOCK_AVAILABLE ),
    EnumMap::Pair( "DockUnavailable",   Cockpit::DOCK_UNAVAILABLE ),
    EnumMap::Pair( "DockFailed",        Cockpit::DOCK_FAILED ),
    EnumMap::Pair( "JumpAvailable",     Cockpit::JUMP_AVAILABLE ),    
    EnumMap::Pair( "JumpUnavailable",   Cockpit::JUMP_UNAVAILABLE ),    
    EnumMap::Pair( "JumpFailed",        Cockpit::JUMP_FAILED ),    

    EnumMap::Pair( "Lock",              Cockpit::LOCK_WARNING ),
    EnumMap::Pair( "MissileLock",       Cockpit::MISSILELOCK_WARNING ),
    EnumMap::Pair( "Eject",             Cockpit::EJECT_WARNING ),

};

const EnumMap element_map( element_names, sizeof (element_names)/sizeof (element_names[0]) );
const EnumMap attribute_map( attribute_names, sizeof (attribute_names)/sizeof (attribute_names[0]) );
}

using XMLSupport::EnumMap;
using XMLSupport::Attribute;
using XMLSupport::AttributeList;
using namespace CockpitXML;

string getRes( string inp )
{
    string::size_type where = inp.rfind( "." );
    int x = g_game.x_resolution;
    if (x < 700) x = 640;
    else if (x < 840)
        x = 800;
    else if (x < 1100)
        x = 1024;
    else if (x < 1400)
        x = 1280;
    else if (x < 1700)
        x = 1600;
    else x = 1600;
    string rez = XMLSupport::tostring( x );
    if (where == string::npos)
        return inp+"_"+rez+".spr";
    else
        return inp.substr( 0, where )+"_"+rez+".spr";
}

void GameCockpit::beginElement( const string &name, const AttributeList &attributes )
{
    static bool cockpit_smooth   =
        XMLSupport::parse_bool( vs_config->getVariable( "graphics", "cockpit_smooth_texture", "false" ) );
    static bool panel_smooth     =
        XMLSupport::parse_bool( vs_config->getVariable( "graphics", "panel_smooth_texture", "true" ) );
    static bool crosshair_smooth =
        XMLSupport::parse_bool( vs_config->getVariable( "graphics", "crosshair_smooth_texture", "true" ) );
    AttributeList::const_iterator iter;
    Gauge::DIRECTION tmpdir = Gauge::GAUGE_UP;
    VSSprite   **newsprite  = NULL;
    VDU **newvdu = NULL;
    VSSprite    *adjsprite  = NULL;
    std::string  gaugename( "shieldstat.spr" );
    std::string  myfont( "9x12.font" );
    Names elem = (Names) element_map.lookup( name );
    Names attr;
    unsigned int mymodes = 0;
    float     xsize = -1, ysize = -1, xcent = FLT_MAX, ycent = FLT_MAX;
    float     leftx = -10;
    float     rightx     = -10;
    float     topy         = -10;
    float     boty         = -10;
    short     rows         = 13;
    short     cols         = 15;
    unsigned int default_mode = VDU::TARGET;
    VSSprite *oldpit       = NULL;
    bool      replaced[4]  = {false, false, false, false};
    int       counter      = 0;
    switch ((int)elem)
    {
    case COCKPIT:
        for (iter = attributes.begin(); iter != attributes.end(); iter++) {
            attr = (Names) attribute_map.lookup( (*iter).name );
            switch (attr)
            {
            case MYFONT:
                myfont    = (*iter).value;
                break;
            case RED:
                textcol.r = XMLSupport::parse_float( (*iter).value );
                break;
            case GREEN:
                textcol.g = XMLSupport::parse_float( (*iter).value );
                break;
            case BLUE:
                textcol.b = XMLSupport::parse_float( (*iter).value );
                break;
            case ALPH:
                textcol.a = XMLSupport::parse_float( (*iter).value );
                break;
            case VIEWOFFSET:
                viewport_offset = XMLSupport::parse_float( (*iter).value );
                break;
            case COCKPITOFFSET:
                cockpit_offset  = XMLSupport::parse_float( (*iter).value );
                break;
            case XFILE:
                {
                    std::string tmp = getRes( (*iter).value );
                    oldpit = Pit[0];
                    Pit[0] = new VSSprite( tmp.c_str(), cockpit_smooth ? BILINEAR : NEAREST );
                    if ( !Pit[0]->LoadSuccess() ) {
                        delete Pit[0];
                        Pit[0] = new VSSprite( (*iter).value.c_str(), cockpit_smooth ? BILINEAR : NEAREST );
                    }
                    replaced[0] = true;
                    if (oldpit)
                        delete oldpit;
                    break;
                }
            case SOUNDFILE:
                SetSoundFile( (*iter).value );
                break;
            case MESH:
                mesh = Mesh::LoadMeshes( (*iter).value.c_str(), Vector( 1, 1, 1 ), 0, NULL );
                break;
            case FRONT:
            case BACK:
            case LEFT:
            case RIGHT:
                {
                    std::string tmp = getRes( (*iter).value );
                    oldpit = Pit[attr-FRONT];
                    Pit[attr-FRONT] = new VSSprite( tmp.c_str(), cockpit_smooth ? BILINEAR : NEAREST );
                    if ( !Pit[attr-FRONT]->LoadSuccess() ) {
                        delete Pit[attr-FRONT];
                        Pit[attr-FRONT] = new VSSprite( (*iter).value.c_str(), cockpit_smooth ? BILINEAR : NEAREST );
                    }
                    replaced[attr-FRONT] = true;
                    if (oldpit)
                        delete oldpit;
                    break;
                }
            default:
                break;
            }
        }
        text = new TextPlane();
        for (counter = 0; counter < 4; ++counter)
            if (!replaced[counter]) {
                delete Pit[counter];
                Pit[counter] = NULL;
            }
        break;
    case UnitImages< void >::SHIELD4:
    case UnitImages< void >::SHIELD5:
    case UnitImages< void >::SHIELD6:
    case UnitImages< void >::SHIELD7:
        shield8 = true;
        goto pastarmor8;
    case UnitImages< void >::ARMOR4:
    case UnitImages< void >::ARMOR5:
    case UnitImages< void >::ARMOR6:
    case UnitImages< void >::ARMOR7:
        armor8 = true;
pastarmor8:
    case UnitImages< void >::JUMP:
    case UnitImages< void >::MISSILELOCK:
    case UnitImages< void >::ECM:
    case UnitImages< void >::ARMORF:
    case UnitImages< void >::ARMORR:
    case UnitImages< void >::ARMORL:
    case UnitImages< void >::ARMORB:
    case UnitImages< void >::FUEL:
    case UnitImages< void >::SHIELDF:
    case UnitImages< void >::SHIELDL:
    case UnitImages< void >::SHIELDR:
    case UnitImages< void >::SHIELDB:
    case UnitImages< void >::TARGETSHIELDF:
    case UnitImages< void >::TARGETSHIELDB:
    case UnitImages< void >::TARGETSHIELDR:
    case UnitImages< void >::TARGETSHIELDL:
    case UnitImages< void >::ENERGY:
    case UnitImages< void >::WARPENERGY:
    case UnitImages< void >::EJECT:
    case UnitImages< void >::LOCK:
    case UnitImages< void >::HULL:
    case UnitImages< void >::KPS:
    case UnitImages< void >::SETKPS:
    case UnitImages< void >::AUTOPILOT:
    case UnitImages< void >::COLLISION:
    case UnitImages< void >::COCKPIT_FPS:
    case UnitImages< void >::WARPFIELDSTRENGTH:
    case UnitImages< void >::MASSEFFECT:
    case UnitImages< void >::AUTOPILOT_MODAL:
    case UnitImages< void >::SPEC_MODAL:
    case UnitImages< void >::FLIGHTCOMPUTER_MODAL:
    case UnitImages< void >::TURRETCONTROL_MODAL:
    case UnitImages< void >::ECM_MODAL:
    case UnitImages< void >::CLOAK_MODAL:
    case UnitImages< void >::TRAVELMODE_MODAL:
    case UnitImages< void >::RECIEVINGFIRE_MODAL:
    case UnitImages< void >::RECEIVINGMISSILES_MODAL:
    case UnitImages< void >::RECEIVINGMISSILELOCK_MODAL:
    case UnitImages< void >::RECEIVINGTARGETLOCK_MODAL:
    case UnitImages< void >::COLLISIONWARNING_MODAL:
    case UnitImages< void >::CANJUMP_MODAL:
    case UnitImages< void >::CANDOCK_MODAL:
        for (iter = attributes.begin(); iter != attributes.end(); iter++) {
            switch ( attribute_map.lookup( (*iter).name ) )
            {
            case XFILE:
                gaugename = (*iter).value;
                break;
            case NETWORK:
                if ( XMLSupport::parse_bool( (*iter).value ) != false )
                    return;                      //Don't show if not in multiplayer (or single if false)

                break;
            case TOPY:
                topy   = XMLSupport::parse_float( (*iter).value );
                break;
            case BOTY:
                boty   = XMLSupport::parse_float( (*iter).value );
                break;
            case LEFT:
                leftx  = XMLSupport::parse_float( (*iter).value );
                break;
            case RIGHT:
                rightx = XMLSupport::parse_float( (*iter).value );
                break;
            case XSIZE:
                xsize  = XMLSupport::parse_float( (*iter).value );
                break;
            case YSIZE:
                ysize  = XMLSupport::parse_float( (*iter).value );
                break;
            case XCENT:
                xcent  = XMLSupport::parse_float( (*iter).value );
                break;
            case YCENT:
                ycent  = XMLSupport::parse_float( (*iter).value );
                break;
            case G_UP:
                tmpdir = Gauge::GAUGE_UP;
                break;
            case G_DOWN:
                tmpdir = Gauge::GAUGE_DOWN;
                break;
            case G_LEFT:
                tmpdir = Gauge::GAUGE_LEFT;
                break;
            case G_RIGHT:
                tmpdir = Gauge::GAUGE_RIGHT;
                break;
            case G_TIME:
                tmpdir = Gauge::GAUGE_TIME;
                break;
            }
        }
        gauges[elem] = new Gauge( gaugename.c_str(), tmpdir );
        if (xsize != -1)
            gauges[elem]->SetSize( xsize, ysize );
        if (xcent != FLT_MAX)
            gauges[elem]->SetPosition( xcent, ycent );
        if (leftx != -10 && rightx != -10 && topy != -10 && boty != -10) {
            gauges[elem]->SetPosition( .5*(leftx+rightx), .5*(topy+boty) );
            gauges[elem]->SetSize( fabs( leftx-rightx ), fabs( topy-boty ) );
        }
        break;
    case CROSSHAIRS:
    case PANEL:
        if (elem == CROSSHAIRS) {
            if (Panel.size() == 0)
                Panel.push_back( NULL );
            if ( Panel.front() ) {
                delete Panel.front();
                Panel.front() = NULL;
            }
            newsprite = &Panel.front();
        } else {
            if (Panel.size() == 0)              /* Create NULL crosshairs */
                Panel.push_back( NULL );
            Panel.push_back( NULL );
            newsprite = &Panel.back();
        }
        goto loadsprite;
    case RADAR:
        newsprite = &radarSprites[0];
        goto loadsprite;
    case REARRADAR:
        newsprite = &radarSprites[1];
        goto loadsprite;
    case LVDU:
        vdu.push_back( NULL );
        newvdu  = &vdu.back();
        mymodes = VDU::MANIFEST|VDU::WEAPON|VDU::DAMAGE|VDU::OBJECTIVES|VDU::SHIELD;
        default_mode = VDU::OBJECTIVES;
#ifdef NETCOMM_WEBCAM
        mymodes = mymodes|VDU::WEBCAM;
#endif

        goto loadsprite;
    case RVDU:
        vdu.push_back( NULL );
        newvdu  = &vdu.back();
        mymodes = VDU::TARGETMANIFEST|VDU::NAV|VDU::TARGET;
        default_mode = VDU::TARGET;
        goto loadsprite;
    case AVDU:
        vdu.push_back( NULL );
        newvdu  = &vdu.back();
        mymodes = VDU::MSG;
        for (iter = attributes.begin(); iter != attributes.end(); iter++) {
            switch ( attribute_map.lookup( (*iter).name ) )
            {
            case VDUTYPE:
                {
                    mymodes = parse_vdu_type( (*iter).value.c_str() );
                    std::string firstmode = (*iter).value.substr( 0, (*iter).value.find( " " ) );
                    default_mode = parse_vdu_type( firstmode.c_str() );
                    break;
                }
            default:
                break;
            }
        }
        goto loadsprite;
loadsprite:
        for (iter = attributes.begin(); iter != attributes.end(); iter++) {
            switch ( attribute_map.lookup( (*iter).name ) )
            {
            case NETWORK:
                if ( XMLSupport::parse_bool( (*iter).value ) != false )
                    return;                      //Don't show if not in multiplayer (or single if false)

                break;
            case XFILE:
                if (newsprite) {
                    std::string tmp = getRes( (*iter).value );
                    bool bil = elem == PANEL ? panel_smooth : crosshair_smooth;
                    (*newsprite) = new VSSprite( tmp.c_str(), bil ? BILINEAR : NEAREST );
                    if ( !(*newsprite)->LoadSuccess() ) {
                        delete (*newsprite);
                        (*newsprite) = new VSSprite( (*iter).value.c_str(), bil ? BILINEAR : NEAREST );
                    }
                    adjsprite = *newsprite;
                } else if (newvdu) {
                    VDU *tmp = new VDU( (*iter).value.c_str(), text, mymodes, rows, cols, &StartArmor[0], &maxhull );
                    (*newvdu) = tmp;
                    adjsprite = *newvdu;
                    if (tmp->getMode() != default_mode) {
                        for (int i = 0; i < 32; ++i) {
                            tmp->SwitchMode( NULL );
                            if (tmp->getMode() == default_mode)
                                break;
                        }
                    }
                }
                break;
            case TOPY:
                topy   = XMLSupport::parse_float( (*iter).value );
                break;
            case BOTY:
                boty   = XMLSupport::parse_float( (*iter).value );
                break;
            case LEFT:
                leftx  = XMLSupport::parse_float( (*iter).value );
                break;
            case RIGHT:
                rightx = XMLSupport::parse_float( (*iter).value );
                break;
            case XSIZE:
                xsize  = XMLSupport::parse_float( (*iter).value );
                break;
            case YSIZE:
                ysize  = XMLSupport::parse_float( (*iter).value );
                break;
            case XCENT:
                xcent  = XMLSupport::parse_float( (*iter).value );
                break;
            case YCENT:
                ycent  = XMLSupport::parse_float( (*iter).value );
                break;
            case ROWS:
                rows   = XMLSupport::parse_int( (*iter).value );
                break;
            case COLS:
                cols   = XMLSupport::parse_int( (*iter).value );
                break;
                break;
            }
        }
        if (adjsprite) {
            if (xsize != -1)
                adjsprite->SetSize( xsize, ysize );
            if (xcent != FLT_MAX)
                adjsprite->SetPosition( xcent, ycent );
            if (leftx != -10 && rightx != -10 && topy != -10 && boty != -10) {
                adjsprite->SetPosition( .5*(leftx+rightx), .5*(topy+boty) );
                adjsprite->SetSize( fabs( leftx-rightx ), fabs( topy-boty ) );
            }
        } else if ( newsprite == &Panel.back() ) {
            Panel.erase( Panel.end()-1 );             //don't want null panels
        }
        break;
    case EVENT:
        {
            std::string soundfile;
            bool looping = false;
            float gain = 1.0f;
            EVENTID event = Cockpit::NUM_EVENTS;
            
            for (iter = attributes.begin(); iter != attributes.end(); iter++) {
                switch ( attribute_map.lookup( (*iter).name ) )
                {
                case SOUNDFILE:
                    soundfile = (*iter).value;
                    break;
                case LOOPING:
                    looping = XMLSupport::parse_bool( (*iter).value );
                    break;
                case EVENT:
                    event = (Cockpit::EVENTID) attribute_map.lookup( (*iter).value );
                    break;
                case GAIN:
                    gain = XMLSupport::parse_float( (*iter).value );
                    break;
                default:
                    break;
                }
            }
            
            if (!soundfile.empty() && (event > 0) && (event < Cockpit::NUM_EVENTS)) {
                SetSoundForEvent(event, SoundContainer(soundfile, looping, gain));
            }
        }
        break;
    default:
        break;
    }
}

void GameCockpit::endElement( const string &name )
{
}

using namespace VSFileSystem;

void GameCockpit::LoadXML( const char *filename )
{

    VSFile    f;
    if (filename[0] != '\0')
        if(f.OpenReadOnly( filename, CockpitFile ))
            LoadXML(f);
}

void GameCockpit::LoadXML( VSFileSystem::VSFile &f )
{
    if ( !f.Valid() ) {
        cockpit_offset  = 0;
        viewport_offset = 0;
        Panel.push_back( new VSSprite( "crosshairs.spr" ) );
        return;
    }
    XML_Parser parser = XML_ParserCreate( NULL );
    XML_SetUserData( parser, this );
    XML_SetElementHandler( parser, &Cockpit::beginElement, &Cockpit::endElement );

    XML_Parse( parser, ( f.ReadFull() ).c_str(), f.Size(), 1 );
    /*
     *  do {
     * #ifdef BIDBG
     *  char *buf = (XML_Char*)XML_GetBuffer(parser, chunk_size);
     * #else
     *  char buf[chunk_size];
     * #endif
     *  int length;
     *
     *  length = VSFileSystem::vs_read (buf,1, chunk_size,inFile);
     *  //length = inFile.gcount();
     * #ifdef BIDBG
     *  XML_ParseBuffer(parser, length, VSFileSystem::vs_feof(inFile));
     * #else
     *  XML_Parse (parser,buf,length,VSFileSystem::vs_feof(inFile));
     * #endif
     *  } while(!VSFileSystem::vs_feof(inFile));
     *  VSFileSystem::vs_close (inFile);
     */
    XML_ParserFree( parser );
}

