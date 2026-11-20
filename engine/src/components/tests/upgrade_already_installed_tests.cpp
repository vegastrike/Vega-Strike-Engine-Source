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
#include "cmd/unit_csv_factory.h"
#include "resource/cargo.h"

static const std::string upgrades_suffix = "__upgrades";

// Set up UnitCSVFactory entries so GetComponentTypeFromName returns the right type
static void registerUpgradeType(const std::string &name, const std::string &type) {
    std::map<std::string, std::string> data = {
        {"Upgrade_Type", type}
    };
    UnitCSVFactory::LoadUnit(name + upgrades_suffix, data);
}

// Helper: create a Cargo that represents an armor upgrade
static Cargo makeArmorCargo(const std::string &name) {
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

// -------------------------------------------------------------------
// Armor tests: UpgradeAlreadyInstalled ("is it fitted?")
// -------------------------------------------------------------------

// Ship-default armor with meaningful values is recognized as fitted.
TEST(UpgradeAlreadyInstalled, ShipDefaultArmorRecognized) {
    registerUpgradeType("armor01", "Armor");

    // "15/15/15" means value=15, adjusted_max=15, max=15 per facet.
    // 4 facets × 15 = 60.0 total, well above the 10.0 threshold.
    std::map<std::string, std::string> ship_data = {
        {"armor", "15/15/15"}
    };
    UnitCSVFactory::LoadUnit("test_ship_armor", ship_data);

    ComponentsManager cm;
    cm.armor.Load("test_ship_armor");

    Cargo cargo = makeArmorCargo("armor01");
    EXPECT_TRUE(cm.UpgradeAlreadyInstalled(cargo));
}

// Ship-default armor with negligible values is NOT recognized as fitted.
TEST(UpgradeAlreadyInstalled, NegligibleArmorNotRecognized) {
    registerUpgradeType("armor01b", "Armor");

    // "2/2/2" means max=2 per facet.
    // 4 facets × 2 = 8.0 total, below the 10.0 threshold.
    std::map<std::string, std::string> ship_data = {
        {"armor", "2/2/2"}
    };
    UnitCSVFactory::LoadUnit("test_ship_armor_negligible", ship_data);

    ComponentsManager cm;
    cm.armor.Load("test_ship_armor_negligible");

    Cargo cargo = makeArmorCargo("armor01b");
    EXPECT_FALSE(cm.UpgradeAlreadyInstalled(cargo));
}

// Armor at exactly the threshold (total 10.0) is NOT recognized.
TEST(UpgradeAlreadyInstalled, ArmorAtThresholdNotRecognized) {
    registerUpgradeType("armor01c", "Armor");

    // "2.5/2.5/2.5" means max=2.5 per facet.
    // 4 facets × 2.5 = 10.0 total, exactly at the threshold.
    std::map<std::string, std::string> ship_data = {
        {"armor", "2.5/2.5/2.5"}
    };
    UnitCSVFactory::LoadUnit("test_ship_armor_threshold", ship_data);

    ComponentsManager cm;
    cm.armor.Load("test_ship_armor_threshold");

    Cargo cargo = makeArmorCargo("armor01c");
    EXPECT_FALSE(cm.UpgradeAlreadyInstalled(cargo));
}

// Purchased armor is recognized as fitted.
TEST(UpgradeAlreadyInstalled, PurchasedArmorRecognized) {
    registerUpgradeType("armor02", "Armor");

    std::map<std::string, std::string> armor_data = {
        {"armor", "100/100/100"}
    };
    UnitCSVFactory::LoadUnit("armor02" + upgrades_suffix, armor_data);

    ComponentsManager cm;
    cm.armor.Upgrade("armor02" + upgrades_suffix);

    Cargo cargo = makeArmorCargo("armor02");
    EXPECT_TRUE(cm.UpgradeAlreadyInstalled(cargo));
}

// Downgraded armor is NOT recognized as fitted.
TEST(UpgradeAlreadyInstalled, DowngradedArmorNotRecognized) {
    registerUpgradeType("armor03", "Armor");

    std::map<std::string, std::string> armor_data = {
        {"armor", "100/100/100"}
    };
    UnitCSVFactory::LoadUnit("armor03" + upgrades_suffix, armor_data);

    ComponentsManager cm;
    cm.armor.Upgrade("armor03" + upgrades_suffix);
    cm.armor.Downgrade();

    Cargo cargo = makeArmorCargo("armor03");
    EXPECT_FALSE(cm.UpgradeAlreadyInstalled(cargo));
}

// Ship with no armor can buy armor.
TEST(UpgradeAlreadyInstalled, NoArmorCanBuy) {
    registerUpgradeType("armor04", "Armor");

    std::map<std::string, std::string> no_armor_data = {
        {"Name", "NoArmorShip"}
    };
    UnitCSVFactory::LoadUnit("test_no_armor_ship", no_armor_data);

    ComponentsManager cm;
    cm.armor.Load("test_no_armor_ship");

    Cargo cargo = makeArmorCargo("armor04");
    EXPECT_FALSE(cm.UpgradeAlreadyInstalled(cargo));
}

// -------------------------------------------------------------------
// Shield tests: UpgradeAlreadyInstalled ("is it fitted?")
// -------------------------------------------------------------------

// Ship-default shield with meaningful values is recognized as fitted.
TEST(UpgradeAlreadyInstalled, ShipDefaultShieldRecognized) {
    registerUpgradeType("shield01", "Shield");

    // "10/10/10" means max=10 per facet.
    // 4 facets × 10 = 40.0 total, well above the 10.0 threshold.
    std::map<std::string, std::string> ship_data = {
        {"shield", "10/10/10"},
        {"shield_facets", "4"},
        {"Shield_Recharge", "1/1/1"}
    };
    UnitCSVFactory::LoadUnit("test_ship_shield", ship_data);

    ComponentsManager cm;
    cm.shield.Load("test_ship_shield");

    Cargo cargo = makeShieldCargo("shield01");
    EXPECT_TRUE(cm.UpgradeAlreadyInstalled(cargo));
}

// Ship-default shield with negligible values is NOT recognized as fitted.
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

    ComponentsManager cm;
    cm.shield.Load("test_ship_shield_negligible");

    Cargo cargo = makeShieldCargo("shield01b");
    EXPECT_FALSE(cm.UpgradeAlreadyInstalled(cargo));
}

// Shield at exactly the threshold (total 10.0) is NOT recognized.
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

    ComponentsManager cm;
    cm.shield.Load("test_ship_shield_threshold");

    Cargo cargo = makeShieldCargo("shield01c");
    EXPECT_FALSE(cm.UpgradeAlreadyInstalled(cargo));
}

// Purchased shield is recognized as fitted.
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

    Cargo cargo = makeShieldCargo("shield02");
    EXPECT_TRUE(cm.UpgradeAlreadyInstalled(cargo));
}

// Downgraded shield is NOT recognized as fitted.
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

    Cargo cargo = makeShieldCargo("shield03");
    EXPECT_FALSE(cm.UpgradeAlreadyInstalled(cargo));
}

// Ship with no shield can buy shield.
TEST(UpgradeAlreadyInstalled, NoShieldCanBuy) {
    registerUpgradeType("shield04", "Shield");

    std::map<std::string, std::string> no_shield_data = {
        {"Name", "NoShieldShip"}
    };
    UnitCSVFactory::LoadUnit("test_no_shield_ship", no_shield_data);

    ComponentsManager cm;
    cm.shield.Load("test_no_shield_ship");

    Cargo cargo = makeShieldCargo("shield04");
    EXPECT_FALSE(cm.UpgradeAlreadyInstalled(cargo));
}

// -------------------------------------------------------------------
// Armor tests: UpgradeAlreadyBought ("was it bought?")
// -------------------------------------------------------------------

// Ship-default armor was NOT bought (upgrade_key is empty).
TEST(UpgradeAlreadyBought, ShipDefaultArmorNotBought) {
    registerUpgradeType("armor10", "Armor");

    std::map<std::string, std::string> ship_data = {
        {"armor", "15/15/15"}
    };
    UnitCSVFactory::LoadUnit("test_ship_armor_bought", ship_data);

    ComponentsManager cm;
    cm.armor.Load("test_ship_armor_bought");

    Cargo cargo = makeArmorCargo("armor10");
    EXPECT_FALSE(cm.UpgradeAlreadyBought(cargo));
}

// Purchased armor WAS bought (upgrade_key is set).
TEST(UpgradeAlreadyBought, PurchasedArmorBought) {
    registerUpgradeType("armor11", "Armor");

    std::map<std::string, std::string> armor_data = {
        {"armor", "100/100/100"}
    };
    UnitCSVFactory::LoadUnit("armor11" + upgrades_suffix, armor_data);

    ComponentsManager cm;
    cm.armor.Upgrade("armor11" + upgrades_suffix);

    Cargo cargo = makeArmorCargo("armor11");
    EXPECT_TRUE(cm.UpgradeAlreadyBought(cargo));
}

// Downgraded armor was NOT bought (not installed).
TEST(UpgradeAlreadyBought, DowngradedArmorNotBought) {
    registerUpgradeType("armor12", "Armor");

    std::map<std::string, std::string> armor_data = {
        {"armor", "100/100/100"}
    };
    UnitCSVFactory::LoadUnit("armor12" + upgrades_suffix, armor_data);

    ComponentsManager cm;
    cm.armor.Upgrade("armor12" + upgrades_suffix);
    cm.armor.Downgrade();

    Cargo cargo = makeArmorCargo("armor12");
    EXPECT_FALSE(cm.UpgradeAlreadyBought(cargo));
}

// Ship with no armor was NOT bought.
TEST(UpgradeAlreadyBought, NoArmorNotBought) {
    registerUpgradeType("armor13", "Armor");

    std::map<std::string, std::string> no_armor_data = {
        {"Name", "NoArmorShipBought"}
    };
    UnitCSVFactory::LoadUnit("test_no_armor_ship_bought", no_armor_data);

    ComponentsManager cm;
    cm.armor.Load("test_no_armor_ship_bought");

    Cargo cargo = makeArmorCargo("armor13");
    EXPECT_FALSE(cm.UpgradeAlreadyBought(cargo));
}

// -------------------------------------------------------------------
// Shield tests: UpgradeAlreadyBought ("was it bought?")
// -------------------------------------------------------------------

// Ship-default shield was NOT bought (upgrade_key is empty).
TEST(UpgradeAlreadyBought, ShipDefaultShieldNotBought) {
    registerUpgradeType("shield10", "Shield");

    std::map<std::string, std::string> ship_data = {
        {"shield", "10/10/10"},
        {"shield_facets", "4"},
        {"Shield_Recharge", "1/1/1"}
    };
    UnitCSVFactory::LoadUnit("test_ship_shield_bought", ship_data);

    ComponentsManager cm;
    cm.shield.Load("test_ship_shield_bought");

    Cargo cargo = makeShieldCargo("shield10");
    EXPECT_FALSE(cm.UpgradeAlreadyBought(cargo));
}

// Purchased shield WAS bought (upgrade_key is set).
TEST(UpgradeAlreadyBought, PurchasedShieldBought) {
    registerUpgradeType("shield11", "Shield");

    std::map<std::string, std::string> shield_data = {
        {"shield", "100/100/100"},
        {"shield_facets", "4"},
        {"Shield_Recharge", "10/10/10"}
    };
    UnitCSVFactory::LoadUnit("shield11" + upgrades_suffix, shield_data);

    ComponentsManager cm;
    cm.shield.Upgrade("shield11" + upgrades_suffix);

    Cargo cargo = makeShieldCargo("shield11");
    EXPECT_TRUE(cm.UpgradeAlreadyBought(cargo));
}

// Downgraded shield was NOT bought (not installed).
TEST(UpgradeAlreadyBought, DowngradedShieldNotBought) {
    registerUpgradeType("shield12", "Shield");

    std::map<std::string, std::string> shield_data = {
        {"shield", "100/100/100"},
        {"shield_facets", "4"},
        {"Shield_Recharge", "10/10/10"}
    };
    UnitCSVFactory::LoadUnit("shield12" + upgrades_suffix, shield_data);

    ComponentsManager cm;
    cm.shield.Upgrade("shield12" + upgrades_suffix);
    cm.shield.Downgrade();

    Cargo cargo = makeShieldCargo("shield12");
    EXPECT_FALSE(cm.UpgradeAlreadyBought(cargo));
}

// Ship with no shield was NOT bought.
TEST(UpgradeAlreadyBought, NoShieldNotBought) {
    registerUpgradeType("shield13", "Shield");

    std::map<std::string, std::string> no_shield_data = {
        {"Name", "NoShieldShipBought"}
    };
    UnitCSVFactory::LoadUnit("test_no_shield_ship_bought", no_shield_data);

    ComponentsManager cm;
    cm.shield.Load("test_no_shield_ship_bought");

    Cargo cargo = makeShieldCargo("shield13");
    EXPECT_FALSE(cm.UpgradeAlreadyBought(cargo));
}
