// -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil -*-
/**
 * autodocking.h
 *
 * Copyright (C) 2001-2002 Daniel Horn
 * Copyright (C) 2002-2019 pyramid3d and other Vega Strike Contributors
 * Copyright (C) 2019-2021 Stephen G. Tuggy, and other Vega Strike Contributors
 * Copyright (C) 2022-2023 Stephen G. Tuggy, Benjamen R. Meyer
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
#ifndef VEGA_STRIKE_ENGINE_CMD_AI_AUTO_DOCKING_H
#define VEGA_STRIKE_ENGINE_CMD_AI_AUTO_DOCKING_H

#include <deque>
#include <boost/shared_ptr.hpp>
#include "cmd/ai/order.h"
#include "vec.h"

class Unit;

namespace Orders {

// Navigate the player to the closest auto-dockable port.
//
// If the station is too far away, the normal auto pilot will move the player
// to the station.
//
// An auto-dockable port may have a row of waypoints associated with it. This
// order will face and move towards the first waypoint. When this is reached,
// it will face and move towards the next waypoint, and so on until the docking
// port is reached.
//
// When the docking port is reached the player is docked at the station.
//
// Pre-condition:
//  o A station has been selected.
//  o Docking clearance has been obtained.
//  o The station has at least one docking port that is auto-dockable, available,
//    and big enough to fit the player.
//  o The nearest available docking port must be in line-of-sight.
//
// Post-condition:
//  o One of
//    - The player has been docked.
//    - No suitable docking port is available.
//    - The player or the station has been destroyed.
//
// Limitations:
//  o The player moves directly towards the first waypoint. If the station is
//    between the player and the waypoint, the player will collide with the
//    station.
//  o If auto-docking is disengaged after travelling through a couple of
//    waypoints and then re-engaged, then the player will start by moving directly
//    towards the first waypoint.
//  o The player cannot auto-dock on capital ships. It cannot handle moving
//    stations, and it does not request clearance at fixed intervals to prevent
//    the capital ship from moving away.
//  o Cannot dock capital ships. A capital ship must align one of its docking
//    ports with one of the station's docking ports to dock, and this order does
//    not do that.

class AutoDocking : public Order {
    typedef void (AutoDocking::*StateFunction)(Unit *, Unit *);

public:
    typedef std::deque<int> DockingPath;

    AutoDocking(Unit *destination);

    void Execute();

    static bool CanDock(Unit *player, Unit *station);

protected:
    // States
    void InitialState(Unit *player, Unit *station);
    void SelectionState(Unit *, Unit *);
    void ApproachState(Unit *, Unit *);
    void DockingState(Unit *, Unit *);
    void DockedState(Unit *, Unit *);
    void UndockingState(Unit *, Unit *);
    void DepartureState(Unit *, Unit *);
    void AbortState(Unit *, Unit *);
    void EndState(Unit *, Unit *);

    void EnqueuePort(Unit *, Unit *, size_t);
    void EraseOrders();

private:
    StateFunction state;
    UnitContainer target;
    // waypoints followed by docking port (back)
    DockingPath dockingPath;
};

} // namespace Orders

#endif //VEGA_STRIKE_ENGINE_CMD_AI_AUTO_DOCKING_H
