/*
 * config_xml.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2026 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file. Specifically: Alexander Rawass
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
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

/*
 *  xml Configuration written by Alexander Rawass <alexannika@users.sourceforge.net>
 */

#define PY_SSIZE_T_CLEAN
#include <boost/python.hpp>
#include <expat.h>
#include "root_generic/xml_support.h"
#include "src/vegastrike.h"
#include <assert.h>
#include "src/config_xml.h"
#include "root_generic/easydom.h"
#include "cmd/ai/flykeyboard.h"
#include "cmd/ai/firekeyboard.h"
#include "cmd/music.h"
#include "gfx/loc_select.h"
#include "src/audiolib.h"
#include "src/in_joystick.h"
#include "src/main_loop.h" //for CockpitKeys
#include "gfx/cockpit.h"
#include "src/in_kb_data.h"
#include "src/python/python_compile.h"
#include "src/vs_logging.h"
#include "src/sdl_key_converter.h"
#include "configuration/configuration.h"

/* *********************************************************** */

GameVegaConfig::GameVegaConfig(const char *configfile) : VegaConfig(configfile) {
    initCommandMap();
    initKeyMap();
    //set hatswitches to off
    for (int h = 0; h < MAX_HATSWITCHES; h++) {
        hatswitch_margin[h] = 2.0;
        for (int v = 0; v < MAX_VALUES; v++) {
            hatswitch[h][v] = 2.0;
        }
    }
    for (int i = 0; i < MAX_AXES; i++) {
        axis_axis[i] = -1;
        axis_joy[i] = -1;
    }
}

/* *********************************************************** */

#if 1

const float volinc = 1;
const float dopinc = .1;
void RunPythonPress(const KBData &, KBSTATE);
void RunPythonRelease(const KBData &, KBSTATE);
void RunPythonToggle(const KBData &, KBSTATE);
void RunPythonPhysicsFrame(const KBData &, KBSTATE);
void incmusicvol(const KBData &, KBSTATE a);
void decmusicvol(const KBData &, KBSTATE a);
bool screenshotkey = false;

void doReloadShader(const KBData &, KBSTATE a) {
    if (a == PRESS) {
        GFXReloadDefaultShader();
    }
}

void doScreenshot(const KBData &, KBSTATE a) {
    if (a == PRESS) {
        screenshotkey = true;
    }
}

void incvol(const KBData &, KBSTATE a) {
#ifdef HAVE_AL
    if (a == DOWN) {
        AUDChangeVolume(AUDGetVolume() + volinc);
    }
#endif
}

void decvol(const KBData &, KBSTATE a) {
#ifdef HAVE_AL
    if (a == DOWN) {
        AUDChangeVolume(AUDGetVolume() - volinc);
    }
#endif
}

void mute(const KBData &, KBSTATE a) {
#ifdef HAVE_AL
#endif
}

void incdop(const KBData &, KBSTATE a) {
#ifdef HAVE_AL
    if (a == DOWN) {
        AUDChangeDoppler(AUDGetDoppler() + dopinc);
    }
#endif
}

void decdop(const KBData &, KBSTATE a) {
#ifdef HAVE_AL
    if (a == DOWN) {
        AUDChangeDoppler(AUDGetDoppler() - dopinc);
    }
#endif
}

#endif //1

/* *********************************************************** */

void GameVegaConfig::initKeyMap() {
    //mapping from special key string to glut key
    key_map["space"] = ' ';
    key_map["return"] = WSK_RETURN;
    key_map["enter"] = WSK_KP_ENTER;
    key_map["function-1"] = WSK_F1;
    key_map["function-2"] = WSK_F2;
    key_map["function-3"] = WSK_F3;
    key_map["function-4"] = WSK_F4;
    key_map["function-5"] = WSK_F5;
    key_map["function-6"] = WSK_F6;
    key_map["function-7"] = WSK_F7;
    key_map["function-8"] = WSK_F8;
    key_map["function-9"] = WSK_F9;
    key_map["function-10"] = WSK_F10;
    key_map["function-11"] = WSK_F11;
    key_map["function-12"] = WSK_F12;
    key_map["function-13"] = WSK_F13;
    key_map["function-14"] = WSK_F14;
    key_map["function-15"] = WSK_F15;
    key_map["keypad-insert"] = WSK_KP0;
    key_map["keypad-0"] = WSK_KP0;
    key_map["keypad-1"] = WSK_KP1;
    key_map["keypad-2"] = WSK_KP2;
    key_map["keypad-3"] = WSK_KP3;
    key_map["keypad-4"] = WSK_KP4;
    key_map["keypad-5"] = WSK_KP5;
    key_map["keypad-6"] = WSK_KP6;
    key_map["keypad-7"] = WSK_KP7;
    key_map["keypad-8"] = WSK_KP8;
    key_map["keypad-9"] = WSK_KP9;
    key_map["less-than"] = '<';
    key_map["greater-than"] = '>';
    key_map["keypad-numlock"] = WSK_NUMLOCK;
    key_map["keypad-period"] = WSK_KP_PERIOD;
    key_map["keypad-delete"] = WSK_KP_PERIOD;
    key_map["keypad-divide"] = WSK_KP_DIVIDE;
    key_map["keypad-multiply"] = WSK_KP_MULTIPLY;
    key_map["keypad-minus"] = WSK_KP_MINUS;
    key_map["keypad-plus"] = WSK_KP_PLUS;
    key_map["keypad-enter"] = WSK_KP_ENTER;
    key_map["keypad-equals"] = WSK_KP_EQUALS;
    key_map["scrollock"] = WSK_SCROLLOCK;
    key_map["right-ctrl"] = WSK_RCTRL;
    key_map["left-ctrl"] = WSK_LCTRL;
    key_map["right-alt"] = WSK_RALT;
    key_map["left-alt"] = WSK_LALT;
    key_map["right-meta"] = WSK_RMETA;
    key_map["left-meta"] = WSK_LMETA;
    key_map["cursor-left"] = WSK_LEFT;
    key_map["cursor-up"] = WSK_UP;
    key_map["cursor-right"] = WSK_RIGHT;
    key_map["cursor-down"] = WSK_DOWN;
    key_map["cursor-pageup"] = WSK_PAGEUP;
    key_map["cursor-pagedown"] = WSK_PAGEDOWN;
    key_map["cursor-home"] = WSK_HOME;
    key_map["cursor-end"] = WSK_END;
    key_map["cursor-insert"] = WSK_INSERT;
    key_map["backspace"] = WSK_BACKSPACE;
    key_map["capslock"] = WSK_CAPSLOCK;
    key_map["cursor-delete"] = WSK_DELETE;
    key_map["tab"] = WSK_TAB;
    key_map["esc"] = WSK_ESCAPE;
    key_map["break"] = WSK_BREAK;
    key_map["pause"] = WSK_PAUSE;
}

/* *********************************************************** */
extern void inc_time_compression(const KBData &, KBSTATE a);
extern void JoyStickToggleKey(const KBData &, KBSTATE a);
extern void SuicideKey(const KBData &, KBSTATE a);
extern void dec_time_compression(const KBData &, KBSTATE a);
extern void reset_time_compression(const KBData &, KBSTATE a);
extern void MapKey(const KBData &, KBSTATE a);
extern void VolUp(const KBData &, KBSTATE a);
extern void VolDown(const KBData &, KBSTATE a);

using namespace CockpitKeys;
CommandMap initGlobalCommandMap();
static CommandMap commandMap = initGlobalCommandMap();

static void ComposeFunctions(const KBData &composition, KBSTATE k) {
    std::string s = composition.data;
    while (s.length()) {
        std::string::size_type where = s.find(" ");
        std::string t = s.substr(0, where);
        if (where != std::string::npos) {
            s = s.substr(where + 1);
        } else {
            s = "";
        }
        where = t.find("(");
        std::string args;
        if (where != string::npos) {
            args = t.substr(where + 1);
            std::string::size_type paren = args.find(")");
            if (paren != string::npos) {
                args = args.substr(0, paren);
            }
            t = t.substr(0, where);
        }
        CommandMap::iterator i = commandMap.find(t);
        if (i != commandMap.end()) {
            (*i).second(args, k);
        }
    }
}

static void ComposeFunctionsToggle(const KBData &composition, KBSTATE k) {
    if (k == PRESS || k == RELEASE) {
        ComposeFunctions(composition, k);
    }
}

void GameVegaConfig::initCommandMap() // DELETE ME
{
}

/* *********************************************************** */

// Fill the runtime input tables from the merged JSON config (the "actions"
// and "axes" sections of bindings.json), replacing the old vegastrike.config
// XML <bind>/<axis> path. The runtime input system itself
// (BindKey/BindJoyKey/BindDigitalHatswitchKey + axis_joy[]/joystick[].axis_axis[])
// is unchanged -- we populate the same in-memory structures the XML parser used to.
void GameVegaConfig::bindKeys() {
    const auto & cfg = configuration();
    CommandMap & cmd_map = commandMap;  // command name -> handler (initGlobalCommandMap)

    // Bind one keyboard entry (a single printable char, or a named special key).
    auto bind_keyboard = [&](const std::string & cmd, const vega_config::Configuration::ActionBinding & b) {
        KBHandler handler = cmd_map[cmd];
        if (handler == nullptr) {
            VS_LOG(error, (boost::format("No such command: %1%") % cmd));
            return;
        }
        int mod = getModifier(b.modifier);
        if (b.key.length() == 1) {
            // single printable char (shift already encoded, e.g. '+' for Shift+=)
            BindKey(b.key[0], mod, 0, handler, KBData());
        } else {
            // special key name (tab, cursor-up, function-1, keypad-*, ...)
            auto it = key_map.find(b.key);
            if (it == key_map.end()) {
                VS_LOG(error, (boost::format("No such special key: %1%") % b.key));
                return;
            }
            BindKey(it->second, mod, 0, handler, KBData());
        }
    };

    // Bind one mouse or joystick button entry.
    auto bind_button = [&](const std::string & cmd, const vega_config::Configuration::ActionBinding & b) {
        KBHandler handler = cmd_map[cmd];
        if (handler == nullptr) {
            VS_LOG(error, (boost::format("No such command: %1%") % cmd));
            return;
        }
        // Mouse buttons bind to the reserved mouse-joystick slot regardless of
        // joystick.enabled; joystick buttons only bind when the joystick is
        // enabled (so a Keyboard/Joystick selector can disable them).
        if (!b.is_mouse && !cfg.joystick.enabled) {
            return;
        }
        int joy_nr = b.is_mouse ? MOUSE_JOYSTICK : b.joystick;
        if (joy_nr >= MAX_JOYSTICKS || joystick[joy_nr] == nullptr || !joystick[joy_nr]->isAvailable()) {
            VS_LOG(warning, (boost::format("refusing to bind command %1% to joystick (not available)") % cmd));
            return;
        }
        BindJoyKey(joy_nr, b.button, handler, KBData());
    };

    // Bind one digital-hatswitch entry.
    auto bind_hat = [&](const std::string & cmd, const vega_config::Configuration::ActionBinding & b) {
        KBHandler handler = cmd_map[cmd];
        if (handler == nullptr) {
            VS_LOG(error, (boost::format("No such command: %1%") % cmd));
            return;
        }
        // Hatswitch is a joystick feature - ignore it when the joystick is disabled.
        if (!cfg.joystick.enabled) {
            return;
        }
        int joy_nr = b.joystick;
        if (joy_nr >= MAX_JOYSTICKS || joystick[joy_nr] == nullptr || !joystick[joy_nr]->isAvailable()
            || b.hatswitch >= joystick[joy_nr]->nr_of_hats) {
            VS_LOG(warning, (boost::format("refusing to bind command %1% to hatswitch (not available)") % cmd));
            return;
        }
        int dir_index = VS_HAT_CENTERED;
        if (b.direction == "up") { dir_index = VS_HAT_UP; }
        else if (b.direction == "right") { dir_index = VS_HAT_RIGHT; }
        else if (b.direction == "down") { dir_index = VS_HAT_DOWN; }
        else if (b.direction == "left") { dir_index = VS_HAT_LEFT; }
        else if (b.direction == "rightup") { dir_index = VS_HAT_RIGHTUP; }
        else if (b.direction == "rightdown") { dir_index = VS_HAT_RIGHTDOWN; }
        else if (b.direction == "leftup") { dir_index = VS_HAT_LEFTUP; }
        else if (b.direction == "leftdown") { dir_index = VS_HAT_LEFTDOWN; }
        BindDigitalHatswitchKey(joy_nr, b.hatswitch, dir_index, handler, KBData());
    };

    // Walk every action and bind each of its four device arrays.
    for (const auto & action : cfg.actions) {
        const std::string & cmd = action.first;
        const auto & bindings = action.second;
        for (const auto & b : bindings.keyboard) { bind_keyboard(cmd, b); }
        for (const auto & b : bindings.mouse) { bind_button(cmd, b); }
        for (const auto & b : bindings.joystick) { bind_button(cmd, b); }
        for (const auto & b : bindings.hat) { bind_hat(cmd, b); }
    }

    // Axes: x/y/z/throttle -> axis_joy[] + joystick[].axis_axis[]/axis_inverse[].
    // The runtime reads these in flyjoystick.cpp; same contract as the old XML doAxis.
    for (const auto & entry : cfg.axes) {
        const std::string & role = entry.first;
        const auto & ar = entry.second;
        int idx = -1;
        if (role == "x") { idx = AXIS_X; }
        else if (role == "y") { idx = AXIS_Y; }
        else if (role == "z") { idx = AXIS_Z; }
        else if (role == "throttle") { idx = AXIS_THROTTLE; }
        else { VS_LOG(warning, (boost::format("unknown axis %1%") % role)); continue; }

        // Bind the axes according to the selected flight-control device
        // (input.device), WITHOUT mutating the stored config (so the joystick
        // bindings survive a mode switch and come back when Joystick is re-selected).
        //   - Keyboard: no device drives any axis -> all roles unbind.
        //   - Mouse: only x/y (source=mouse) bind; z/throttle unbind.
        //   - Joystick: the configured roles bind (joystick.enabled governs).
        const std::string & device = cfg.input.device;
        bool is_mouse_axis = (ar.source == "mouse");
        if (device == "keyboard") {
            continue;  // keyboard drives no axes
        }
        if (device == "mouse" && idx != AXIS_X && idx != AXIS_Y) {
            continue;  // mouse drives only x/y; keep z/throttle unbound
        }
        // The device for x/y is decided by the per-role source in bindings.json
        // (axes.x/y.source), NOT by the global mouse.enabled flag - otherwise
        // enabling the mouse would steal x/y away from the joystick. Mouse only
        // drives x/y when that role is explicitly configured as source=mouse.
        // Joystick axes only apply when the joystick is enabled (Keyboard/Mouse modes disable it).
        if (!is_mouse_axis && !cfg.joystick.enabled) {
            continue;
        }
        int joy_nr;
        bool inverse = ar.inverse;
        int axis_nr = ar.axis;
        if (is_mouse_axis && (idx == AXIS_X || idx == AXIS_Y)) {
            // Mouse drives x/y via the reserved mouse-joystick slot; use its
            // x/y physical axes (0/1) and honor the global mouse inverse_x/y.
            joy_nr = MOUSE_JOYSTICK;
            axis_nr = (idx == AXIS_X) ? 0 : 1;
            inverse = (idx == AXIS_X) ? cfg.mouse.inverse_x : cfg.mouse.inverse_y;
        } else {
            joy_nr = (ar.source == "mouse") ? MOUSE_JOYSTICK : ar.joystick;
        }
        if (joy_nr >= MAX_JOYSTICKS || joystick[joy_nr] == nullptr) {
            VS_LOG(warning, (boost::format("refusing to assign axis %1% to joystick (not available)") % role));
            continue;
        }
        axis_joy[idx] = joy_nr;
        joystick[joy_nr]->axis_axis[idx] = axis_nr;
        joystick[joy_nr]->axis_inverse[idx] = inverse;
    }
}

/* *********************************************************** */

/* *********************************************************** */
CommandMap initGlobalCommandMap() {
    //I don't knwo why this gives linker errors!
    CommandMap commandMap;
    commandMap["NoPositionalKey"] = mute;
    commandMap["DopplerInc"] = incdop;
    commandMap["Cockpit::NavScreen"] = GameCockpit::NavScreen;
    commandMap["DopplerDec"] = decdop;
    commandMap["VolumeInc"] = VolUp;
    commandMap["VolumeDec"] = VolDown;
    commandMap["MusicVolumeInc"] = incmusicvol;
    commandMap["MusicVolumeDec"] = decmusicvol;
    commandMap["SetShieldsOneThird"] = FireKeyboard::SetShieldsOneThird;
    commandMap["SetShieldsOff"] = FireKeyboard::SetShieldsOff;
    commandMap["SetShieldsTwoThird"] = FireKeyboard::SetShieldsTwoThird;
    commandMap["SwitchControl"] = GameCockpit::SwitchControl;
    commandMap["Respawn"] = GameCockpit::Respawn;
    commandMap["TurretControl"] = GameCockpit::TurretControl;

    commandMap["TimeInc"] = inc_time_compression;
    commandMap["TimeDec"] = dec_time_compression;
    commandMap["TimeReset"] = reset_time_compression;
    //mapping from command string to keyboard handler
    //Networking bindings

    commandMap["SwitchWebcam"] = FlyByKeyboard::SwitchWebcam;
    commandMap["SwitchSecured"] = FlyByKeyboard::SwitchSecured;
    commandMap["ChangeCommStatus"] = FlyByKeyboard::ChangeCommStatus;
    commandMap["UpFreq"] = FlyByKeyboard::UpFreq;
    commandMap["DownFreq"] = FlyByKeyboard::DownFreq;

    commandMap["ThrustModeKey"] = FlyByKeyboard::KSwitchFlightMode;
    commandMap["ThrustRight"] = FlyByKeyboard::KThrustRight;
    commandMap["ThrustLeft"] = FlyByKeyboard::KThrustLeft;
    commandMap["ThrustBack"] = FlyByKeyboard::KThrustBack;
    commandMap["ThrustFront"] = FlyByKeyboard::KThrustFront;
    commandMap["ThrustDown"] = FlyByKeyboard::KThrustDown;
    commandMap["ThrustUp"] = FlyByKeyboard::KThrustUp;
    commandMap["TextMessage"] = CockpitKeys::TextMessageKey;
    commandMap["JoyStickToggleKey"] = JoyStickToggleKey;
    commandMap["ToggleAutotracking"] = FireKeyboard::ToggleAutotracking;
    commandMap["SheltonKey"] = FlyByKeyboard::SheltonKey;
    commandMap["MatchSpeedKey"] = FlyByKeyboard::MatchSpeedKey;
    commandMap["PauseKey"] = FireKeyboard::TogglePause;
    commandMap["ConfigKey"] = FireKeyboard::ToggleConfigScreen;
    commandMap["JumpKey"] = FlyByKeyboard::JumpKey;
    commandMap["AutoKey"] = FlyByKeyboard::AutoKey;
    commandMap["SwitchCombatMode"] = FlyByKeyboard::SwitchCombatModeKey;
    commandMap["StartKey"] = FlyByKeyboard::StartKey;
    commandMap["StopKey"] = FlyByKeyboard::StopKey;
    commandMap["Screenshot"] = doScreenshot;
    commandMap["UpKey"] = FlyByKeyboard::UpKey;
    commandMap["DownKey"] = FlyByKeyboard::DownKey;
    commandMap["LeftKey"] = FlyByKeyboard::LeftKey;
    commandMap["RightKey"] = FlyByKeyboard::RightKey;
    commandMap["ABKey"] = FlyByKeyboard::ABKey;
    commandMap["AccelKey"] = FlyByKeyboard::AccelKey;
    commandMap["DecelKey"] = FlyByKeyboard::DecelKey;
    commandMap["RollLeftKey"] = FlyByKeyboard::RollLeftKey;
    commandMap["RollRightKey"] = FlyByKeyboard::RollRightKey;
    commandMap["SetVelocityRefKey"] = FlyByKeyboard::SetVelocityRefKey;
    commandMap["SetVelocityNullKey"] = FlyByKeyboard::SetNullVelocityRefKey;
    commandMap["ToggleGlow"] = FireKeyboard::ToggleGlow;
    commandMap["ToggleWarpDrive"] = FireKeyboard::ToggleWarpDrive;
    commandMap["ToggleAnimation"] = FireKeyboard::ToggleAnimation;
    commandMap["CommAttackTarget"] = FireKeyboard::AttackTarget;
    commandMap["ASAP"] = FlyByKeyboard::EngageSpecAuto;

    commandMap["CommHelpMeOutCrit"] = FireKeyboard::HelpMeOutCrit;
    commandMap["CommHelpMeOutFaction"] = FireKeyboard::HelpMeOutFaction;
    commandMap["JoinFlightgroup"] = FireKeyboard::JoinFg;
    commandMap["CommAttackTarget"] = FireKeyboard::AttackTarget;
    commandMap["CommHelpMeOut"] = FireKeyboard::HelpMeOut;
    commandMap["CommFormUp"] = FireKeyboard::FormUp;
    commandMap["CommDockWithMe"] = FireKeyboard::DockWithMe;
    commandMap["CommDefendTarget"] = FireKeyboard::DefendTarget;
    commandMap["CommDockAtTarget"] = FireKeyboard::DockTarget;
    commandMap["CommHoldPosition"] = FireKeyboard::HoldPosition;
    //Added for nearest unit targeting -ch
    commandMap["NearestHostileTargetKey"] = FireKeyboard::NearestHostileTargetKey;
    commandMap["NearestDangerousHostileKey"] = FireKeyboard::NearestDangerousHostileKey;
    commandMap["NearestFriendlyKey"] = FireKeyboard::NearestFriendlyKey;
    commandMap["NearestBaseKey"] = FireKeyboard::NearestBaseKey;
    commandMap["NearestPlanetKey"] = FireKeyboard::NearestPlanetKey;
    commandMap["NearestJumpKey"] = FireKeyboard::NearestJumpKey;

    commandMap["CommBreakForm"] = FireKeyboard::BreakFormation;

    commandMap["Comm1Key"] = FireKeyboard::PressComm1Key;
    commandMap["Comm2Key"] = FireKeyboard::PressComm2Key;
    commandMap["Comm3Key"] = FireKeyboard::PressComm3Key;
    commandMap["Comm4Key"] = FireKeyboard::PressComm4Key;
    commandMap["Comm5Key"] = FireKeyboard::PressComm5Key;
    commandMap["Comm6Key"] = FireKeyboard::PressComm6Key;
    commandMap["Comm7Key"] = FireKeyboard::PressComm7Key;
    commandMap["Comm8Key"] = FireKeyboard::PressComm8Key;
    commandMap["Comm9Key"] = FireKeyboard::PressComm9Key;
    commandMap["Comm10Key"] = FireKeyboard::PressComm10Key;

    commandMap["SaveTarget1"] = FireKeyboard::SaveTarget1Key;
    commandMap["SaveTarget2"] = FireKeyboard::SaveTarget2Key;
    commandMap["SaveTarget3"] = FireKeyboard::SaveTarget3Key;
    commandMap["SaveTarget4"] = FireKeyboard::SaveTarget4Key;
    commandMap["SaveTarget5"] = FireKeyboard::SaveTarget5Key;
    commandMap["SaveTarget6"] = FireKeyboard::SaveTarget6Key;
    commandMap["SaveTarget7"] = FireKeyboard::SaveTarget7Key;
    commandMap["SaveTarget8"] = FireKeyboard::SaveTarget8Key;
    commandMap["SaveTarget9"] = FireKeyboard::SaveTarget9Key;
    commandMap["SaveTarget10"] = FireKeyboard::SaveTarget10Key;

    commandMap["RestoreTarget1"] = FireKeyboard::RestoreTarget1Key;
    commandMap["RestoreTarget2"] = FireKeyboard::RestoreTarget2Key;
    commandMap["RestoreTarget3"] = FireKeyboard::RestoreTarget3Key;
    commandMap["RestoreTarget4"] = FireKeyboard::RestoreTarget4Key;
    commandMap["RestoreTarget5"] = FireKeyboard::RestoreTarget5Key;
    commandMap["RestoreTarget6"] = FireKeyboard::RestoreTarget6Key;
    commandMap["RestoreTarget7"] = FireKeyboard::RestoreTarget7Key;
    commandMap["RestoreTarget8"] = FireKeyboard::RestoreTarget8Key;
    commandMap["RestoreTarget9"] = FireKeyboard::RestoreTarget9Key;
    commandMap["RestoreTarget10"] = FireKeyboard::RestoreTarget10Key;

    commandMap["FreeSlaves"] = FireKeyboard::FreeSlaveKey;
    commandMap["Enslave"] = FireKeyboard::EnslaveKey;
    commandMap["EjectCargoKey"] = FireKeyboard::EjectCargoKey;
    commandMap["EjectNonMissionCargoKey"] = FireKeyboard::EjectNonMissionCargoKey;
    commandMap["EjectKey"] = FireKeyboard::EjectKey;
    commandMap["EjectDockKey"] = FireKeyboard::EjectDockKey;
    commandMap["SuicideKey"] = SuicideKey;
    commandMap["TurretAIOn"] = FireKeyboard::TurretAIOn;
    commandMap["TurretAIOff"] = FireKeyboard::TurretAIOff;
    commandMap["TurretAIFireAtWill"] = FireKeyboard::TurretFireAtWill;

    commandMap["DockKey"] = FireKeyboard::DockKey;
    commandMap["UnDockKey"] = FireKeyboard::UnDockKey;
    commandMap["RequestClearenceKey"] = FireKeyboard::RequestClearenceKey;
    commandMap["FireKey"] = FireKeyboard::FireKey;
    commandMap["MissileKey"] = FireKeyboard::MissileKey;
    commandMap["TargetKey"] = FireKeyboard::TargetKey;
    commandMap["LockTargetKey"] = FireKeyboard::LockKey;
    commandMap["ReverseTargetKey"] = FireKeyboard::ReverseTargetKey;
    commandMap["PickTargetKey"] = FireKeyboard::PickTargetKey;
    commandMap["SubUnitTargetKey"] = FireKeyboard::SubUnitTargetKey;
    commandMap["NearestTargetKey"] = FireKeyboard::NearestTargetKey;
    commandMap["MissileTargetKey"] = FireKeyboard::MissileTargetKey;
    commandMap["IncomingMissileTargetKey"] = FireKeyboard::IncomingMissileTargetKey;
    commandMap["ReverseMissileTargetKey"] = FireKeyboard::ReverseMissileTargetKey;
    commandMap["ReverseIncomingMissileTargetKey"] = FireKeyboard::ReverseIncomingMissileTargetKey;
    commandMap["ThreatTargetKey"] = FireKeyboard::ThreatTargetKey;
    commandMap["SigTargetKey"] = FireKeyboard::SigTargetKey;
    commandMap["UnitTargetKey"] = FireKeyboard::UnitTargetKey;
    commandMap["ReversePickTargetKey"] = FireKeyboard::ReversePickTargetKey;
    commandMap["ReverseNearestTargetKey"] = FireKeyboard::ReverseNearestTargetKey;
    commandMap["ReverseThreatTargetKey"] = FireKeyboard::ReverseThreatTargetKey;
    commandMap["ReverseSigTargetKey"] = FireKeyboard::ReverseSigTargetKey;
    commandMap["ReverseUnitTargetKey"] = FireKeyboard::ReverseUnitTargetKey;
    commandMap["MissionTargetKey"] = FireKeyboard::MissionTargetKey;
    commandMap["ReverseMissionTargetKey"] = FireKeyboard::ReverseMissionTargetKey;
    commandMap["TurretTargetKey"] = FireKeyboard::TargetTurretKey;
    commandMap["TurretPickTargetKey"] = FireKeyboard::PickTargetTurretKey;
    commandMap["TurretNearestTargetKey"] = FireKeyboard::NearestTargetTurretKey;
    commandMap["TurretThreatTargetKey"] = FireKeyboard::ThreatTargetTurretKey;
    commandMap["WeapSelKey"] = FireKeyboard::WeapSelKey;
    commandMap["MisSelKey"] = FireKeyboard::MisSelKey;
    commandMap["ReverseWeapSelKey"] = FireKeyboard::ReverseWeapSelKey;
    commandMap["ReverseMisSelKey"] = FireKeyboard::ReverseMisSelKey;
    commandMap["CloakKey"] = FireKeyboard::CloakKey;
    commandMap["ECMKey"] = FireKeyboard::ECMKey;

    commandMap["RunPythonPress"] = RunPythonPress;
    commandMap["RunPythonRelease"] = RunPythonRelease;
    commandMap["RunPythonToggle"] = RunPythonToggle;
    commandMap["RunPythonPhysicsFrame"] = RunPythonPhysicsFrame;
    commandMap["ComposeFunctions"] = ComposeFunctions;
    commandMap["ComposeFunctionsToggle"] = ComposeFunctionsToggle;
    commandMap["Cockpit::ScrollDown"] = CockpitKeys::ScrollDown;
    commandMap["Cockpit::ScrollUp"] = CockpitKeys::ScrollUp;

    commandMap["Cockpit::PitchDown"] = CockpitKeys::PitchDown;
    commandMap["Cockpit::PitchUp"] = CockpitKeys::PitchUp;
    commandMap["Cockpit::YawLeft"] = CockpitKeys::YawLeft;
    commandMap["Cockpit::YawRight"] = CockpitKeys::YawRight;
    commandMap["Cockpit::LookDown"] = CockpitKeys::LookDown;
    commandMap["Cockpit::LookUp"] = CockpitKeys::LookUp;
    commandMap["Cockpit::LookLeft"] = CockpitKeys::LookLeft;
    commandMap["Cockpit::LookRight"] = CockpitKeys::LookRight;
    commandMap["Cockpit::ViewTarget"] = CockpitKeys::ViewTarget;
    commandMap["Cockpit::OutsideTarget"] = CockpitKeys::OutsideTarget;
    commandMap["Cockpit::PanTarget"] = CockpitKeys::PanTarget;
    commandMap["Cockpit::ZoomOut"] = CockpitKeys::ZoomOut;
    commandMap["Cockpit::ZoomIn"] = CockpitKeys::ZoomIn;
    commandMap["Cockpit::ZoomReset"] = CockpitKeys::ZoomReset;
    commandMap["Cockpit::Inside"] = CockpitKeys::Inside;
    commandMap["Cockpit::InsideLeft"] = CockpitKeys::InsideLeft;
    commandMap["Cockpit::InsideRight"] = CockpitKeys::InsideRight;
    commandMap["Cockpit::InsideBack"] = CockpitKeys::InsideBack;
    commandMap["Cockpit::SwitchLVDU"] = CockpitKeys::SwitchLVDU;

    commandMap["Cockpit::CommMode"] = CockpitKeys::CommModeVDU;
    commandMap["Cockpit::TargetMode"] = CockpitKeys::TargetModeVDU;
    commandMap["Cockpit::ManifestMode"] = CockpitKeys::ManifestModeVDU;
    commandMap["Cockpit::ViewMode"] = CockpitKeys::ViewModeVDU;
    commandMap["Cockpit::DamageMode"] = CockpitKeys::DamageModeVDU;
    commandMap["Cockpit::GunMode"] = CockpitKeys::GunModeVDU;
    commandMap["Cockpit::ReverseGunMode"] = CockpitKeys::ReverseGunModeVDU;
    commandMap["Cockpit::MissileMode"] = CockpitKeys::MissileModeVDU;
    commandMap["Cockpit::ReverseMissileMode"] = CockpitKeys::ReverseMissileModeVDU;
    commandMap["Cockpit::ObjectiveMode"] = CockpitKeys::ObjectiveModeVDU;
    commandMap["Cockpit::ScanningMode"] = CockpitKeys::ScanningModeVDU;

    commandMap["Cockpit::MapKey"] = MapKey;
    commandMap["Cockpit::SwitchRVDU"] = CockpitKeys::SwitchRVDU;
    commandMap["Cockpit::SwitchMVDU"] = CockpitKeys::SwitchMVDU;
    commandMap["Cockpit::SwitchURVDU"] = CockpitKeys::SwitchURVDU;
    commandMap["Cockpit::SwitchULVDU"] = CockpitKeys::SwitchULVDU;
    commandMap["Cockpit::SwitchUMVDU"] = CockpitKeys::SwitchUMVDU;
    commandMap["Cockpit::Behind"] = CockpitKeys::Behind;
    commandMap["Cockpit::Pan"] = CockpitKeys::Pan;
    commandMap["Cockpit::SkipMusicTrack"] = CockpitKeys::SkipMusicTrack;

    commandMap["Cockpit::Quit"] = CockpitKeys::Quit;

    commandMap["Joystick::Mode::InertialXY"] = FlyByKeyboard::JoyInertialXYPulsorKey;
    commandMap["Joystick::Mode::InertialXYToggle"] = FlyByKeyboard::JoyInertialXYToggleKey;
    commandMap["Joystick::Mode::InertialXZ"] = FlyByKeyboard::JoyInertialXYPulsorKey;
    commandMap["Joystick::Mode::InertialXZToggle"] = FlyByKeyboard::JoyInertialXYToggleKey;
    commandMap["Joystick::Mode::Roll"] = FlyByKeyboard::JoyRollPulsorKey;
    commandMap["Joystick::Mode::RollToggle"] = FlyByKeyboard::JoyRollToggleKey;
    commandMap["Joystick::Mode::Bank"] = FlyByKeyboard::JoyBankPulsorKey;
    commandMap["Joystick::Mode::BankToggle"] = FlyByKeyboard::JoyBankToggleKey;

    commandMap["Flight::Mode::InertialPulsor"] = FlyByKeyboard::InertialPulsorKey;
    commandMap["Flight::Mode::InertialToggle"] = FlyByKeyboard::InertialToggleKey;

    commandMap["NewShader"] = doReloadShader;

    return commandMap;
}

