// -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil -*-
/*
 * upgradeable_unit.h
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
#ifndef VEGA_STRIKE_ENGINE_CMD_UPGRADEABLE_UNIT_H
#define VEGA_STRIKE_ENGINE_CMD_UPGRADEABLE_UNIT_H

#include <string>
#include <vector>


class Unit;
class Mount;

const std::string UPGRADES_SUFFIX = "__upgrades";

// A struct to hold all results of the upgrade operation
struct UpgradeOperationResult {
    double percent = 0.0;       // Old part percent operational
    bool success = false;       // Can we upgrade/downgrade
    bool upgradeable = false;   // Temp variable. Until we map all types.
};



// TODO: make this into a subclass of unit later

class UpgradeableUnit
{
public:
    UpgradeableUnit();
    virtual ~UpgradeableUnit() {}
    UpgradeOperationResult UpgradeUnit(const std::string upgrade_name,
                     bool upgrade, bool apply);
    bool RepairUnit(const std::string upgrade_name);
    void UpgradeUnit(const std::string &upgrades);
    bool UpgradeMounts(const Unit *up,
                  int subunitoffset,
                  bool touchme,
                  bool downgrade,
                  int &numave,
                  double &percentage);
};

#endif //VEGA_STRIKE_ENGINE_CMD_UPGRADEABLE_UNIT_H
