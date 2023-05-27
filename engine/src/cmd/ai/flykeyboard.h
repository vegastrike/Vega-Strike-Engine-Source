/**
 * flykeyboard.h
 *
 * Copyright (C) Daniel Horn
 * Copyright (C) 2020 pyramid3d, Stephen G. Tuggy, and other Vega Strike
 * contributors
 * Copyright (C) 2022 Stephen G. Tuggy
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
#ifndef VEGA_STRIKE_ENGINE_CMD_AI_FLY_KEYBOARD_H
#define VEGA_STRIKE_ENGINE_CMD_AI_FLY_KEYBOARD_H

#include "in.h"
#if defined (__APPLE__) || defined (MACOSX)
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include "flybywire.h"

class FlyByKeyboard : public FlyByWire {
    Order *autopilot;
    Vector axis_key;
    unsigned int last_jumped;
protected:
    enum joy_mode_enum {
        joyModeNormal = 0,
        joyModeInertialXY = 1,
        joyModeInertialXZ = 2,
        joyModeRoll = 4,
        joyModeBank = 8
    };
    char joy_mode; //do not mix different modes
protected:
    void KeyboardUp(float val);
    void KeyboardRight(float val);
    void KeyboardRollRight(float val);
    float clamp_axis(float v);
    float reduce_axis(float v);
public:
    static void SwitchWebcam(const KBData &, KBSTATE);
    static void SwitchSecured(const KBData &, KBSTATE);
    static void ChangeCommStatus(const KBData &, KBSTATE);
    static void DownFreq(const KBData &, KBSTATE);
    static void UpFreq(const KBData &, KBSTATE);
    static void KSwitchFlightMode(const KBData &, KBSTATE);
    static void KThrustRight(const KBData &, KBSTATE);
    static void KThrustLeft(const KBData &, KBSTATE);
    static void KThrustUp(const KBData &, KBSTATE);
    static void KThrustDown(const KBData &, KBSTATE);
    static void KThrustFront(const KBData &, KBSTATE);
    static void KThrustBack(const KBData &, KBSTATE);
    static void SheltonKey(const KBData &, KBSTATE);
    static void InertialToggleKey(const KBData &, KBSTATE);
    static void InertialPulsorKey(const KBData &, KBSTATE);
    static void JoyInertialXYPulsorKey(const KBData &, KBSTATE);
    static void JoyInertialXZPulsorKey(const KBData &, KBSTATE);
    static void JoyInertialXYToggleKey(const KBData &, KBSTATE);
    static void JoyInertialXZToggleKey(const KBData &, KBSTATE);
    static void JoyRollPulsorKey(const KBData &, KBSTATE);
    static void JoyRollToggleKey(const KBData &, KBSTATE);
    static void JoyBankPulsorKey(const KBData &, KBSTATE);
    static void JoyBankToggleKey(const KBData &, KBSTATE);
    static void StartKey(const KBData &, KBSTATE);
    static void StopKey(const KBData &, KBSTATE);
    static void UpKey(const KBData &, KBSTATE);
    static void DownKey(const KBData &, KBSTATE);
    static void LeftKey(const KBData &, KBSTATE);
    static void RightKey(const KBData &, KBSTATE);
    static void ABKey(const KBData &, KBSTATE);
    static void AccelKey(const KBData &, KBSTATE);
    static void DecelKey(const KBData &, KBSTATE);
    static void RollLeftKey(const KBData &, KBSTATE);
    static void RollRightKey(const KBData &, KBSTATE);
    static void MatchSpeedKey(const KBData &, KBSTATE);
    static void JumpKey(const KBData &, KBSTATE);
    static void AutoKey(const KBData &, KBSTATE);
    static void SwitchCombatModeKey(const KBData &, KBSTATE);
    static void EngageSpecAuto(const KBData &, KBSTATE);
    static void StopAutoKey(const KBData &, KBSTATE);
    static void SetVelocityRefKey(const KBData &, KBSTATE);
    static void SetNullVelocityRefKey(const KBData &, KBSTATE);
    int whichplayer;
    bool inauto;
    FlyByKeyboard(unsigned int whichplayer);
    virtual void Destroy();
    virtual ~FlyByKeyboard();
    void Execute(bool); //should we reset our ang velocity
    void Execute();
};

#endif //VEGA_STRIKE_ENGINE_CMD_AI_FLY_KEYBOARD_H
