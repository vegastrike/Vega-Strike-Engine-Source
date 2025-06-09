/*
 * comm_ai.h
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
#ifndef VEGA_STRIKE_ENGINE_CMD_AI_COMM_H
#define VEGA_STRIKE_ENGINE_CMD_AI_COMM_H

#include "order.h"

#include <vector>
/// this class is meant to be inherited by AI wishing to process comm messages

class CommunicatingAI : public Order {
protected:
    float anger; //how easily he will target you if you taunt him enough (-.00001 is always -.4 is often -.6 is normal
    float appease; //how easily he will switch from you if you silence his will to kill (-.5)
    float moodswingyness;
    float randomresponse;
    float mood;
    UnitContainer contraband_searchee;
    Vector SpeedAndCourse;
    int which_cargo_item{};
    void GetMadAt(Unit *which, int howMad);
protected:
public:
    virtual void Destroy();

    virtual float getMood() {
        return mood;
    }

    Unit *GetRandomUnit(float PlayerProbability, float TargetProbability);
    void RandomInitiateCommunication(float PlayerProbability, float TargetProbability);
    void TerminateContrabandSearch(bool foundcontraband);
    void InitiateContrabandSearch(float PlayerProbability, float TargetProbability);
    void UpdateContrabandSearch();
    CommunicatingAI(int ttype,
            int stype,
            float mood = 0,
            float anger = -666 /*-.5*/,
            float appeasement = 666,
            float moodswingyness = 666 /*.2*/,
            float randomnessresponse = 666 /*.8*/ );
    virtual void ProcessCommMessage(class CommunicationMessage &c);
    virtual void AdjustRelationTo(Unit *un, float factor);
    virtual ~CommunicatingAI();
    virtual int selectCommunicationMessage(class CommunicationMessage &c, Unit *);
    virtual int selectCommunicationMessageMood(CommunicationMessage &c, float mood);
};

void AllUnitsCloseAndEngage(Unit *un, int faction);

#endif //VEGA_STRIKE_ENGINE_CMD_AI_COMM_H
