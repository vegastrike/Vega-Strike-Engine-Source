/*
 * csv_tests.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Creator: Daniel Horn
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

#include "unit_csv_factory.h"
#include <iostream>
#include <fstream>
#include <iterator>

TEST(CSV, Sanity) {
    // This may not work for all deployments.
    // Consider standardizing this.
    /*std::ifstream ifs("../../data/units/units.csv", std::ifstream::in);
    std::stringstream buffer;
    buffer << ifs.rdbuf();

    UnitCSVFactory factory;
    std::string csv_contents{buffer.str()};
    factory.ProcessCSV(csv_contents, false);*/

    /*for (auto const& x : UnitCSVFactory::units)
    {
        std::string unit = x.first;
        std::map<std::string, std::string> unit_attributes = x.second;

        for (auto const& y : unit_attributes)
        {
            std::string key = y.first;
            std::string value = y.second;

            std::cout << x.first  << ':'
                      << y.first  << ':'
                      << y.second << std::endl;
        }
    }*/
}
