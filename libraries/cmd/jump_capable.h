/*
 * jump_capable.h
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2026 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
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

#include "src/star_system.h"

#include <string>

// This includes both spec (in-system FTL) and jump drives support.
// TODO: consider moving actual decision making code to actual module providing capability to ship
class JumpCapable {
public:
    StarSystem *activeStarSystem;

    JumpCapable();

    virtual ~JumpCapable();

    void ActivateJumpDrive(int destination = 0);
    void AddDestination(const std::string &);
    bool AutoPilotTo(Unit *un, bool automaticenergyrealloc);
    bool AutoPilotToErrorMessage(const Unit *un, bool automaticenergyrealloc,
            std::string &failuremessage, int recursive_level = 2);
    float GetNearestObjectSignificantDistance(Unit **nearest_unit = nullptr) const;
    // A body the ship can never fly through or around the far side of: planets,
    // suns (suns are planets) and bases/stations. The SPEC clear-space model never
    // culls these. Asteroids are NOT hard (you can fly through/thread them).
    static bool IsHardBody(const Unit *unit);
    // The clear-space bubble radius: the full SPEC compression range, capped at the
    // range to the target so the bubble shrinks to nothing on arrival. Pass the
    // target range, or a large value when there is no target.
    static double WarpClearanceRadius(double target_range);
    // Significant distance to another unit, shortened by the SPEC drop-out zone
    // (warp_min_range) when that unit is a hard body, so the whole zone is cleared
    // rather than only the object's surface. This is what the clear-space model
    // compares against WarpClearanceRadius.
    double GetWarpClearanceDistance(const Unit *other) const;
    float CourseDeviation(const Vector &OriginalCourse, const Vector &FinalCourse) const;
    void DeactivateJumpDrive();
    const std::vector<std::string> &GetDestinations() const;
    StarSystem *getStarSystem();
    const StarSystem *getStarSystem() const;
    Vector GetWarpRefVelocity() const;
    Vector GetWarpVelocity() const;
    bool InCorrectStarSystem(const StarSystem *);
    virtual bool TransferUnitToSystem(StarSystem *new_system);
    virtual bool TransferUnitToSystem(unsigned int whichJumpQueue,
            class StarSystem *&previouslyActiveStarSystem,
            bool DoSightAndSound);
};

#endif //VEGA_STRIKE_ENGINE_CMD_JUMP_CAPABLE_H
