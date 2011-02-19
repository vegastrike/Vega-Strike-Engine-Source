// -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil -*-

#include <cassert>
#include <iterator>
#include <boost/iterator/counting_iterator.hpp>
#include "cmd/unit_generic.h"
#include "cmd/unit_util.h"
#include "cmd/ai/navigation.h"
#include "autodocking.h"

namespace
{
typedef std::pair<size_t, size_t> PortRange;

// Find waypoints if we can travel through all of them.
boost::optional<size_t> FindWaypoint(Unit *player,
                                     const std::vector<DockingPorts>& dockingPorts,
                                     size_t port)
{
    if (!dockingPorts[port].IsConnected())
        return port;

    // Subsequent waypoints belong to this port and are listed from closest
    // to farthest.
    size_t i = port + 1;
    for (; i < dockingPorts.size(); ++i)
    {
        const DockingPorts& waypoint = dockingPorts[i];
        if (waypoint.IsDockable()) // No further waypoints
            return i - 1;
        // Do not use docking port is one of the waypoints are too small
        if (waypoint.GetRadius() < player->rSize())
            return boost::optional<size_t>();
    }
    return i;
}

// Find suitable docking port and associated waypoints.
boost::optional<PortRange> FindDockingPort(Unit *player,
                                           Unit *station)
{
    // FIXME: Prefer outside docking ports (because they are more safe to travel to)
    // FIXME: Ensure line-of-sight to first point
    // FIXME: Start at the closest waypoint (and skip those before it)

    const std::vector<DockingPorts>& dockingPorts = station->DockingPortLocations();

    boost::optional<PortRange> candidate;
    float shortestDistance = std::numeric_limits<float>::max();
    bool isPlanet = station->isPlanet();
    for (size_t i = 0; i < dockingPorts.size(); ++i)
    {
        if (dockingPorts[i].IsOccupied())
            continue;

        // Auto-dockable ports must be marked as connected (even if they have
        // no associated waypoints)
        if (!dockingPorts[i].IsConnected())
            continue;

        // Does our ship fit into the docking port?
        if (dockingPorts[i].GetRadius() < player->rSize())
            continue;

        // DockingPorts::GetPosition() returns coordinates relative to the station
        QVector dockingPosition = Transform(station->GetTransformation(),
                                            dockingPorts[i].GetPosition().Cast());
        float distance = (dockingPosition - player->Position()).Magnitude();
        if (shortestDistance > distance)
        {
            if (isPlanet)
            {
                shortestDistance = distance;
                candidate = PortRange(i, i);
            }
            else
            {
                boost::optional<size_t> waypoint = FindWaypoint(player, dockingPorts, i);
                if (waypoint)
                {
                    shortestDistance = distance;
                    candidate = PortRange(i, *waypoint);
                }
            }
        }
    }
    return candidate;
}

} // anonymous namespace

namespace Orders
{

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
    if (player == NULL || station == NULL)
    {
        done = true;
    }
    else
    {
        (this->*state)(player, station);
    }
}

void AutoDocking::EndState(Unit *player, Unit *station)
{
    player->autopilotactive = false;
    done = true;
}

void AutoDocking::AbortState(Unit *player, Unit *station)
{
    eraseType(ALLTYPES);
    state = &AutoDocking::EndState;
}

void AutoDocking::InitialState(Unit *player, Unit *station)
{
    if (UnitUtil::isCapitalShip(player))
    {
        // A capital ship must align one of its docking ports with one of the
        // the station's docking ports. This docking script cannot do that.
        state = &AutoDocking::AbortState;
    }
    else if (!UnitUtil::isDockableUnit(station))
    {
        state = &AutoDocking::AbortState;
    }
    else if (!UnitUtil::isCloseEnoughToDock(player, station))
    {
        // Autopilot the ship closer to the station
        AutoLongHaul *longHaul = new AutoLongHaul();
        longHaul->SetParent(player);
        EnqueueOrder(longHaul);
        state = &AutoDocking::DistantApproachState;
    }
    else if (station->IsCleared(player))
    {
        state = &AutoDocking::SelectionState;
    }
    else
    {
        state = &AutoDocking::AbortState;
    }
}

void AutoDocking::DistantApproachState(Unit *player, Unit *station)
{
    // Move to station from afar
    Order::Execute();
    if (Done())
    {
        eraseType(ALLTYPES);
        done = false;
        state = &AutoDocking::SelectionState;
    }
}

void AutoDocking::SelectionState(Unit *player, Unit *station)
{
    eraseType(ALLTYPES);

    boost::optional<PortRange> range = FindDockingPort(player, station);
    if (range)
    {
        port = range->first;
    }
    else
    {
        port.reset();
        state = &AutoDocking::AbortState;
        return;
    }

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
    // Iterate backwards to enqueue the waypoints from the farthest to the closest.
    typedef std::reverse_iterator< boost::counting_iterator<size_t> > PortIterator;
    PortIterator beginRange(range->second + 1); // counting_iterator expects an open ended interval
    PortIterator endRange(range->first);
    for (PortIterator it = beginRange; it != endRange; ++it)
    {
        const DockingPorts& currentPort = station->DockingPortLocations()[*it];
        QVector position = Transform(station->GetTransformation(),
                                     currentPort.GetPosition().Cast());

        Order *facing = new ChangeHeading(position, accuracy, turningSpeed, terminateAfterUse);
        Order *movement = new MoveTo(position, useAfterburner, accuracy, terminateAfterUse);
        if (currentPort.IsInside())
        {
            // Perform facing and movement sequentially when navigating inside the station
            const unsigned int blockedDuringFacing = MOVEMENT;
            EnqueueOrder(new Sequence(player, facing, blockedDuringFacing));
            const unsigned int blockedDuringMovement = FACING;
            EnqueueOrder(new Sequence(player, movement, blockedDuringMovement));
        }
        else
        {
            // Perform facing and movement simultaneously outside the station
            EnqueueOrder(new Join(player, facing, movement));
        }
    }

    state = &AutoDocking::ApproachState;
}

void AutoDocking::ApproachState(Unit *player, Unit *station)
{
    assert(port);

    // Move to docking port
    if (station->DockingPortLocations()[*port].IsOccupied())
    {
        // Another ship has docked at our port. Find a new port.
        state = &AutoDocking::SelectionState;
    }
    else if (station->CanDockWithMe(player) == *port)
    {
        state = &AutoDocking::DockState;
    }
    else
    {
        // FIXME: Request clearance X times with fixed interval to keep capital ship immobile
        Order::Execute();
    }
}

void AutoDocking::DockState(Unit *player, Unit *station)
{
    assert(port);

    eraseType(ALLTYPES);
    player->ForceDock(station, *port);
    state = &AutoDocking::EndState;
}

} // namespace Orders
