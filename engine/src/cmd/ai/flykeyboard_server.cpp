/**
 * flykeyboard_server.cpp
 *
 * Copyright (c) 2001-2002 Daniel Horn
 * Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
 * Copyright (c) 2019-2021 Stephen G. Tuggy, and other Vega Strike Contributors
 * Copyright (C) 2022 Stephen G. Tuggy
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


#include "flykeyboard.h"
#include "autodocking.h"

FlyByKeyboard::FlyByKeyboard(unsigned int whichplayer) : FlyByWire(), axis_key(0, 0, 0) {
    this->last_jumped = 0;
    this->whichplayer = whichplayer;
    autopilot = NULL;
    inauto = false;
    joy_mode = 0;
}

void FlyByKeyboard::Execute(bool resetangvelocity) {
    if (autopilot) {
        autopilot->Execute();
    }
    if (queryType(FACING | MOVEMENT)) {
        Order::Execute();
        if (queryType(FACING | MOVEMENT) == NULL) {
            FlyByKeyboard::inauto = false;
        }
        done = false;
    } else {
        FlyByWire::Execute();
    }
}

void FlyByKeyboard::DownFreq(const KBData &, KBSTATE k) {
}

void FlyByKeyboard::UpFreq(const KBData &, KBSTATE k) {
}

void FlyByKeyboard::ChangeCommStatus(const KBData &, KBSTATE k) {
}

void FlyByKeyboard::SwitchWebcam(const KBData &, KBSTATE k) {
}

void FlyByKeyboard::SwitchSecured(const KBData &, KBSTATE k) {
}

void FlyByKeyboard::SetVelocityRefKey(const KBData &, KBSTATE k) {
}

void FlyByKeyboard::SetNullVelocityRefKey(const KBData &, KBSTATE k) {
}

void FlyByKeyboard::SheltonKey(const KBData &, KBSTATE k) {
}

void FlyByKeyboard::InertialToggleKey(const KBData &, KBSTATE k) {
}

void FlyByKeyboard::InertialPulsorKey(const KBData &, KBSTATE k) {
}

void FlyByKeyboard::JoyInertialXYToggleKey(const KBData &, KBSTATE k) {
}

void FlyByKeyboard::JoyInertialXZToggleKey(const KBData &, KBSTATE k) {
}

void FlyByKeyboard::JoyRollToggleKey(const KBData &, KBSTATE k) {
}

void FlyByKeyboard::JoyBankToggleKey(const KBData &, KBSTATE k) {
}

void FlyByKeyboard::JoyInertialXYPulsorKey(const KBData &, KBSTATE k) {
}

void FlyByKeyboard::JoyInertialXZPulsorKey(const KBData &, KBSTATE k) {
}

void FlyByKeyboard::JoyRollPulsorKey(const KBData &, KBSTATE k) {
}

void FlyByKeyboard::JoyBankPulsorKey(const KBData &, KBSTATE k) {
}

void FlyByKeyboard::JumpKey(const KBData &, KBSTATE k) {
}

void FlyByKeyboard::UpKey(const KBData &, KBSTATE k) {
}

void FlyByKeyboard::KThrustRight(const KBData &, KBSTATE k) {
}

void FlyByKeyboard::KThrustLeft(const KBData &, KBSTATE k) {
}

void FlyByKeyboard::KThrustUp(const KBData &, KBSTATE k) {
}

void FlyByKeyboard::KThrustDown(const KBData &, KBSTATE k) {
}

void FlyByKeyboard::KThrustFront(const KBData &, KBSTATE k) {
}

void FlyByKeyboard::KThrustBack(const KBData &, KBSTATE k) {
}

void FlyByKeyboard::DownKey(const KBData &, KBSTATE k) {
}

void FlyByKeyboard::LeftKey(const KBData &, KBSTATE k) {
}

void FlyByKeyboard::KSwitchFlightMode(const KBData &, KBSTATE k) {
}

void FlyByKeyboard::RightKey(const KBData &, KBSTATE k) {
}

void FlyByKeyboard::ABKey(const KBData &, KBSTATE k) {
}

void FlyByKeyboard::AutoKey(const KBData &, KBSTATE k) {
}

void FlyByKeyboard::EngageSpecAuto(const KBData &, KBSTATE k) {
}

void FlyByKeyboard::SwitchCombatModeKey(const KBData &, KBSTATE k) {
}

void FlyByKeyboard::StopAutoKey(const KBData &, KBSTATE k) {
}

void FlyByKeyboard::StopKey(const KBData &, KBSTATE k) {
}

void FlyByKeyboard::AccelKey(const KBData &, KBSTATE k) {
}

void FlyByKeyboard::DecelKey(const KBData &, KBSTATE k) {
}

void FlyByKeyboard::StartKey(const KBData &, KBSTATE k) {
}

void FlyByKeyboard::RollRightKey(const KBData &, KBSTATE k) {
}

void FlyByKeyboard::MatchSpeedKey(const KBData &, KBSTATE k) {
}

void FlyByKeyboard::RollLeftKey(const KBData &, KBSTATE k) {
}

