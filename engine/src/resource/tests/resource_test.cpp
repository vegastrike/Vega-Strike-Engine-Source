/*
 * resource_test.cpp
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

#include "resource.h"

TEST(Resource, Sanity) {
    Resource<float> resource = Resource<float>(10.0f, 0.0f);
    EXPECT_EQ(resource.Value(), 10.0f);

    resource.Set(5.0f);
    EXPECT_EQ(resource.Value(), 5.0f);

    resource += 5.0f;
    EXPECT_EQ(resource.Value(), 10.0f);

    resource -= 2.0f;
    EXPECT_EQ(resource.Value(), 8.0f);

    resource -= 12.0f;
    EXPECT_EQ(resource.Value(), 0.0f);
}
