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
    CoreVector core_vector(1, 1, 1); // Left top front

    DamageableObject object;

    DamageableLayer hull = DamageableLayer(0, FacetConfiguration::one, 50, Damage(1.0,1.0), true);
    DamageableLayer armor = DamageableLayer(1, FacetConfiguration::four, 100, Damage(1.0,1.0), false);
    DamageableLayer shield = DamageableLayer(2, FacetConfiguration::four, 200, Damage(1.0,0.0), false);

    EXPECT_EQ(hull.Layer(), 0);
    EXPECT_EQ(armor.Layer(), 1);
    EXPECT_EQ(shield.Layer(), 2);

    object.AddLayer(&hull);
    object.AddLayer(&armor);
    object.AddLayer(&shield);

    Damage damage;
    damage.normal_damage = 10;
    damage.phase_damage = 0;

    InflictedDamage inflicted_damage = object.DealDamage(core_vector, damage);
    EXPECT_EQ(object.layers[0]->Percent(0), 1.0);
    EXPECT_EQ(object.layers[1]->Percent(0), 1.0);
    EXPECT_EQ(object.layers[2]->Percent(0), 0.95);
    EXPECT_EQ(damage.normal_damage, 0);
    EXPECT_EQ(inflicted_damage.total_damage, 10);
    EXPECT_EQ(inflicted_damage.normal_damage, 10);
    EXPECT_EQ(inflicted_damage.phase_damage, 0);
    EXPECT_EQ(inflicted_damage.inflicted_damage_by_layer[0], 0);
    EXPECT_EQ(inflicted_damage.inflicted_damage_by_layer[1], 0);
    EXPECT_EQ(inflicted_damage.inflicted_damage_by_layer[2], 10);
}
