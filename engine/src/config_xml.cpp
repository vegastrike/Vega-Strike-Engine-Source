/*
 * config_xml.cpp
 *
 * Copyright (C) 2001-2002 Daniel Horn
 * Copyright (C) Alexander Rawass
 * Copyright (C) 2020 Stephen G. Tuggy, pyramid3d, and other Vega Strike contributors
 * Copyright (C) 2021 Stephen G. Tuggy
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

/*
 *  xml Configuration written by Alexander Rawass <alexannika@users.sourceforge.net>
 */

#include <expat.h>
#include "xml_support.h"
#include "vegastrike.h"
#include <assert.h>
#include "config_xml.h"
#include "easydom.h"
#include "cmd/ai/flykeyboard.h"
#include "cmd/ai/firekeyboard.h"
#include "cmd/music.h"
#include "gfx/loc_select.h"
#include "audiolib.h"
#include "in_joystick.h"
#include "main_loop.h" //for CockpitKeys
#include "gfx/cockpit.h"
#include "in_kb_data.h"
#include "python/python_compile.h"
#include "gfx/screenshot.h"
#include "vs_logging.h"

/* *********************************************************** */

GameVegaConfig::GameVegaConfig( const char *configfile ) : VegaConfig( configfile )
{
    initCommandMap();
    initKeyMap();
    //set hatswitches to off
    for (int h = 0; h < MAX_HATSWITCHES; h++) {
        hatswitch_margin[h] = 2.0;
        for (int v = 0; v < MAX_VALUES; v++)
            hatswitch[h][v] = 2.0;
    }
    for (int i = 0; i < MAX_AXES; i++) {
        axis_axis[i] = -1;
        axis_joy[i]  = -1;
    }
}

/* *********************************************************** */

#if 1

const float volinc = 1;
const float dopinc = .1;
void RunPythonPress( const KBData &, KBSTATE );
void RunPythonRelease( const KBData &, KBSTATE );
void RunPythonToggle( const KBData &, KBSTATE );
void RunPythonPhysicsFrame( const KBData &, KBSTATE );
void incmusicvol( const KBData&, KBSTATE a );
void decmusicvol( const KBData&, KBSTATE a );
bool screenshotkey = false;

void doReloadShader( const KBData&, KBSTATE a )
{
    if (a == PRESS)
        GFXReloadDefaultShader();
}

void doScreenshot( const KBData&, KBSTATE a )
{
    if (a == PRESS)
        screenshotkey = true;
}

void incvol( const KBData&, KBSTATE a )
{
#ifdef HAVE_AL
    if (a == DOWN)
        AUDChangeVolume( AUDGetVolume()+volinc );
#endif
}

void decvol( const KBData&, KBSTATE a )
{
#ifdef HAVE_AL
    if (a == DOWN)
        AUDChangeVolume( AUDGetVolume()-volinc );
#endif
}

void mute( const KBData&, KBSTATE a )
{
#ifdef HAVE_AL
#endif
}

void incdop( const KBData&, KBSTATE a )
{
#ifdef HAVE_AL
    if (a == DOWN)
        AUDChangeDoppler( AUDGetDoppler()+dopinc );
#endif
}
void decdop( const KBData&, KBSTATE a )
{
#ifdef HAVE_AL
    if (a == DOWN)
        AUDChangeDoppler( AUDGetDoppler()-dopinc );
#endif
}

#endif //1
/* *********************************************************** */

void GameVegaConfig::initKeyMap()
{
    //mapping from special key string to glut key
    key_map["space"]           = ' ';
    key_map["return"]          = WSK_RETURN;
    key_map["enter"]           = WSK_KP_ENTER;
    key_map["function-1"]      = WSK_F1;
    key_map["function-2"]      = WSK_F2;
    key_map["function-3"]      = WSK_F3;
    key_map["function-4"]      = WSK_F4;
    key_map["function-5"]      = WSK_F5;
    key_map["function-6"]      = WSK_F6;
    key_map["function-7"]      = WSK_F7;
    key_map["function-8"]      = WSK_F8;
    key_map["function-9"]      = WSK_F9;
    key_map["function-10"]     = WSK_F10;
    key_map["function-11"]     = WSK_F11;
    key_map["function-12"]     = WSK_F12;
    key_map["function-13"]     = WSK_F13;
    key_map["function-14"]     = WSK_F14;
    key_map["function-15"]     = WSK_F15;
    key_map["keypad-insert"]   = WSK_KP0;
    key_map["keypad-0"]        = WSK_KP0;
    key_map["keypad-1"]        = WSK_KP1;
    key_map["keypad-2"]        = WSK_KP2;
    key_map["keypad-3"]        = WSK_KP3;
    key_map["keypad-4"]        = WSK_KP4;
    key_map["keypad-5"]        = WSK_KP5;
    key_map["keypad-6"]        = WSK_KP6;
    key_map["keypad-7"]        = WSK_KP7;
    key_map["keypad-8"]        = WSK_KP8;
    key_map["keypad-9"]        = WSK_KP9;
    key_map["less-than"]       = '<';
    key_map["greater-than"]    = '>';
    key_map["keypad-numlock"]  = WSK_NUMLOCK;
    key_map["keypad-period"]   = WSK_KP_PERIOD;
    key_map["keypad-delete"]   = WSK_KP_PERIOD;
    key_map["keypad-divide"]   = WSK_KP_DIVIDE;
    key_map["keypad-multiply"] = WSK_KP_MULTIPLY;
    key_map["keypad-minus"]    = WSK_KP_MINUS;
    key_map["keypad-plus"]     = WSK_KP_PLUS;
    key_map["keypad-enter"]    = WSK_KP_ENTER;
    key_map["keypad-equals"]   = WSK_KP_EQUALS;
    key_map["scrollock"]       = WSK_SCROLLOCK;
    key_map["right-ctrl"]      = WSK_RCTRL;
    key_map["left-ctrl"]       = WSK_LCTRL;
    key_map["right-alt"]       = WSK_RALT;
    key_map["left-alt"]        = WSK_LALT;
    key_map["right-meta"]      = WSK_RMETA;
    key_map["left-meta"]       = WSK_LMETA;
    key_map["cursor-left"]     = WSK_LEFT;
    key_map["cursor-up"]       = WSK_UP;
    key_map["cursor-right"]    = WSK_RIGHT;
    key_map["cursor-down"]     = WSK_DOWN;
    key_map["cursor-pageup"]   = WSK_PAGEUP;
    key_map["cursor-pagedown"] = WSK_PAGEDOWN;
    key_map["cursor-home"]     = WSK_HOME;
    key_map["cursor-end"]      = WSK_END;
    key_map["cursor-insert"]   = WSK_INSERT;
    key_map["backspace"]       = WSK_BACKSPACE;
    key_map["capslock"]        = WSK_CAPSLOCK;
    key_map["cursor-delete"]   = WSK_DELETE;
    key_map["tab"]             = WSK_TAB;
    key_map["esc"]             = WSK_ESCAPE;
    key_map["break"]           = WSK_BREAK;
    key_map["pause"]           = WSK_PAUSE;
}

/* *********************************************************** */
extern void inc_time_compression( const KBData&, KBSTATE a );
extern void JoyStickToggleKey( const KBData&, KBSTATE a );
extern void SuicideKey( const KBData&, KBSTATE a );
extern void dec_time_compression( const KBData&, KBSTATE a );
extern void reset_time_compression( const KBData&, KBSTATE a );
extern void MapKey( const KBData&, KBSTATE a );
extern void VolUp( const KBData&, KBSTATE a );
extern void VolDown( const KBData&, KBSTATE a );

using namespace CockpitKeys;
CommandMap initGlobalCommandMap();
static CommandMap commandMap = initGlobalCommandMap();

static void ComposeFunctions( const KBData &composition, KBSTATE k )
{
    std::string s = composition.data;
    while ( s.length() ) {
        std::string::size_type where = s.find( " " );
        std::string t = s.substr( 0, where );
        if (where != std::string::npos)
            s = s.substr( where+1 );
        else
            s = "";
        where = t.find( "(" );
        std::string args;
        if (where != string::npos) {
            args = t.substr( where+1 );
            std::string::size_type paren = args.find( ")" );
            if (paren != string::npos)
                args = args.substr( 0, paren );
            t = t.substr( 0, where );
        }
        CommandMap::iterator i = commandMap.find( t );
        if ( i != commandMap.end() )
            (*i).second( args, k );
    }
}

static void ComposeFunctionsToggle( const KBData &composition, KBSTATE k )
{
    if (k == PRESS || k == RELEASE)
        ComposeFunctions( composition, k );
}

void GameVegaConfig::initCommandMap() // DELETE ME
{
}

/* *********************************************************** */

void GameVegaConfig::doBindings( configNode *node )
{
    vector< easyDomNode* >::const_iterator siter;
    for (siter = node->subnodes.begin(); siter != node->subnodes.end(); siter++) {
        configNode *cnode = (configNode*) (*siter);
        if ( (cnode)->Name() == "bind" ) {
            checkBind( cnode );
        } else if ( ( (cnode)->Name() == "axis" ) ) {
            doAxis( cnode );
        } else {
            VS_LOG(warning, (boost::format("Unknown tag: %1%") % (cnode)->Name()));
        }
    }
}

/* *********************************************************** */

void GameVegaConfig::doAxis( configNode *node )
{
    string name = node->attr_value( "name" );
    string myjoystick = node->attr_value( "joystick" );
    string axis = node->attr_value( "axis" );
    string invertstr = node->attr_value( "inverse" );
    string mouse_str = node->attr_value( "mouse" );
    if ( name.empty() || ( mouse_str.empty() && myjoystick.empty() ) || axis.empty() ) {
        VS_LOG(warning, "no correct axis description given ");
        return;
    }
    int joy_nr  = atoi( myjoystick.c_str() );
    if ( !mouse_str.empty() )
        joy_nr = MOUSE_JOYSTICK;
    int axis_nr = atoi( axis.c_str() );

    //no checks for correct number yet

    bool inverse = false;
    if ( !invertstr.empty() )
        inverse = XMLSupport::parse_bool( invertstr );
    if (name == "x") {
        axis_joy[0] = joy_nr;
        joystick[joy_nr]->axis_axis[0] = axis_nr;
        joystick[joy_nr]->axis_inverse[0] = inverse;
    } else if (name == "y") {
        axis_joy[1] = joy_nr;
        joystick[joy_nr]->axis_axis[1] = axis_nr;
        joystick[joy_nr]->axis_inverse[1] = inverse;
    } else if (name == "z") {
        axis_joy[2] = joy_nr;
        joystick[joy_nr]->axis_axis[2] = axis_nr;
        joystick[joy_nr]->axis_inverse[2] = inverse;
    } else if (name == "throttle") {
        axis_joy[3] = joy_nr;
        joystick[joy_nr]->axis_axis[3] = axis_nr;
        joystick[joy_nr]->axis_inverse[3] = inverse;
    } else if (name == "hatswitch") {
        string nr_str     = node->attr_value( "nr" );
        string margin_str = node->attr_value( "margin" );
        if ( nr_str.empty() || margin_str.empty() ) {
            VS_LOG(warning, "you have to assign a number and a margin to the hatswitch");
            return;
        }
        int   nr     = atoi( nr_str.c_str() );

        float margin = atof( margin_str.c_str() );
        hatswitch_margin[nr]   = margin;

        hatswitch_axis[nr]     = axis_nr;
        hatswitch_joystick[nr] = joy_nr;

        vector< easyDomNode* >::const_iterator siter;

        hs_value_index = 0;
        for (siter = node->subnodes.begin(); siter != node->subnodes.end(); siter++) {
            configNode *cnode = (configNode*) (*siter);
            checkHatswitch( nr, cnode );
        }
    } else {
        VS_LOG(warning, (boost::format("unknown axis %1%") % name));
        return;
    }
}

/* *********************************************************** */

void GameVegaConfig::checkHatswitch( int nr, configNode *node )
{
    if (node->Name() != "hatswitch") {
        VS_LOG(warning, "not a hatswitch node ");
        return;
    }
    string strval = node->attr_value( "value" );
    float  val    = atof( strval.c_str() );
    if (val > 1.0 || val < -1.0) {
        VS_LOG(error, "only hatswitch values from -1.0 to 1.0 allowed");
        return;
    }
    hatswitch[nr][hs_value_index] = val;
    VS_LOG(info, (boost::format("setting hatswitch nr %1% %2% = %3%") % nr % hs_value_index % val));
    hs_value_index++;
}

/* *********************************************************** */

void GameVegaConfig::checkBind( configNode *node )
{
    if (node->Name() != "bind") {
        VS_LOG(warning, "not a bind node ");
        return;
    }
    std::string tmp        = node->attr_value( "modifier" );
    int       modifier     = getModifier( tmp.c_str() );

    string    cmdstr       = node->attr_value( "command" );
    string    player_bound = node->attr_value( "player" );
    if ( player_bound.empty() )
        player_bound = "0";
    KBHandler handler      = commandMap[cmdstr];
    if (handler == NULL) {
        VS_LOG(error, (boost::format("No such command: %1%") % cmdstr));
        return;
    }
    string player_str = node->attr_value( "player" );
    string joy_str    = node->attr_value( "joystick" );
    string mouse_str  = node->attr_value( "mouse" );
    string keystr     = node->attr_value( "key" );
    string additional_data = node->attr_value( "data" );
    string buttonstr  = node->attr_value( "button" );
    string hat_str    = node->attr_value( "hatswitch" );
    string dighswitch = node->attr_value( "digital-hatswitch" );
    string direction  = node->attr_value( "direction" );
    if ( !player_str.empty() ) {
        if ( !joy_str.empty() ) {
            int jn = atoi( joy_str.c_str() );
            if (jn < MAX_JOYSTICKS) {
                joystick[jn]->player = atoi( player_str.c_str() );
            }
        } else if ( !mouse_str.empty() ) {
            joystick[MOUSE_JOYSTICK]->player = atoi( player_str.c_str() );
        }
    }
    if ( !keystr.empty() ) {
        //normal keyboard key
        //now map the command to a callback function and bind it
        if (keystr.length() == 1) {
            BindKey( keystr[0], modifier, XMLSupport::parse_int( player_bound ), handler, KBData( additional_data ) );
        } else {
            int glut_key = key_map[keystr];
            if (glut_key == 0) {
                VS_LOG(error, (boost::format("No such special key: %1%") % keystr));
                return;
            }
            BindKey( glut_key, modifier, XMLSupport::parse_int( player_bound ), handler, KBData( additional_data ) );
        }
    } else if ( !buttonstr.empty() ) {
        //maps a joystick button or analogue hatswitch button
        int button_nr = atoi( buttonstr.c_str() );
        if ( joy_str.empty() && mouse_str.empty() ) {
            //it has to be the analogue hatswitch
            if ( hat_str.empty() ) {
                VS_LOG(error, "you got to give an analogue hatswitch number");
                return;
            }
            int hatswitch_nr = atoi( hat_str.c_str() );

            BindHatswitchKey( hatswitch_nr, button_nr, handler, KBData( additional_data ) );

        } else {
            //joystick button
            int joystick_nr;
            if ( mouse_str.empty() ) {
                joystick_nr = atoi( joy_str.c_str() );
            } else {
                joystick_nr = (MOUSE_JOYSTICK);
            }
            if ( joystick[joystick_nr]->isAvailable() ) {
                //now map the command to a callback function and bind it

                //yet to check for correct buttons/joy-nr

                BindJoyKey( joystick_nr, button_nr, handler, KBData( additional_data ) );
            } else {
                static bool first = true;
                if (first) {
                    VS_LOG(warning, "\nrefusing to bind command to joystick (joy-nr too high)");
                    first = false;
                }
            }
        }
    } else if ( !( dighswitch.empty() || direction.empty() || ( mouse_str.empty() && joy_str.empty() ) ) ) {
        //digital hatswitch or ...
        if ( dighswitch.empty() || direction.empty() || ( mouse_str.empty() && joy_str.empty() ) ) {
            VS_LOG(error, "you have to specify joystick,digital-hatswitch,direction");
            return;
        }
        int hsw_nr = atoi( dighswitch.c_str() );

        int joy_nr;
        if ( mouse_str.empty() )
            joy_nr = atoi( joy_str.c_str() );
        else
            joy_nr = MOUSE_JOYSTICK;
        if ( !(joystick[joy_nr]->isAvailable() && hsw_nr < joystick[joy_nr]->nr_of_hats) ) {
            VS_LOG(error, "refusing to bind digital hatswitch: no such hatswitch");
            return;
        }
        int dir_index;
        if (direction == "center") {
            dir_index = VS_HAT_CENTERED;
        } else if (direction == "up") {
            dir_index = VS_HAT_UP;
        } else if (direction == "right") {
            dir_index = VS_HAT_RIGHT;
        } else if (direction == "left") {
            dir_index = VS_HAT_LEFT;
        } else if (direction == "down") {
            dir_index = VS_HAT_DOWN;
        } else if (direction == "rightup") {
            dir_index = VS_HAT_RIGHTUP;
        } else if (direction == "rightdown") {
            dir_index = VS_HAT_RIGHTDOWN;
        } else if (direction == "leftup") {
            dir_index = VS_HAT_LEFTUP;
        } else if (direction == "leftdown") {
            dir_index = VS_HAT_LEFTDOWN;
        } else {
            VS_LOG(error, "no valid direction string");
            return;
        }
        BindDigitalHatswitchKey( joy_nr, hsw_nr, dir_index, handler, KBData( additional_data ) );
        VS_LOG(info, (boost::format("Bound joy %1% hatswitch %2% dir_index %3% to command %4%") % joy_nr % hsw_nr % dir_index % cmdstr));
    } else {
        return;
    }
}

/* *********************************************************** */

void GameVegaConfig::bindKeys()
{
    doBindings( bindings );
}

/* *********************************************************** */
CommandMap initGlobalCommandMap()
{
    //I don't knwo why this gives linker errors!
    CommandMap commandMap;
    commandMap["NoPositionalKey"]    = mute;
    commandMap["DopplerInc"] = incdop;
    commandMap["Cockpit::NavScreen"] = Cockpit::NavScreen;
    commandMap["DopplerDec"]         = decdop;
    commandMap["VolumeInc"]          = VolUp;
    commandMap["VolumeDec"]          = VolDown;
    commandMap["MusicVolumeInc"]     = incmusicvol;
    commandMap["MusicVolumeDec"]     = decmusicvol;
    commandMap["SetShieldsOneThird"] = FireKeyboard::SetShieldsOneThird;
    commandMap["SetShieldsOff"]      = FireKeyboard::SetShieldsOff;
    commandMap["SetShieldsTwoThird"] = FireKeyboard::SetShieldsTwoThird;
    commandMap["SwitchControl"]      = Cockpit::SwitchControl;
    commandMap["Respawn"] = Cockpit::Respawn;
    commandMap["TurretControl"]      = Cockpit::TurretControl;

    commandMap["TimeInc"]   = inc_time_compression;
    commandMap["TimeDec"]   = dec_time_compression;
    commandMap["TimeReset"] = reset_time_compression;
    //mapping from command string to keyboard handler
    //Networking bindings

    commandMap["SwitchWebcam"]     = FlyByKeyboard::SwitchWebcam;
    commandMap["SwitchSecured"]    = FlyByKeyboard::SwitchSecured;
    commandMap["ChangeCommStatus"] = FlyByKeyboard::ChangeCommStatus;
    commandMap["UpFreq"]                           = FlyByKeyboard::UpFreq;
    commandMap["DownFreq"]                         = FlyByKeyboard::DownFreq;

    commandMap["ThrustModeKey"]                    = FlyByKeyboard::KSwitchFlightMode;
    commandMap["ThrustRight"]                      = FlyByKeyboard::KThrustRight;
    commandMap["ThrustLeft"]                       = FlyByKeyboard::KThrustLeft;
    commandMap["ThrustBack"]                       = FlyByKeyboard::KThrustBack;
    commandMap["ThrustFront"]                      = FlyByKeyboard::KThrustFront;
    commandMap["ThrustDown"]                       = FlyByKeyboard::KThrustDown;
    commandMap["ThrustUp"]                         = FlyByKeyboard::KThrustUp;
    commandMap["TextMessage"]                      = CockpitKeys::TextMessageKey;
    commandMap["JoyStickToggleKey"]                = JoyStickToggleKey;
    commandMap["ToggleAutotracking"]               = FireKeyboard::ToggleAutotracking;
    commandMap["SheltonKey"]                       = FlyByKeyboard::SheltonKey;
    commandMap["MatchSpeedKey"]                    = FlyByKeyboard::MatchSpeedKey;
    commandMap["PauseKey"]                         = FireKeyboard::TogglePause;
    commandMap["JumpKey"]                          = FlyByKeyboard::JumpKey;
    commandMap["AutoKey"]                          = FlyByKeyboard::AutoKey;
    commandMap["SwitchCombatMode"]                 = FlyByKeyboard::SwitchCombatModeKey;
    commandMap["StartKey"]                         = FlyByKeyboard::StartKey;
    commandMap["StopKey"]                          = FlyByKeyboard::StopKey;
    commandMap["Screenshot"]                       = doScreenshot;
    commandMap["UpKey"] = FlyByKeyboard::UpKey;
    commandMap["DownKey"]                          = FlyByKeyboard::DownKey;
    commandMap["LeftKey"]                          = FlyByKeyboard::LeftKey;
    commandMap["RightKey"]                         = FlyByKeyboard::RightKey;
    commandMap["ABKey"] = FlyByKeyboard::ABKey;
    commandMap["AccelKey"]                         = FlyByKeyboard::AccelKey;
    commandMap["DecelKey"]                         = FlyByKeyboard::DecelKey;
    commandMap["RollLeftKey"]                      = FlyByKeyboard::RollLeftKey;
    commandMap["RollRightKey"]                     = FlyByKeyboard::RollRightKey;
    commandMap["SetVelocityRefKey"]                = FlyByKeyboard::SetVelocityRefKey;
    commandMap["SetVelocityNullKey"]               = FlyByKeyboard::SetNullVelocityRefKey;
    commandMap["ToggleGlow"]                       = FireKeyboard::ToggleGlow;
    commandMap["ToggleWarpDrive"]                  = FireKeyboard::ToggleWarpDrive;
    commandMap["ToggleAnimation"]                  = FireKeyboard::ToggleAnimation;
    commandMap["CommAttackTarget"]                 = FireKeyboard::AttackTarget;
    commandMap["ASAP"] = FlyByKeyboard::EngageSpecAuto;

    commandMap["CommHelpMeOutCrit"]                = FireKeyboard::HelpMeOutCrit;
    commandMap["CommHelpMeOutFaction"]             = FireKeyboard::HelpMeOutFaction;
    commandMap["JoinFlightgroup"]                  = FireKeyboard::JoinFg;
    commandMap["CommAttackTarget"]                 = FireKeyboard::AttackTarget;
    commandMap["CommHelpMeOut"]                    = FireKeyboard::HelpMeOut;
    commandMap["CommFormUp"]                       = FireKeyboard::FormUp;
    commandMap["CommDockWithMe"]                   = FireKeyboard::DockWithMe;
    commandMap["CommDefendTarget"]                 = FireKeyboard::DefendTarget;
    commandMap["CommDockAtTarget"]                 = FireKeyboard::DockTarget;
    commandMap["CommHoldPosition"]                 = FireKeyboard::HoldPosition;
    //Added for nearest unit targeting -ch
    commandMap["NearestHostileTargetKey"]          = FireKeyboard::NearestHostileTargetKey;
    commandMap["NearestDangerousHostileKey"]       = FireKeyboard::NearestDangerousHostileKey;
    commandMap["NearestFriendlyKey"]               = FireKeyboard::NearestFriendlyKey;
    commandMap["NearestBaseKey"]                   = FireKeyboard::NearestBaseKey;
    commandMap["NearestPlanetKey"]                 = FireKeyboard::NearestPlanetKey;
    commandMap["NearestJumpKey"]                   = FireKeyboard::NearestJumpKey;

    commandMap["CommBreakForm"]                    = FireKeyboard::BreakFormation;

    commandMap["Comm1Key"]                         = FireKeyboard::PressComm1Key;
    commandMap["Comm2Key"]                         = FireKeyboard::PressComm2Key;
    commandMap["Comm3Key"]                         = FireKeyboard::PressComm3Key;
    commandMap["Comm4Key"]                         = FireKeyboard::PressComm4Key;
    commandMap["Comm5Key"]                         = FireKeyboard::PressComm5Key;
    commandMap["Comm6Key"]                         = FireKeyboard::PressComm6Key;
    commandMap["Comm7Key"]                         = FireKeyboard::PressComm7Key;
    commandMap["Comm8Key"]                         = FireKeyboard::PressComm8Key;
    commandMap["Comm9Key"]                         = FireKeyboard::PressComm9Key;
    commandMap["Comm10Key"]                        = FireKeyboard::PressComm10Key;

    commandMap["SaveTarget1"]                      = FireKeyboard::SaveTarget1Key;
    commandMap["SaveTarget2"]                      = FireKeyboard::SaveTarget2Key;
    commandMap["SaveTarget3"]                      = FireKeyboard::SaveTarget3Key;
    commandMap["SaveTarget4"]                      = FireKeyboard::SaveTarget4Key;
    commandMap["SaveTarget5"]                      = FireKeyboard::SaveTarget5Key;
    commandMap["SaveTarget6"]                      = FireKeyboard::SaveTarget6Key;
    commandMap["SaveTarget7"]                      = FireKeyboard::SaveTarget7Key;
    commandMap["SaveTarget8"]                      = FireKeyboard::SaveTarget8Key;
    commandMap["SaveTarget9"]                      = FireKeyboard::SaveTarget9Key;
    commandMap["SaveTarget10"]                     = FireKeyboard::SaveTarget10Key;

    commandMap["RestoreTarget1"]                   = FireKeyboard::RestoreTarget1Key;
    commandMap["RestoreTarget2"]                   = FireKeyboard::RestoreTarget2Key;
    commandMap["RestoreTarget3"]                   = FireKeyboard::RestoreTarget3Key;
    commandMap["RestoreTarget4"]                   = FireKeyboard::RestoreTarget4Key;
    commandMap["RestoreTarget5"]                   = FireKeyboard::RestoreTarget5Key;
    commandMap["RestoreTarget6"]                   = FireKeyboard::RestoreTarget6Key;
    commandMap["RestoreTarget7"]                   = FireKeyboard::RestoreTarget7Key;
    commandMap["RestoreTarget8"]                   = FireKeyboard::RestoreTarget8Key;
    commandMap["RestoreTarget9"]                   = FireKeyboard::RestoreTarget9Key;
    commandMap["RestoreTarget10"]                  = FireKeyboard::RestoreTarget10Key;

    commandMap["FreeSlaves"]                       = FireKeyboard::FreeSlaveKey;
    commandMap["Enslave"]                          = FireKeyboard::EnslaveKey;
    commandMap["EjectCargoKey"]                    = FireKeyboard::EjectCargoKey;
    commandMap["EjectNonMissionCargoKey"]          = FireKeyboard::EjectNonMissionCargoKey;
    commandMap["EjectKey"]                         = FireKeyboard::EjectKey;
    commandMap["EjectDockKey"]                     = FireKeyboard::EjectDockKey;
    commandMap["SuicideKey"]                       = SuicideKey;
    commandMap["TurretAIOn"]                       = FireKeyboard::TurretAIOn;
    commandMap["TurretAIOff"]                      = FireKeyboard::TurretAIOff;
    commandMap["TurretAIFireAtWill"]               = FireKeyboard::TurretFireAtWill;

    commandMap["DockKey"]                          = FireKeyboard::DockKey;
    commandMap["UnDockKey"]                        = FireKeyboard::UnDockKey;
    commandMap["RequestClearenceKey"]              = FireKeyboard::RequestClearenceKey;
    commandMap["FireKey"]                          = FireKeyboard::FireKey;
    commandMap["MissileKey"]                       = FireKeyboard::MissileKey;
    commandMap["TargetKey"]                        = FireKeyboard::TargetKey;
    commandMap["LockTargetKey"]                    = FireKeyboard::LockKey;
    commandMap["ReverseTargetKey"]                 = FireKeyboard::ReverseTargetKey;
    commandMap["PickTargetKey"]                    = FireKeyboard::PickTargetKey;
    commandMap["SubUnitTargetKey"]                 = FireKeyboard::SubUnitTargetKey;
    commandMap["NearestTargetKey"]                 = FireKeyboard::NearestTargetKey;
    commandMap["MissileTargetKey"]                 = FireKeyboard::MissileTargetKey;
    commandMap["IncomingMissileTargetKey"]         = FireKeyboard::IncomingMissileTargetKey;
    commandMap["ReverseMissileTargetKey"]          = FireKeyboard::ReverseMissileTargetKey;
    commandMap["ReverseIncomingMissileTargetKey"]  = FireKeyboard::ReverseIncomingMissileTargetKey;
    commandMap["ThreatTargetKey"]                  = FireKeyboard::ThreatTargetKey;
    commandMap["SigTargetKey"]                     = FireKeyboard::SigTargetKey;
    commandMap["UnitTargetKey"]                    = FireKeyboard::UnitTargetKey;
    commandMap["ReversePickTargetKey"]             = FireKeyboard::ReversePickTargetKey;
    commandMap["ReverseNearestTargetKey"]          = FireKeyboard::ReverseNearestTargetKey;
    commandMap["ReverseThreatTargetKey"]           = FireKeyboard::ReverseThreatTargetKey;
    commandMap["ReverseSigTargetKey"]              = FireKeyboard::ReverseSigTargetKey;
    commandMap["ReverseUnitTargetKey"]             = FireKeyboard::ReverseUnitTargetKey;
    commandMap["MissionTargetKey"]                 = FireKeyboard::MissionTargetKey;
    commandMap["ReverseMissionTargetKey"]          = FireKeyboard::ReverseMissionTargetKey;
    commandMap["TurretTargetKey"]                  = FireKeyboard::TargetTurretKey;
    commandMap["TurretPickTargetKey"]              = FireKeyboard::PickTargetTurretKey;
    commandMap["TurretNearestTargetKey"]           = FireKeyboard::NearestTargetTurretKey;
    commandMap["TurretThreatTargetKey"]            = FireKeyboard::ThreatTargetTurretKey;
    commandMap["WeapSelKey"]                       = FireKeyboard::WeapSelKey;
    commandMap["MisSelKey"]                        = FireKeyboard::MisSelKey;
    commandMap["ReverseWeapSelKey"]                = FireKeyboard::ReverseWeapSelKey;
    commandMap["ReverseMisSelKey"]                 = FireKeyboard::ReverseMisSelKey;
    commandMap["CloakKey"]                         = FireKeyboard::CloakKey;
    commandMap["ECMKey"]                           = FireKeyboard::ECMKey;

    commandMap["RunPythonPress"]                   = RunPythonPress;
    commandMap["RunPythonRelease"]                 = RunPythonRelease;
    commandMap["RunPythonToggle"]                  = RunPythonToggle;
    commandMap["RunPythonPhysicsFrame"]            = RunPythonPhysicsFrame;
    commandMap["ComposeFunctions"]                 = ComposeFunctions;
    commandMap["ComposeFunctionsToggle"]           = ComposeFunctionsToggle;
    commandMap["Cockpit::ScrollDown"]              = CockpitKeys::ScrollDown;
    commandMap["Cockpit::ScrollUp"]                = CockpitKeys::ScrollUp;

    commandMap["Cockpit::PitchDown"]               = CockpitKeys::PitchDown;
    commandMap["Cockpit::PitchUp"]                 = CockpitKeys::PitchUp;
    commandMap["Cockpit::YawLeft"]                 = CockpitKeys::YawLeft;
    commandMap["Cockpit::YawRight"]                = CockpitKeys::YawRight;
    commandMap["Cockpit::LookDown"]                = CockpitKeys::LookDown;
    commandMap["Cockpit::LookUp"]                  = CockpitKeys::LookUp;
    commandMap["Cockpit::LookLeft"]                = CockpitKeys::LookLeft;
    commandMap["Cockpit::LookRight"]               = CockpitKeys::LookRight;
    commandMap["Cockpit::ViewTarget"]              = CockpitKeys::ViewTarget;
    commandMap["Cockpit::OutsideTarget"]           = CockpitKeys::OutsideTarget;
    commandMap["Cockpit::PanTarget"]               = CockpitKeys::PanTarget;
    commandMap["Cockpit::ZoomOut"]                 = CockpitKeys::ZoomOut;
    commandMap["Cockpit::ZoomIn"]                  = CockpitKeys::ZoomIn;
    commandMap["Cockpit::ZoomReset"]               = CockpitKeys::ZoomReset;
    commandMap["Cockpit::Inside"]                  = CockpitKeys::Inside;
    commandMap["Cockpit::InsideLeft"]              = CockpitKeys::InsideLeft;
    commandMap["Cockpit::InsideRight"]             = CockpitKeys::InsideRight;
    commandMap["Cockpit::InsideBack"]              = CockpitKeys::InsideBack;
    commandMap["Cockpit::SwitchLVDU"]              = CockpitKeys::SwitchLVDU;

    commandMap["Cockpit::CommMode"]                = CockpitKeys::CommModeVDU;
    commandMap["Cockpit::TargetMode"]              = CockpitKeys::TargetModeVDU;
    commandMap["Cockpit::ManifestMode"]            = CockpitKeys::ManifestModeVDU;
    commandMap["Cockpit::ViewMode"]                = CockpitKeys::ViewModeVDU;
    commandMap["Cockpit::DamageMode"]              = CockpitKeys::DamageModeVDU;
    commandMap["Cockpit::GunMode"]                 = CockpitKeys::GunModeVDU;
    commandMap["Cockpit::ReverseGunMode"]          = CockpitKeys::ReverseGunModeVDU;
    commandMap["Cockpit::MissileMode"]             = CockpitKeys::MissileModeVDU;
    commandMap["Cockpit::ReverseMissileMode"]      = CockpitKeys::ReverseMissileModeVDU;
    commandMap["Cockpit::ObjectiveMode"]           = CockpitKeys::ObjectiveModeVDU;
    commandMap["Cockpit::ScanningMode"]            = CockpitKeys::ScanningModeVDU;

    commandMap["Cockpit::MapKey"]                  = MapKey;
    commandMap["Cockpit::SwitchRVDU"]              = CockpitKeys::SwitchRVDU;
    commandMap["Cockpit::SwitchMVDU"]              = CockpitKeys::SwitchMVDU;
    commandMap["Cockpit::SwitchURVDU"]             = CockpitKeys::SwitchURVDU;
    commandMap["Cockpit::SwitchULVDU"]             = CockpitKeys::SwitchULVDU;
    commandMap["Cockpit::SwitchUMVDU"]             = CockpitKeys::SwitchUMVDU;
    commandMap["Cockpit::Behind"]                  = CockpitKeys::Behind;
    commandMap["Cockpit::Pan"]                     = CockpitKeys::Pan;
    commandMap["Cockpit::SkipMusicTrack"]          = CockpitKeys::SkipMusicTrack;

    commandMap["Cockpit::Quit"]                    = CockpitKeys::Quit;

    commandMap["Joystick::Mode::InertialXY"]       = FlyByKeyboard::JoyInertialXYPulsorKey;
    commandMap["Joystick::Mode::InertialXYToggle"] = FlyByKeyboard::JoyInertialXYToggleKey;
    commandMap["Joystick::Mode::InertialXZ"]       = FlyByKeyboard::JoyInertialXYPulsorKey;
    commandMap["Joystick::Mode::InertialXZToggle"] = FlyByKeyboard::JoyInertialXYToggleKey;
    commandMap["Joystick::Mode::Roll"]             = FlyByKeyboard::JoyRollPulsorKey;
    commandMap["Joystick::Mode::RollToggle"]       = FlyByKeyboard::JoyRollToggleKey;
    commandMap["Joystick::Mode::Bank"]             = FlyByKeyboard::JoyBankPulsorKey;
    commandMap["Joystick::Mode::BankToggle"]       = FlyByKeyboard::JoyBankToggleKey;

    commandMap["Flight::Mode::InertialPulsor"]     = FlyByKeyboard::InertialPulsorKey;
    commandMap["Flight::Mode::InertialToggle"]     = FlyByKeyboard::InertialToggleKey;

    commandMap["ConsoleKeys::BringConsole"]        = ConsoleKeys::BringConsole;
    commandMap["NewShader"]                        = doReloadShader;

    return commandMap;
}

