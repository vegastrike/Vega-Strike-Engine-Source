/*
 * jump_capable.h
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
#ifndef VEGA_STRIKE_ENGINE_CMD_JUMP_CAPABLE_H
#define VEGA_STRIKE_ENGINE_CMD_JUMP_CAPABLE_H

#include "star_system.h"
#include "energetic.h"

#include <string>

// This includes both spec (in-system FTL) and jump drives support.
// TODO: consider moving actual decision making code to actual module providing capability to ship
class JumpCapable {
public:
    StarSystem *activeStarSystem;

public:
    JumpCapable();

    void ActivateJumpDrive(int destination = 0);
    void AddDestination(const std::string &);
    bool AutoPilotTo(Unit *un, bool automaticenergyrealloc);
    bool AutoPilotToErrorMessage(const Unit *un, bool automaticenergyrealloc,
            std::string &failuremessage, int recursive_level = 2);
    float CalculateNearestWarpUnit(float minmultiplier, Unit **nearest_unit,
            bool count_negative_warp_units) const;
    float CourseDeviation(const Vector &OriginalCourse, const Vector &FinalCourse) const;
    void DeactivateJumpDrive();
    const std::vector<std::string> &GetDestinations() const;
    const Energetic::UnitJump &GetJumpStatus() const;
    StarSystem *getStarSystem();
    const StarSystem *getStarSystem() const;
    Vector GetWarpRefVelocity() const;
    Vector GetWarpVelocity() const;
    bool InCorrectStarSystem(StarSystem *);
    virtual bool TransferUnitToSystem(StarSystem *NewSystem);
    virtual bool TransferUnitToSystem(unsigned int whichJumpQueue,
            class StarSystem *&previouslyActiveStarSystem,
            bool DoSightAndSound);
};

#endif //VEGA_STRIKE_ENGINE_CMD_JUMP_CAPABLE_H
