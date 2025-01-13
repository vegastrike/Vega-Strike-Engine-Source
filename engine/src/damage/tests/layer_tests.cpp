/*
 * layer_tests.cpp
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

#include "damageable_layer.h"
#include "core_vector.h"

// Demonstrate some basic assertions.
TEST(Layer, Sanity) {
    Health health(10, 10, 0);
    DamageableLayer layer = DamageableLayer(0, FacetConfiguration::four, health, true);

    EXPECT_EQ(layer.GetFacetIndex(CoreVector(0, 0, 0)), 0);

    EXPECT_EQ(layer.GetFacetIndex(CoreVector(1, 0, 1)), 0);
    EXPECT_EQ(layer.GetFacetIndex(CoreVector(1, 0, 2)), 2);
    EXPECT_EQ(layer.GetFacetIndex(CoreVector(2, 0, 1)), 0);

    EXPECT_EQ(layer.GetFacetIndex(CoreVector(-1, 0, 1)), 2);
    EXPECT_EQ(layer.GetFacetIndex(CoreVector(-1, 0, 2)), 2);
    EXPECT_EQ(layer.GetFacetIndex(CoreVector(-2, 0, 1)), 1);

    EXPECT_EQ(layer.GetFacetIndex(CoreVector(1, 0, -1)), 0);
    EXPECT_EQ(layer.GetFacetIndex(CoreVector(1, 0, -2)), 3);
    EXPECT_EQ(layer.GetFacetIndex(CoreVector(2, 0, -1)), 0);

    EXPECT_EQ(layer.GetFacetIndex(CoreVector(-1, 0, -1)), 3);
    EXPECT_EQ(layer.GetFacetIndex(CoreVector(-1, 0, -2)), 3);
    EXPECT_EQ(layer.GetFacetIndex(CoreVector(-2, 0, -1)), 1);

}
