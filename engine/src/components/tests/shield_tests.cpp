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
#include "components/energy_container.h"
#include "components/ftl_drive.h"
#include "components/cloak.h"
#include "cmd/unit_csv_factory.h"

static const std::string kUpgradesSuffix = "__upgrades";

TEST(Shield, LoadSingleValue) {
    std::map<std::string, std::string> shield_data = {
        {"shield", "100/100/100"},
        {"shield_facets", "4"},
        {"Shield_Recharge", "10/10/10"}
    };
    UnitCSVFactory::LoadUnit("test_shield_single" + kUpgradesSuffix, shield_data);

    EnergyContainer container(ComponentType::Capacitor);
    FtlDrive ftl;
    Cloak cloak;
    Shield shield(&container, &ftl, &cloak);
    shield.Load("test_shield_single" + kUpgradesSuffix);

    EXPECT_TRUE(shield.Installed());
    EXPECT_EQ(shield.TotalMaxLayerValue(), 400.0);  // 4 facets × 100
}

TEST(Shield, LoadNoShield) {
    std::map<std::string, std::string> no_shield_data = {
        {"Name", "NoShieldShip"}
    };
    UnitCSVFactory::LoadUnit("test_no_shield", no_shield_data);

    EnergyContainer container(ComponentType::Capacitor);
    FtlDrive ftl;
    Cloak cloak;
    Shield shield(&container, &ftl, &cloak);
    shield.Load("test_no_shield");

    // Should have 4 facets with 0 max values, number_of_facets should be 4
    EXPECT_FALSE(shield.Installed());
    EXPECT_EQ(shield.TotalMaxLayerValue(), 0.0);
}

TEST(Shield, UpgradeThenUpgradeDoesNotStack) {
    // This test would have caught the PR #1893 bug where facets.clear() was
    // removed but push_back was kept, causing facets to grow from 4 to 8.
    std::map<std::string, std::string> shield1_data = {
        {"shield", "50/50/50"},
        {"shield_facets", "4"},
        {"Shield_Recharge", "10/10/10"}
    };
    std::map<std::string, std::string> shield2_data = {
        {"shield", "100/100/100"},
        {"shield_facets", "4"},
        {"Shield_Recharge", "20/20/20"}
    };
    UnitCSVFactory::LoadUnit("test_shield1" + kUpgradesSuffix, shield1_data);
    UnitCSVFactory::LoadUnit("test_shield2" + kUpgradesSuffix, shield2_data);

    EnergyContainer container(ComponentType::Capacitor);
    FtlDrive ftl;
    Cloak cloak;
    Shield shield(&container, &ftl, &cloak);
    shield.Load("test_shield1" + kUpgradesSuffix);

    EXPECT_EQ(shield.TotalMaxLayerValue(), 200.0);  // 4 facets × 50

    // Upgrade to shield2
    shield.Upgrade("test_shield2" + kUpgradesSuffix);

    // Should still be 4 facets, each with max 100 = 400 total
    // Bug would produce 8 facets with total 600 (4×50 + 4×100)
    EXPECT_EQ(shield.TotalMaxLayerValue(), 400.0);
}

TEST(Shield, DowngradeZerosOut) {
    std::map<std::string, std::string> shield_data = {
        {"shield", "100/100/100"},
        {"shield_facets", "4"},
        {"Shield_Recharge", "10/10/10"}
    };
    UnitCSVFactory::LoadUnit("test_shield_downgrade" + kUpgradesSuffix, shield_data);

    EnergyContainer container(ComponentType::Capacitor);
    FtlDrive ftl;
    Cloak cloak;
    Shield shield(&container, &ftl, &cloak);
    shield.Load("test_shield_downgrade" + kUpgradesSuffix);
    EXPECT_TRUE(shield.Installed());

    shield.Downgrade();

    EXPECT_FALSE(shield.Installed());
    EXPECT_EQ(shield.TotalMaxLayerValue(), 0.0);
}

TEST(Shield, CanUpgradeAfterLoadWithNoShield) {
    // Ships without shields should still be able to upgrade to shields.
    // This test would have caught the bug where number_of_facets = 0
    // caused Shield::CanUpgrade to always return false.
    std::map<std::string, std::string> no_shield_data = {
        {"Name", "NoShieldShip"}
    };
    std::map<std::string, std::string> shield_upgrade_data = {
        {"shield", "50/50/50"},
        {"shield_facets", "4"},
        {"Shield_Recharge", "10/10/10"}
    };
    UnitCSVFactory::LoadUnit("test_no_shield_ship", no_shield_data);
    UnitCSVFactory::LoadUnit("test_shield_upgrade" + kUpgradesSuffix, shield_upgrade_data);

    EnergyContainer container(ComponentType::Capacitor);
    FtlDrive ftl;
    Cloak cloak;
    Shield shield(&container, &ftl, &cloak);
    shield.Load("test_no_shield_ship");

    EXPECT_FALSE(shield.Installed());
    EXPECT_TRUE(shield.CanUpgrade("test_shield_upgrade" + kUpgradesSuffix));
}
