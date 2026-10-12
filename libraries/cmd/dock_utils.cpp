/*
 * dock_utils.cpp
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

#include "dock_utils.h"

#include "unit_generic.h"
#include "universe_util.h"
#include "configuration/configuration.h"
#include "physics.h"

#include <boost/format.hpp>
#include <algorithm>

// Which docking rule applies: the simple "dock when close" test, or the docking zones you fly the
// ship into. An empty mode means the setting is not in the config yet, in which case the older
// simple_dock boolean decides, so an overlay that only sets that still works.
bool DockIsSimple() {
    const std::string &mode = configuration().dock.mode;
    if (mode == "zones") {
        return false;
    }
    if (mode == "simple") {
        return true;
    }
    return configuration().dock.simple_dock;
}

bool inside_usable_dock(const DockingPorts &dock, const QVector &pos, const float radius, const bool ignore_occupancy) {
    if (!ignore_occupancy && dock.IsOccupied()) {
        return false;
    }
    return IsShorterThan(pos - dock.GetPosition(), static_cast<double>(radius + dock.GetRadius()));
}

namespace {

// A unit's physical transform, rather than the one the renderer last composed for it:
// cumulative_transformation starts as the identity and is only written while the unit is drawn, so a
// body the player has not looked at yet would measure at the origin.
Matrix PhysicsTransform(const Unit *unit) {
    Matrix m;
    unit->curr_physical_state.to_matrix(m);
    return m;
}

} // anonymous namespace

double DistanceTwoTargets(Unit *first_unit, Unit *second_unit) {
    // LocalPosition, not Position: the latter is the renderer's cumulative transform, which is only
    // written while the unit is being drawn and starts as the identity.
    double distance = (first_unit->LocalPosition() - second_unit->LocalPosition()).Magnitude();

    if(first_unit->getUnitType() == Vega_UnitType::planet) {
        distance -= first_unit->rSize();
    }

    if(second_unit->getUnitType() == Vega_UnitType::planet) {
        distance -= second_unit->rSize();
    }

    return std::max(0.0, distance);
}

/**
 * @brief The distance at which a body counts as dockable
 * @param dock - the body being docked with
 * @return the distance, measured from a planet's surface or a ship's centre
 */
double DockingDistance(const Unit *dock) {
    const double range = configuration().dock.simple_dock_range_dbl;
    if (dock->getUnitType() == Vega_UnitType::planet) {
        const double zone = dock->rSize() * (configuration().dock.dock_planet_radius_percent_dbl - 1.0);
        return std::max(range, zone);
    }
    return range;
}

/**
 * @brief check whether a ship can dock
 * @param dock - the dock unit
 * @param ship - the docking unit
 * @param ignore_occupancy - don't check if dock is already occupied
 * @returns the dock number or -1 for fail
 */
int CanDock(Unit *dock, Unit *ship, const bool ignore_occupancy) {
    constexpr double kDefinitelyTooFar = 20000.0;

    // Nowhere to dock. Exit
    if(dock->pImage->dockingports.empty()) {
        return -1;
    }

    // Jump point. Exit
    if(!dock->pImage->destination.empty()) {
        return -1;
    }

    // A sun/star is never dockable, regardless of how it is otherwise typed
    // (a data file can mislabel a star as a normal body).
    if (UnitUtil::isSun(dock)) {
        return -1;
    }

    double range = DistanceTwoTargets(dock, ship);

    // Dockable when inside the body's docking distance. A planet is always in this branch: zones
    // mode is about flying into a station's docking port, and a planet has no such thing to fly
    // into, so its own zone is the whole test either way.
    if (dock->getUnitType() == Vega_UnitType::planet || DockIsSimple()) {
        return range < DockingDistance(dock) ? 0 : -1;
    }

    if (range > kDefinitelyTooFar) {
        // Definitely too far. Short-circuit the rest of the tests.
        return -1;
    }

    //don't need to check relation: already cleared.

    // If your unit has docking ports then we check if any of our docking
    // ports overlap with any of the station's docking ports.
    // Otherwise, we simply check if our unit overlaps with any of the
    // station's docking ports.
    for (unsigned int i = 0; i < dock->pImage->dockingports.size(); ++i) {
        if (!ship->pImage->dockingports.empty()) {
            for (unsigned int j = 0; j < ship->pImage->dockingports.size(); ++j) {
                if (inside_usable_dock(dock->pImage->dockingports[i],
                        InvTransform(PhysicsTransform(dock),
                                Transform(PhysicsTransform(ship),
                                        ship->pImage->dockingports[j].GetPosition().Cast())),
                        ship->pImage->dockingports[j].GetRadius(), ignore_occupancy)) {
                    // We cannot dock if we are already docked
                    if (((ship->docked & (Unit::DOCKED_INSIDE | Unit::DOCKED)) == 0) && (!(dock->docked & Unit::DOCKED_INSIDE))) {
                        return i;
                    }
                }
            }
        }
        if (inside_usable_dock(dock->pImage->dockingports[i],
                InvTransform(PhysicsTransform(dock), ship->LocalPosition()), ship->rSize(), ignore_occupancy)) {
            return i;
        }
    }

    return -1;
}

namespace {

// The distance from the ship to the nearest of the dock's docking ports, which is what a zones-mode
// approach is aiming for. Ports are in the dock's own frame, so they come out to world space first.
// Returns -1 when there are none.
double NearestPortDistance(const Unit *unit, const Unit *dock) {
    if (dock->pImage == nullptr || dock->pImage->dockingports.empty()) {
        return -1.0;
    }
    const Matrix dock_tf = PhysicsTransform(dock);
    double nearest = -1.0;
    for (const DockingPorts &port : dock->pImage->dockingports) {
        const QVector world = Transform(dock_tf, port.GetPosition().Cast());
        const double distance = (world - unit->LocalPosition()).Magnitude();
        if (nearest < 0.0 || distance < nearest) {
            nearest = distance;
        }
    }
    return nearest;
}

} // anonymous namespace

std::string GetDockingText(Unit *unit, Unit *target, double range) {
    // Nowhere to dock. Exit
    if (target->pImage->dockingports.empty()) {
        return std::string();
    }

    // Jump point. Exit
    if (!target->pImage->destination.empty()) {
        return std::string();
    }

    // A sun/star is never dockable.
    if (UnitUtil::isSun(target)) {
        return std::string();
    }


    // Planets/non-planets calculate differently
    if (target->getUnitType() == Vega_UnitType::planet) {
        // TODO: move from here. We shouldn't have kill and land logic here.
        if (range < 0) {
            unit->hull.Destroy();
        }

        const double docking_distance = DockingDistance(target);
        range -= docking_distance;
        if (range < 0) {
            return std::string("Docking: Ready");
        } else if (range < target->rSize()) {
            return std::string("Docking: ") + PrettyDistanceString(range);
        }
    } else if (DockIsSimple() && !target->pImage->dockingports.empty() &&
        range < configuration().dock.count_to_dock_range_dbl) {
        if (range <= DockingDistance(target)) {
            return std::string("Docking: Ready");
        } else {
            return std::string("Docking: ") + PrettyDistanceString(range - DockingDistance(target));
        }
    } else if (CanDock(target, unit, false) >= 0) {
        return std::string("Docking: Ready");
    } else if (CanDock(target, unit, true) >= 0) {
        return std::string("Docking: Auto Ready");
    } else if (!target->pImage->dockingports.empty() && range < configuration().dock.count_to_dock_range_dbl) {
        // Docking zones: in range of the station but not in a port yet. The distance is to the
        // nearest port rather than the station's centre, which is not where the ship has to go.
        const double port_range = NearestPortDistance(unit, target);
        if (port_range >= 0.0) {
            return std::string("Docking: ") + PrettyDistanceString(port_range);
        }
    }

    return std::string();
}


std::string PrettyDistanceString(double distance) {
    if (configuration().physics.game_speed_lying) {
        distance /= configuration().physics.game_speed_dbl;
    }

    // Distance in km
    static const double light_second = c;
    static const double light_minute = light_second * 60;
    static const double light_hour = light_minute * 60;
    static const double light_day = light_hour * 24;
    static const double light_year = light_day * 365;

    // Use meters up to 20,000 m
    if (distance < 20000) {
        return (boost::format("%.0lf meters") % distance).str();
    }

    // Use kilometers with two decimals up to 100 km
    if (distance < 100000) {
        return (boost::format("%.2lf kilometers") % (distance / 1000)).str();
    }

    // Use kilometers without decimals up to a light second
    if (distance < light_second) {
        return (boost::format("%.0lf kilometers") % (distance / 1000)).str();
    }

    // Use light seconds up to 2 light minutes
    if (distance < 2 * light_minute) {
        return (boost::format("%.2lf light seconds") % (distance / light_second)).str();
    }

    // Use light minutes up to 120
    if (distance < (120 * light_minute)) {
        return (boost::format("%.2lf light minutes") % (distance / light_minute)).str();
    }

    //use light hours up to 48
    if (distance < (48 * light_hour)) {
        return (boost::format("%.2lf light hours") % (distance / light_hour)).str();
    }

    // Use light days up to 365
    if (distance < (365 * light_day)) {
        return (boost::format("%.2lf light days") % (distance / light_day)).str();
    }

    // Use light years
    return (boost::format("%.2lf light years") % (distance / light_year)).str();
}
