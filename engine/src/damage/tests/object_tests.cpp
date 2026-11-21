/*
 * object_tests.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2026 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
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

#include "damage/damageable_object.h"

// Demonstrate some basic assertions.
TEST(DamageableObject, Sanity) {
    CoreVector core_vector(1, 1, 1); // Left top front

    DamageableObject object;

    DamageableLayer hull = DamageableLayer(DamageLayer::hull, FacetConfiguration::one, 50, Damage(1.0,1.0), true);
    DamageableLayer armor = DamageableLayer(DamageLayer::armor, FacetConfiguration::four, 100, Damage(1.0,1.0), false);
    DamageableLayer shield = DamageableLayer(DamageLayer::shield, FacetConfiguration::four, 200, Damage(1.0,0.0), false);

    EXPECT_EQ(hull.Layer(), DamageLayer::hull);
    EXPECT_EQ(armor.Layer(), DamageLayer::armor);
    EXPECT_EQ(shield.Layer(), DamageLayer::shield);

    object.AddLayer(&hull);
    object.AddLayer(&armor);
    object.AddLayer(&shield);

    Damage damage;
    damage.normal_damage = 10;
    damage.phase_damage = 0;

    InflictedDamage inflicted_damage = object.DealDamage(core_vector, damage);
    EXPECT_EQ(object.GetLayer(DamageLayer::hull)->Percent(0), 1.0);
    EXPECT_EQ(object.GetLayer(DamageLayer::armor)->Percent(0), 1.0);
    EXPECT_EQ(object.GetLayer(DamageLayer::shield)->Percent(0), 0.95);
    EXPECT_EQ(damage.normal_damage, 0);
    EXPECT_EQ(inflicted_damage.total_damage, 10);
    EXPECT_EQ(inflicted_damage.normal_damage, 10);
    EXPECT_EQ(inflicted_damage.phase_damage, 0);
    EXPECT_EQ(inflicted_damage.inflicted_damage_by_layer.at(DamageLayer::hull), 0);
    EXPECT_EQ(inflicted_damage.inflicted_damage_by_layer.at(DamageLayer::armor), 0);
    EXPECT_EQ(inflicted_damage.inflicted_damage_by_layer.at(DamageLayer::shield), 10);
}
