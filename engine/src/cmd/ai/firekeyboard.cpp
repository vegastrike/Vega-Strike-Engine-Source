/*
 * firekeyboard.cpp
 *
 * Copyright (C) 2001-2025 Daniel Horn, pyramid3d, Stephen G. Tuggy,
 * and other Vega Strike contributors
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */


/// Keyboard parsing
/// Parses keyboard commands

#define PY_SSIZE_T_CLEAN
#include <boost/python.hpp>
#include <set>
#include "firekeyboard.h"
#include "flybywire.h"
#include "navigation.h"
#include "src/in_joystick.h"
#include "cmd/unit_generic.h"
#include "communication.h"
#include "gfx/cockpit.h"
#include "gfx/animation.h"
#include "src/audiolib.h"
#include "src/config_xml.h"
#include "cmd/images.h"
#include "cmd/planet.h"
#include "cmd/script/flightgroup.h"
#include "cmd/script/mission.h"
#include "root_generic/vs_globals.h"
#include "gfx/car_assist.h"
#include "cmd/unit_util.h"
#include <algorithm>
#include "fire.h"
#include "docking.h"
#include "cmd/pilot.h"
//for getatmospheric
#include "cmd/role_bitmask.h"
#include "cmd/script/pythonmission.h"
#include "src/universe_util.h"
#include "src/universe.h"
#include "cmd/mount_size.h"
#include "cmd/weapon_info.h"
#include "src/vs_logging.h"
#include "cmd/unit_util.h"

extern bool toggle_pause();

FireKeyboard::FireKeyboard(unsigned int whichplayer, unsigned int whichjoystick) : Order(WEAPON, 0) {
    memset(savedTargets, 0, sizeof(void *) * NUMSAVEDTARGETS);
    this->autotrackingtoggle = 1;
    this->whichjoystick = whichjoystick;
    this->whichplayer = whichplayer;
    gunspeed = gunrange = .0001;
    refresh_target = true;
}

const unsigned int NUMCOMMKEYS = 10;

struct FIREKEYBOARDTYPE {
    FIREKEYBOARDTYPE() {
        toggleautotracking = togglewarpdrive = toggleglow = toggleanimation = lockkey = ECMkey = commKeys[0] =
                commKeys[1] = commKeys[2] = UP;
        commKeys[3] = commKeys[4] = commKeys[5] = commKeys[6] = commKeys[7] = commKeys[8] = commKeys[9] = turretaikey =
                UP;
        turretoffkey = turretfaw = saveTargetKeys[0] = saveTargetKeys[1] = saveTargetKeys[2] = saveTargetKeys[3] =
                saveTargetKeys[4] = UP;
        saveTargetKeys[5] = saveTargetKeys[6] = saveTargetKeys[7] = saveTargetKeys[8] = saveTargetKeys[9] =
                turretaikey = restoreTargetKeys[0] = UP;
        restoreTargetKeys[1] = restoreTargetKeys[2] = restoreTargetKeys[3] = restoreTargetKeys[4] =
                restoreTargetKeys[5] = restoreTargetKeys[6] = UP;
        restoreTargetKeys[7] = restoreTargetKeys[8] = restoreTargetKeys[9] = turretaikey = missiontargetkey =
                rmissiontargetkey = UP;
        ejectdock = eject = ejectcargo = ejectnonmissioncargo = firekey = missilekey = jfirekey = jtargetkey =
                jmissilekey = weapk = misk = UP;
        rweapk = rmisk = cloakkey = neartargetkey = targetskey = targetukey = threattargetkey = picktargetkey =
                subtargetkey = targetkey = UP;
        rneartargetkey = rtargetskey = rtargetukey = rthreattargetkey = rpicktargetkey = rtargetkey =
                nearturrettargetkey = threatturrettargetkey = UP;
        pickturrettargetkey = turrettargetkey = enslave = freeslave = incomingmissiletargetkey =
                rincomingmissiletargetkey = nearesthostilekey = UP;
        nearestdangeroushostilekey = missiletargetkey = rmissiletargetkey = nearestfriendlykey = nearestbasekey =
                nearestplanetkey = nearestjumpkey = togglepausekey = UP;
        shieldpowerstate = 1;
        doc = und = req = 0;
    }

    KBSTATE firekey;
    float shieldpowerstate;
    bool doc;
    bool und;
    bool req;
    KBSTATE missiletargetkey;
    KBSTATE incomingmissiletargetkey;
    KBSTATE rmissiletargetkey;
    KBSTATE rincomingmissiletargetkey;
    KBSTATE rneartargetkey;
    KBSTATE rthreattargetkey;
    KBSTATE rpicktargetkey;
    KBSTATE rtargetkey;
    KBSTATE rtargetskey;
    KBSTATE rtargetukey;
    KBSTATE missilekey;
    KBSTATE missiontargetkey;
    KBSTATE rmissiontargetkey;
    KBSTATE jfirekey;
    KBSTATE jtargetkey;
    KBSTATE jmissilekey;
    KBSTATE weapk;
    KBSTATE misk;
    KBSTATE rweapk;
    KBSTATE rmisk;
    KBSTATE eject;
    KBSTATE ejectdock;
    KBSTATE lockkey;
    KBSTATE ejectcargo;
    KBSTATE ejectnonmissioncargo;
    KBSTATE cloakkey;
    KBSTATE ECMkey;
    KBSTATE neartargetkey;
    KBSTATE threattargetkey;
    KBSTATE picktargetkey;
    KBSTATE subtargetkey;
    KBSTATE targetkey;
    KBSTATE targetskey;
    KBSTATE targetukey;
    KBSTATE turretaikey;
    KBSTATE turretoffkey;
    KBSTATE turretfaw;
    KBSTATE toggleglow;
    KBSTATE toggleautotracking;
    KBSTATE togglewarpdrive;
    KBSTATE toggleanimation;
    KBSTATE commKeys[NUMCOMMKEYS];
    KBSTATE saveTargetKeys[NUMSAVEDTARGETS];
    KBSTATE restoreTargetKeys[NUMSAVEDTARGETS];
    KBSTATE nearturrettargetkey;
    KBSTATE threatturrettargetkey;
    KBSTATE pickturrettargetkey;
    KBSTATE turrettargetkey;
    KBSTATE enslave;
    KBSTATE freeslave;
    //Added for nearest targets keys --ch
    KBSTATE nearesthostilekey;
    KBSTATE nearestdangeroushostilekey;
    KBSTATE nearestfriendlykey;
    KBSTATE nearestbasekey;
    KBSTATE nearestplanetkey;
    KBSTATE nearestjumpkey;
    KBSTATE togglepausekey;
};

static std::vector<FIREKEYBOARDTYPE> vectorOfKeyboardInput;

static FIREKEYBOARDTYPE &g() {
    while (vectorOfKeyboardInput.size() <= (unsigned int) _Universe->CurrentCockpit() || vectorOfKeyboardInput.size()
            <= (unsigned int) MAX_JOYSTICKS) {
        vectorOfKeyboardInput.push_back(FIREKEYBOARDTYPE());
    }
    return vectorOfKeyboardInput[_Universe->CurrentCockpit()];
}

FIREKEYBOARDTYPE &FireKeyboard::f() {
    return vectorOfKeyboardInput[whichplayer];
}

FIREKEYBOARDTYPE &FireKeyboard::j() {
    return vectorOfKeyboardInput[whichjoystick];
}

void FireKeyboard::SetShieldsOneThird(const KBData &, KBSTATE k) {
    if (k == PRESS) {
        float pow = 1. / 3;
        static soundContainer sc;
        if (sc.sound < 0) {
            const std::string str = vega_config::config->cockpit_audio.shield; /* default: "vdu_d" */);
            sc.loadsound(str);
        }
        sc.playsound();
        if (g().shieldpowerstate != pow) {
            g().shieldpowerstate = pow;
        } else {
            g().shieldpowerstate = 1;
        }
    }
}

void FireKeyboard::SetShieldsOff(const KBData &, KBSTATE k) {
    if (k == PRESS) {
        float pow = 0;
        static soundContainer sc;
        if (sc.sound < 0) {
            const std::string str = vega_config::config->cockpit_audio.shield; /* default: "vdu_d" */);
            sc.loadsound(str);
        }
        sc.playsound();
        if (g().shieldpowerstate != pow) {
            g().shieldpowerstate = pow;
        } else {
            g().shieldpowerstate = 1;
        }
    }
}

void FireKeyboard::SetShieldsTwoThird(const KBData &, KBSTATE k) {
    if (k == PRESS) {
        float pow = 2. / 3;
        static soundContainer sc;
        if (sc.sound < 0) {
            const std::string str = vega_config::config->cockpit_audio.shield; /* default: "vdu_d" */);
            sc.loadsound(str);
        }
        sc.playsound();
        if (g().shieldpowerstate != pow) {
            g().shieldpowerstate = pow;
        } else {
            g().shieldpowerstate = 1;
        }
    }
}

void FireKeyboard::ToggleGlow(const KBData &, KBSTATE k) {
    if (k == PRESS) {
        g().toggleglow = PRESS;
    }
}

void FireKeyboard::ToggleAutotracking(const KBData &, KBSTATE k) {
    if (k == PRESS) {
        g().toggleautotracking = PRESS;
    }
}

void FireKeyboard::ToggleWarpDrive(const KBData &, KBSTATE k) {
    if (k == PRESS) {
        g().togglewarpdrive = PRESS;
    }
}

void FireKeyboard::ToggleAnimation(const KBData &, KBSTATE k) {
    if (k == PRESS) {
        g().toggleanimation = PRESS;
    }
}

void FireKeyboard::PressComm1Key(const KBData &, KBSTATE k) {
    if (k == PRESS) {
        g().commKeys[0] = PRESS;
    }
}

void FireKeyboard::PressComm2Key(const KBData &, KBSTATE k) {
    if (k == PRESS) {
        g().commKeys[1] = PRESS;
    }
}

void FireKeyboard::PressComm3Key(const KBData &, KBSTATE k) {
    if (k == PRESS) {
        g().commKeys[2] = PRESS;
    }
}

void FireKeyboard::PressComm4Key(const KBData &, KBSTATE k) {
    if (k == PRESS) {
        g().commKeys[3] = PRESS;
    }
}

void FireKeyboard::PressComm5Key(const KBData &, KBSTATE k) {
    if (k == PRESS) {
        g().commKeys[4] = PRESS;
    }
}

void FireKeyboard::PressComm6Key(const KBData &, KBSTATE k) {
    if (k == PRESS) {
        g().commKeys[5] = PRESS;
    }
}

void FireKeyboard::PressComm7Key(const KBData &, KBSTATE k) {
    if (k == PRESS) {
        g().commKeys[6] = PRESS;
    }
}

void FireKeyboard::PressComm8Key(const KBData &, KBSTATE k) {
    if (k == PRESS) {
        g().commKeys[7] = PRESS;
    }
}

void FireKeyboard::PressComm9Key(const KBData &, KBSTATE k) {
    if (k == PRESS) {
        g().commKeys[8] = PRESS;
    }
}

void FireKeyboard::PressComm10Key(const KBData &, KBSTATE k) {
    if (k == PRESS) {
        g().commKeys[9] = PRESS;
    }
}

void FireKeyboard::SaveTarget1Key(const KBData &, KBSTATE k) {
    if (k == PRESS) {
        g().saveTargetKeys[0] = PRESS;
    }
}

void FireKeyboard::SaveTarget2Key(const KBData &, KBSTATE k) {
    if (k == PRESS) {
        g().saveTargetKeys[1] = PRESS;
    }
}

void FireKeyboard::SaveTarget3Key(const KBData &, KBSTATE k) {
    if (k == PRESS) {
        g().saveTargetKeys[2] = PRESS;
    }
}

void FireKeyboard::SaveTarget4Key(const KBData &, KBSTATE k) {
    if (k == PRESS) {
        g().saveTargetKeys[3] = PRESS;
    }
}

void FireKeyboard::SaveTarget5Key(const KBData &, KBSTATE k) {
    if (k == PRESS) {
        g().saveTargetKeys[4] = PRESS;
    }
}

void FireKeyboard::SaveTarget6Key(const KBData &, KBSTATE k) {
    if (k == PRESS) {
        g().saveTargetKeys[5] = PRESS;
    }
}

void FireKeyboard::SaveTarget7Key(const KBData &, KBSTATE k) {
    if (k == PRESS) {
        g().saveTargetKeys[6] = PRESS;
    }
}

void FireKeyboard::SaveTarget8Key(const KBData &, KBSTATE k) {
    if (k == PRESS) {
        g().saveTargetKeys[7] = PRESS;
    }
}

void FireKeyboard::SaveTarget9Key(const KBData &, KBSTATE k) {
    if (k == PRESS) {
        g().saveTargetKeys[8] = PRESS;
    }
}

void FireKeyboard::SaveTarget10Key(const KBData &, KBSTATE k) {
    if (k == PRESS) {
        g().saveTargetKeys[9] = PRESS;
    }
}

void FireKeyboard::RestoreTarget1Key(const KBData &, KBSTATE k) {
    if (k == PRESS) {
        g().restoreTargetKeys[0] = PRESS;
    }
}

void FireKeyboard::RestoreTarget2Key(const KBData &, KBSTATE k) {
    if (k == PRESS) {
        g().restoreTargetKeys[1] = PRESS;
    }
}

void FireKeyboard::RestoreTarget3Key(const KBData &, KBSTATE k) {
    if (k == PRESS) {
        g().restoreTargetKeys[2] = PRESS;
    }
}

void FireKeyboard::RestoreTarget4Key(const KBData &, KBSTATE k) {
    if (k == PRESS) {
        g().restoreTargetKeys[3] = PRESS;
    }
}

void FireKeyboard::RestoreTarget5Key(const KBData &, KBSTATE k) {
    if (k == PRESS) {
        g().restoreTargetKeys[4] = PRESS;
    }
}

void FireKeyboard::RestoreTarget6Key(const KBData &, KBSTATE k) {
    if (k == PRESS) {
        g().restoreTargetKeys[5] = PRESS;
    }
}

void FireKeyboard::RestoreTarget7Key(const KBData &, KBSTATE k) {
    if (k == PRESS) {
        g().restoreTargetKeys[6] = PRESS;
    }
}

void FireKeyboard::RestoreTarget8Key(const KBData &, KBSTATE k) {
    if (k == PRESS) {
        g().restoreTargetKeys[7] = PRESS;
    }
}

void FireKeyboard::RestoreTarget9Key(const KBData &, KBSTATE k) {
    if (k == PRESS) {
        g().restoreTargetKeys[8] = PRESS;
    }
}

void FireKeyboard::RestoreTarget10Key(const KBData &, KBSTATE k) {
    if (k == PRESS) {
        g().restoreTargetKeys[9] = PRESS;
    }
}

extern void LeadMe(Unit *un, string directive, string speech, bool changetarget);

static void LeadMe(string directive, string speech, bool changetarget) {
    Unit *un = _Universe->AccessCockpit()->GetParent();
    if (un) {
        LeadMe(un, directive, speech, changetarget);
    }
}

void FireKeyboard::RequestClearenceKey(const KBData &, KBSTATE k) {
    if (k == PRESS) {
        g().req = true;
    }
    if (k == RELEASE) {
        g().req = false;
    }
}

void FireKeyboard::DockKey(const KBData &, KBSTATE k) {
    Unit *u = _Universe->AccessCockpit()->GetParent();
    if (k == PRESS && u && (u->isSubUnit() == false)) {
        g().doc = true;
    }
    if (k == RELEASE && u && (u->isSubUnit() == false)) {
        g().doc = false;
    }
}

void FireKeyboard::UnDockKey(const KBData &, KBSTATE k) {
    Unit *u = _Universe->AccessCockpit()->GetParent();
    if (k == PRESS && u && (u->isSubUnit() == false)) {
        g().und = true;
    }
    if (k == RELEASE && u && (u->isSubUnit() == false)) {
        g().und = false;
    }
}

void FireKeyboard::EjectKey(const KBData &, KBSTATE k) {
    if (k == PRESS) {
        LeadMe("", "I am ejecting! Record the current location of my ship.", false); //used to clear group target
        LeadMe("e", "Then get over here and pick me up!", false);
        g().eject = k;
    }
}

void FireKeyboard::EjectDockKey(const KBData &, KBSTATE k) {
    if (k == PRESS) {
        g().ejectdock = k;
        g().doc = true;
    }
    if (k == RELEASE) {
        g().doc = false;
    }
}

void FireKeyboard::TurretAIOn(const KBData &, KBSTATE k) {
    if (k == PRESS) {
        g().turretaikey = k;
    }
}

void FireKeyboard::TurretAIOff(const KBData &, KBSTATE k) {
    if (k == PRESS) {
        g().turretoffkey = k;
    }
}

void FireKeyboard::TurretFireAtWill(const KBData &, KBSTATE k) {
    if (k == PRESS) {
        g().turretfaw = k;
    }
}

void FireKeyboard::MissionTargetKey(const KBData &, KBSTATE k) {
    if (k == PRESS) {
        g().missiontargetkey = PRESS;
    }
}

void FireKeyboard::ReverseMissionTargetKey(const KBData &, KBSTATE k) {
    if (k == PRESS) {
        g().rmissiontargetkey = PRESS;
    }
}

void FireKeyboard::EjectCargoKey(const KBData &, KBSTATE k) {
    if (k == PRESS) {
        g().ejectcargo = k;
    }
}

void FireKeyboard::EnslaveKey(const KBData &, KBSTATE k) {
    if (k == PRESS) {
        g().enslave = k;
    }
}

void FireKeyboard::FreeSlaveKey(const KBData &, KBSTATE k) {
    if (k == PRESS) {
        g().freeslave = k;
    }
}

void FireKeyboard::EjectNonMissionCargoKey(const KBData &, KBSTATE k) {
    if (k == PRESS) {
        g().ejectnonmissioncargo = k;
    }
}

void FireKeyboard::CloakKey(const KBData &, KBSTATE k) {
    if (k == PRESS) {
        g().cloakkey = k;
    }
}

void FireKeyboard::LockKey(const KBData &, KBSTATE k) {
    if (k == PRESS) {
        g().lockkey = k;
    }
}

void FireKeyboard::ECMKey(const KBData &, KBSTATE k) {
    if (k == PRESS) {
        g().ECMkey = k;
    }
}

void FireKeyboard::FireKey(const KBData &, KBSTATE k) {
    if (g().firekey == DOWN && k == UP) {
        return;
    }
    if (k == UP && g().firekey == RELEASE) {
    } else {
        g().firekey = k;
    }
}

void ExamineWhenTargetKey();

void FireKeyboard::TargetKey(const KBData &, KBSTATE k) {
    if (g().targetkey != PRESS) {
        g().targetkey = k;
    }
    if (k == RESET) {
        g().targetkey = PRESS;
    }
    if (k == PRESS) {
        ExamineWhenTargetKey();
    }
}

void FireKeyboard::PickTargetKey(const KBData &, KBSTATE k) {
    if (g().picktargetkey != PRESS) {
        g().picktargetkey = k;
    }
    if (k == RESET) {
        g().picktargetkey = PRESS;
    }
    if (k == PRESS) {
        ExamineWhenTargetKey();
    }
}

void FireKeyboard::MissileTargetKey(const KBData &, KBSTATE k) {
    if (g().missiletargetkey != PRESS) {
        g().missiletargetkey = k;
    }
    if (k == PRESS) {
        ExamineWhenTargetKey();
    }
}

void FireKeyboard::IncomingMissileTargetKey(const KBData &, KBSTATE k) {
    if (g().incomingmissiletargetkey != PRESS) {
        g().incomingmissiletargetkey = k;
    }
    if (k == PRESS) {
        ExamineWhenTargetKey();
    }
}

void FireKeyboard::ReverseMissileTargetKey(const KBData &, KBSTATE k) {
    if (g().rmissiletargetkey != PRESS) {
        g().rmissiletargetkey = k;
    }
    if (k == PRESS) {
        ExamineWhenTargetKey();
    }
}

void FireKeyboard::ReverseIncomingMissileTargetKey(const KBData &, KBSTATE k) {
    if (g().rincomingmissiletargetkey != PRESS) {
        g().rincomingmissiletargetkey = k;
    }
    if (k == PRESS) {
        ExamineWhenTargetKey();
    }
}

void FireKeyboard::NearestTargetKey(const KBData &, KBSTATE k) {
    if (g().neartargetkey != PRESS) {
        g().neartargetkey = k;
    }
    if (k == PRESS) {
        ExamineWhenTargetKey();
    }
}

void FireKeyboard::SubUnitTargetKey(const KBData &, KBSTATE k) {
    if (g().subtargetkey != PRESS) {
        g().subtargetkey = k;
    }
    if (k == PRESS) {
        ExamineWhenTargetKey();
    }
}

void FireKeyboard::ThreatTargetKey(const KBData &, KBSTATE k) {
    if (g().threattargetkey != PRESS) {
        g().threattargetkey = k;
    }
    if (k == PRESS) {
        ExamineWhenTargetKey();
    }
}

void FireKeyboard::UnitTargetKey(const KBData &, KBSTATE k) {
    if (g().targetukey != PRESS) {
        g().targetukey = k;
    }
    if (k == PRESS) {
        ExamineWhenTargetKey();
    }
}

void FireKeyboard::SigTargetKey(const KBData &, KBSTATE k) {
    if (g().targetskey != PRESS) {
        g().targetskey = k;
    }
    if (k == PRESS) {
        ExamineWhenTargetKey();
    }
}

void FireKeyboard::ReverseTargetKey(const KBData &, KBSTATE k) {
    if (g().rtargetkey != PRESS) {
        g().rtargetkey = k;
    }
    if (k == RESET) {
        g().rtargetkey = PRESS;
    }
    if (k == PRESS) {
        ExamineWhenTargetKey();
    }
}

void FireKeyboard::ReversePickTargetKey(const KBData &, KBSTATE k) {
    if (g().rpicktargetkey != PRESS) {
        g().rpicktargetkey = k;
    }
    if (k == RESET) {
        g().rpicktargetkey = PRESS;
    }
    if (k == PRESS) {
        ExamineWhenTargetKey();
    }
}

void FireKeyboard::ReverseNearestTargetKey(const KBData &, KBSTATE k) {
    if (g().rneartargetkey != PRESS) {
        g().rneartargetkey = k;
    }
    if (k == PRESS) {
        ExamineWhenTargetKey();
    }
}

void FireKeyboard::ReverseThreatTargetKey(const KBData &, KBSTATE k) {
    if (g().rthreattargetkey != PRESS) {
        g().rthreattargetkey = k;
    }
    if (k == PRESS) {
        ExamineWhenTargetKey();
    }
}

void FireKeyboard::ReverseUnitTargetKey(const KBData &, KBSTATE k) {
    if (g().rtargetukey != PRESS) {
        g().rtargetukey = k;
    }
    if (k == PRESS) {
        ExamineWhenTargetKey();
    }
}

void FireKeyboard::ReverseSigTargetKey(const KBData &, KBSTATE k) {
    if (g().rtargetskey != PRESS) {
        g().rtargetskey = k;
    }
    if (k == PRESS) {
        ExamineWhenTargetKey();
    }
}

void FireKeyboard::NearestHostileTargetKey(const KBData &, KBSTATE k) {
    if (g().nearesthostilekey != PRESS) {
        g().nearesthostilekey = k;
    }
    if (k == PRESS) {
        ExamineWhenTargetKey();
    }
}

void FireKeyboard::NearestDangerousHostileKey(const KBData &, KBSTATE k) {
    if (g().nearestdangeroushostilekey != PRESS) {
        g().nearestdangeroushostilekey = k;
    }
    if (k == PRESS) {
        ExamineWhenTargetKey();
    }
}

void FireKeyboard::NearestFriendlyKey(const KBData &, KBSTATE k) {
    if (g().nearestfriendlykey != PRESS) {
        g().nearestfriendlykey = k;
    }
    if (k == PRESS) {
        ExamineWhenTargetKey();
    }
}

void FireKeyboard::NearestBaseKey(const KBData &, KBSTATE k) {
    if (g().nearestbasekey != PRESS) {
        g().nearestbasekey = k;
    }
    if (k == PRESS) {
        ExamineWhenTargetKey();
    }
}

void FireKeyboard::NearestPlanetKey(const KBData &, KBSTATE k) {
    if (g().nearestplanetkey != PRESS) {
        g().nearestplanetkey = k;
    }
    if (k == PRESS) {
        ExamineWhenTargetKey();
    }
}

void FireKeyboard::NearestJumpKey(const KBData &, KBSTATE k) {
    if (g().nearestjumpkey != PRESS) {
        g().nearestjumpkey = k;
    }
    if (k == PRESS) {
        ExamineWhenTargetKey();
    }
}

void FireKeyboard::TogglePause(const KBData &, KBSTATE k) {
    if (k == PRESS) {
        VS_LOG(info, "FireKeyboard::TogglePause(): Pause key detected");
        g().togglepausekey = k;
    }
}

extern unsigned int DoSpeech(Unit *un, Unit *player_un, const FSM::Node &convNode);
extern Unit *GetThreat(Unit *par, Unit *leader);

void HelpOut(bool crit, std::string conv) {
    Unit *un = _Universe->AccessCockpit()->GetParent();
    if (un) {
        Unit *par = NULL;
        DoSpeech(un, NULL, FSM::Node::MakeNode(conv, .1));
        for (un_iter ui = _Universe->activeStarSystem()->getUnitList().createIterator();
                (par = (*ui));
                ++ui) {
            if ((crit && UnitUtil::getFactionRelation(par, un) > 0) || par->faction == un->faction) {
                Unit *threat = GetThreat(par, un);
                CommunicationMessage c(par, un, NULL, 0);
                if (threat) {
                    par->Target(threat);
                    c.SetCurrentState(c.fsm->GetYesNode(), NULL, 0);
                } else {
                    c.SetCurrentState(c.fsm->GetNoNode(), NULL, 0);
                }
                Order *o = un->getAIState();
                if (o) {
                    o->Communicate(c);
                }
            }
        }
    }
}

void FireKeyboard::JoinFg(const KBData &, KBSTATE k) {
    if (k == PRESS) {
        Unit *un = _Universe->AccessCockpit()->GetParent();
        if (un) {
            Unit *targ = un->Target();
            if (targ) {
                if (targ->faction == un->faction) {
                    Flightgroup *fg = targ->getFlightgroup();
                    if (fg) {
                        if (fg != un->getFlightgroup()) {
                            if (un->getFlightgroup()) {
                                un->getFlightgroup()->Decrement(un);
                            }
                            fg->nr_ships_left++;
                            fg->nr_ships++;
                            un->SetFg(fg, fg->nr_ships_left - 1);
                        }
                    }
                }
            }
        }
    }
}

void FireKeyboard::AttackTarget(const KBData &, KBSTATE k) {
    if (k == PRESS) {
        LeadMe("k", "Attack my target!", true);
    }
}

void FireKeyboard::HelpMeOut(const KBData &, KBSTATE k) {
    if (k == PRESS) {
        LeadMe("h", "Help me out!", false);
    }
}

void FireKeyboard::HelpMeOutFaction(const KBData &, KBSTATE k) {
    if (k == PRESS) {
        HelpOut(false, "Help me out! I need critical assistance!");
    }
}

void FireKeyboard::HelpMeOutCrit(const KBData &, KBSTATE k) {
    if (k == PRESS) {
        HelpOut(true, "Help me out! Systems going critical!");
    }
}

void FireKeyboard::DockWithMe(const KBData &, KBSTATE k) {
    if (k == PRESS) {
        LeadMe("l", "Get in front of me and prepare to be tractored in.", false);
    }
}

void FireKeyboard::DefendTarget(const KBData &, KBSTATE k) {
    if (k == PRESS) {
        LeadMe("p", "Defend my target!", true);
    }
}

void FireKeyboard::DockTarget(const KBData &, KBSTATE k) {
    if (k == PRESS) {
        LeadMe("t", "Dock at my target!", true);
    }
}

void FireKeyboard::HoldPosition(const KBData &, KBSTATE k) {
    if (k == PRESS) {
        LeadMe("s", "Hold Position!", true);
    }
}

void FireKeyboard::FormUp(const KBData &, KBSTATE k) {
    if (k == PRESS) {
        LeadMe("f", "Form on my wing.", false);
    }
}

void FireKeyboard::BreakFormation(const KBData &, KBSTATE k) {
    if (k == PRESS) {
        LeadMe("", "Break formation!", false);         //used to clear group target

        LeadMe("b", "Pick a target and open fire!", false);
    }
}

void FireKeyboard::TargetTurretKey(const KBData &, KBSTATE k) {
    if (g().turrettargetkey != PRESS) {
        g().turrettargetkey = k;
    }
    if (k == RESET) {
        g().turrettargetkey = PRESS;
    }
    if (k == PRESS) {
        ExamineWhenTargetKey();
    }
}

void FireKeyboard::PickTargetTurretKey(const KBData &, KBSTATE k) {
    if (g().pickturrettargetkey != PRESS) {
        g().pickturrettargetkey = k;
    }
    if (k == RESET) {
        g().pickturrettargetkey = PRESS;
    }
    if (k == PRESS) {
        ExamineWhenTargetKey();
    }
}

void FireKeyboard::NearestTargetTurretKey(const KBData &, KBSTATE k) {
    if (g().nearturrettargetkey != PRESS) {
        g().nearturrettargetkey = k;
    }
    if (k == PRESS) {
        ExamineWhenTargetKey();
    }
}

void FireKeyboard::ThreatTargetTurretKey(const KBData &, KBSTATE k) {
    if (g().threatturrettargetkey != PRESS) {
        g().threatturrettargetkey = k;
    }
    if (k == PRESS) {
        ExamineWhenTargetKey();
    }
}

void FireKeyboard::ReverseWeapSelKey(const KBData &, KBSTATE k) {
    if (g().rweapk != PRESS) {
        g().rweapk = k;
    }
}

void FireKeyboard::ReverseMisSelKey(const KBData &, KBSTATE k) {
    if (g().rmisk != PRESS) {
        g().rmisk = k;
    }
}

void FireKeyboard::WeapSelKey(const KBData &, KBSTATE k) {
    if (g().weapk != PRESS) {
        g().weapk = k;
    }
}

void FireKeyboard::MisSelKey(const KBData &, KBSTATE k) {
    if (g().misk != PRESS) {
        g().misk = k;
    }
}

void FireKeyboard::MissileKey(const KBData &, KBSTATE k) {
    if (k == PRESS) {
        g().missilekey = k;
    }
}

static bool isNotTurretOwner(Unit *parent, Unit *un) {
    return parent->isSubUnit() == false || un != parent->owner;
}

bool TargMission(Unit *me, Unit *target) {
    for (unsigned int i = 0; i < active_missions.size(); ++i) {
        if (active_missions[i]->runtime.pymissions) {
            vector<UnitContainer *> *relevant = &active_missions[i]->runtime.pymissions->relevant_units;
            vector<UnitContainer *>::iterator ir = relevant->begin();
            vector<UnitContainer *>::iterator ie = relevant->end();
            for (; ir != ie; ++ir) {
                if (**ir == target) {
                    return true;
                }
            }
        }
    }
    return false;
}

bool TargAll(Unit *me, Unit *target) {
    const bool can_target_sun = vega_config::config->graphics.can_target_sun;
    return (me->InRange(target, true,
            false)
            || me->InRange(target, true, true)) && (can_target_sun || !UnitUtil::isSun(target)) && isNotTurretOwner(
            me,
            target);
}

bool TargSig(Unit *me, Unit *target) {
    const bool can_target_asteroid = vega_config::config->graphics.can_target_asteroid;

    bool ret =
            me->InRange(target, false,
                    true) && (UnitUtil::isSignificant(target) || TargMission(me, target)) && isNotTurretOwner(
                    me,
                    target);
    if (can_target_asteroid == false) {
        if (target->isUnit() == Vega_UnitType::asteroid || target->name.get().find("Asteroid") == 0) {
            ret = false;
        }
    }
    return ret;
}

extern Unit *getTopLevelOwner();

bool TargUn(Unit *me, Unit *target) {
    const bool can_target_cargo = vega_config::config->graphics.can_target_cargo;
    int up = FactionUtil::GetUpgradeFaction();
    return me->InRange(target, true,
            false)
            && (target->isUnit() == Vega_UnitType::unit
                    || target->isUnit() == Vega_UnitType::enhancement) && getTopLevelOwner() != target->owner
            && (can_target_cargo || target->faction != up) && isNotTurretOwner(me, target);
}

bool TargMissile(Unit *me, Unit *target) {
    return me->InRange(target, true, false) && (target->isUnit() == Vega_UnitType::missile) && isNotTurretOwner(me, target);
}

bool TargIncomingMissile(Unit *me, Unit *target) {
    Unit *tt = target->Target();
    return TargMissile(me, target)
            && (tt == me || (me->isSubUnit() && tt == _Universe->AccessCockpit()->GetSaveParent()));
}

bool TargFront(Unit *me, Unit *target) {
    if (!TargAll(me, target)) {
        return false;
    }
    QVector delta(target->Position() - me->Position());
    double mm = delta.Magnitude();
    double tempmm = mm - target->rSize();
    if (tempmm > 0.0001) {
        if ((me->ToLocalCoordinates(Vector(delta.i, delta.j, delta.k)).k / tempmm) > .995) {
            return true;
        }
    }
    return false;
}

bool TargThreat(Unit *me, Unit *target) {
    if (!TargAll(me, target)) {
        return false;
    }
    if (target->isUnit() == Vega_UnitType::missile) {
        return false;
    }
    if (target->Target() == me) {
        return true;
    }
    if (me->Threat() == target) {
        return true;
    }
    return false;
}

bool TargNear(Unit *me, Unit *target) {
    const bool can_target_sun = vega_config::config->graphics.can_target_sun;
    return (me->getRelation(target) < 0
            || TargThreat(me,
                    target)
            || target->getRelation(me) < 0)
            && TargAll(me,
                    target) && target->isUnit() != Vega_UnitType::missile
            && (can_target_sun || !UnitUtil::isSun(target)) && isNotTurretOwner(me,
            target);
}

//Target the nearest unit of a specified type
//Possible types are:
//0 = hostile
//1 = hostile targetting me
//2 = friendly
//3 = base
//4 = planet
//5 = jump point
bool getNearestTargetUnit(Unit *me, int iType) {
    QVector pos(me->Position());
    Unit *un = NULL;
    Unit *targ = NULL;
    double minrange = FLT_MAX;
    for (un_iter i = _Universe->activeStarSystem()->getUnitList().createIterator(); (un = (*i)); ++i) {
        if (un == me) {
            continue;
        }
        if (un->Destroyed()) {
            continue;
        }
        if (!(me->InRange(un, true, false))
                || !(me->InRange(un, true, true))) {
            continue;
        }
        if ((iType == 0)
                && ((un->isUnit() != Vega_UnitType::unit)
                        || !me->isEnemy(un))) {
            continue;
        }
        if ((iType == 1)
                && ((un->isUnit() != Vega_UnitType::unit)
                        || (!me->isEnemy(un)
                                && (un->Target() != me)))) {
            continue;
        }
        if ((iType == 2)
                && ((un->isUnit() != Vega_UnitType::unit)
                        || me->isEnemy(un)
                        || (UnitUtil::getFlightgroupName(un) == "Base"))) {
            continue;
        }
        if ((iType == 3)
                && (UnitUtil::getFlightgroupName(un) != "Base")) {
            continue;
        }
        if ((iType == 4)
                && ((!un->isPlanet())
                        || (un->isJumppoint()))) {
            continue;
        }
        if ((iType == 5)
                && (!un->isJumppoint())) {
            continue;
        }
        double temp = (un->Position() - pos).Magnitude();
        if (targ == NULL) {
            targ = un;
            minrange = temp;
        } else if (temp < minrange) {
            targ = un;
            minrange = temp;
        }
    }
    if (targ == NULL) {
        return false;
    }
    me->Target(targ);

    return true;
}

bool ChooseTargets(Unit *me, bool (*typeofunit)(Unit *, Unit *), bool reverse) {
    UnitCollection &drawlist = _Universe->activeStarSystem()->getUnitList();
    vector<Unit *> vec;
    Unit *target;
    for (un_iter iter = drawlist.createIterator(); (target = *iter) != NULL; ++iter) {
        vec.push_back(target);
    }
    if (vec.size() == 0) {
        return false;
    }
    if (reverse) {
        std::reverse(vec.begin(), vec.end());
    }
    std::vector<Unit *>::const_iterator veciter = std::find(vec.begin(), vec.end(), me->Target());
    if (veciter != vec.end()) {
        ++veciter;
    }
    int cur = 0;
    while (1) {
        while (veciter != vec.end()) {
            if (((*veciter) != me) && ((*veciter)->hull.Get() >= 0) && typeofunit(me, (*veciter))) {
                me->Target(*veciter);

                if ((*veciter) != NULL) {
                    if (reverse) {
                        static soundContainer foosound;
                        if (foosound.sound < 0) {
                            const std::string str = vega_config::config->cockpit_audio.target; /* default: "vdu_b" */);
                            foosound.loadsound(str);
                        }
                        foosound.playsound();
                    } else {
                        static soundContainer foobersound;
                        if (foobersound.sound < 0) {
                            const std::string str = vega_config::config->cockpit_audio.target_reverse; /* default: "vdu_a" */);
                            foobersound.loadsound(str);
                        }
                        foobersound.playsound();
                    }
                }
                return true;
            }
            ++veciter;
        }
        ++cur;
        if (cur >= 2) {
            break;
        }
        veciter = vec.begin();
    }
    return true;
}

void ChooseSubTargets(Unit *me) {
    Unit *parent = UnitUtil::owner(me->Target());
    if (!parent) {
        return;
    }
    un_iter uniter = parent->getSubUnits();
    if (parent == me->Target()) {
        if (!(*uniter)) {
            return;
        }
        me->Target(*uniter);
        return;
    }
    Unit *tUnit;
    for (; (tUnit = *uniter) != NULL; ++uniter) {
        if (tUnit == me->Target()) {
            ++uniter;
            tUnit = *uniter;
            break;
        }
    }
    if (tUnit) {
        me->Target(tUnit);
    } else {
        me->Target(parent);
    }
}

FireKeyboard::~FireKeyboard() {
#ifdef ORDERDEBUG
    VS_LOG_AND_FLUSH(trace, (boost::format("fkb%1$x") % this));
#endif
}

bool FireKeyboard::ShouldFire(Unit *targ) {
    float dist = FLT_MAX;
    float mrange;
    if (gunspeed == .0001) {
        parent->getAverageGunSpeed(gunspeed, gunrange, mrange);
    }
    float angle = parent->cosAngleTo(targ, dist, gunspeed, gunrange);
    targ->Threaten(parent, angle / (dist < .8 ? .8 : dist));
    if (targ == parent->Target()) {
        distance = dist;
    }
    return dist < .8 && angle > 1;
}

static bool UnDockNow(Unit *me, Unit *targ) {
    bool ret = false;
    Unit *un;
    for (un_iter i = _Universe->activeStarSystem()->getUnitList().createIterator();
            (un = *i) != NULL;
            ++i) {
        if (un->isDocked(me)) {
            if (me->UnDock(un)) {
                ret = true;
            }
        }
    }
    return ret;
}

void Enslave(Unit *, bool);

void abletodock(int dock) {
    switch (dock) {
        case 5: {
            static soundContainer reqsound;
            if (reqsound.sound == -2) {
                const std::string str = vega_config::config->cockpit_audio.undocking_complete; /* default: "undocking_complete" */);
                reqsound.loadsound(str);
            }
            reqsound.playsound();
            break;
        }
        case 4: {
            static soundContainer reqsound;
            if (reqsound.sound == -2) {
                const std::string str = vega_config::config->cockpit_audio.undocking_failed; /* default: "undocking_failed" */);
                reqsound.loadsound(str);
            }
            reqsound.playsound();
            break;
        }
        case 3: {
            static soundContainer reqsound;
            const std::string otherstr = vega_config::config->audio.automatic_docking_zone; /* default: "automatic_landing_zone.wav" */);
            if (otherstr != "" && rand() < RAND_MAX / 2) {
                static int s = AUDCreateSoundWAV(otherstr, false);
                AUDPlay(s, QVector(0, 0, 0), Vector(0, 0, 0), 1);
            } else {
                if (reqsound.sound == -2) {
                    const std::string str = vega_config::config->cockpit_audio.docking_complete; /* default: "docking_complete" */);
                    reqsound.loadsound(str);
                }
                reqsound.playsound();
            }
            break;
        }
        case 2: {
            static soundContainer reqsound;
            if (reqsound.sound == -2) {
                const std::string str = vega_config::config->cockpit_audio.docking_failed; /* default: "docking_failed" */);
                reqsound.loadsound(str);
            }
            reqsound.playsound();
            break;
        }
        case 1: {
            static soundContainer reqsound;
            if (reqsound.sound == -2) {
                const std::string str = vega_config::config->cockpit_audio.docking_granted; /* default: "request_granted" */);
                reqsound.loadsound(str);
            }
            reqsound.playsound();
            break;
        }
        case 0: {
            static soundContainer reqsound;
            if (reqsound.sound == -2) {
                const std::string str = vega_config::config->cockpit_audio.docking_denied; /* default: "request_denied" */);
                reqsound.loadsound(str);
            }
            reqsound.playsound();
            break;
        }
    }
}

static bool SuperDock(Unit *parent, Unit *target) {
    if (UnitUtil::isCloseEnoughToDock(parent, target)) {
        if (UnitUtil::isDockableUnit(target)) {
            for (unsigned int i = 0; i < target->GetImageInformation().dockingports.size(); ++i) {
                if (!target->GetImageInformation().dockingports[i].IsOccupied()) {
                    return parent->ForceDock(target, i) != 0;
                }
            }
        }
    }
    return false;
}

static bool TryDock(Unit *parent, Unit *targ, unsigned char playa, int severity) {
    const float min_docking_relationship = vega_config::config->ai.min_docking_relationship;
    const bool can_dock_to_enemy_base = vega_config::config->ai.can_dock_to_enemy_base;
    const bool nojumpinSPEC = vega_config::config->physics.noSPECJUMP;
    bool SPEC_interference = targ && parent && nojumpinSPEC
            && (targ->ftl_drive.Enabled() || parent->ftl_drive.Enabled());
    unsigned char gender = 0;
    vector<Animation *> *anim = NULL;
    if (SPEC_interference) {
        //FIXME js_NUDGE -- need some indicator of non-interaction because one or both objects are in SPEC.
        return false;
    }
    anim = targ->pilot->getCommFaces(gender);

    bool isDone = false;
    if (targ->getRelation(parent) >= min_docking_relationship
            || (can_dock_to_enemy_base && UnitUtil::getFlightgroupName(targ) == "Base")) {
        bool hasDock = severity == 0 ? parent->Dock(targ) : SuperDock(parent, targ);

        CommunicationMessage c(targ, parent, anim, gender);
        if (hasDock) {
            isDone = true;
            c.SetCurrentState(c.fsm->GetDockNode(), anim, gender);
            abletodock(3);
            if (parent->getAIState()) {
                parent->getAIState()->Communicate(c);
            }
            parent->UpgradeInterface(targ);
        } else if (UnDockNow(parent, targ)) {
            isDone = true;
            c.SetCurrentState(c.fsm->GetUnDockNode(), anim, gender);
            if (parent->getAIState()) {
                parent->getAIState()->Communicate(c);
            }
            abletodock(5);
        }
    } else if (parent->GetComputerData().target == targ) {
        CommunicationMessage c(targ, parent, anim, gender);
        c.SetCurrentState(c.fsm->GetNoNode(), anim, gender);
        if (parent->getAIState()) {
            parent->getAIState()->Communicate(c);
        }
    }
    return isDone;
}

static bool ExecuteRequestClearenceKey(Unit *parent, Unit *endt) {
    bool tmp = endt->RequestClearance(parent);
    if (endt->getRelation(parent) >= 0) {
        if (endt->ftl_drive.Enabled()) {
            endt->graphicOptions.WarpRamping = 1;
        }
        endt->ftl_drive.Disable();
        const float clearencetime = (vega_config::config->general.docking_time /* default: "20" */);
        endt->EnqueueAIFirst(new Orders::ExecuteFor(new Orders::MatchVelocity(Vector(0, 0, 0),
                Vector(0, 0, 0),
                true,
                false,
                true), clearencetime));
    }
    return tmp;
}

static void DoDockingOps(Unit *parent, Unit *targ, unsigned char playa, unsigned char gender) {
    const int maxseverity = vega_config::config->ai.dock_to_area /* default: "false" */ ? 2 : 1;
    Unit *endt = targ;
    bool wasdock = vectorOfKeyboardInput[playa].doc;
    if (vectorOfKeyboardInput[playa].doc) {
        bool isDone = false;
        if (targ) {
            for (int severity = 0; severity < maxseverity; ++severity) {
                targ->RequestClearance(parent);
                if ((isDone = TryDock(parent, targ, playa, severity)) != false) {
                    parent->EndRequestClearance(targ);
                    break;
                } else {
                    parent->EndRequestClearance(targ);
                }
            }
        }
        if (!isDone) {
            for (int severity = 0; severity < maxseverity && !isDone; ++severity) {
                for (un_iter u = _Universe->activeStarSystem()->getUnitList().createIterator();
                        (targ = *u) != NULL && !isDone;
                        ++u) {
                    //Let's make sure potentials are actually in range, and have
                    //docking ports before we try to dock with them.
                    if ((targ != parent)
                            && (UnitUtil::isDockableUnit(targ))
                            && (UnitUtil::isCloseEnoughToDock(parent, targ))) {
                        targ->RequestClearance(parent);
                        if (TryDock(parent, targ, playa, severity)) {
                            parent->Target(targ);
                            isDone = true;
                            parent->EndRequestClearance(targ);
                            break;
                        } else {
                            parent->EndRequestClearance(targ);
                        }
                    }
                }
            }
        }
        if (!isDone) {
            if (endt) {
                ExecuteRequestClearenceKey(parent, endt);
            }
            abletodock(0);
        }
        vectorOfKeyboardInput[playa].doc = false;
    }
    if (vectorOfKeyboardInput[playa].req && endt != NULL) {
        bool request = ExecuteRequestClearenceKey(parent, endt);
        if (!request) {
            mission->msgcenter->add("game",
                    "all",
                    "[Computer] Cannot dock with insubstantial object, target another object and retry.");
            abletodock(0);
            return;
        } else if (!wasdock) {
            abletodock(1);
        }
        vectorOfKeyboardInput[playa].req = false;
    }
    if (vectorOfKeyboardInput[playa].und && endt != NULL) {
        CommunicationMessage c(endt, parent, NULL, 0);
        if (UnDockNow(parent, endt)) {
            c.SetCurrentState(c.fsm->GetUnDockNode(), NULL, 0);
            abletodock(5);
        } else {
            c.SetCurrentState(c.fsm->GetFailDockNode(), NULL, 0);
            abletodock(4);
        }
        parent->getAIState()->Communicate(c);
        vectorOfKeyboardInput[playa].und = 0;
    }
}

using std::list;

unsigned int FireKeyboard::DoSpeechAndAni(Unit *un, Unit *parent, class CommunicationMessage &c) {
    this->AdjustRelationTo(un, c.getCurrentState()->messagedelta);
    unsigned int retval = DoSpeech(un, parent, *c.getCurrentState());
    if (parent == _Universe->AccessCockpit()->GetParent()) {
        _Universe->AccessCockpit()->SetCommAnimation(c.ani, un);
    }
    this->refresh_target = true;
    return retval;
}

static void MyFunction() {
    //quit it--he's dead all ready
    const std::string comm_static = vega_config::config->graphics.comm_static; /* default: "static.ani" */);
    //dead dead dead dead
    static Animation Statuc(comm_static.c_str());
    //yep really dead
    _Universe->AccessCockpit()->SetCommAnimation(&Statuc, NULL);
}

void FireKeyboard::ProcessCommMessage(class CommunicationMessage &c) {
    Unit *un = c.sender.GetUnit();
    unsigned int whichsound = 0;
    bool foundValidMessage = false;
    if (_Universe->AccessCockpit()->CheckCommAnimation(un)) {
        return;
    }          //wait till later

    bool reallydospeech = false;
    if (un && !un->Destroyed()) {
        reallydospeech = true;
        for (list<CommunicationMessage>::iterator i = resp.begin(); i != resp.end(); i++) {
            if ((*i).sender.GetUnit() == un) {
                if ((i = resp.erase(i)) == resp.end()) {
                    break;
                }
            }
        }
        resp.push_back(c);
        if (!foundValidMessage) {
            whichsound = DoSpeechAndAni(un, parent, c);
        }
    } else if (0) {
        //none of this happens
        whichsound = DoSpeech(NULL, NULL, *c.getCurrentState());
        //this is when a unit is already dead
        if (parent == _Universe->AccessCockpit()->GetParent()) {
            MyFunction();
        }
        //mmhmm! Gcc-4.1 hack -- otherwise linker failure
    }
    float gain;
    int sound = c.getCurrentState()->GetSound(c.sex, whichsound, gain);
    if (reallydospeech && !AUDIsPlaying(sound)) {
        AUDPlay(sound, QVector(0, 0, 0), Vector(0, 0, 0), gain);
    }
}

using std::list;

static CommunicationMessage *GetTargetMessageQueue(Unit *targ, std::list<CommunicationMessage> &messagequeue) {
    CommunicationMessage *mymsg = NULL;
    for (list<CommunicationMessage>::iterator i = messagequeue.begin(); i != messagequeue.end(); i++) {
        if ((*i).sender.GetUnit() == targ) {
            mymsg = &(*i);
            break;
        }
    }
    return mymsg;
}

extern std::set<Unit *> arrested_list_do_not_dereference;

void Arrested(Unit *parent) {
    std::string fac = UniverseUtil::GetGalaxyFaction(UniverseUtil::getSystemFile());
    int own = FactionUtil::GetFactionIndex(fac);
    const std::string po = vega_config::config->galaxy.police_faction; /* default: "homeland-security" */);
    int police = FactionUtil::GetFactionIndex(po);
    int police2 = FactionUtil::GetFactionIndex(po + "_" + fac);
    float ownrel = UnitUtil::getRelationFromFaction(parent, own);
    bool attack = ownrel < 0;
    if (!attack) {
        Unit *contra = FactionUtil::GetContraband(own);
        if (contra) {
            for (unsigned int i = 0; (!attack) && i < parent->numCargo(); ++i) {
                Cargo *ci = &parent->GetCargo(i);
                for (unsigned int j = 0; j < contra->numCargo(); ++j) {
                    Cargo *cj = &contra->GetCargo(j);
                    if (ci->GetName() == cj->GetName()) {
                        attack = true;
                        break;
                    }
                }
            }
        }
    }
    if (!attack) {
        Unit *un;
        for (un_iter i = _Universe->activeStarSystem()->getUnitList().createIterator();
                (un = *i) != NULL;
                ++i) {
            if (un->faction == own || un->faction == police || un->faction == police2) {
                if (un->Target() == parent || un->getRelation(parent) < 0) {
                    int parentCp = _Universe->whichPlayerStarship(parent);
                    if (parentCp != -1) {
                        UniverseUtil::adjustRelationModifier(parentCp, fac, -ownrel - .1);
                    }
                    attack = true;
                    break;
                }
            }
        }
    }
    if (attack) {
        const std::string prison_system = vega_config::config->galaxy.prison_system; /* default: "Sol/Nu_Pheonix" */
        std::string psys = prison_system + "_" + fac;
        if (UnitUtil::getUnitSystemFile(parent) != psys) {
            UnitUtil::JumpTo(parent, psys);
            UniverseUtil::IOmessage(
                    0,
                    "game",
                    "all",
                    parent->name
                            + ", you are under arrest!  You will be taken to the prison system and will be tried for your crimes.");
        } else {
            Unit *un;
            Unit *owner = NULL;
            Unit *base = NULL;
            for (un_iter i = _Universe->activeStarSystem()->getUnitList().createIterator(); (un = *i) != NULL; ++i) {
                if (owner == NULL && un->getFlightgroup() && un->faction == own) {
                    if (UnitUtil::isSignificant(un) && (!un->isJumppoint())) {
                        owner = un;
                    }
                }
                if (UnitUtil::isSignificant(un) && (!un->isJumppoint())) {
                    base = un;
                }
            }
            if (owner == NULL) {
                owner = base;
            }
            if (owner) {
                Order *tmp = parent->aistate;
                parent->aistate = NULL;
                parent->PrimeOrders(new Orders::DockingOps(owner, tmp, true, true));
                arrested_list_do_not_dereference.insert(parent);
                for (int i = parent->numCargo() - 1; i >= 0; --i) {
                    parent->RemoveCargo(i, parent->GetCargo((unsigned int) i).GetQuantity(), true);
                }
                UniverseUtil::IOmessage(
                        0,
                        "game",
                        "all",
                        parent->name
                                +
                                        ", your cargo has been confiscated and scanned. Here your ship will be kept until you complete your reintegration into society through our reprogramming pod(tm) system.");
                int whichCp = _Universe->whichPlayerStarship(parent);
                UniverseUtil::adjustRelationModifierInt(whichCp, own, -UnitUtil::getRelationToFaction(parent, own));
            }
        }
    }
}

static void ForceChangeTarget(Unit *parent) {
    Unit *curtarg = parent->Target();
    ChooseTargets(parent, TargUn, false);
    const bool force_change_only_unit = vega_config::config->graphics.target_null_if_no_unit;
    if (parent->Target() == curtarg) {
        if (force_change_only_unit) {
            parent->Target(NULL);
        } else {
            ChooseTargets(parent, TargNear, false);
            if (parent->Target() == curtarg) {
                ChooseTargets(parent, TargAll, false);
            }
        }
    }
}

int SelectDockPort(Unit *utdw, Unit *parent);

void FireKeyboard::SetParent(Unit *parent1) {
    this->Order::SetParent(parent1);
    const bool allow_special_with_weapons = vega_config::config->physics.special_and_normal_gun_combo;
    if (!allow_special_with_weapons) {
        parent->ToggleWeapon(false, true /*reverse*/ );
        parent->ToggleWeapon(false, false /*reverse*/ );
    }
}

void FireKeyboard::Execute() {
    while (vectorOfKeyboardInput.size() <= whichplayer || vectorOfKeyboardInput.size() <= whichjoystick) {
        vectorOfKeyboardInput.push_back(FIREKEYBOARDTYPE());
    }
    ProcessCommunicationMessages(SIMULATION_ATOM, true);
    Unit *targ = parent->Target();
    DoDockingOps(parent, targ, whichplayer, parent->pilot->getGender());

    if (targ) {
        double mm = 0.0;
        ShouldFire(targ);
        if (targ->Destroyed()) {
            parent->Target(NULL);
            ForceChangeTarget(parent);
            refresh_target = true;
        } else if (false == parent->InRange(targ, mm, true, true, true) && !parent->TargetLocked()) {
            ChooseTargets(parent, TargUn, false);                 //only go for other active units in cone
            if (parent->Target() == NULL) {
                parent->Target(targ);
            }
        }
    } else {
        ForceChangeTarget(parent);
        refresh_target = true;
    }

    float f_result = f().shieldpowerstate;
    if (f_result != 1) {
        parent->shield.AdjustPower(f_result);
    }
    if (f().firekey == PRESS || f().jfirekey == PRESS || j().firekey == DOWN || j().jfirekey == DOWN) {
        if (!_Universe->AccessCockpit()->CanDrawNavSystem()) {
            const bool allow_special_with_weapons = vega_config::config->physics.special_and_normal_gun_combo;
            if (!allow_special_with_weapons) {
                bool special = false;
                bool normal = false;
                int nm = parent->getNumMounts();
                int i;
                for (i = 0; i < nm; ++i) {
                    if (parent->mounts[i].status == Mount::ACTIVE) {
                        special = special || isSpecialGunMount(as_integer(parent->mounts[i].type->size));
                        normal = normal || isNormalGunMount(as_integer(parent->mounts[i].type->size));
                    }
                }
                for (i = 0; i < nm; ++i) {
                    if (special && normal) {
                        if (parent->mounts[i].status == Mount::ACTIVE) {
                            if (isSpecialGunMount(as_integer(parent->mounts[i].type->size))) {
                                parent->mounts[i].status = Mount::INACTIVE;
                            }
                        }
                    }
                }
            }
            parent->Fire(ROLES::EVERYTHING_ELSE | ROLES::FIRE_GUNS, false);
        }
    }
    if (f().missilekey == PRESS || j().jmissilekey == PRESS) {
        if (!_Universe->AccessCockpit()->CanDrawNavSystem()) {
            parent->Fire(ROLES::FIRE_MISSILES | ROLES::EVERYTHING_ELSE, false);
        }
        if (f().missilekey == PRESS) {
            f().missilekey = DOWN;
        }
        if (j().jmissilekey == PRESS) {
            j().jmissilekey = DOWN;
        }
    } else if (f().firekey == RELEASE || j().jfirekey == RELEASE) {
        f().firekey = UP;
        j().jfirekey = UP;
        parent->UnFire();
    }
    if (f().cloakkey == PRESS) {
        f().cloakkey = DOWN;
        parent->cloak.Toggle();
    }
    if (f().lockkey == PRESS) {
        f().lockkey = DOWN;
        parent->radar.ToggleLock();
    }
    if (f().ECMkey == PRESS) {
        f().ECMkey = DOWN;
        parent->ecm.Toggle();;
    }
    if (f().targetkey == PRESS || j().jtargetkey == PRESS) {
        f().targetkey = DOWN;
        j().jtargetkey = DOWN;
        ChooseTargets(parent, TargAll, false);
        refresh_target = true;
    }
    if (f().rtargetkey == PRESS) {
        f().rtargetkey = DOWN;
        ChooseTargets(parent, TargAll, true);
        refresh_target = true;
    }
    if (f().missiontargetkey == PRESS) {
        f().missiontargetkey = DOWN;
        ChooseTargets(parent, TargMission, false);
        refresh_target = true;
    }
    if (f().rmissiontargetkey == PRESS) {
        f().rmissiontargetkey = DOWN;
        ChooseTargets(parent, TargMission, true);
        refresh_target = true;
    }
    if (f().targetskey == PRESS) {
        f().targetskey = DOWN;
        ChooseTargets(parent, TargSig, false);
        refresh_target = true;
        parent->radar.Lock(UnitUtil::isSignificant(parent));
    }
    if (f().targetukey == PRESS) {
        f().targetukey = DOWN;
        const bool smart_targetting = vega_config::config->graphics.smart_targetting_key;
        Unit *tmp = parent->Target();
        bool sysobj = false;
        if (tmp) {
            if (tmp->owner == getTopLevelOwner()) {
                sysobj = true;
            }
        }
        ChooseTargets(parent, TargUn, false);
        if ((parent->Target() == NULL) && tmp == parent->Target() && sysobj && smart_targetting) {
            ChooseTargets(parent, TargSig, false);
            if (tmp == parent->Target()) {
                ChooseTargets(parent, TargAll, false);
            }
        }
        refresh_target = true;
    }
    if (f().picktargetkey == PRESS) {
        f().picktargetkey = DOWN;
        ChooseTargets(parent, TargFront, false);
        refresh_target = true;
    }
    if (f().neartargetkey == PRESS) {
        ChooseTargets(parent, TargNear, false);
        f().neartargetkey = DOWN;
        refresh_target = true;
    }
    if (f().missiletargetkey == PRESS) {
        ChooseTargets(parent, TargMissile, false);
        f().missiletargetkey = DOWN;
        refresh_target = true;
    }
    if (f().incomingmissiletargetkey == PRESS) {
        ChooseTargets(parent, TargIncomingMissile, false);
        f().incomingmissiletargetkey = DOWN;
        refresh_target = true;
    }
    if (f().rmissiletargetkey == PRESS) {
        ChooseTargets(parent, TargMissile, true);
        f().rmissiletargetkey = DOWN;
        refresh_target = true;
    }
    if (f().rincomingmissiletargetkey == PRESS) {
        ChooseTargets(parent, TargIncomingMissile, true);
        f().rincomingmissiletargetkey = DOWN;
        refresh_target = true;
    }
    if (f().threattargetkey == PRESS) {
        ChooseTargets(parent, TargThreat, false);
        f().threattargetkey = DOWN;
        refresh_target = true;
    }
    if (f().subtargetkey == PRESS) {
        ChooseSubTargets(parent);
        f().subtargetkey = DOWN;
        refresh_target = true;
    }
    if (f().rpicktargetkey == PRESS) {
        f().rpicktargetkey = DOWN;
        ChooseTargets(parent, TargFront, true);
        refresh_target = true;
    }
    if (f().rneartargetkey == PRESS) {
        ChooseTargets(parent, TargNear, true);
        f().rneartargetkey = DOWN;
        refresh_target = true;
    }
    if (f().rthreattargetkey == PRESS) {
        ChooseTargets(parent, TargThreat, true);
        f().rthreattargetkey = DOWN;
        refresh_target = true;
    }
    if (f().rtargetskey == PRESS) {
        f().rtargetskey = DOWN;
        ChooseTargets(parent, TargSig, true);
        refresh_target = true;
        parent->radar.Lock(UnitUtil::isSignificant(parent));
    }
    if (f().rtargetukey == PRESS) {
        f().rtargetukey = DOWN;
        const bool smart_targetting = vega_config::config->graphics.smart_targetting_key;
        Unit *tmp = parent->Target();
        bool sysobj = false;
        if (tmp) {
            if (tmp->owner == getTopLevelOwner()) {
                sysobj = true;
            }
        }
        ChooseTargets(parent, TargUn, true);
        if (tmp == parent->Target() && sysobj && smart_targetting) {
            ChooseTargets(parent, TargFront, true);
            if (tmp == parent->Target()) {
                ChooseTargets(parent, TargAll, true);
            }
        }
        refresh_target = true;
    }
    if (f().turretaikey == PRESS) {
        parent->SetTurretAI();
        parent->TargetTurret(parent->Target());
        f().turretaikey = DOWN;
    }
    const bool noturretai = vega_config::config->ai.no_turret_ai;
    static int taicounter = 0;
    if (f().turretoffkey == PRESS || (noturretai && taicounter++ % 128 == 0)) {
        parent->DisableTurretAI();
        f().turretoffkey = DOWN;
    }
    if (f().turretfaw == PRESS) {
        parent->TurretFAW();
        f().turretfaw = DOWN;
    }
    if (f().turretaikey == RELEASE) {
        f().turretaikey = UP;
    }
    if (f().turrettargetkey == PRESS) {
        f().turrettargetkey = DOWN;
        parent->TargetTurret(parent->Target());
        refresh_target = true;
    }
    if (f().pickturrettargetkey == PRESS) {
        f().pickturrettargetkey = DOWN;
        parent->TargetTurret(parent->Target());
        refresh_target = true;
    }
    if (f().nearturrettargetkey == PRESS) {
        parent->TargetTurret(parent->Target());
        f().nearturrettargetkey = DOWN;
        refresh_target = true;
    }
    if (f().threatturrettargetkey == PRESS) {
        parent->TargetTurret(parent->Target());
        f().threatturrettargetkey = DOWN;
        refresh_target = true;
    }
    //Added for nearest unit targeting -ch
    if (f().nearesthostilekey == PRESS) {
        getNearestTargetUnit(parent, 0);
        f().nearesthostilekey = DOWN;
        refresh_target = true;
    }
    if (f().nearestdangeroushostilekey == PRESS) {
        getNearestTargetUnit(parent, 1);
        f().nearestdangeroushostilekey = DOWN;
        refresh_target = true;
    }
    if (f().nearestfriendlykey == PRESS) {
        getNearestTargetUnit(parent, 2);
        f().nearestfriendlykey = DOWN;
        refresh_target = true;
    }
    if (f().nearestbasekey == PRESS) {
        getNearestTargetUnit(parent, 3);
        f().nearestbasekey = DOWN;
        refresh_target = true;
    }
    if (f().nearestplanetkey == PRESS) {
        getNearestTargetUnit(parent, 4);
        f().nearestplanetkey = DOWN;
        refresh_target = true;
    }
    if (f().nearestjumpkey == PRESS) {
        getNearestTargetUnit(parent, 5);
        f().nearestjumpkey = DOWN;
        refresh_target = true;
    }
    if (f().togglepausekey == PRESS) {
        f().togglepausekey = DOWN;
        VS_LOG(info, "Pause key pressed");
        if (toggle_pause()) {
            VS_LOG(info, "Calling _Universe->AccessCockpit()->OnPauseBegin();");
            _Universe->AccessCockpit()->OnPauseBegin();
        } else {
            VS_LOG(info, "Calling _Universe->AccessCockpit()->OnPauseEnd();");
            _Universe->AccessCockpit()->OnPauseEnd();
        }
    }
    if (f().weapk == PRESS || f().rweapk == PRESS) {
        bool forward;
        if (f().weapk == PRESS) {
            f().weapk = DOWN;
            forward = true;
        }
        if (f().rweapk == PRESS) {
            f().rweapk = DOWN;
            forward = false;
        }
        parent->UnFire();
        parent->ToggleWeapon(false, forward);
        static soundContainer weapsound;
        if (weapsound.sound < 0) {
            const std::string str = vega_config::config->cockpit_audio.weapon_switch; /* default: "vdu_d" */);
            weapsound.loadsound(str);
        }
        weapsound.playsound();
    }
    if (f().toggleanimation == PRESS) {
        f().toggleanimation = DOWN;
        parent->graphicOptions.Animating = parent->graphicOptions.Animating ? 0 : 1;
    }
    if (f().toggleglow == PRESS) {
        f().toggleglow = DOWN;
        static bool isvis = true;
        isvis = !isvis;
        parent->SetGlowVisible(isvis);
    }
    if (f().togglewarpdrive == PRESS) {
        f().togglewarpdrive = DOWN;
        parent->ftl_drive.Toggle();
        parent->graphicOptions.WarpRamping = 1;
    }
    if (f().toggleautotracking == PRESS) {
        f().toggleautotracking = DOWN;
        parent->radar.ToggleTracking();
    }
    if (f().misk == PRESS || f().rmisk == PRESS) {
        bool forward;
        if (f().misk == PRESS) {
            f().misk = DOWN;
            forward = true;
        }
        if (f().rmisk == PRESS) {
            f().rmisk = DOWN;
            forward = false;
        }
        parent->ToggleWeapon(true, forward);
        static soundContainer missound;
        if (missound.sound < 0) {
            const std::string str = vega_config::config->cockpit_audio.missile_switch; /* default: "vdu_d" */);
            missound.loadsound(str);
        }
        missound.playsound();
    }
    unsigned int i;
    for (i = 0; i < NUMSAVEDTARGETS; i++) {
        if (f().saveTargetKeys[i] == PRESS) {
            f().saveTargetKeys[i] = RELEASE;
            savedTargets[i] = parent->Target();
        }
        if (f().restoreTargetKeys[i] == PRESS && parent->radar.CanLock()) {
            f().restoreTargetKeys[i] = RELEASE;
            Unit *un;
            for (un_iter u = _Universe->activeStarSystem()->getUnitList().createIterator();
                    (un = *u) != NULL;
                    ++u) {
                if (un == savedTargets[i]) {
                    parent->Target(un);
                    break;
                }
            }
        }
    }
    for (i = 0; i < NUMCOMMKEYS; i++) {
        if (f().commKeys[i] == PRESS) {
            f().commKeys[i] = RELEASE;
            Unit *targ = parent->Target();
            if (targ) {
                CommunicationMessage *mymsg = GetTargetMessageQueue(targ, resp);
                FSM *fsm = FactionUtil::GetConversation(parent->faction, targ->faction);
                if (mymsg == NULL || mymsg->curstate >= static_cast<int>(fsm->nodes.size())) {
                    CommunicationMessage c(parent, targ, i, NULL, parent->pilot->getGender());
                    unsigned int whichspeech = DoSpeech(targ, targ, *c.getCurrentState());
                    float gain;
                    int sound = c.getCurrentState()->GetSound(c.sex, whichspeech, gain);
                    if (!AUDIsPlaying(sound)) {
                        AUDPlay(sound, QVector(0, 0, 0), Vector(0, 0, 0), gain);
                    }
                    Order *o = targ->getAIState();
                    if (o) {
                        o->Communicate(c);
                    }
                } else {
                    FSM *tmp = mymsg->fsm;
                    mymsg->fsm = fsm;
                    FSM::Node *n = mymsg->getCurrentState();
                    if (i < n->edges.size()) {
                        CommunicationMessage c(parent, targ, *mymsg, i, NULL, parent->pilot->getGender());
                        unsigned int whichmessage = DoSpeech(targ, targ, *c.getCurrentState());
                        float gain;
                        int sound = c.getCurrentState()->GetSound(c.sex, whichmessage, gain);
                        if (!AUDIsPlaying(sound)) {
                            AUDPlay(sound, QVector(0, 0, 0), Vector(0, 0, 0), gain);
                        }
                        Order *oo = targ->getAIState();
                        if (oo) {
                            oo->Communicate(c);
                        }
                    }
                    mymsg->fsm = tmp;
                }
            }
        }
    }
    if (refresh_target) {
        Unit *targ;
        if ((targ = parent->Target())) {
            if (parent->isSubUnit()) {
                parent->TargetTurret(targ);
            }
            CommunicationMessage *mymsg = GetTargetMessageQueue(targ, resp);
            FSM *fsm = FactionUtil::GetConversation(parent->faction, targ->faction);
            if (mymsg == NULL) {
                _Universe->AccessCockpit()->communication_choices =
                        fsm->GetEdgesString(fsm->getDefaultState(parent->getRelation(targ)));
            } else {
                _Universe->AccessCockpit()->communication_choices = fsm->GetEdgesString(mymsg->curstate);
            }
        } else {
            _Universe->AccessCockpit()->communication_choices = "\nNo Communication\nLink\nEstablished";
        }
    }
    if (f().enslave == PRESS || f().freeslave == PRESS) {
        Enslave(parent, f().enslave == PRESS);
        f().enslave = RELEASE;
        f().freeslave = RELEASE;
    }
    if (f().ejectcargo == PRESS || f().ejectnonmissioncargo == PRESS) {
        bool missiontoo = (f().ejectcargo == PRESS);
        f().ejectnonmissioncargo = RELEASE;
        f().ejectcargo = RELEASE;
        int offset = _Universe->AccessCockpit()->getScrollOffset(VDU::MANIFEST);
        if (offset < 3) {
            offset = 0;
        } else {
            offset -= 3;
        }
        for (; offset < static_cast<int>(parent->numCargo()); ++offset) {
            Cargo *tmp = &parent->GetCargo(offset);
            if (tmp->GetCategory().find("upgrades") == string::npos && (missiontoo || tmp->GetMissionFlag() == false)) {
                parent->EjectCargo(offset);
                break;
            }
        }
        if (missiontoo) {
            f().ejectnonmissioncargo = DOWN;
        } else {
            f().ejectcargo = DOWN;
        }
    }
    //i think this ejects the pilot? yep it does
    if (f().eject == PRESS) {
        f().eject = DOWN;
        Cockpit *cp = NULL;
        if ((parent->name != "eject") && (parent->name != "Pilot") && (cp = _Universe->isPlayerStarship(parent))) {
            cp->Eject();
        }
    }
    //eject pilot and warp pilot to the docking screen instantly.
    if (f().ejectdock == PRESS) {
        f().ejectdock = DOWN;
        Unit *utdw = parent;
        Cockpit *cp =
                NULL;          //check if docking ports exist, no docking ports = no need to ejectdock so don't do anything
        if ((SelectDockPort(utdw, parent) > -1) && (cp = _Universe->isPlayerStarship(parent))) {
            cp->EjectDock();
        }              //use specialized ejectdock in the future
    }
    const bool actually_arrest = vega_config::config->ai.arrest_energy_zero;
    if (actually_arrest && parent->reactor.Capacity() == 0) {
        Arrested(parent);
    }
}

