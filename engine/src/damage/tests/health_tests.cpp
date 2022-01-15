/*
 * health_tests.cpp
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

#include "health.h"


// Demonstrate some basic assertions.
TEST(Shield, Sanity)
{
    Damage damage;
    InflictedDamage inflicted_damage(3);
    damage.normal_damage = 10;
    EXPECT_EQ(damage.normal_damage, 10);

    Health health(100, 100, 25);

    EXPECT_EQ(health.health, 100);
    EXPECT_EQ(health.max_health, 100);
    EXPECT_EQ(health.regeneration, 25);
    EXPECT_TRUE(health.regenerative);
    EXPECT_FALSE(health.destroyed);
    EXPECT_TRUE(health.enabled);

    health.DealDamage(damage, inflicted_damage);
    EXPECT_EQ(health.health, 90);
    EXPECT_EQ(damage.normal_damage, 0);

    health.Disable();
    health.Regenerate(1.0f);
    EXPECT_EQ(health.health, 0);

    health.Enable();
    health.Regenerate(health.regeneration);
    EXPECT_EQ(health.health, 25);

    damage.normal_damage = 110;
    health.DealDamage(damage, inflicted_damage);
    EXPECT_EQ(health.health, 0);
    EXPECT_TRUE(health.regenerative);
    EXPECT_FALSE(health.destroyed);
    EXPECT_TRUE(health.enabled);
    EXPECT_EQ(damage.normal_damage, 85);
}

TEST(Armor, Sanity)
{
    Damage damage;
    InflictedDamage inflicted_damage(3);
    damage.normal_damage = 10;
    EXPECT_EQ(damage.normal_damage, 10);

    Health health(100, 100, 0);
    health.vulnerabilities.normal_damage = 1;

    EXPECT_EQ(health.health, 100);
    EXPECT_EQ(health.max_health, 100);
    EXPECT_EQ(health.regeneration, 0);
    EXPECT_FALSE(health.regenerative);
    EXPECT_FALSE(health.destroyed);
    EXPECT_FALSE(health.enabled);

    health.DealDamage(damage, inflicted_damage);
    EXPECT_EQ(health.health, 90);
    EXPECT_EQ(damage.normal_damage, 0);

    health.Disable();
    health.Regenerate(1.0f);
    EXPECT_EQ(health.health, 90);
    EXPECT_FALSE(health.enabled);

    health.Enable();
    health.Regenerate(1.0f);
    EXPECT_EQ(health.health, 90);
    EXPECT_FALSE(health.enabled);

    damage.normal_damage = 110;
    health.DealDamage(damage, inflicted_damage);
    EXPECT_EQ(health.health, 0);
    EXPECT_FALSE(health.regenerative);
    EXPECT_TRUE(health.destroyed);
    EXPECT_FALSE(health.enabled);
    EXPECT_EQ(damage.normal_damage, 20);
}
