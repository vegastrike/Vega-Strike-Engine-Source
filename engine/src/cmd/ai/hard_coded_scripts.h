/*
 * hard_coded_scripts.h
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Creator: Daniel Horn
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
#ifndef VEGA_STRIKE_ENGINE_CMD_AI_HARD_CODED_SCRIPTS_H
#define VEGA_STRIKE_ENGINE_CMD_AI_HARD_CODED_SCRIPTS_H

typedef void CCScript(Order *script, Unit *un);

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

#endif //VEGA_STRIKE_ENGINE_CMD_AI_HARD_CODED_SCRIPTS_H
