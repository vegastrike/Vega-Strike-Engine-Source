/*
 * random_tests.cpp
 *
 * Copyright (C) 2001-2024 Daniel Horn, Benjaman Meyer, Roy Falk, Stephen G. Tuggy,
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

#include <gtest/gtest.h>

#include "random_utils.h"

TEST(Random, Sanity) {
    for(int i = 0;i<100;i++) {
        int random_int = randomInt(10);
        EXPECT_GE(random_int, 0);
        EXPECT_LE(random_int, 10);
    }    

    for(int i = 0;i<100;i++) {
        int random_double = randomDouble();
        EXPECT_GE(random_double, 0.0);
        EXPECT_LE(random_double, 1.0);
    } 
}
