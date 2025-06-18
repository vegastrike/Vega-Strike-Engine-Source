/*
 * operational_tests.cpp
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
#include <gtest/gtest.h>
#include <string>

#include "components/hull.h" 
#include "cmd/unit_csv_factory.h"

static const std::string upgrades_suffix_string = "__upgrades";
const std::string reactor_string = "reactor";


std::map<std::string,std::string> reactor_map = {
    { "Hull", "69/100"}
};

TEST(OperationalTests, DamageSystems) {
    UnitCSVFactory::LoadUnit(reactor_string + upgrades_suffix_string, reactor_map);

    Hull hull;

    hull.Load(reactor_string + upgrades_suffix_string);

    EXPECT_EQ(hull.PercentOperational(), 0.69);
}