/*
 * upgrade_already_installed_tests.cpp
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
#include "components/shield.h"
#include "components/energy_container.h"
#include "components/ftl_drive.h"
#include "components/cloak.h"
#include "cmd/unit_csv_factory.h"

static const std::string upgrades_suffix = "__upgrades";

// Set up UnitCSVFactory entries so GetComponentTypeFromName returns the right type
static void registerUpgradeType(const std::string &name, const std::string &type) {
    std::map<std::string, std::string> data = {
        {"Upgrade_Type", type}
    };
    UnitCSVFactory::LoadUnit(name + upgrades_suffix, data);
}

// -------------------------------------------------------------------
// Armor tests: verify the conditions UpgradeAlreadyInstalled checks
// -------------------------------------------------------------------

// Ship-default armor with meaningful values (above 10.0 threshold) should
// have Installed()=true and TotalMaxLayerValue() > 10.0, so the base
// should NOT offer to sell a replacement.
TEST(UpgradeAlreadyInstalled, ShipDefaultArmorRecognized) {
    registerUpgradeType("armor01", "Armor");

    // "15/15/15" means value=15, adjusted_max=15, max=15 per facet.
    // 4 facets × 15 = 60.0 total, well above the 10.0 threshold.
    std::map<std::string, std::string> ship_data = {
        {"armor", "15/15/15"}
    };
    UnitCSVFactory::LoadUnit("test_ship_armor", ship_data);

    Armor armor;
    armor.Load("test_ship_armor");

    EXPECT_TRUE(armor.Installed());
    EXPECT_TRUE(armor.GetUpgradeKey().empty());
    EXPECT_GT(armor.TotalMaxLayerValue(), 10.0);
}

// Ship-default armor with negligible values (below 10.0 threshold) should
// have Installed()=true but TotalMaxLayerValue() < 10.0, so the base
// WILL offer to sell armor.
TEST(UpgradeAlreadyInstalled, NegligibleArmorNotRecognized) {
    registerUpgradeType("armor01b", "Armor");

    // "2/2/2" means value=2, adjusted_max=2, max=2 per facet.
    // 4 facets × 2 = 8.0 total, below the 10.0 threshold.
    std::map<std::string, std::string> ship_data = {
        {"armor", "2/2/2"}
    };
    UnitCSVFactory::LoadUnit("test_ship_armor_negligible", ship_data);

    Armor armor;
    armor.Load("test_ship_armor_negligible");

    EXPECT_TRUE(armor.Installed());
    EXPECT_TRUE(armor.GetUpgradeKey().empty());
    EXPECT_LT(armor.TotalMaxLayerValue(), 10.0);
}

// Armor at exactly the threshold (total 10.0) is NOT recognized since
// the check is > 10.0 (exclusive).
TEST(UpgradeAlreadyInstalled, ArmorAtThresholdNotRecognized) {
    registerUpgradeType("armor01c", "Armor");

    // "2.5/2.5/2.5" means max=2.5 per facet.
    // 4 facets × 2.5 = 10.0 total, exactly at the threshold.
    std::map<std::string, std::string> ship_data = {
        {"armor", "2.5/2.5/2.5"}
    };
    UnitCSVFactory::LoadUnit("test_ship_armor_threshold", ship_data);

    Armor armor;
    armor.Load("test_ship_armor_threshold");

    EXPECT_TRUE(armor.Installed());
    EXPECT_EQ(armor.TotalMaxLayerValue(), 10.0);
    // The threshold check is > 10.0, so exactly 10.0 is not recognized
}

// Purchased armor (via Upgrade) sets upgrade_key, so it is recognized
// regardless of TotalMaxLayerValue.
TEST(UpgradeAlreadyInstalled, PurchasedArmorRecognized) {
    registerUpgradeType("armor02", "Armor");

    std::map<std::string, std::string> armor_data = {
        {"armor", "100/100/100"}
    };
    UnitCSVFactory::LoadUnit("armor02" + upgrades_suffix, armor_data);

    Armor armor;
    armor.Upgrade("armor02" + upgrades_suffix);

    EXPECT_TRUE(armor.Installed());
    EXPECT_FALSE(armor.GetUpgradeKey().empty());
}

// Downgraded armor has Installed()=false and zero values, so it is NOT
// recognized as already installed.
TEST(UpgradeAlreadyInstalled, DowngradedArmorNotRecognized) {
    registerUpgradeType("armor03", "Armor");

    std::map<std::string, std::string> armor_data = {
        {"armor", "100/100/100"}
    };
    UnitCSVFactory::LoadUnit("armor03" + upgrades_suffix, armor_data);

    Armor armor;
    armor.Upgrade("armor03" + upgrades_suffix);
    armor.Downgrade();

    EXPECT_FALSE(armor.Installed());
    EXPECT_EQ(armor.TotalMaxLayerValue(), 0.0);
}

// Ship with no armor has Installed()=false, so the base WILL offer armor.
TEST(UpgradeAlreadyInstalled, NoArmorCanBuy) {
    registerUpgradeType("armor04", "Armor");

    std::map<std::string, std::string> no_armor_data = {
        {"Name", "NoArmorShip"}
    };
    UnitCSVFactory::LoadUnit("test_no_armor_ship", no_armor_data);

    Armor armor;
    armor.Load("test_no_armor_ship");

    EXPECT_FALSE(armor.Installed());
    EXPECT_EQ(armor.TotalMaxLayerValue(), 0.0);
}

// -------------------------------------------------------------------
// Shield tests: verify the conditions UpgradeAlreadyInstalled checks
// -------------------------------------------------------------------

// Ship-default shield with meaningful values (above 10.0 threshold) should
// have Installed()=true and TotalMaxLayerValue() > 10.0.
TEST(UpgradeAlreadyInstalled, ShipDefaultShieldRecognized) {
    registerUpgradeType("shield01", "Shield");

    // "10/10/10" means value=10, adjusted_max=10, max=10 per facet.
    // 4 facets × 10 = 40.0 total, well above the 10.0 threshold.
    std::map<std::string, std::string> ship_data = {
        {"shield", "10/10/10"},
        {"shield_facets", "4"},
        {"Shield_Recharge", "1/1/1"}
    };
    UnitCSVFactory::LoadUnit("test_ship_shield", ship_data);

    EnergyContainer container(ComponentType::Capacitor);
    FtlDrive ftl;
    Cloak cloak;
    Shield shield(&container, &ftl, &cloak);
    shield.Load("test_ship_shield");

    EXPECT_TRUE(shield.Installed());
    EXPECT_TRUE(shield.GetUpgradeKey().empty());
    EXPECT_GT(shield.TotalMaxLayerValue(), 10.0);
}

// Ship-default shield with negligible values (below 10.0 threshold) should
// have Installed()=true but TotalMaxLayerValue() < 10.0.
TEST(UpgradeAlreadyInstalled, NegligibleShieldNotRecognized) {
    registerUpgradeType("shield01b", "Shield");

    // "2/2/2" means max=2 per facet.
    // 4 facets × 2 = 8.0 total, below the 10.0 threshold.
    std::map<std::string, std::string> ship_data = {
        {"shield", "2/2/2"},
        {"shield_facets", "4"},
        {"Shield_Recharge", "1/1/1"}
    };
    UnitCSVFactory::LoadUnit("test_ship_shield_negligible", ship_data);

    EnergyContainer container(ComponentType::Capacitor);
    FtlDrive ftl;
    Cloak cloak;
    Shield shield(&container, &ftl, &cloak);
    shield.Load("test_ship_shield_negligible");

    EXPECT_TRUE(shield.Installed());
    EXPECT_TRUE(shield.GetUpgradeKey().empty());
    EXPECT_LT(shield.TotalMaxLayerValue(), 10.0);
}

// Shield at exactly the threshold (total 10.0) is NOT recognized since
// the check is > 10.0 (exclusive).
TEST(UpgradeAlreadyInstalled, ShieldAtThresholdNotRecognized) {
    registerUpgradeType("shield01c", "Shield");

    // "2.5/2.5/2.5" means max=2.5 per facet.
    // 4 facets × 2.5 = 10.0 total, exactly at the threshold.
    std::map<std::string, std::string> ship_data = {
        {"shield", "2.5/2.5/2.5"},
        {"shield_facets", "4"},
        {"Shield_Recharge", "1/1/1"}
    };
    UnitCSVFactory::LoadUnit("test_ship_shield_threshold", ship_data);

    EnergyContainer container(ComponentType::Capacitor);
    FtlDrive ftl;
    Cloak cloak;
    Shield shield(&container, &ftl, &cloak);
    shield.Load("test_ship_shield_threshold");

    EXPECT_TRUE(shield.Installed());
    EXPECT_EQ(shield.TotalMaxLayerValue(), 10.0);
    // The threshold check is > 10.0, so exactly 10.0 is not recognized
}

// Purchased shield (via Upgrade) sets upgrade_key, so it is recognized.
TEST(UpgradeAlreadyInstalled, PurchasedShieldRecognized) {
    registerUpgradeType("shield02", "Shield");

    std::map<std::string, std::string> shield_data = {
        {"shield", "100/100/100"},
        {"shield_facets", "4"},
        {"Shield_Recharge", "10/10/10"}
    };
    UnitCSVFactory::LoadUnit("shield02" + upgrades_suffix, shield_data);

    EnergyContainer container(ComponentType::Capacitor);
    FtlDrive ftl;
    Cloak cloak;
    Shield shield(&container, &ftl, &cloak);
    shield.Upgrade("shield02" + upgrades_suffix);

    EXPECT_TRUE(shield.Installed());
    EXPECT_FALSE(shield.GetUpgradeKey().empty());
}

// Downgraded shield has Installed()=false and zero values.
TEST(UpgradeAlreadyInstalled, DowngradedShieldNotRecognized) {
    registerUpgradeType("shield03", "Shield");

    std::map<std::string, std::string> shield_data = {
        {"shield", "100/100/100"},
        {"shield_facets", "4"},
        {"Shield_Recharge", "10/10/10"}
    };
    UnitCSVFactory::LoadUnit("shield03" + upgrades_suffix, shield_data);

    EnergyContainer container(ComponentType::Capacitor);
    FtlDrive ftl;
    Cloak cloak;
    Shield shield(&container, &ftl, &cloak);
    shield.Upgrade("shield03" + upgrades_suffix);
    shield.Downgrade();

    EXPECT_FALSE(shield.Installed());
    EXPECT_EQ(shield.TotalMaxLayerValue(), 0.0);
}

// Ship with no shield has Installed()=false.
TEST(UpgradeAlreadyInstalled, NoShieldCanBuy) {
    registerUpgradeType("shield04", "Shield");

    std::map<std::string, std::string> no_shield_data = {
        {"Name", "NoShieldShip"}
    };
    UnitCSVFactory::LoadUnit("test_no_shield_ship", no_shield_data);

    EnergyContainer container(ComponentType::Capacitor);
    FtlDrive ftl;
    Cloak cloak;
    Shield shield(&container, &ftl, &cloak);
    shield.Load("test_no_shield_ship");

    EXPECT_FALSE(shield.Installed());
    EXPECT_EQ(shield.TotalMaxLayerValue(), 0.0);
}
