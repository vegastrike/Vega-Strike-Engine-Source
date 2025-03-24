/**
 * firekeyboard.h
 *
 * Copyright (C) Daniel Horn
 * Copyright (C) 2020 pyramid3d, Stephen G. Tuggy, and other Vega Strike
 * contributors
 * Copyright (C) 2022-2023 Stephen G. Tuggy, Benjamen R. Meyer
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
#ifndef VEGA_STRIKE_ENGINE_CMD_AI_FIRE_KEYBOARD_H
#define VEGA_STRIKE_ENGINE_CMD_AI_FIRE_KEYBOARD_H

#include "src/in.h"
#include "order.h"
#include "event_xml.h"
#include "communication.h"
//all unified AI's should inherit from FireAt, so they can choose targets together.
#define NUMSAVEDTARGETS 10
class FireKeyboard : public Order {
    bool itts;
    bool refresh_target;
    float gunspeed;
    float gunrange;
    float rxntime;
    float delay;
    float autotrackingtoggle;
    bool ShouldFire(Unit *targ);
    std::list<CommunicationMessage> resp;
public:
    virtual void SetParent(Unit *parent1);
    static void SetShieldsOff(const KBData &, KBSTATE);
    static void SetShieldsOneThird(const KBData &, KBSTATE);
    static void SetShieldsTwoThird(const KBData &, KBSTATE);
    static void ToggleGlow(const KBData &, KBSTATE);
    static void ToggleWarpDrive(const KBData &, KBSTATE);
    static void ToggleAutotracking(const KBData &, KBSTATE);
    static void ToggleAnimation(const KBData &, KBSTATE);
    static void PressComm1Key(const KBData &, KBSTATE);
    static void PressComm2Key(const KBData &, KBSTATE);
    static void PressComm3Key(const KBData &, KBSTATE);
    static void PressComm4Key(const KBData &, KBSTATE);
    static void PressComm5Key(const KBData &, KBSTATE);
    static void PressComm6Key(const KBData &, KBSTATE);
    static void PressComm7Key(const KBData &, KBSTATE);
    static void PressComm8Key(const KBData &, KBSTATE);
    static void PressComm9Key(const KBData &, KBSTATE);
    static void PressComm10Key(const KBData &, KBSTATE);
    static void SaveTarget1Key(const KBData &, KBSTATE);
    static void SaveTarget2Key(const KBData &, KBSTATE);
    static void SaveTarget3Key(const KBData &, KBSTATE);
    static void SaveTarget4Key(const KBData &, KBSTATE);
    static void SaveTarget5Key(const KBData &, KBSTATE);
    static void SaveTarget6Key(const KBData &, KBSTATE);
    static void SaveTarget7Key(const KBData &, KBSTATE);
    static void SaveTarget8Key(const KBData &, KBSTATE);
    static void SaveTarget9Key(const KBData &, KBSTATE);
    static void SaveTarget10Key(const KBData &, KBSTATE);
    static void RestoreTarget1Key(const KBData &, KBSTATE);
    static void RestoreTarget2Key(const KBData &, KBSTATE);
    static void RestoreTarget3Key(const KBData &, KBSTATE);
    static void RestoreTarget4Key(const KBData &, KBSTATE);
    static void RestoreTarget5Key(const KBData &, KBSTATE);
    static void RestoreTarget6Key(const KBData &, KBSTATE);
    static void RestoreTarget7Key(const KBData &, KBSTATE);
    static void RestoreTarget8Key(const KBData &, KBSTATE);
    static void RestoreTarget9Key(const KBData &, KBSTATE);
    static void RestoreTarget10Key(const KBData &, KBSTATE);
    static void RequestClearenceKey(const KBData &, KBSTATE);
    static void UnDockKey(const KBData &, KBSTATE);
    static void EjectKey(const KBData &, KBSTATE);
    static void EjectDockKey(const KBData &, KBSTATE);
    static void EjectCargoKey(const KBData &, KBSTATE);
    static void EjectNonMissionCargoKey(const KBData &, KBSTATE);
    static void DockKey(const KBData &, KBSTATE);
    static void FireKey(const KBData &, KBSTATE);
    static void MissileKey(const KBData &, KBSTATE);
    static void TargetKey(const KBData &, KBSTATE k);
    static void MissionTargetKey(const KBData &, KBSTATE k);
    static void ReverseMissionTargetKey(const KBData &, KBSTATE k);
    static void MissileTargetKey(const KBData &, KBSTATE k);
    static void IncomingMissileTargetKey(const KBData &, KBSTATE k);
    static void ReverseMissileTargetKey(const KBData &, KBSTATE k);
    static void ReverseIncomingMissileTargetKey(const KBData &, KBSTATE k);
    static void PickTargetKey(const KBData &, KBSTATE k);
    static void NearestTargetKey(const KBData &, KBSTATE k);
    static void SubUnitTargetKey(const KBData &, KBSTATE k);
    static void ThreatTargetKey(const KBData &, KBSTATE k);
    static void UnitTargetKey(const KBData &, KBSTATE k);
    static void SigTargetKey(const KBData &, KBSTATE k);
    static void ReverseTargetKey(const KBData &, KBSTATE k);
    static void ReversePickTargetKey(const KBData &, KBSTATE k);
    static void ReverseNearestTargetKey(const KBData &, KBSTATE k);
    static void ReverseThreatTargetKey(const KBData &, KBSTATE k);
    static void ReverseUnitTargetKey(const KBData &, KBSTATE k);
    static void ReverseSigTargetKey(const KBData &, KBSTATE k);
    static void NearestTargetTurretKey(const KBData &, KBSTATE);
    static void ThreatTargetTurretKey(const KBData &, KBSTATE);
    static void TargetTurretKey(const KBData &, KBSTATE);
    static void PickTargetTurretKey(const KBData &, KBSTATE);
    static void JFireKey(KBSTATE, float, float, int);
    static void JMissileKey(KBSTATE, float, float, int);
    static void JTargetKey(KBSTATE, float, float, int);
    static void WeapSelKey(const KBData &, KBSTATE);
    static void MisSelKey(const KBData &, KBSTATE);
    static void ReverseWeapSelKey(const KBData &, KBSTATE);
    static void ReverseMisSelKey(const KBData &, KBSTATE);
    static void CloakKey(const KBData &, KBSTATE);
    static void LockKey(const KBData &, KBSTATE);
    static void ECMKey(const KBData &, KBSTATE);
    static void HelpMeOut(const KBData &, KBSTATE);
    static void HelpMeOutFaction(const KBData &, KBSTATE);
    static void EnslaveKey(const KBData &, KBSTATE);
    static void FreeSlaveKey(const KBData &, KBSTATE);
    static void HelpMeOutCrit(const KBData &, KBSTATE);
    static void JoinFg(const KBData &, KBSTATE);
    static void BreakFormation(const KBData &, KBSTATE);
    static void FormUp(const KBData &, KBSTATE);
    static void DockWithMe(const KBData &, KBSTATE);
    static void DefendTarget(const KBData &, KBSTATE);
    //IAmDave - new flightgroup commands...
    static void DockTarget(const KBData &, KBSTATE k);
    static void HoldPosition(const KBData &, KBSTATE k);
    //IAmDave - ...new flightgroup commands end.
    static void AttackTarget(const KBData &, KBSTATE);
    static void TurretAIOn(const KBData &, KBSTATE);
    static void TurretAIOff(const KBData &, KBSTATE);
    static void TurretFireAtWill(const KBData &, KBSTATE);
//Added for targeting nearest units. --ch
    static void NearestHostileTargetKey(const KBData &, KBSTATE k);
    static void NearestDangerousHostileKey(const KBData &, KBSTATE k);
    static void NearestFriendlyKey(const KBData &, KBSTATE k);
    static void NearestBaseKey(const KBData &, KBSTATE k);
    static void NearestPlanetKey(const KBData &, KBSTATE k);
    static void NearestJumpKey(const KBData &, KBSTATE k);
    static void TogglePause(const KBData &, KBSTATE);

protected:
    void *savedTargets[NUMSAVEDTARGETS];
    float distance;
#if 0
    void ChooseTargets( bool targetturrets ); //chooses n targets and puts the best to attack in unit's target container
    void ChooseRTargets( bool targetturrets ); //chooses n targets and puts the best to attack in unit's target container
    void ChooseNearTargets( bool targetturrets ); //chooses n targets and puts the best to attack in unit's target container
    void ChooseThreatTargets( bool targetturrets ); //chooses n targets and puts the best to attack in unit's target container
    void PickTargets( bool targetturrets ); //chooses the target which is nearest to the center of the screen
#endif
    unsigned int whichplayer;
    unsigned int whichjoystick;
    struct FIREKEYBOARDTYPE &f();
    struct FIREKEYBOARDTYPE &j();
    unsigned int DoSpeechAndAni(Unit *un, Unit *parent, class CommunicationMessage &c);
public:
    virtual void ProcessCommMessage(class CommunicationMessage &c);
    FireKeyboard(unsigned int whichjoystick, unsigned int whichplayer); //weapon prefs?
    virtual void Execute();
    virtual ~FireKeyboard();
};

#endif //VEGA_STRIKE_ENGINE_CMD_AI_FIRE_KEYBOARD_H
