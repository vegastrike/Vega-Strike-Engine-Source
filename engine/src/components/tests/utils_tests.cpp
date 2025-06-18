/*
 * utils_tests.cpp
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

#include "components/component_utils.h" 

std::string dummy_color(double percent) {
    if(percent == 1.0) {
        return "white:";
    } else if(percent == 0.0) {
        return "grey:";
    } else if(percent > 0.66) {
        return "yellow:";
    } else if(percent > 0.33) {
        return "orange:";
    } else {
        return "red:";
    } 
}


TEST(UtilsTests, PrintFormattedComponentInHud) {
    double percents[] = {1.0, 0.0, 0.8, 0.6, 0.4, 0.2, 0.1};

    std::string expected_outputs[] = {"white:Dummy (100%)white:\n",
                                      "grey:Dummy (0%)white:\n",
                                      "yellow:Dummy (80%)white:\n",
                                      "orange:Dummy (60%)white:\n",
                                      "orange:Dummy (40%)white:\n",
                                      "red:Dummy (20%)white:\n",
                                      "red:Dummy (10%)white:\n"};

    for(int i=0;i<7;i++) {
        std::string output = PrintFormattedComponentInHud(percents[i], "Dummy", true, dummy_color);
        EXPECT_EQ(output, expected_outputs[i]);
    }
}