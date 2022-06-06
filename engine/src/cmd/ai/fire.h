/**
 * fire.h
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


#ifndef _CMD_TARGET_AI_H_
#define _CMD_TARGET_AI_H_
#include "comm_ai.h"
#include "event_xml.h"
//all unified AI's should inherit from FireAt, so they can choose targets together.
bool RequestClearence(class Unit *parent, class Unit *targ, unsigned char sex);
Unit *getAtmospheric(Unit *targ);
namespace Orders {
class FireAt : public CommunicatingAI {
protected:
    bool ShouldFire(Unit *targ, bool &missilelock);
    float missileprobability{};
    float lastmissiletime{};
    float delay{};
    float agg{};
    float distance{};
    float lastchangedtarg{};
    bool had_target{};
    void FireWeapons(bool shouldfire, bool lockmissile);
    virtual void ChooseTargets(int num,
            bool force = false); //chooses n targets and puts the best to attack in unit's target container
    bool isJumpablePlanet(Unit *);
    void ReInit(float agglevel);
    virtual void SignalChosenTarget();
public:
//Other new Order functions that can be called from Python.
    virtual void ChooseTarget() {
        ChooseTargets(1, true);
    }

    void PossiblySwitchTarget(bool istargetjumpableplanet);
    virtual bool PursueTarget(Unit *, bool leader);
    void AddReplaceLastOrder(bool replace);
    void ExecuteLastScriptFor(float time);
    void FaceTarget(bool end);
    void FaceTargetITTS(bool end);
    void MatchLinearVelocity(bool terminate, Vector vec, bool afterburn, bool local);
    void MatchAngularVelocity(bool terminate, Vector vec, bool local);
    void ChangeHeading(QVector vec);
    void ChangeLocalDirection(Vector vec);
    void MoveTo(QVector, bool afterburn);
    void MatchVelocity(bool terminate, Vector vec, Vector angvel, bool afterburn, bool local);
    void Cloak(bool enable, float seconds);
    void FormUp(QVector pos);
    void FormUpToOwner(QVector pos);
    void FaceDirection(float distToMatchFacing, bool finish);
    void XMLScript(std::string script);
    void LastPythonScript();

    virtual void SetParent(Unit *parent) {
        CommunicatingAI::SetParent(parent);
    }

    Unit *GetParent() {
        return CommunicatingAI::GetParent();
    }

    FireAt(float aggressivitylevel); //weapon prefs?
    FireAt();
    virtual void Execute();

    virtual std::string Pickle() {
        return std::string();
    }                                                 //these are to serialize this AI
    virtual void UnPickle(std::string) {
    }

    virtual ~FireAt();
};
}
#endif

