/**
 * aggressive.h
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


#ifndef __AGGRESSIVE_AI_H
#define __AGGRESSIVE_AI_H
#include "fire.h"

class Flightgroup;
namespace Orders {
class AggressiveAI : public FireAt {
    bool obedient; //am I currently obedient
    bool last_time_insys;
    char jump_time_check;
    float last_jump_distance;
    float last_jump_time;
    float currentpriority;
    float creationtime;
protected:
    void SignalChosenTarget();
    AIEvents::ElemAttrMap *logic;
    float logiccurtime;
    float interruptcurtime;
    QVector nav;
    UnitContainer navDestination;
    float lurk_on_arrival;
    bool ProcessLogicItem(const AIEvents::AIEvresult &item);
    bool ExecuteLogicItem(const AIEvents::AIEvresult &item);
    bool ProcessLogic(AIEvents::ElemAttrMap &logic, bool inter); //returns if found anything
    std::string last_directive;
    void ReCommandWing(Flightgroup *fg);
    bool ProcessCurrentFgDirective(Flightgroup *fg);
public:
    virtual void SetParent(Unit *parent1);
    enum types {
        AGGAI, MOVEMENT, FACING, UNKNOWN, DISTANCE, METERDISTANCE, THREAT, FSHIELD, LSHIELD, RSHIELD, BSHIELD, FARMOR,
        BARMOR, LARMOR, RARMOR, HULL, RANDOMIZ, FSHIELD_HEAL_RATE, BSHIELD_HEAL_RATE, LSHIELD_HEAL_RATE,
        RSHIELD_HEAL_RATE,
        FARMOR_HEAL_RATE, BARMOR_HEAL_RATE, LARMOR_HEAL_RATE, RARMOR_HEAL_RATE, HULL_HEAL_RATE, TARGET_FACES_YOU,
        TARGET_IN_FRONT_OF_YOU, TARGET_GOING_YOUR_DIRECTION
    };
    AggressiveAI(const char *file, Unit *target = NULL);
    void ExecuteNoEnemies();
    void Execute();

    virtual std::string getOrderDescription()
    {
        return "aggressive";
    }

    void AfterburnerJumpTurnTowards(Unit *target);
    float Fshield_prev;
    float Fshield_rate_old;
    double Fshield_prev_time;
    float Bshield_prev;
    float Bshield_rate_old;
    double Bshield_prev_time;
    float Lshield_prev;
    float Lshield_rate_old;
    double Lshield_prev_time;
    float Rshield_prev;
    float Rshield_rate_old;
    double Rshield_prev_time;
    float Farmour_prev;
    float Farmour_rate_old;
    double Farmour_prev_time;
    float Barmour_prev;
    float Barmour_rate_old;
    double Barmour_prev_time;
    float Larmour_prev;
    float Larmour_rate_old;
    double Larmour_prev_time;
    float Rarmour_prev;
    float Rarmour_rate_old;
    double Rarmour_prev_time;
    float Hull_prev;
    float Hull_rate_old;
    double Hull_prev_time;
    int personalityseed;
};
}

#endif

