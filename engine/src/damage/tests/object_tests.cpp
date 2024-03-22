/*
 * object_tests.cpp
 *
 * Copyright (C) 2021-2022 Daniel Horn, Roy Falk, Stephen G. Tuggy,
 * and other Vega Strike contributors
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

#include "damageable_object.h"

// Demonstrate some basic assertions.
TEST(DamageableObject, Sanity) {
    /*CoreVector core_vector(1, 1, 1); // Left top front

    DamageableObject object;

    Health hull_health(0, 50, 0.0f);
    Health armor_health(1, 100, 0.0f);
    Health shield_health(2, 150, 10.0f);

    DamageableLayer hull = DamageableLayer(0, FacetConfiguration::one, hull_health, true);
    DamageableLayer armor = DamageableLayer(1, FacetConfiguration::eight, armor_health, false);
    DamageableLayer shield = DamageableLayer(2, FacetConfiguration::four, shield_health, false);

    EXPECT_EQ(hull.layer_index, 0);
    EXPECT_EQ(armor.layer_index, 1);
    EXPECT_EQ(shield.layer_index, 2);

    object.layers[0] = hull;
    object.layers[1] = armor;
    object.layers[2] = shield;

    Damage damage;
    damage.normal_damage = 10;

    InflictedDamage inflicted_damage = object.DealDamage(core_vector, damage);
    EXPECT_EQ(object.layers[0].facets[0].health, 50);
    EXPECT_EQ(object.layers[1].facets[0].health, 100);
    EXPECT_EQ(object.layers[2].facets[0].health, 140);
    EXPECT_EQ(damage.normal_damage, 0);
    EXPECT_EQ(inflicted_damage.total_damage, 10);
    EXPECT_EQ(inflicted_damage.normal_damage, 10);
    EXPECT_EQ(inflicted_damage.phase_damage, 0);
    EXPECT_EQ(inflicted_damage.inflicted_damage_by_layer[0], 0);
    EXPECT_EQ(inflicted_damage.inflicted_damage_by_layer[1], 0);
    EXPECT_EQ(inflicted_damage.inflicted_damage_by_layer[2], 10);*/
}
