/*
 * shield_tests.cpp
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
#include <map>

#include "components/shield.h"

#include "cmd/unit_csv_factory.h"

static const std::string shield_key = "shield_test__upgrades";

static std::map<std::string, std::string> shield_map = {
    { "Name", "Shield"},
    { "shield", "100"},
    { "shield_facets", "4"},
    { "Shield_Recharge", "10"}
};

// Repairing a damaged shield restores every facet and the regeneration
TEST(Shield, RepairDamaged) {
    UnitCSVFactory::LoadUnit(shield_key, shield_map);

    Shield shield(nullptr, nullptr, nullptr);
    shield.Load(shield_key);
    EXPECT_EQ(shield.TotalMaxLayerValue(), 400.0);

    shield.DamageByPercent(0.5);
    EXPECT_TRUE(shield.Damaged());

    shield.Repair();
    EXPECT_EQ(shield.TotalLayerValue(), 400.0);
    EXPECT_FALSE(shield.Damaged());
    EXPECT_EQ(shield.PercentOperational(), 1.0);
}

// Damage over 95% destroys the facets and the regeneration, which repair
// doesn't restore, so the shield still reports itself damaged
TEST(Shield, RepairDestroyed) {
    UnitCSVFactory::LoadUnit(shield_key, shield_map);

    Shield shield(nullptr, nullptr, nullptr);
    shield.Load(shield_key);

    shield.DamageByPercent(0.96);
    EXPECT_EQ(shield.TotalLayerValue(), 0.0);

    shield.Repair();
    EXPECT_EQ(shield.TotalLayerValue(), 0.0);
    EXPECT_TRUE(shield.Damaged());
    EXPECT_EQ(shield.PercentOperational(), 0.0);
}
