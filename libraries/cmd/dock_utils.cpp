/*
 * dock_utils.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
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

#include "dock_utils.h"

#include "unit_generic.h"
#include "universe_util.h"
#include "configuration/configuration.h"
#include "physics.h"

#include <boost/format.hpp>

bool inside_usable_dock(const DockingPorts &dock, const QVector &pos, const float radius, const bool ignore_occupancy) {
    if (!ignore_occupancy && dock.IsOccupied()) {
        return false;
    }
    return IsShorterThan(pos - dock.GetPosition(), static_cast<double>(radius + dock.GetRadius()));
}

double DistanceTwoTargets(Unit *first_unit, Unit *second_unit) {
    double distance = (first_unit->Position() - second_unit->Position()).Magnitude();

    if(first_unit->getUnitType() == Vega_UnitType::planet) {
        distance -= first_unit->rSize();
    }

    if(second_unit->getUnitType() == Vega_UnitType::planet) {
        distance -= second_unit->rSize();
    }

    return (std::max)(0.0, distance);
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

    double range = DistanceTwoTargets(dock, ship);

    // Planet Code
    if (dock->getUnitType() == Vega_UnitType::planet) {
        range -= dock->rSize() * (configuration()->dock.dock_planet_radius_percent - 1);
        if (range < 0) {
            return 0;
        } else {
            return -1;
        }
    }

    if (configuration()->dock.simple_dock) {
        if (range < configuration()->dock.simple_dock_range) {
            return 0;
        } else {
            return -1;
        }
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
                        InvTransform(dock->GetTransformation(),
                                Transform(ship->GetTransformation(),
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
                InvTransform(dock->GetTransformation(), ship->Position()), ship->rSize(), ignore_occupancy)) {
            return i;
        }
    }

    return -1;
}

std::string GetDockingText(Unit *unit, Unit *target, double range) {
    // Nowhere to dock. Exit
    if (target->pImage->dockingports.empty()) {
        return std::string();
    }

    // Jump point. Exit
    if (!target->pImage->destination.empty()) {
        return std::string();
    }


    // Planets/non-planets calculate differently
    if (target->getUnitType() == Vega_UnitType::planet) {
        // TODO: move from here. We shouldn't have kill and land logic here.
        if (range < 0) {
            unit->hull.Destroy();
        }

        range -= target->rSize() * (configuration()->dock.dock_planet_radius_percent - 1);
        if (range < 0) {
            return std::string("Docking: Ready");
        } else if (range < target->rSize()) {
            return std::string("Docking: ") + PrettyDistanceString(range);
        }
    } else if (configuration()->dock.simple_dock && !target->pImage->dockingports.empty() &&
        range < configuration()->dock.count_to_dock_range) {
        if (range <= configuration()->dock.simple_dock_range) {
            return std::string("Docking: Ready");
        } else {
            return std::string("Docking: ") + PrettyDistanceString(range - configuration()->dock.simple_dock_range);
        }
    } else if (CanDock(target, unit, false) >= 0) {
        return std::string("Docking: Ready");
    } else if (CanDock(target, unit, true) >= 0) {
        return std::string("Docking: Auto Ready");
    }

    return std::string();
}


std::string PrettyDistanceString(double distance) {
    if (configuration()->physics.game_speed_lying) {
        distance /= configuration()->physics.game_speed;
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
