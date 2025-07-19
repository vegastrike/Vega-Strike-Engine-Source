/*
 * main_loop.cpp
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


#include <cstdlib>
#ifndef _WIN32
#include <fenv.h>
#endif
#include <cstdio>
#include <cassert>
#include <cstring>
#include "root_generic/lin_time.h"
#include "cmd/movable.h"
#include "src/vegastrike.h"
#include "root_generic/vs_globals.h"
#include "src/in.h"
#include "gfx_generic/mesh.h"
#include "gfx/sprite.h"
#include "src/physics.h"
#include "src/gfxlib.h"
#include "cmd/bolt.h"
#include "gfx/loc_select.h"
#include <string>
#include "cmd/collection.h"
#include "src/star_system.h"
#include "cmd/planet.h"
#include "gfx_generic/sphere.h"
#include "gfx/coord_select.h"
#include "cmd/building.h"
#include "cmd/ai/fire.h"
#include "cmd/ai/aggressive.h"
#include "cmd/ai/navigation.h"
#include "cmd/beam.h"
#include "gfx/halo.h"
#include "gfx_generic/matrix.h"
#include "cmd/ai/flyjoystick.h"
#include "cmd/ai/firekeyboard.h"
#include "cmd/ai/script.h"
#include "gfx/cockpit.h"
#include "gfx/aux_texture.h"
#include "gfx/background.h"
#include "cmd/music.h"
#include "src/main_loop.h"
#include "cmd/music.h"
#include "src/audiolib.h"
#include "cmd/nebula.h"
#include "src/vs_logging.h"
#include "cmd/script/mission.h"
#include "root_generic/xml_support.h"
#include "src/config_xml.h"
#include "cmd/ai/missionscript.h"
#include "cmd/enhancement.h"
#include "cmd/cont_terrain.h"
#include "cmd/script/flightgroup.h"
#include "src/force_feedback.h"
#include "src/universe_util.h"
#include "src/save_util.h"
#include "src/in_kb_data.h"
#include "src/vs_random.h"
#include "cmd/enhancement.h"

#include "root_generic/options.h"

#include "audio/SceneManager.h"

#ifndef NO_GFX
#include "gldrv/gl_globals.h"
#include "src/vs_exit.h"
#endif

#define KEYDOWN(name, key) (name[key]&0x80)

Unit **fighters;
Unit *carrier = nullptr;
Unit *fighter = nullptr;
Unit *fighter2 = nullptr;
Unit *midway = nullptr;
GFXBOOL capture;
GFXBOOL quit = GFXFALSE;
bool _Slew = true;
bool QuitAllow = false;
extern bool cleanexit;
#ifndef _WIN32
int allexcept = FE_DIVBYZERO; //|FE_INVALID;//|FE_OVERFLOW|FE_UNDERFLOW;
#else
int allexcept = 0;
#endif
int shiftup(int);
string getUnitNameAndFgNoBase(Unit *target);
ContinuousTerrain *myterrain;
int numf = 0;
CoordinateSelect *locSel = NULL;
SphereMesh *bg2 = NULL;
ClickList *shipList = NULL;

float SIMULATION_ATOM = 0.0f;
float AUDIO_ATOM = 0.0f;

void VolUp(const KBData &, KBSTATE newState) {
    if (newState == PRESS) {
        float gain = AUDGetListenerGain();
        if (gain < 1) {
            gain += .0625;
            if (gain > 1) {
                gain = 1;
            }
            AUDListenerGain(gain);
            Music::ChangeVolume(0);
        }
    }
}

void VolDown(const KBData &, KBSTATE newState) {
    if (newState == PRESS) {
        float gain = AUDGetListenerGain();
        if (gain > 0) {
            gain -= .03125;
            if (gain < 0) {
                gain = 0;
            }
            AUDListenerGain(gain);
            Music::ChangeVolume(0);
        }
    }
}

static void SwitchVDUTo(VDU::VDU_MODE v) {
    int i;
    static int whichvdu = 1;
    for (int j = 0; j < 3; ++j) {
        if (v != _Universe->AccessCockpit()->getVDUMode(whichvdu) || (v != VDU::VIEW && v != VDU::WEAPON)) {
            whichvdu += 1;
            whichvdu %= 2;
        }
        int curmode = _Universe->AccessCockpit()->getVDUMode(whichvdu);
        if (v == _Universe->AccessCockpit()->getVDUMode(whichvdu)) {
            if (v == VDU::VIEW) {
                _Universe->AccessCockpit()->VDUSwitch(whichvdu);
            }                  //get different view mode
            return;
        }
        for (i = 0; i < 32; ++i) {
            _Universe->AccessCockpit()->VDUSwitch(whichvdu);
            if (v == _Universe->AccessCockpit()->getVDUMode(whichvdu)) {
                return;
            }
        }
        for (i = 0; i < 32; ++i) {
            _Universe->AccessCockpit()->VDUSwitch(whichvdu);
            if (curmode == _Universe->AccessCockpit()->getVDUMode(whichvdu)) {
                break;
            }
        }
    }
}

void ExamineWhenTargetKey() {
    //if (game_options()->switchToTargetModeOnKey) {
    if (configuration()->graphics.hud.switch_to_target_mode_on_key) {
        int view = 0;
        int examine = 0;
        for (; view < 2; ++view) {
            if (_Universe->AccessCockpit()->getVDUMode(view) == VDU::VIEW) {
                break;
            }
        }
        for (; examine < 2; ++examine) {
            if (_Universe->AccessCockpit()->getVDUMode(examine) == VDU::TARGET) {
                break;
            }
        }
        if ((examine == 2) && (view == 2)) {
            SwitchVDUTo(VDU::TARGET);
        }
    }
}

namespace CockpitKeys {
unsigned int textmessager = 0;
static bool waszero = false;

void TextMessageCallback(unsigned int ch, unsigned int mod, bool release, int x, int y) {
    GameCockpit *gcp = dynamic_cast< GameCockpit * > ( _Universe->AccessCockpit(textmessager));
    assert(gcp != nullptr);
    gcp->editingTextMessage = true;
    if ((release
            && (waszero || ch == WSK_KP_ENTER || ch == WSK_ESCAPE)) || (release == false && (ch == ']' || ch == '['))) {
        waszero = false;
        gcp->editingTextMessage = false;
        RestoreKB();
    }
    if (release || (ch == ']' || ch == '[')) {
        return;
    }
    unsigned int code =
            ((WSK_MOD_LSHIFT == (mod & WSK_MOD_LSHIFT)) || (WSK_MOD_RSHIFT == (mod & WSK_MOD_RSHIFT))) ? shiftup(
                    ch) : ch;
    if (textmessager < _Universe->numPlayers()) {
        if (ch == WSK_BACKSPACE || ch == WSK_DELETE) {
            gcp->textMessage = gcp->textMessage.substr(0, gcp->textMessage.length() - 1);
        } else if (ch == WSK_RETURN || ch == WSK_KP_ENTER) {
            if (gcp->textMessage.length() != 0) {
                std::string name = gcp->savegame->GetCallsign();
                if (gcp->textMessage[0] == '/') {
                    string cmd;
                    string args;
                    std::string::size_type space = gcp->textMessage.find(' ');
                    if (space) {
                        cmd = gcp->textMessage.substr(1, space - 1);
                        args = gcp->textMessage.substr(space + 1);
                    } else {
                        cmd = gcp->textMessage.substr(1);
                        //Send custom message to itself.
                    }
                    UniverseUtil::receivedCustom(textmessager, true, cmd, args, string());
                }
                waszero = false;
            } else {
                waszero = true;
            }
            gcp->textMessage = "";
        } else if (code != 0 && code <= 127) {
            char newstr[2] = {(char) code, 0};
            gcp->textMessage += newstr;
        }
    } else {
        RestoreKB();
        gcp->editingTextMessage = false;
    }
}

void TextMessageKey(const KBData &, KBSTATE newState) {
    if (newState == PRESS) {
        if (game_options()->chat_only_in_network) {
            return;
        }
        winsys_set_keyboard_func(TextMessageCallback);
        textmessager = _Universe->CurrentCockpit();
    }
}

void QuitNow() {
    if (!cleanexit) {
        cleanexit = true;
        if (configuration()->general.write_savegame_on_exit) {
            _Universe->WriteSaveGame(true);              //gotta do important stuff first
        }
        for (size_t i = 0; i < active_missions.size(); ++i) {
            if (active_missions[i]) {
                active_missions[i]->DirectorEnd();
            }
        }
        if (forcefeedback != nullptr) {
            delete forcefeedback;
            forcefeedback = nullptr;
        }
        VSExit(0);
    }
}

void SkipMusicTrack(const KBData &, KBSTATE newState) {
    if (newState == PRESS) {
        VS_LOG(info, "skipping");
        muzak->Skip();
    }
}

static void _PitchDown(KBSTATE newState, int fromCam = 0, int toCam = NUM_CAM - 1) {
    static Vector Q;
    static Vector R;
    for (int i = fromCam; i <= toCam; i++) {
        if (newState == PRESS) {
            if (QuitAllow) {
                QuitNow();
            }
            Q = _Universe->AccessCockpit()->AccessCamera(i)->Q;
            R = _Universe->AccessCockpit()->AccessCamera(i)->R;
            _Universe->AccessCockpit()->AccessCamera(i)->myPhysics.ApplyBalancedLocalTorque(-Q,
                    R,
                    configuration()->graphics.camera_pan_speed);
        }
        if (_Slew && newState == RELEASE) {
            _Universe->AccessCockpit()->AccessCamera(i)->myPhysics.SetAngularVelocity(Vector(0, 0, 0));
        }
    }
}

static void _PitchUp(KBSTATE newState, int fromCam = 0, int toCam = NUM_CAM - 1) {
    static Vector Q;
    static Vector R;
    for (int i = fromCam; i <= toCam; i++) {
        if (newState == PRESS) {
            Q = _Universe->AccessCockpit()->AccessCamera(i)->Q;
            R = _Universe->AccessCockpit()->AccessCamera(i)->R;
            _Universe->AccessCockpit()->AccessCamera(i)->myPhysics.ApplyBalancedLocalTorque(Q,
                    R,
                    configuration()->graphics.camera_pan_speed);
        }
        if (_Slew && newState == RELEASE) {
            _Universe->AccessCockpit()->AccessCamera(i)->myPhysics.SetAngularVelocity(Vector(0, 0, 0));
        }
    }
}

static void _YawLeft(KBSTATE newState, int fromCam = 0, int toCam = NUM_CAM - 1) {
    static Vector P;
    static Vector R;
    for (int i = fromCam; i <= toCam; i++) {
        if (newState == PRESS) {
            P = _Universe->AccessCockpit()->AccessCamera(i)->P;
            R = _Universe->AccessCockpit()->AccessCamera(i)->R;
            _Universe->AccessCockpit()->AccessCamera(i)->myPhysics.ApplyBalancedLocalTorque(-P,
                    R,
                    configuration()->graphics.camera_pan_speed);
        }
        if (_Slew && newState == RELEASE) {
            _Universe->AccessCockpit()->AccessCamera(i)->myPhysics.SetAngularVelocity(Vector(0, 0, 0));
        }
    }
}

static void _YawRight(KBSTATE newState, int fromCam = 0, int toCam = NUM_CAM - 1) {
    for (int i = fromCam; i <= toCam; i++) {
        static Vector P;
        static Vector R;
        if (newState == PRESS) {
            P = _Universe->AccessCockpit()->AccessCamera(i)->P;
            R = _Universe->AccessCockpit()->AccessCamera(i)->R;
            _Universe->AccessCockpit()->AccessCamera(i)->myPhysics.ApplyBalancedLocalTorque(P,
                    R,
                    configuration()->graphics.camera_pan_speed);
        }
        if (_Slew && newState == RELEASE) {
            _Universe->AccessCockpit()->AccessCamera(i)->myPhysics.SetAngularVelocity(Vector(0, 0, 0));
        }
    }
}

void PitchDown(const KBData &, KBSTATE newState) {
    _PitchDown(newState);
}

void PitchUp(const KBData &, KBSTATE newState) {
    _PitchUp(newState);
}

void YawLeft(const KBData &, KBSTATE newState) {
    _YawLeft(newState);
}

void YawRight(const KBData &, KBSTATE newState) {
    _YawRight(newState);
}

static void InitPanInside() {
    YawLeft(std::string(), RELEASE);
    YawRight(std::string(), RELEASE);
    PitchUp(std::string(), RELEASE);
    PitchDown(std::string(), RELEASE);
    _Universe->AccessCockpit()->SetView(CP_PANINSIDE);
}

void LookDown(const KBData &kbdata, KBSTATE newState) {
    if (newState == PRESS && QuitAllow) {
        QuitNow();
    }
    if (newState == PRESS || newState == DOWN) {
        if (_Universe->AccessCockpit()->GetView() <= CP_RIGHT) {
            InitPanInside();
        } else if (_Universe->AccessCockpit()->GetView() == CP_PANINSIDE) {
            _Universe->AccessCockpit()->SetInsidePanPitchSpeed(configuration()->graphics.camera_pan_speed * 1000.0);
        } else {
            PitchDown(kbdata, newState);
        }
    } else if (newState == RELEASE) {
        if (_Universe->AccessCockpit()->GetView() == CP_PANINSIDE) {
            _Universe->AccessCockpit()->SetInsidePanPitchSpeed(0);
        } else {
            PitchDown(kbdata, newState);
        }
    }
}

void LookUp(const KBData &kbdata, KBSTATE newState) {
    if (newState == PRESS || newState == DOWN) {
        if (_Universe->AccessCockpit()->GetView() <= CP_RIGHT) {
            InitPanInside();
        } else if (_Universe->AccessCockpit()->GetView() == CP_PANINSIDE) {
            _Universe->AccessCockpit()->SetInsidePanPitchSpeed(-configuration()->graphics.camera_pan_speed * 1000.0);
        } else {
            PitchUp(kbdata, newState);
        }
    } else if (newState == RELEASE) {
        if (_Universe->AccessCockpit()->GetView() == CP_PANINSIDE) {
            _Universe->AccessCockpit()->SetInsidePanPitchSpeed(0);
        } else {
            PitchUp(kbdata, newState);
        }
    }
}

void LookLeft(const KBData &kbdata, KBSTATE newState) {
    if (newState == PRESS || newState == DOWN) {
        if (_Universe->AccessCockpit()->GetView() <= CP_RIGHT) {
            InitPanInside();
        } else if (_Universe->AccessCockpit()->GetView() == CP_PANINSIDE) {
            _Universe->AccessCockpit()->SetInsidePanYawSpeed(configuration()->graphics.camera_pan_speed * 1000.0);
        } else {
            YawLeft(kbdata, newState);
        }
    } else if (newState == RELEASE) {
        if (_Universe->AccessCockpit()->GetView() == CP_PANINSIDE) {
            _Universe->AccessCockpit()->SetInsidePanYawSpeed(0);
        } else {
            YawLeft(kbdata, newState);
        }
    }
}

void LookRight(const KBData &kbdata, KBSTATE newState) {
    if (newState == PRESS || newState == DOWN) {
        if (_Universe->AccessCockpit()->GetView() <= CP_RIGHT) {
            InitPanInside();
        } else if (_Universe->AccessCockpit()->GetView() == CP_PANINSIDE) {
            _Universe->AccessCockpit()->SetInsidePanYawSpeed(-configuration()->graphics.camera_pan_speed * 1000.0);
        } else {
            YawRight(kbdata, newState);
        }
    } else if (newState == RELEASE) {
        if (_Universe->AccessCockpit()->GetView() == CP_PANINSIDE) {
            _Universe->AccessCockpit()->SetInsidePanYawSpeed(0);
        } else {
            YawRight(kbdata, newState);
        }
    }
}

void Quit(const KBData &, KBSTATE newState) {
    if (newState == PRESS) {
        QuitAllow = !QuitAllow;
    }
}

void Inside(const KBData &, KBSTATE newState) {
    {
        if (_Universe->activeStarSystem() && _Universe->activeStarSystem()->getBackground()) {
            _Universe->activeStarSystem()->getBackground()->EnableBG(configuration()->graphics.background);
        }
    }
    int tmp = (configuration()->graphics.cockpit ? 1 : 0);
    if (newState == PRESS && (_Universe->AccessCockpit()->GetView() == CP_FRONT)
            && configuration()->graphics.disabled_cockpit_allowed) {
        YawLeft(KBData(), RELEASE);
        YawRight(KBData(), RELEASE);
        PitchUp(KBData(), RELEASE);
        PitchDown(KBData(), RELEASE);
        string cockpit = "disabled-cockpit.cpt";
        if (_Universe->AccessCockpit()->GetParent()) {
            cockpit = _Universe->AccessCockpit()->GetParent()->getCockpit();
        }
        Unit *u = nullptr;
        if ((_Universe->AccessCockpit()->GetParent() != nullptr)
                && (_Universe->AccessCockpit()->GetParent()->name == "return_to_cockpit")
                && (_Universe->AccessCockpit()->GetParent()->owner != nullptr)
                && (u = findUnitInStarsystem(_Universe->AccessCockpit()->GetParent()->owner))) {
            cockpit = u->getCockpit();
        }
        _Universe->AccessCockpit()->Init(cockpit.c_str(), !((tmp) && _Universe->AccessCockpit()->GetParent()));
        tmp = (tmp + 1) % 2;
    }
    if (newState == PRESS || newState == DOWN) {
        _Universe->AccessCockpit()->SetView(CP_FRONT);
    }
}

void ZoomOut(const KBData &, KBSTATE newState) {
    if (newState == PRESS || newState == DOWN) {
        _Universe->AccessCockpit()->zoomfactor += GetElapsedTime() / getTimeCompression();
    }
}

static float scrolltime = 0;

void ScrollUp(const KBData &, KBSTATE newState) {
    scrolltime += GetElapsedTime();
    if (newState == PRESS || (newState == DOWN && scrolltime >= .5)) {
        scrolltime = 0;
        VS_LOG(info, (boost::format("Enabling exceptions %1%") % allexcept));
        _Universe->AccessCockpit()->ScrollAllVDU(-1);
    }
}

void ScrollDown(const KBData &, KBSTATE newState) {
    scrolltime += GetElapsedTime();
    if (newState == PRESS || (newState == DOWN && scrolltime >= .5)) {
        scrolltime = 0;
        VS_LOG(info, "Disabling exceptions");
        _Universe->AccessCockpit()->ScrollAllVDU(1);
    }
}

void ZoomIn(const KBData &, KBSTATE newState) {
    if (newState == PRESS || newState == DOWN) {
        _Universe->AccessCockpit()->zoomfactor -= GetElapsedTime() / getTimeCompression();
    }
}

void ZoomReset(const KBData &, KBSTATE newState) {
    if (newState == PRESS || newState == DOWN) {
        _Universe->AccessCockpit()->zoomfactor = 1.f;
    }
}

void InsideLeft(const KBData &, KBSTATE newState) {
    if (newState == PRESS || newState == DOWN) {
        YawLeft(std::string(), RELEASE);
        YawRight(std::string(), RELEASE);
        PitchUp(std::string(), RELEASE);
        PitchDown(std::string(), RELEASE);

        _Universe->AccessCockpit()->SetView(CP_LEFT);
    }
}

void InsideRight(const KBData &, KBSTATE newState) {
    if (newState == PRESS || newState == DOWN) {
        _Universe->AccessCockpit()->SetView(CP_RIGHT);
    }
}

void PanTarget(const KBData &, KBSTATE newState) {
    if (newState == PRESS || newState == DOWN) {
        _Universe->AccessCockpit()->SetView(CP_PANTARGET);
    }
}

void ViewTarget(const KBData &, KBSTATE newState) {
    if (newState == PRESS || newState == DOWN) {
        YawLeft(std::string(), RELEASE);
        YawRight(std::string(), RELEASE);
        PitchUp(std::string(), RELEASE);
        PitchDown(std::string(), RELEASE);

        _Universe->AccessCockpit()->SetView(CP_VIEWTARGET);
    }
}

void OutsideTarget(const KBData &, KBSTATE newState) {
    if (newState == PRESS || newState == DOWN) {
        YawLeft(std::string(), RELEASE);
        YawRight(std::string(), RELEASE);
        PitchUp(std::string(), RELEASE);
        PitchDown(std::string(), RELEASE);

        _Universe->AccessCockpit()->SetView(CP_TARGET);
    }
}

void InsideBack(const KBData &, KBSTATE newState) {
    if (newState == PRESS || newState == DOWN) {
        YawLeft(std::string(), RELEASE);
        YawRight(std::string(), RELEASE);
        PitchUp(std::string(), RELEASE);
        PitchDown(std::string(), RELEASE);

        _Universe->AccessCockpit()->SetView(CP_BACK);
    }
}

void CommModeVDU(const KBData &, KBSTATE newState) {
    if (newState == PRESS) {
        static soundContainer sc;
        if (sc.sound < 0) {
            sc.loadsound(game_options()->comm);
        }
        sc.playsound();
        SwitchVDUTo(VDU::COMM);
    }
}

void ScanningModeVDU(const KBData &, KBSTATE newState) {
    if (newState == PRESS) {
        static soundContainer sc;
        if (sc.sound < 0) {
            sc.loadsound(game_options()->scanning);
        }
        sc.playsound();
        SwitchVDUTo(VDU::SCANNING);
    }
}

void ObjectiveModeVDU(const KBData &, KBSTATE newState) {
    if (newState == PRESS) {
        static soundContainer sc;
        if (sc.sound < 0) {
            sc.loadsound(game_options()->objective);
        }
        sc.playsound();

        SwitchVDUTo(VDU::OBJECTIVES);
    }
}

void TargetModeVDU(const KBData &, KBSTATE newState) {
    if (newState == PRESS) {
        static soundContainer sc;
        if (sc.sound < 0) {
            sc.loadsound(game_options()->examine);
        }
        sc.playsound();

        SwitchVDUTo(VDU::TARGET);
    }
}

void ViewModeVDU(const KBData &, KBSTATE newState) {
    if (newState == PRESS) {
        static soundContainer sc;
        if (sc.sound < 0) {
            sc.loadsound(game_options()->view);
        }
        sc.playsound();
        SwitchVDUTo(VDU::VIEW);
    }
}

void DamageModeVDU(const KBData &, KBSTATE newState) {
    if (newState == PRESS) {
        static soundContainer sc;
        if (sc.sound < 0) {
            sc.loadsound(game_options()->repair);
        }
        sc.playsound();

        SwitchVDUTo(VDU::DAMAGE);
    }
}

void ManifestModeVDU(const KBData &, KBSTATE newState) {
    if (newState == PRESS) {
        static soundContainer sc;
        if (sc.sound < 0) {
            sc.loadsound(game_options()->manifest);
        }
        sc.playsound();

        SwitchVDUTo(VDU::MANIFEST);
    }
}

void GunModeVDU(const KBData &s, KBSTATE newState) {
    if (newState == PRESS) {
        SwitchVDUTo(VDU::WEAPON);
    }
    FireKeyboard::WeapSelKey(s, newState);
}

void ReverseGunModeVDU(const KBData &s, KBSTATE newState) {
    if (newState == PRESS) {
        SwitchVDUTo(VDU::WEAPON);
    }
    FireKeyboard::ReverseWeapSelKey(s, newState);
}

void MissileModeVDU(const KBData &s, KBSTATE newState) {
    if (newState == PRESS) {
        SwitchVDUTo(VDU::WEAPON);
    }
    FireKeyboard::MisSelKey(s, newState);
}

void ReverseMissileModeVDU(const KBData &s, KBSTATE newState) {
    if (newState == PRESS) {
        SwitchVDUTo(VDU::WEAPON);
    }
    FireKeyboard::ReverseMisSelKey(s, newState);
}

void SwitchLVDU(const KBData &, KBSTATE newState) {
    if (newState == PRESS) {
        _Universe->AccessCockpit()->VDUSwitch(0);
    }
}

void SwitchRVDU(const KBData &, KBSTATE newState) {
    if (newState == PRESS) {
        _Universe->AccessCockpit()->VDUSwitch(1);
    }
}

void SwitchMVDU(const KBData &, KBSTATE newState) {
    if (newState == PRESS) {
        _Universe->AccessCockpit()->VDUSwitch(2);
    }
}

void SwitchULVDU(const KBData &, KBSTATE newState) {
    if (newState == PRESS) {
        _Universe->AccessCockpit()->VDUSwitch(3);
    }
}

void SwitchURVDU(const KBData &, KBSTATE newState) {
    if (newState == PRESS) {
        _Universe->AccessCockpit()->VDUSwitch(4);
    }
}

void SwitchUMVDU(const KBData &, KBSTATE newState) {
    if (newState == PRESS) {
        _Universe->AccessCockpit()->VDUSwitch(5);
    }
}

void Behind(const KBData &, KBSTATE newState) {
    if (newState == PRESS || newState == DOWN) {
        YawLeft(std::string(), RELEASE);
        YawRight(std::string(), RELEASE);
        PitchUp(std::string(), RELEASE);
        PitchDown(std::string(), RELEASE);

        _Universe->AccessCockpit()->SetView(CP_CHASE);
    }
}

void Pan(const KBData &, KBSTATE newState) {
    if (newState == PRESS || newState == DOWN) {
        YawLeft(std::string(), RELEASE);
        YawRight(std::string(), RELEASE);
        PitchUp(std::string(), RELEASE);
        PitchDown(std::string(), RELEASE);

        _Universe->AccessCockpit()->SetView(CP_PAN);
    }
}
}

using namespace CockpitKeys;

void InitializeInput() {
    BindKey(SDL_SCANCODE_ESCAPE, 0, 0, Quit, KBData());     //always have quit on esc
}

void IncrementStartupVariable() {
    if (mission->getVariable("savegame", "").length() == 0) {
        return;
    }
    int len = getSaveDataLength(0, "436457r1K3574r7uP71m35");
    float var = FLT_MAX;
    if (len <= 0) {
        pushSaveData(0, "436457r1K3574r7uP71m35", 1);
        var = 1;
    } else {
        var = getSaveData(0, "436457r1K3574r7uP71m35", 0);
        putSaveData(0, "436457r1K3574r7uP71m35", 0, var + 1);
    }
    if (var <= configuration()->general.times_to_show_help_screen) {
        GameCockpit::NavScreen(KBData(), PRESS);
    }          //HELP FIXME
}

void createObjects(std::vector<std::string> &fighter0name,
        std::vector<StarSystem *> &ssys,
        std::vector<QVector> &savedloc,
        vector<vector<std::string> > &savefiles) {
    vector<std::string> fighter0mods;
    vector<int> fighter0indices;

    static Vector * terrain_scale;
    static bool initialized = false;
    if (!initialized) {
        initialized = true;
        terrain_scale = new Vector(game_options()->xscale, game_options()->yscale, game_options()->zscale);
    }

    myterrain = nullptr;
    std::string stdstr = mission->getVariable("terrain", "");
    if (stdstr.length() > 0) {
        Terrain *terr = new Terrain(stdstr.c_str(), *terrain_scale, game_options()->mass, game_options()->radius);
        Matrix tmp;
        ScaleMatrix(tmp, *terrain_scale);
        QVector pos;
        mission->GetOrigin(pos, stdstr);
        tmp.p = -pos;
        terr->SetTransformation(tmp);
    }
    stdstr = mission->getVariable("continuousterrain", "");
    if (stdstr.length() > 0) {
        myterrain = new ContinuousTerrain(stdstr.c_str(), *terrain_scale, game_options()->mass);
        Matrix tmp;
        Identity(tmp);
        QVector pos;
        mission->GetOrigin(pos, stdstr);
        tmp.p = -pos;
        myterrain->SetTransformation(tmp);
    }
    BindKey(1, CoordinateSelect::MouseMoveHandle);

    int numf = mission->number_of_ships;

    fighters = new Unit *[numf];
    int *tmptarget = new int[numf];

    GFXEnable(TEXTURE0);
    GFXEnable(TEXTURE1);

    vsUMap<string, int> targetmap;

    char fightername[1024] = "hornet.xunit";
    int a = 0;

    vector<Flightgroup *>::const_iterator siter;
    vector<Flightgroup *> fg = mission->flightgroups;
    int squadnum = 0;
    for (siter = fg.begin(); siter != fg.end(); siter++) {
        Flightgroup *fg = *siter;
        string fg_name = fg->name;
        string fullname = fg->type;         //+ ".xunit";
        strcpy(fightername, fullname.c_str());
        string ainame = fg->ainame;
        float fg_radius = 0.0;
        Cockpit *cp = NULL;
        for (int s = 0; s < fg->nr_ships; s++) {
            if (a >= mission->number_of_ships) {
                a -= 22;
                VS_LOG(error, "Error: in createObjects: more ships in flightgroups than in total for mission!");
                VS_LOG(error,
                        (boost::format("Variables a=%1%, fg-number-of-ships=%2%, total nr=%3%, fact=%4%, fgname=%5%")
                                % a % fg->nr_ships % mission->number_of_ships % fg->faction.c_str()
                                % fg->name.c_str()));
                break;
            }
            numf++;
            QVector pox(1000 + 150 * a, 100 * a, 100);

            const double tmp = static_cast<double>(s) * static_cast<double>(fg_radius) * 3.0;
            pox.i = fg->pos.i + tmp;
            pox.j = fg->pos.j + tmp;
            pox.k = fg->pos.k + tmp;
            if (pox.i == pox.j && pox.j == pox.k && pox.k == 0) {
                pox.i = rand() * 10000. / RAND_MAX - 5000;
                pox.j = rand() * 10000. / RAND_MAX - 5000;
                pox.k = rand() * 10000. / RAND_MAX - 5000;
            }
            tmptarget[a] = FactionUtil::GetFactionIndex(fg->faction);             //that should not be in xml?
            int fg_terrain = -1;
            if (fg_terrain == -1 || (fg_terrain == -2 && myterrain == NULL)) {
                string modifications("");
                if (s == 0 && squadnum < (int) fighter0name.size()) {
                    cp = _Universe->AccessCockpit(squadnum);
                    cp->activeStarSystem = ssys[squadnum];
                    fighter0indices.push_back(a);
                    if (fighter0name[squadnum].length() == 0) {
                        fighter0name[squadnum] = string(fightername);
                    } else {
                        strcpy(fightername, fighter0name[squadnum].c_str());
                    }
                    if (mission->getVariable("savegame", "").length() > 0) {
                        if (savedloc[squadnum].i != FLT_MAX) {
                            pox = UniverseUtil::SafeEntrancePoint(savedloc[squadnum]);
                        }
                        vector<std::string> *dat = &cp->savegame->getMissionStringData("jump_from");
                        if (dat->size()) {
                            std::string srcsys = (*dat)[0];
                            Unit *grav;
                            for (un_iter ui = cp->activeStarSystem->gravitationalUnits().createIterator();
                                    (grav = *ui) != NULL;
                                    ++ui) {
                                size_t siz = grav->GetDestinations().size();
                                for (unsigned int i = 0; i < siz; ++i) {
                                    if (srcsys == grav->GetDestinations()[i]) {
                                        QVector newpos = grav->LocalPosition()
                                                + QVector(vsrandom.uniformExc(-grav->rSize() / 4, grav->rSize() / 4),
                                                        vsrandom.uniformExc(-grav->rSize() / 4,
                                                                grav->rSize() / 4),
                                                        vsrandom.uniformExc(-grav->rSize() / 4,
                                                                grav->rSize() / 4));
                                        if (grav->getUnitType() != Vega_UnitType::planet) {
                                            newpos = UniverseUtil::SafeEntrancePoint(newpos);
                                        }
                                        cp->savegame->SetPlayerLocation(newpos);
                                        pox = newpos;
                                    }
                                }
                            }
                            dat->clear();
                        }
                        fighter0mods.push_back(modifications = game_options()->getCallsign(squadnum));
                        VS_LOG(info,
                                (boost::format("FOUND MODIFICATION = %1% FOR PLAYER #%2%") % modifications.c_str()
                                        % squadnum));
                    } else {
                        fighter0mods.push_back("");
                    }
                }
                Cockpit *backupcp = _Universe->AccessCockpit();
                if (squadnum < (int) fighter0name.size()) {
                    _Universe->pushActiveStarSystem(_Universe->AccessCockpit(squadnum)->activeStarSystem);
                    _Universe->SetActiveCockpit(_Universe->AccessCockpit(squadnum));
                }

                VS_LOG(info, (boost::format("CREATING A LOCAL SHIP : %1%") % fightername));
                fighters[a] = new Unit(fightername, false, tmptarget[a], modifications, fg, s);

                _Universe->activeStarSystem()->AddUnit(fighters[a]);
                if (s == 0 && squadnum < (int) fighter0name.size()) {
                    _Universe->AccessCockpit(squadnum)->Init(fighters[a]->getCockpit().c_str());
                    _Universe->AccessCockpit(squadnum)->SetParent(fighters[a], fighter0name[squadnum].c_str(),
                            fighter0mods[squadnum].c_str(), pox);
                }
                if (squadnum < (int) fighter0name.size()) {
                    _Universe->popActiveStarSystem();
                    _Universe->SetActiveCockpit(backupcp);
                }
            } else {
                bool isvehicle = false;
                if (fg_terrain == -2) {
                    fighters[a] =
                            new Building(myterrain, isvehicle, fightername, false, tmptarget[a], string(""), fg);
                } else {
                    if (fg_terrain >= (int) _Universe->activeStarSystem()->numTerrain()) {
                        ContinuousTerrain *t;
                        assert(
                                fg_terrain - _Universe->activeStarSystem()->numTerrain()
                                        < _Universe->activeStarSystem()->numContTerrain());
                        t = _Universe->activeStarSystem()
                                ->getContTerrain(fg_terrain - _Universe->activeStarSystem()->numTerrain());
                        fighters[a] = new Building(t, isvehicle, fightername, false, tmptarget[a], string(
                                ""), fg);
                    } else {
                        Terrain *t = _Universe->activeStarSystem()->getTerrain(fg_terrain);
                        fighters[a] = new Building(t, isvehicle, fightername, false, tmptarget[a], string(
                                ""), fg);
                    }
                }
                _Universe->activeStarSystem()->AddUnit(fighters[a]);
            }
            VS_LOG(info, (boost::format("pox %1% %2% %3%") % pox.i % pox.j % pox.k));
            fighters[a]->SetPosAndCumPos(pox);
            fg_radius = fighters[a]->rSize();
            if (benchmark > 0.0 || (s != 0 || squadnum >= (int) fighter0name.size())) {
                fighters[a]->LoadAIScript(ainame);
                fighters[a]->SetTurretAI();
            }
            a++;
        }         //for nr_ships
        squadnum++;
    }     //end of for flightgroups

    delete[] tmptarget;
    for (int m_i = 0; m_i < muzak_count; m_i++) {
        muzak[m_i].SetParent(fighters[0]);
    }
    FactionUtil::LoadFactionPlaylists();
    AUDListenerSize(fighters[0]->rSize() * 4);
    for (unsigned int cnum = 0; cnum < fighter0indices.size(); cnum++) {
        if (benchmark == -1) {
            fighters[fighter0indices[cnum]]->EnqueueAI(new FlyByJoystick(cnum));
            fighters[fighter0indices[cnum]]->EnqueueAI(new FireKeyboard(cnum, cnum));
        }
        fighters[fighter0indices[cnum]]->SetTurretAI();
        fighters[fighter0indices[cnum]]->DisableTurretAI();
    }
    shipList = _Universe->activeStarSystem()->getClickList();
    locSel = new CoordinateSelect(QVector(0, 0, 5));
    UpdateTime();

    mission->DirectorInitgame();
    IncrementStartupVariable();
}

void AddUnitToSystem(const SavedUnits *su) {
    Unit *un = NULL;
    switch (su->type) {
        case Vega_UnitType::enhancement:
            un =
                    new Enhancement(su->filename.get().c_str(), FactionUtil::GetFactionIndex(su->faction), string(""));
            un->SetPosition(QVector(0, 0, 0));
            break;
        case Vega_UnitType::unit:
        default:
            un = new Unit(su->filename.get().c_str(), false, FactionUtil::GetFactionIndex(su->faction));
            un->EnqueueAI(new Orders::AggressiveAI("default.agg.xml"));
            un->SetTurretAI();
            if (_Universe->AccessCockpit()->GetParent()) {
                un->SetPosition(_Universe->AccessCockpit()->GetParent()->Position()
                        + QVector(rand() * 10000. / RAND_MAX - 5000,
                                rand() * 10000. / RAND_MAX - 5000,
                                rand() * 10000. / RAND_MAX - 5000));
            }
            break;
    }
    _Universe->activeStarSystem()->AddUnit(un);
}

void destroyObjects() {
    if (myterrain != nullptr) {
        delete myterrain;
        myterrain = nullptr;
    }
    Terrain::DeleteAll();
    delete[] fighters;
    delete locSel;
}

int getmicrosleep() {
    return game_options()->threadtime;
}

void restore_main_loop() {
    RestoreKB();
    RestoreMouse();
    GFXLoop(main_loop);
}

void main_loop() {
    //Evaluate number of loops per second each XX loops
    if (loop_count == 500) {
        last_check = cur_check;
        cur_check = getNewTime();
        if (last_check != 1) {
            //Time to update test
            avg_loop = ((nb_checks - 1) * avg_loop + (loop_count / (cur_check - last_check))) / (nb_checks);
            nb_checks = nb_checks + 1;
        }
        loop_count = -1;
    }
    loop_count++;

    //Execute DJ script
    Music::MuzakCycle();

    _Universe->StartDraw();
    if (myterrain) {
        myterrain->AdjustTerrain(_Universe->activeStarSystem());
    }

#ifndef NO_GFX
    VS_LOG(trace,
            (boost::format("Drawn %1% vertices in %2% batches") % gl_vertices_this_frame % gl_batches_this_frame));
    gl_vertices_this_frame = 0;
    gl_batches_this_frame = 0;
#endif

    //Commit audio scene status to renderer
    if (g_game.sound_enabled) {
        Audio::SceneManager::getSingleton()->commit();
    }
}
