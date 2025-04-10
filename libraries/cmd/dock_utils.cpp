/*
 * dock_utils.cpp
 *
 * Copyright (C) 2001-2023 Daniel Horn, Benjamen Meyer, Roy Falk, Stephen G. Tuggy,
 * and other Vega Strike contributors.
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
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */

#include "dock_utils.h"

#include <universe.h>

#include "unit_generic.h"
#include "universe_util.h"
#include "configuration/configuration.h"
#include "physics.h"

#include <boost/format.hpp>

bool insideDock(const DockingPorts &dock, const QVector &pos, float radius) {
    if (dock.IsOccupied()) {
        return false;
    }
    return IsShorterThan(pos - dock.GetPosition(), double(radius + dock.GetRadius()));
}

double DistanceTwoTargets(Unit *first_unit, Unit *second_unit) {
    double distance = (first_unit->Position() - second_unit->Position()).Magnitude();

    if(first_unit->isUnit() == Vega_UnitType::planet) {
        distance -= first_unit->rSize();
    }

    if(second_unit->isUnit() == Vega_UnitType::planet) {
        distance -= second_unit->rSize();
    }

    return std::max(0.0, distance);
}

/**
 * @brief check whether a ship can dock
 * @param dock - the dock unit
 * @param ship - the docking unit
 * @param ignore_occupancy - don't check if dock is already occupied
 * @returns the dock number or -1 for fail
 */
int CanDock(Unit *dock, Unit *ship, bool ignore_occupancy) {
    // Nowhere to dock. Exit
    if(dock->pImage->dockingports.empty()) {
        return -1;
    }

    // Jump point. Exit
    if(!dock->pImage->destination.empty()) {
        return -1;
    }

    bool is_player_starship = _Universe->isPlayerStarship(ship);

    double range = DistanceTwoTargets(dock, ship);

    // Planet Code
    if (dock->isUnit() == Vega_UnitType::planet) {
        range -= dock->rSize() * (configuration()->dock.dock_planet_radius_percent - 1);
        if (range < 0) {
            if (is_player_starship) {
                VS_LOG(debug, "Planet. Within range. CanDock = 0");
            }
            return 0;
        } else {
            if (is_player_starship) {
                VS_LOG(debug, "Planet. Out of range. CanDock = -1");
            }
            return -1;
        }
    }


    if(configuration()->dock.simple_dock) {
        range = DistanceTwoTargets(dock, ship);
        if (is_player_starship) {
            VS_LOG(debug, (boost::format("CanDock: simple_dock is true. range = %1%") % range));
        }

        if (range < configuration()->dock.simple_dock_range) {
            if (is_player_starship) {
                VS_LOG(debug, "simple_dock is true. Within range. CanDock = 0");
            }
            return 0;
        } else {
            if (is_player_starship) {
                VS_LOG(debug, "simple_dock is true. Out of range. CanDock = -1");
            }
        }
    }

    if (ignore_occupancy) {
        if (is_player_starship) {
            VS_LOG(debug, "CanDock: ignore_occupancy is true");
        }
        for (unsigned int i = 0; i < dock->pImage->dockingports.size(); ++i) {
            if (!dock->pImage->dockingports[i].IsOccupied()) {
                if (is_player_starship) {
                    VS_LOG(debug, (boost::format("CanDock: ignore_occupancy is true and found a dock that isn't occupied. Returning %1%") % i));
                }
                return i;
            }
        }
    }

    //don't need to check relation: already cleared.

    // If your unit has docking ports then we check if any of our docking
    // ports overlap with any of the station's docking ports.
    // Otherwise, we simply check if our unit overlaps with any of the
    // station's docking ports.
    for (unsigned int i = 0; i < dock->pImage->dockingports.size(); ++i) {
        if (!dock->pImage->dockingports.empty()) {
            for (unsigned int j = 0; j < dock->pImage->dockingports.size(); ++j) {
                if (insideDock(dock->pImage->dockingports[i],
                        InvTransform(dock->GetTransformation(),
                                Transform(dock->GetTransformation(),
                                        dock->pImage->dockingports[j].GetPosition().Cast())),
                        dock->pImage->dockingports[j].GetRadius())) {
                    // We cannot dock if we are already docked
                    if (((dock->docked & (dock->DOCKED_INSIDE | dock->DOCKED)) == 0) && (!(dock->docked & dock->DOCKED_INSIDE))) {
                        if (is_player_starship) {
                            VS_LOG(debug, (boost::format("CanDock: innermost return statement: returning %1%") % i));
                        }
                        return i;
                    }
                }
            }
        } else if (insideDock(dock->pImage->dockingports[i],
                InvTransform(dock->GetTransformation(), dock->Position()),
                dock->rSize())) {
            if (is_player_starship) {
                VS_LOG(debug, (boost::format("CanDock: second to last return statement: returning %1%") % i));
            }
            return i;
        }
    }

    if (is_player_starship) {
        VS_LOG(debug, "CanDock fell through to the end. Returning -1");
    }
    return -1;
}

std::string GetDockingText(Unit *unit, Unit *target, double range) {
    //If I can dock, don't bother displaying distance
    if (CanDock(target, unit, false) != -1) {
        return std::string("Docking: Ready");
    }

    // Nowhere to dock. Exit
    if(target->pImage->dockingports.size() == 0) {
        return std::string();
    }

    // Jump point. Exit
    if(target->pImage->destination.size() > 0) {
        return std::string();
    }


    // Planets/non-planets calculate differently
    if (target->isUnit() == Vega_UnitType::planet) {
        // TODO: move from here. We shouldn't have kill and land logic here.
        if(range <= 0) {
            unit->hull.Destroy();
        }

        range -= target->rSize() * (configuration()->dock.dock_planet_radius_percent - 1);
        if (range > 0 && range < target->rSize()) {
            return std::string("Docking: ") + string(PrettyDistanceString(range));
        }
    } else {
        if(configuration()->dock.simple_dock && target->pImage->dockingports.size() != 0 &&
            range < configuration()->dock.count_to_dock_range) {
            if (range > configuration()->dock.simple_dock_range) {
                return std::string("Docking: ") + string(PrettyDistanceString(range-5000));
            }
        }
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
