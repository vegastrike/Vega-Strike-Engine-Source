/*
 * upgradeable_unit.h
 *
 * Copyright (C) 2001-2023 Daniel Horn, Benjaman Meyer, Roy Falk, Stephen G. Tuggy,
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

// -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil -*-

#ifndef UPGRADEABLEUNIT_H
#define UPGRADEABLEUNIT_H

#include <string>
#include <vector>


class Unit;
class Mount;

// TODO: make this into a subclass of unit later

class UpgradeableUnit
{
public:
    UpgradeableUnit();
    void UpgradeUnit(const std::string &upgrades);
    bool UpgradeMounts(const Unit *up,
                  int subunitoffset,
                  bool touchme,
                  bool downgrade,
                  int &numave,
                  double &percentage);
};

#endif // UPGRADEABLEUNIT_H
