/**
* comm_ai.h
*
* Copyright (c) 2001-2002 Daniel Horn
* Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
* Copyright (c) 2019-2021 Stephen G. Tuggy, and other Vega Strike Contributors
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

#ifndef __COMM_AI_H
#define __COMM_AI_H

#include "order.h"

#include <vector>
/// this class is meant to be inherited by AI wishing to process comm messages

class CommunicatingAI : public Order
{
protected:
    float  anger; //how easily he will target you if you taunt him enough (-.00001 is always -.4 is often -.6 is normal
    float  appease; //how easily he will switch from you if you silence his will to kill (-.5)
    float  moodswingyness;
    float  randomresponse;
    float  mood;
    UnitContainer contraband_searchee;
    Vector SpeedAndCourse;
    int    which_cargo_item;
    void GetMadAt( Unit *which, int howMad );
protected:
public:
    virtual void Destroy();
    virtual float getMood()
    {
        return mood;
    }
    Unit * GetRandomUnit( float PlayerProbability, float TargetProbability );
    void RandomInitiateCommunication( float PlayerProbability, float TargetProbability );
    void TerminateContrabandSearch( bool foundcontraband );
    void InitiateContrabandSearch( float PlayerProbability, float TargetProbability );
    void UpdateContrabandSearch();
    CommunicatingAI( int ttype,
                     int stype,
                     float mood = 0,
                     float anger = -666 /*-.5*/,
                     float appeasement = 666,
                     float moodswingyness = 666 /*.2*/,
                     float randomnessresponse = 666 /*.8*/ );
    virtual void ProcessCommMessage( class CommunicationMessage&c );
    virtual void AdjustRelationTo( Unit *un, float factor );
    virtual ~CommunicatingAI();
    virtual int  selectCommunicationMessage( class CommunicationMessage&c, Unit* );
    virtual int selectCommunicationMessageMood( CommunicationMessage &c, float mood );
};

#endif

