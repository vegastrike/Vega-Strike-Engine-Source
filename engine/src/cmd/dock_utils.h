/*
 * dock_utils.h
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

#ifndef VEGA_STRIKE_ENGINE_CMD_DOCK_UTILS_H
#define VEGA_STRIKE_ENGINE_CMD_DOCK_UTILS_H

#include <string>

class Unit;

double DistanceTwoTargets(Unit *parent, Unit *target);
int CanDock(Unit *dock, Unit *ship, bool ignore_occupancy = false);
std::string GetDockingText(Unit *unit, Unit *target, double range);
std::string PrettyDistanceString(double distance);

#endif // VEGA_STRIKE_ENGINE_CMD_DOCK_UTILS_H
