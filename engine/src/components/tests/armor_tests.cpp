/*
 * armor_tests.cpp
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

#include "components/armor.h"
#include "cmd/unit_csv_factory.h"

static const std::string kUpgradesSuffix = "__upgrades";

TEST(Armor, LoadSingleValue) {
    std::map<std::string, std::string> armor_data = {
        {"armor", "100/100/100"}
    };
    UnitCSVFactory::LoadUnit("test_armor_single" + kUpgradesSuffix, armor_data);

    Armor armor;
    armor.Load("test_armor_single" + kUpgradesSuffix);

    EXPECT_TRUE(armor.Installed());
    EXPECT_EQ(armor.TotalMaxLayerValue(), 400.0);  // 4 facets × 100
}

TEST(Armor, LoadNoArmor) {
    std::map<std::string, std::string> no_armor_data = {
        {"Name", "NoArmorShip"}
    };
    UnitCSVFactory::LoadUnit("test_no_armor", no_armor_data);

    Armor armor;
    armor.Load("test_no_armor");

    // Should have 4 facets with 0 max values
    EXPECT_FALSE(armor.Installed());
    EXPECT_EQ(armor.TotalMaxLayerValue(), 0.0);
}

TEST(Armor, UpgradeThenUpgradeDoesNotStack) {
    // This test would have caught the PR #1893 bug where facets.clear() was
    // removed but push_back was kept, causing facets to grow from 4 to 8.
    std::map<std::string, std::string> armor1_data = {
        {"armor", "50/50/50"}
    };
    std::map<std::string, std::string> armor2_data = {
        {"armor", "100/100/100"}
    };
    UnitCSVFactory::LoadUnit("test_armor1" + kUpgradesSuffix, armor1_data);
    UnitCSVFactory::LoadUnit("test_armor2" + kUpgradesSuffix, armor2_data);

    Armor armor;
    armor.Load("test_armor1" + kUpgradesSuffix);

    EXPECT_EQ(armor.TotalMaxLayerValue(), 200.0);  // 4 facets × 50

    // Upgrade to armor2
    armor.Upgrade("test_armor2" + kUpgradesSuffix);

    // Should still be 4 facets, each with max 100 = 400 total
    // Bug would produce 8 facets with total 600 (4×50 + 4×100)
    EXPECT_EQ(armor.TotalMaxLayerValue(), 400.0);
}

TEST(Armor, DowngradeZerosOut) {
    std::map<std::string, std::string> armor_data = {
        {"armor", "100/100/100"}
    };
    UnitCSVFactory::LoadUnit("test_armor_downgrade" + kUpgradesSuffix, armor_data);

    Armor armor;
    armor.Load("test_armor_downgrade" + kUpgradesSuffix);
    EXPECT_TRUE(armor.Installed());

    armor.Downgrade();

    EXPECT_FALSE(armor.Installed());
    EXPECT_EQ(armor.TotalMaxLayerValue(), 0.0);
}

TEST(Armor, CanUpgradeAfterLoadWithNoArmor) {
    // Ships without armor should still be able to upgrade to armor.
    std::map<std::string, std::string> no_armor_data = {
        {"Name", "NoArmorShip"}
    };
    std::map<std::string, std::string> armor_upgrade_data = {
        {"armor", "50/50/50"}
    };
    UnitCSVFactory::LoadUnit("test_no_armor_ship", no_armor_data);
    UnitCSVFactory::LoadUnit("test_armor_upgrade" + kUpgradesSuffix, armor_upgrade_data);

    Armor armor;
    armor.Load("test_no_armor_ship");

    EXPECT_FALSE(armor.Installed());
    EXPECT_TRUE(armor.CanUpgrade("test_armor_upgrade" + kUpgradesSuffix));
}
