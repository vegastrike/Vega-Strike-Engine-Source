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

#include "components/components_manager.h"
#include "components/energy_container.h"
#include "components/ftl_drive.h"
#include "components/cloak.h"
#include "cmd/unit_csv_factory.h"
#include "resource/cargo.h"

static const std::string upgrades_suffix = "__upgrades";

// Helper: create a Cargo that represents an armor upgrade
static Cargo makeArmorCargo(const std::string &name) {
    // Cargo(name, description, quantity, price, category, mass, volume, ...)
    Cargo cargo;
    cargo.SetName(name);
    cargo.SetCategory("Armor");
    cargo.SetPrice(100000);
    cargo.SetQuantity(1);
    cargo.SetMass(10.0);
    return cargo;
}

// Helper: create a Cargo that represents a shield upgrade
static Cargo makeShieldCargo(const std::string &name) {
    Cargo cargo;
    cargo.SetName(name);
    cargo.SetCategory("Shield");
    cargo.SetPrice(100000);
    cargo.SetQuantity(1);
    cargo.SetMass(10.0);
    return cargo;
}

// Helper: create a Cargo that represents an unknown upgrade type
static Cargo makeUnknownCargo(const std::string &name) {
    Cargo cargo;
    cargo.SetName(name);
    cargo.SetCategory("Unknown");
    cargo.SetPrice(100000);
    cargo.SetQuantity(1);
    cargo.SetMass(10.0);
    return cargo;
}

// Set up UnitCSVFactory entries so GetComponentTypeFromName returns the right type
static void registerUpgradeType(const std::string &name, const std::string &type) {
    std::map<std::string, std::string> data = {
        {"Upgrade_Type", type}
    };
    UnitCSVFactory::LoadUnit(name + upgrades_suffix, data);
}

// Test: ship-default armor with meaningful values (above 10.0 threshold) is recognized as installed
TEST(UpgradeAlreadyInstalled, ShipDefaultArmorRecognized) {
    // Register the upgrade type so GetComponentTypeFromName returns Armor
    registerUpgradeType("armor01", "Armor");

    // Ship has default armor with values above the 10.0 meaningful threshold
    std::map<std::string, std::string> ship_data = {
        {"armor", "15/15/15"}
    };
    UnitCSVFactory::LoadUnit("test_ship_armor", ship_data);

    ComponentsManager cm;
    cm.armor.Load("test_ship_armor");

    // Armor is installed from ship definition, but upgrade_key is empty
    EXPECT_TRUE(cm.armor.Installed());
    EXPECT_TRUE(cm.armor.GetUpgradeKey().empty());
    EXPECT_GT(cm.armor.TotalMaxLayerValue(), 10.0);

    // UpgradeAlreadyInstalled should return true — the base should NOT offer to sell this armor
    Cargo cargo = makeArmorCargo("armor01");
    EXPECT_TRUE(cm.UpgradeAlreadyInstalled(cargo));
}

// Test: ship-default armor with negligible values (below 10.0 threshold) is NOT recognized
TEST(UpgradeAlreadyInstalled, NegligibleArmorNotRecognized) {
    registerUpgradeType("armor01b", "Armor");

    // Ship has default armor with minimal values typical of built-in armor (total 6.0)
    std::map<std::string, std::string> ship_data = {
        {"armor", "2/2/2"}
    };
    UnitCSVFactory::LoadUnit("test_ship_armor_negligible", ship_data);

    ComponentsManager cm;
    cm.armor.Load("test_ship_armor_negligible");

    EXPECT_TRUE(cm.armor.Installed());
    EXPECT_TRUE(cm.armor.GetUpgradeKey().empty());
    EXPECT_LT(cm.armor.TotalMaxLayerValue(), 10.0);

    // Values below 10.0 are treated as negligible, so the base WILL offer to sell armor
    Cargo cargo = makeArmorCargo("armor01b");
    EXPECT_FALSE(cm.UpgradeAlreadyInstalled(cargo));
}

// Test: armor at exactly the threshold (10.0) is NOT recognized (threshold is exclusive)
TEST(UpgradeAlreadyInstalled, ArmorAtThresholdNotRecognized) {
    registerUpgradeType("armor01c", "Armor");

    // TotalMaxLayerValue will be exactly 10.0
    std::map<std::string, std::string> ship_data = {
        {"armor", "10/0/0"}
    };
    UnitCSVFactory::LoadUnit("test_ship_armor_threshold", ship_data);

    ComponentsManager cm;
    cm.armor.Load("test_ship_armor_threshold");

    EXPECT_TRUE(cm.armor.Installed());
    EXPECT_EQ(cm.armor.TotalMaxLayerValue(), 10.0);

    // The threshold check is > 10.0, so exactly 10.0 is not recognized
    Cargo cargo = makeArmorCargo("armor01c");
    EXPECT_FALSE(cm.UpgradeAlreadyInstalled(cargo));
}

// Test: purchased armor (upgrade_key set) is recognized as installed
TEST(UpgradeAlreadyInstalled, PurchasedArmorRecognized) {
    registerUpgradeType("armor02", "Armor");

    std::map<std::string, std::string> armor_data = {
        {"armor", "100/100/100"}
    };
    UnitCSVFactory::LoadUnit("armor02" + upgrades_suffix, armor_data);

    ComponentsManager cm;
    // Simulate purchasing and installing armor
    cm.armor.Upgrade("armor02" + upgrades_suffix);

    EXPECT_TRUE(cm.armor.Installed());
    EXPECT_FALSE(cm.armor.GetUpgradeKey().empty());

    Cargo cargo = makeArmorCargo("armor02");
    EXPECT_TRUE(cm.UpgradeAlreadyInstalled(cargo));
}

// Test: downgraded armor (not installed, zero values) is NOT recognized as installed
TEST(UpgradeAlreadyInstalled, DowngradedArmorNotRecognized) {
    registerUpgradeType("armor03", "Armor");

    std::map<std::string, std::string> armor_data = {
        {"armor", "100/100/100"}
    };
    UnitCSVFactory::LoadUnit("armor03" + upgrades_suffix, armor_data);

    ComponentsManager cm;
    cm.armor.Upgrade("armor03" + upgrades_suffix);
    cm.armor.Downgrade();

    EXPECT_FALSE(cm.armor.Installed());
    EXPECT_EQ(cm.armor.TotalMaxLayerValue(), 0.0);

    Cargo cargo = makeArmorCargo("armor03");
    EXPECT_FALSE(cm.UpgradeAlreadyInstalled(cargo));
}

// Test: ship with no armor can buy armor
TEST(UpgradeAlreadyInstalled, NoArmorCanBuy) {
    registerUpgradeType("armor04", "Armor");

    std::map<std::string, std::string> no_armor_data = {
        {"Name", "NoArmorShip"}
    };
    UnitCSVFactory::LoadUnit("test_no_armor_ship", no_armor_data);

    ComponentsManager cm;
    cm.armor.Load("test_no_armor_ship");

    EXPECT_FALSE(cm.armor.Installed());
    EXPECT_EQ(cm.armor.TotalMaxLayerValue(), 0.0);

    Cargo cargo = makeArmorCargo("armor04");
    EXPECT_FALSE(cm.UpgradeAlreadyInstalled(cargo));
}

// Test: ship-default shield with meaningful values (above 10.0 threshold) is recognized as installed
TEST(UpgradeAlreadyInstalled, ShipDefaultShieldRecognized) {
    registerUpgradeType("shield01", "Shield");

    // Ship has default shield with values above the 10.0 meaningful threshold
    std::map<std::string, std::string> ship_data = {
        {"shield", "10/10/10"},
        {"shield_facets", "4"},
        {"Shield_Recharge", "1/1/1"}
    };
    UnitCSVFactory::LoadUnit("test_ship_shield", ship_data);

    ComponentsManager cm;
    cm.shield.Load("test_ship_shield");

    EXPECT_TRUE(cm.shield.Installed());
    EXPECT_TRUE(cm.shield.GetUpgradeKey().empty());
    EXPECT_GT(cm.shield.TotalMaxLayerValue(), 10.0);

    Cargo cargo = makeShieldCargo("shield01");
    EXPECT_TRUE(cm.UpgradeAlreadyInstalled(cargo));
}

// Test: ship-default shield with negligible values (below 10.0 threshold) is NOT recognized
TEST(UpgradeAlreadyInstalled, NegligibleShieldNotRecognized) {
    registerUpgradeType("shield01b", "Shield");

    // Ship has default shield with minimal values (total 6.0, below threshold)
    std::map<std::string, std::string> ship_data = {
        {"shield", "2/2/2"},
        {"shield_facets", "4"},
        {"Shield_Recharge", "1/1/1"}
    };
    UnitCSVFactory::LoadUnit("test_ship_shield_negligible", ship_data);

    ComponentsManager cm;
    cm.shield.Load("test_ship_shield_negligible");

    EXPECT_TRUE(cm.shield.Installed());
    EXPECT_TRUE(cm.shield.GetUpgradeKey().empty());
    EXPECT_LT(cm.shield.TotalMaxLayerValue(), 10.0);

    // Values below 10.0 are treated as negligible, so the base WILL offer to sell shield
    Cargo cargo = makeShieldCargo("shield01b");
    EXPECT_FALSE(cm.UpgradeAlreadyInstalled(cargo));
}

// Test: shield at exactly the threshold (10.0) is NOT recognized (threshold is exclusive)
TEST(UpgradeAlreadyInstalled, ShieldAtThresholdNotRecognized) {
    registerUpgradeType("shield01c", "Shield");

    // TotalMaxLayerValue will be exactly 10.0
    std::map<std::string, std::string> ship_data = {
        {"shield", "10/0/0"},
        {"shield_facets", "4"},
        {"Shield_Recharge", "1/1/1"}
    };
    UnitCSVFactory::LoadUnit("test_ship_shield_threshold", ship_data);

    ComponentsManager cm;
    cm.shield.Load("test_ship_shield_threshold");

    EXPECT_TRUE(cm.shield.Installed());
    EXPECT_EQ(cm.shield.TotalMaxLayerValue(), 10.0);

    // The threshold check is > 10.0, so exactly 10.0 is not recognized
    Cargo cargo = makeShieldCargo("shield01c");
    EXPECT_FALSE(cm.UpgradeAlreadyInstalled(cargo));
}

// Test: purchased shield (upgrade_key set) is recognized as installed
TEST(UpgradeAlreadyInstalled, PurchasedShieldRecognized) {
    registerUpgradeType("shield02", "Shield");

    std::map<std::string, std::string> shield_data = {
        {"shield", "100/100/100"},
        {"shield_facets", "4"},
        {"Shield_Recharge", "10/10/10"}
    };
    UnitCSVFactory::LoadUnit("shield02" + upgrades_suffix, shield_data);

    ComponentsManager cm;
    cm.shield.Upgrade("shield02" + upgrades_suffix);

    EXPECT_TRUE(cm.shield.Installed());
    EXPECT_FALSE(cm.shield.GetUpgradeKey().empty());

    Cargo cargo = makeShieldCargo("shield02");
    EXPECT_TRUE(cm.UpgradeAlreadyInstalled(cargo));
}

// Test: downgraded shield (not installed, zero values) is NOT recognized as installed
TEST(UpgradeAlreadyInstalled, DowngradedShieldNotRecognized) {
    registerUpgradeType("shield03", "Shield");

    std::map<std::string, std::string> shield_data = {
        {"shield", "100/100/100"},
        {"shield_facets", "4"},
        {"Shield_Recharge", "10/10/10"}
    };
    UnitCSVFactory::LoadUnit("shield03" + upgrades_suffix, shield_data);

    ComponentsManager cm;
    cm.shield.Upgrade("shield03" + upgrades_suffix);
    cm.shield.Downgrade();

    EXPECT_FALSE(cm.shield.Installed());
    EXPECT_EQ(cm.shield.TotalMaxLayerValue(), 0.0);

    Cargo cargo = makeShieldCargo("shield03");
    EXPECT_FALSE(cm.UpgradeAlreadyInstalled(cargo));
}

// Test: ship with no shield can buy shield
TEST(UpgradeAlreadyInstalled, NoShieldCanBuy) {
    registerUpgradeType("shield04", "Shield");

    std::map<std::string, std::string> no_shield_data = {
        {"Name", "NoShieldShip"}
    };
    UnitCSVFactory::LoadUnit("test_no_shield_ship", no_shield_data);

    ComponentsManager cm;
    cm.shield.Load("test_no_shield_ship");

    EXPECT_FALSE(cm.shield.Installed());
    EXPECT_EQ(cm.shield.TotalMaxLayerValue(), 0.0);

    Cargo cargo = makeShieldCargo("shield04");
    EXPECT_FALSE(cm.UpgradeAlreadyInstalled(cargo));
}

// Test: unknown component type returns true (err on the side of caution)
TEST(UpgradeAlreadyInstalled, UnknownTypeReturnsTrue) {
    registerUpgradeType("unknown01", "None");

    ComponentsManager cm;
    Cargo cargo = makeUnknownCargo("unknown01");
    // GetComponentByType returns nullptr for unknown types, so UpgradeAlreadyInstalled returns true
    EXPECT_TRUE(cm.UpgradeAlreadyInstalled(cargo));
}
