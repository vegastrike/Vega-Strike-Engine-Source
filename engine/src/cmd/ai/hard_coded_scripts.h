/**
* hard_coded_scripts.h
*
* Copyright (C) 2001-2002 Daniel Horn
* Copyright (C) 2002-2019 pyramid3d and other Vega Strike Contributors
* Copyright (C) 2019-2022 Stephen G. Tuggy and other Vega Strike Contributors
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

#ifndef HARD_CODED_SCRIPTS_H_
#define HARD_CODED_SCRIPTS_H_

typedef void CCScript (Order*script, Unit*un);

CCScript AfterburnerSlide;
CCScript FlyStraight;
CCScript FlyStraightAfterburner;
CCScript Takeoff;
CCScript DoNothing;
CCScript TakeoffEveryZig;
CCScript AfterburnTurnTowards;
CCScript AfterburnTurnTowardsITTS;
CCScript CloakForScript;
CCScript MoveTo;
CCScript Kickstop;
CCScript MatchVelocity;
CCScript VeerAway;
CCScript VeerAwayITTS;
CCScript CoastToStop;
CCScript SelfDestruct;
CCScript VeerAndVectorAway;
CCScript AfterburnVeerAndVectorAway;
CCScript AfterburnVeerAndTurnAway;
CCScript SheltonSlide;
CCScript SkilledABSlide;
CCScript Stop;
CCScript TurnAway;
CCScript AfterburnTurnAway;
CCScript TurnTowardsITTS;
CCScript TurnTowards;
CCScript BarrelRoll;

CCScript LoopAround;
CCScript AggressiveLoopAround;
CCScript LoopAroundFast;
CCScript AggressiveLoopAroundFast;
CCScript LoopAroundSlow;
CCScript AggressiveLoopAroundSlow;
CCScript DropCargo;
CCScript DropOneCargo;
CCScript DropHalfCargo;
CCScript RollLeft;
CCScript RollRight;
CCScript RollLeftHard;
CCScript RollRightHard;
CCScript EvadeLeftRight;
CCScript EvadeUpDown;
CCScript AfterburnEvadeLeftRight;
CCScript AfterburnEvadeUpDown;
CCScript FacePerpendicularFast;
CCScript FacePerpendicular;
CCScript FacePerpendicularSlow;
CCScript RollFacePerpendicularFast;
CCScript RollFacePerpendicular;
CCScript RollFacePerpendicularSlow;
#endif

