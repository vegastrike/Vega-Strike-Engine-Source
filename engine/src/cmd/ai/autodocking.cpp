/**
 * autodocking.cpp
 *
 * Copyright (C) 2001-2002 Daniel Horn
 * Copyright (C) 2002-2019 pyramid3d and other Vega Strike Contributors
 * Copyright (C) 2019-2021 Stephen G. Tuggy, and other Vega Strike Contributors
 * Copyright (C) 2022 Stephen G. Tuggy
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


// -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil -*-

#include <cassert>
#include <iterator>
#include <boost/optional.hpp>
#include "cmd/unit_generic.h"
#include "cmd/unit_util.h"
#include "cmd/ai/navigation.h"
#include "autodocking.h"

#include "flykeyboard.h"
#include "in.h"
#include "in_kb_data.h"

namespace {

// Find waypoints if we can travel through all of them.
boost::optional<size_t> FindWaypoint(Unit *player,
                                     const std::vector<DockingPorts> &dockingPorts,
                                     size_t port)
{
    if (!dockingPorts[port].IsConnected()) {
        return port;
    }

    // Subsequent waypoints belong to this port and are listed from closest
    // to farthest.
    size_t i = port + 1;
    for (; i < dockingPorts.size(); ++i) {
        const DockingPorts &waypoint = dockingPorts[i];
        if (waypoint.IsDockable()) { // No further waypoints
            break;
        }
        // Do not use docking port if one of the waypoints are too small
        if (waypoint.GetRadius() < player->rSize()) {
            return boost::optional<size_t>();
        }
    }
    return i - 1;
}

// Find suitable docking port and associated waypoints.
Orders::AutoDocking::DockingPath FindDockingPort(Unit *player,
                                                 Unit *station)
{
    // FIXME: Prefer outside docking ports (because they are more safe to travel to)
    // FIXME: Ensure line-of-sight to first point
    // FIXME: Start at the closest waypoint (and skip those before it)

    const std::vector<DockingPorts> &dockingPorts = station->DockingPortLocations();

    typedef std::pair<size_t, size_t> PortRange;
    boost::optional<PortRange> candidate;
    float shortestDistance = std::numeric_limits<float>::max();
    const bool isPlanet = station->isPlanet();
    for (size_t i = 0; i < dockingPorts.size(); ++i) {
        if (dockingPorts[i].IsOccupied()) {
            continue;
        }

        // Auto-dockable ports must be marked as connected (even if they have
        // no associated waypoints)
        if (!dockingPorts[i].IsConnected()) {
            continue;
        }

        // Does our ship fit into the docking port?
        if (dockingPorts[i].GetRadius() < player->rSize()) {
            continue;
        }

        QVector dockingPosition = Transform(station->GetTransformation(),
                                            dockingPorts[i].GetPosition().Cast());
        float distance = (dockingPosition - player->Position()).Magnitude();
        if (shortestDistance > distance) {
            if (isPlanet) {
                shortestDistance = distance;
                candidate = PortRange(i, i);
            } else {
                boost::optional<size_t> waypoint = FindWaypoint(player, dockingPorts, i);
                if (waypoint) {
                    shortestDistance = distance;
                    candidate = PortRange(i, *waypoint);
                }
            }
        }
    }

    Orders::AutoDocking::DockingPath result;
#if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 7)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
    if (candidate) {
        assert(candidate->first <= candidate->second);
        for (size_t i = candidate->first; i <= candidate->second; ++i) {
            result.push_front(i);
        }
    }
    return result;
#if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 7)
#pragma GCC diagnostic pop
#endif
}

} // anonymous namespace

namespace Orders {

AutoDocking::AutoDocking(Unit *destination)
        : Order(MOVEMENT | FACING, SLOCATION),
          state(&AutoDocking::InitialState),
          target(destination)
{
}

void AutoDocking::Execute()
{
    Unit *player = GetParent();
    Unit *station = target.GetUnit();
    // Exit if either the ship or the station has been destroyed
    if (player == NULL || station == NULL) {
        done = true;
    } else {
        (this->*state)(player, station);
    }
}

bool AutoDocking::CanDock(Unit *player, Unit *station)
{
    if (!station->IsCleared(player)) {
        return false;
    } else if (UnitUtil::isCapitalShip(player)) {
        // A capital ship must align one of its docking ports with one of the
        // the station's docking ports. This docking script cannot do that.
        return false;
    } else if (!UnitUtil::isDockableUnit(station)) {
        return false;
    } else if (!UnitUtil::isCloseEnoughToDock(player, station)) {
        return false;
    } else if (FindDockingPort(player, station).empty()) {
        return false;
    }
    return true;
}

void AutoDocking::EndState(Unit *player, Unit *station)
{
    player->autopilotactive = false;
    done = true;
}

void AutoDocking::AbortState(Unit *player, Unit *station)
{
    EraseOrders();
    state = &AutoDocking::EndState;

    // Safety: full stop on abort
    KBData kbdata;
    FlyByKeyboard::StopKey(kbdata, PRESS);
}

void AutoDocking::InitialState(Unit *player, Unit *station)
{
    if (CanDock(player, station)) {
        state = &AutoDocking::SelectionState;
    } else {
        state = &AutoDocking::AbortState;
    }
}

void AutoDocking::SelectionState(Unit *player, Unit *station)
{
    EraseOrders();

    dockingPath = FindDockingPort(player, station);
    if (dockingPath.empty()) {
        state = &AutoDocking::AbortState;
        return;
    }

    // Enqueue the waypoints from the farthest to the closest.
    for (DockingPath::const_iterator it = dockingPath.begin(); it != dockingPath.end(); ++it) {
        EnqueuePort(player, station, *it);
    }

    state = &AutoDocking::ApproachState;
}

void AutoDocking::ApproachState(Unit *player, Unit *station)
{
    assert(!dockingPath.empty());

    // Move to docking port
    if (station->DockingPortLocations()[dockingPath.back()].IsOccupied()) {
        // Another ship has docked at our port. Find a new port.
        state = &AutoDocking::SelectionState;
    } else if (station->CanDockWithMe(player) == dockingPath.back()) {
        state = &AutoDocking::DockingState;
    } else {
        // FIXME: Request clearance X times with fixed interval to keep capital ship immobile
        Order::Execute();
    }
}

void AutoDocking::DockingState(Unit *player, Unit *station)
{
    assert(!dockingPath.empty());

    player->ForceDock(station, dockingPath.back());
    state = &AutoDocking::DockedState;
}

void AutoDocking::DockedState(Unit *player, Unit *station)
{
    EraseOrders();
    state = &AutoDocking::UndockingState;
}

void AutoDocking::UndockingState(Unit *player, Unit *station)
{
    assert(!dockingPath.empty());

    state = &AutoDocking::EndState;

    // Enqueue undocking path if docked at inner port
    if (station->DockingPortLocations()[dockingPath.back()].IsInside()) {
        DockingPath::reverse_iterator it = dockingPath.rbegin();
        ++it; // Skip the docking port itself
        for (; it != dockingPath.rend(); ++it) {
            EnqueuePort(player, station, *it);
            state = &AutoDocking::DepartureState;
        }
    }
}

void AutoDocking::DepartureState(Unit *player, Unit *station)
{
    Order::Execute();
    if (Done()) {
        EraseOrders();
        done = false;
        state = &AutoDocking::EndState;
    }
}

void AutoDocking::EraseOrders()
{
    eraseType(FACING);
    eraseType(MOVEMENT);
}

void AutoDocking::EnqueuePort(Unit *player, Unit *station, size_t port)
{
    // Set the coordinates for the docking port
    const float turningSpeed = 1;
    // If accuracy is too low then the ship does not always turn precisely.
    // An incorrect heading may cause it to fly into corridor walls.
    // If accuracy is too high then the ship turns precisely, but it does
    // not terminate because it is trying to reach a higher accuracy than
    // it can. This causes the ship to freeze.
    const unsigned char accuracy = 7; // Higher means more accurate
    const bool useAfterburner = false;
    const bool terminateAfterUse = true;

    const DockingPorts &currentPort = station->DockingPortLocations()[port];
    QVector position = Transform(station->GetTransformation(),
                                 currentPort.GetPosition().Cast());

    Order *facing = new ChangeHeading(position, accuracy, turningSpeed, terminateAfterUse);
    Order *movement = new MoveTo(position, useAfterburner, accuracy, terminateAfterUse);
    if (currentPort.IsInside()) {
        // Perform facing and movement sequentially when navigating inside the station
        const unsigned int blockedDuringFacing = MOVEMENT;
        EnqueueOrder(new Sequence(player, facing, blockedDuringFacing));
        const unsigned int blockedDuringMovement = FACING;
        EnqueueOrder(new Sequence(player, movement, blockedDuringMovement));
    } else {
        // Perform facing and movement simultaneously outside the station
        EnqueueOrder(new Join(player, facing, movement));
    }
}

} // namespace Orders
