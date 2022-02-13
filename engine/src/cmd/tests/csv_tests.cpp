/*
 * unit_generic.cpp
 *
 * Copyright (C) 2021 Roy Falk
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
    std::ifstream ifs("../../data/units/units.csv", std::ifstream::in);
    std::stringstream buffer;
    buffer << ifs.rdbuf();

    UnitCSVFactory factory;
    factory.ProcessCSV(buffer.str(), false);

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
