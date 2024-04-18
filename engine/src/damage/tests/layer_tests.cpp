/*
 * layer_tests.cpp
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
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */


#include <gtest/gtest.h>
#include <vector>

#include "damageable_layer.h"
#include "core_vector.h"

// Demonstrate some basic assertions.
TEST(Layer, Sanity) {
    Health health(10, 10);
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

TEST(Layer, Sanity_2) {    
    Health health(0, 10);
    EXPECT_EQ(health.health.MaxValue(), 10);
    EXPECT_EQ(health.health.Value(), 10);

    // This is copied in the next line. Do not refer to this variable.
    DamageableLayer layer = DamageableLayer(0, FacetConfiguration::one, health, true);

    std::vector<DamageableLayer> layers = { layer };
    EXPECT_EQ(layers[0].facets[0].health.MaxValue(), 10);
    EXPECT_EQ(layers[0].facets[0].health.Value(), 10);
    
    DamageableLayer* ptr = &layers[0];
    EXPECT_EQ(ptr->facets[0].health.MaxValue(), 10);
    EXPECT_EQ(layers[0].facets[0].health.Value(), 10);

    std::vector<double> new_health = {50};
    ptr->UpdateFacets(new_health);

    EXPECT_EQ(layers[0].facets[0].health.MaxValue(), 50);
    EXPECT_EQ(layers[0].facets[0].health.Value(), 50);

    EXPECT_EQ(ptr->facets[0].health.MaxValue(), 50);
    EXPECT_EQ(ptr->facets[0].health.Value(), 50);

    EXPECT_EQ(ptr, &layers[0]);
}

// How embarrassing
TEST(Layer, StringComparison) {   
    std::string player_ship = "player_ship";
    bool result = (player_ship == "player_ship");
    EXPECT_TRUE(result);
}