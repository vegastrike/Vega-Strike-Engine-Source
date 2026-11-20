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

static const std::string kUpgradesSuffix = "__upgrades";

// Minimal mock of Unit that inherits from ComponentsManager.
// Unit is a massive class with dozens of base classes and heavy dependencies,
// so we mock only what UpgradeAlreadyInstalled/UpgradeAlreadyBought need:
// the ComponentsManager base and an overridable GetUnitKey().
class MockUnit : public ComponentsManager {
    std::string unit_key_;

public:
    explicit MockUnit(const std::string &key) : unit_key_(key) {}

    const std::string &GetUnitKey() const override {
        return unit_key_;
    }
};

// Set up UnitCSVFactory entries so GetComponentTypeFromName returns the right type
static void RegisterUpgradeType(const std::string &name, const std::string &type) {
    std::map<std::string, std::string> data = {
        {"Upgrade_Type", type}
    };
    UnitCSVFactory::LoadUnit(name + kUpgradesSuffix, data);
}

// Helper: create a Cargo that represents an armor upgrade
static Cargo MakeArmorCargo(const std::string &name) {
    Cargo cargo;
    cargo.SetName(name);
    cargo.SetCategory("Armor");
    cargo.SetPrice(100000);
    cargo.SetQuantity(1);
    cargo.SetMass(10.0);
    return cargo;
}

// Helper: create a Cargo that represents a shield upgrade
static Cargo MakeShieldCargo(const std::string &name) {
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
    RegisterUpgradeType("armor01", "Armor");

    // "15/15/15" means value=15, adjusted_max=15, max=15 per facet.
    // 4 facets × 15 = 60.0 total, well above the 10.0 threshold.
    std::map<std::string, std::string> ship_data = {
        {"armor", "15/15/15"}
    };
    UnitCSVFactory::LoadUnit("test_ship_armor", ship_data);

    MockUnit mock_unit("test_ship_armor");
    mock_unit.armor.Load("test_ship_armor");

    Cargo cargo = MakeArmorCargo("armor01");
    EXPECT_TRUE(mock_unit.UpgradeAlreadyInstalled(cargo));
}

// Ship-default armor with negligible values is NOT recognized as fitted.
TEST(UpgradeAlreadyInstalled, NegligibleArmorNotRecognized) {
    RegisterUpgradeType("armor01b", "Armor");

    // "2/2/2" means max=2 per facet.
    // 4 facets × 2 = 8.0 total, below the 10.0 threshold.
    std::map<std::string, std::string> ship_data = {
        {"armor", "2/2/2"}
    };
    UnitCSVFactory::LoadUnit("test_ship_armor_negligible", ship_data);

    MockUnit mock_unit("test_ship_armor_negligible");
    mock_unit.armor.Load("test_ship_armor_negligible");

    Cargo cargo = MakeArmorCargo("armor01b");
    EXPECT_FALSE(mock_unit.UpgradeAlreadyInstalled(cargo));
}

// Armor at exactly the threshold (total 10.0) is NOT recognized.
TEST(UpgradeAlreadyInstalled, ArmorAtThresholdNotRecognized) {
    RegisterUpgradeType("armor01c", "Armor");

    // "2.5/2.5/2.5" means max=2.5 per facet.
    // 4 facets × 2.5 = 10.0 total, exactly at the threshold.
    std::map<std::string, std::string> ship_data = {
        {"armor", "2.5/2.5/2.5"}
    };
    UnitCSVFactory::LoadUnit("test_ship_armor_threshold", ship_data);

    MockUnit mock_unit("test_ship_armor_threshold");
    mock_unit.armor.Load("test_ship_armor_threshold");

    Cargo cargo = MakeArmorCargo("armor01c");
    EXPECT_FALSE(mock_unit.UpgradeAlreadyInstalled(cargo));
}

// Purchased armor is recognized as fitted.
TEST(UpgradeAlreadyInstalled, PurchasedArmorRecognized) {
    RegisterUpgradeType("armor02", "Armor");

    std::map<std::string, std::string> armor_data = {
        {"armor", "100/100/100"}
    };
    UnitCSVFactory::LoadUnit("armor02" + kUpgradesSuffix, armor_data);

    MockUnit mock_unit("test_ship_purchased_armor");
    mock_unit.armor.Upgrade("armor02" + kUpgradesSuffix);

    Cargo cargo = MakeArmorCargo("armor02");
    EXPECT_TRUE(mock_unit.UpgradeAlreadyInstalled(cargo));
}

// Downgraded armor is NOT recognized as fitted.
TEST(UpgradeAlreadyInstalled, DowngradedArmorNotRecognized) {
    RegisterUpgradeType("armor03", "Armor");

    std::map<std::string, std::string> armor_data = {
        {"armor", "100/100/100"}
    };
    UnitCSVFactory::LoadUnit("armor03" + kUpgradesSuffix, armor_data);

    MockUnit mock_unit("test_ship_downgraded_armor");
    mock_unit.armor.Upgrade("armor03" + kUpgradesSuffix);
    mock_unit.armor.Downgrade();

    Cargo cargo = MakeArmorCargo("armor03");
    EXPECT_FALSE(mock_unit.UpgradeAlreadyInstalled(cargo));
}

// Ship with no armor can buy armor.
TEST(UpgradeAlreadyInstalled, NoArmorCanBuy) {
    RegisterUpgradeType("armor04", "Armor");

    std::map<std::string, std::string> no_armor_data = {
        {"Name", "NoArmorShip"}
    };
    UnitCSVFactory::LoadUnit("test_no_armor_ship", no_armor_data);

    MockUnit mock_unit("test_no_armor_ship");
    mock_unit.armor.Load("test_no_armor_ship");

    Cargo cargo = MakeArmorCargo("armor04");
    EXPECT_FALSE(mock_unit.UpgradeAlreadyInstalled(cargo));
}

// -------------------------------------------------------------------
// Shield tests: UpgradeAlreadyInstalled ("is it fitted?")
// -------------------------------------------------------------------

// Ship-default shield with meaningful values is recognized as fitted.
TEST(UpgradeAlreadyInstalled, ShipDefaultShieldRecognized) {
    RegisterUpgradeType("shield01", "Shield");

    // "10/10/10" means max=10 per facet.
    // 4 facets × 10 = 40.0 total, well above the 10.0 threshold.
    std::map<std::string, std::string> ship_data = {
        {"shield", "10/10/10"},
        {"shield_facets", "4"},
        {"Shield_Recharge", "1/1/1"}
    };
    UnitCSVFactory::LoadUnit("test_ship_shield", ship_data);

    MockUnit mock_unit("test_ship_shield");
    mock_unit.shield.Load("test_ship_shield");

    Cargo cargo = MakeShieldCargo("shield01");
    EXPECT_TRUE(mock_unit.UpgradeAlreadyInstalled(cargo));
}

// Ship-default shield with negligible values is NOT recognized as fitted.
TEST(UpgradeAlreadyInstalled, NegligibleShieldNotRecognized) {
    RegisterUpgradeType("shield01b", "Shield");

    // "2/2/2" means max=2 per facet.
    // 4 facets × 2 = 8.0 total, below the 10.0 threshold.
    std::map<std::string, std::string> ship_data = {
        {"shield", "2/2/2"},
        {"shield_facets", "4"},
        {"Shield_Recharge", "1/1/1"}
    };
    UnitCSVFactory::LoadUnit("test_ship_shield_negligible", ship_data);

    MockUnit mock_unit("test_ship_shield_negligible");
    mock_unit.shield.Load("test_ship_shield_negligible");

    Cargo cargo = MakeShieldCargo("shield01b");
    EXPECT_FALSE(mock_unit.UpgradeAlreadyInstalled(cargo));
}

// Shield at exactly the threshold (total 10.0) is NOT recognized.
TEST(UpgradeAlreadyInstalled, ShieldAtThresholdNotRecognized) {
    RegisterUpgradeType("shield01c", "Shield");

    // "2.5/2.5/2.5" means max=2.5 per facet.
    // 4 facets × 2.5 = 10.0 total, exactly at the threshold.
    std::map<std::string, std::string> ship_data = {
        {"shield", "2.5/2.5/2.5"},
        {"shield_facets", "4"},
        {"Shield_Recharge", "1/1/1"}
    };
    UnitCSVFactory::LoadUnit("test_ship_shield_threshold", ship_data);

    MockUnit mock_unit("test_ship_shield_threshold");
    mock_unit.shield.Load("test_ship_shield_threshold");

    Cargo cargo = MakeShieldCargo("shield01c");
    EXPECT_FALSE(mock_unit.UpgradeAlreadyInstalled(cargo));
}

// Purchased shield is recognized as fitted.
TEST(UpgradeAlreadyInstalled, PurchasedShieldRecognized) {
    RegisterUpgradeType("shield02", "Shield");

    std::map<std::string, std::string> shield_data = {
        {"shield", "100/100/100"},
        {"shield_facets", "4"},
        {"Shield_Recharge", "10/10/10"}
    };
    UnitCSVFactory::LoadUnit("shield02" + kUpgradesSuffix, shield_data);

    MockUnit mock_unit("test_ship_purchased_shield");
    mock_unit.shield.Upgrade("shield02" + kUpgradesSuffix);

    Cargo cargo = MakeShieldCargo("shield02");
    EXPECT_TRUE(mock_unit.UpgradeAlreadyInstalled(cargo));
}

// Downgraded shield is NOT recognized as fitted.
TEST(UpgradeAlreadyInstalled, DowngradedShieldNotRecognized) {
    RegisterUpgradeType("shield03", "Shield");

    std::map<std::string, std::string> shield_data = {
        {"shield", "100/100/100"},
        {"shield_facets", "4"},
        {"Shield_Recharge", "10/10/10"}
    };
    UnitCSVFactory::LoadUnit("shield03" + kUpgradesSuffix, shield_data);

    MockUnit mock_unit("test_ship_downgraded_shield");
    mock_unit.shield.Upgrade("shield03" + kUpgradesSuffix);
    mock_unit.shield.Downgrade();

    Cargo cargo = MakeShieldCargo("shield03");
    EXPECT_FALSE(mock_unit.UpgradeAlreadyInstalled(cargo));
}

// Ship with no shield can buy shield.
TEST(UpgradeAlreadyInstalled, NoShieldCanBuy) {
    RegisterUpgradeType("shield04", "Shield");

    std::map<std::string, std::string> no_shield_data = {
        {"Name", "NoShieldShip"}
    };
    UnitCSVFactory::LoadUnit("test_no_shield_ship", no_shield_data);

    MockUnit mock_unit("test_no_shield_ship");
    mock_unit.shield.Load("test_no_shield_ship");

    Cargo cargo = MakeShieldCargo("shield04");
    EXPECT_FALSE(mock_unit.UpgradeAlreadyInstalled(cargo));
}

// -------------------------------------------------------------------
// Armor tests: UpgradeAlreadyBought ("was it bought?")
// -------------------------------------------------------------------

// Ship-default armor was NOT bought (upgrade_key is empty).
TEST(UpgradeAlreadyBought, ShipDefaultArmorNotBought) {
    RegisterUpgradeType("armor10", "Armor");

    std::map<std::string, std::string> ship_data = {
        {"armor", "15/15/15"}
    };
    UnitCSVFactory::LoadUnit("test_ship_armor_bought", ship_data);

    MockUnit mock_unit("test_ship_armor_bought");
    mock_unit.armor.Load("test_ship_armor_bought");

    Cargo cargo = MakeArmorCargo("armor10");
    EXPECT_FALSE(mock_unit.UpgradeAlreadyBought(cargo));
}

// Purchased armor WAS bought (upgrade_key is set).
TEST(UpgradeAlreadyBought, PurchasedArmorBought) {
    RegisterUpgradeType("armor11", "Armor");

    std::map<std::string, std::string> armor_data = {
        {"armor", "100/100/100"}
    };
    UnitCSVFactory::LoadUnit("armor11" + kUpgradesSuffix, armor_data);

    MockUnit mock_unit("test_ship_purchased_armor_bought");
    mock_unit.armor.Upgrade("armor11" + kUpgradesSuffix);

    Cargo cargo = MakeArmorCargo("armor11");
    EXPECT_TRUE(mock_unit.UpgradeAlreadyBought(cargo));
}

// Downgraded armor was NOT bought (not installed).
TEST(UpgradeAlreadyBought, DowngradedArmorNotBought) {
    RegisterUpgradeType("armor12", "Armor");

    std::map<std::string, std::string> armor_data = {
        {"armor", "100/100/100"}
    };
    UnitCSVFactory::LoadUnit("armor12" + kUpgradesSuffix, armor_data);

    MockUnit mock_unit("test_ship_downgraded_armor_bought");
    mock_unit.armor.Upgrade("armor12" + kUpgradesSuffix);
    mock_unit.armor.Downgrade();

    Cargo cargo = MakeArmorCargo("armor12");
    EXPECT_FALSE(mock_unit.UpgradeAlreadyBought(cargo));
}

// Ship with no armor was NOT bought.
TEST(UpgradeAlreadyBought, NoArmorNotBought) {
    RegisterUpgradeType("armor13", "Armor");

    std::map<std::string, std::string> no_armor_data = {
        {"Name", "NoArmorShipBought"}
    };
    UnitCSVFactory::LoadUnit("test_no_armor_ship_bought", no_armor_data);

    MockUnit mock_unit("test_no_armor_ship_bought");
    mock_unit.armor.Load("test_no_armor_ship_bought");

    Cargo cargo = MakeArmorCargo("armor13");
    EXPECT_FALSE(mock_unit.UpgradeAlreadyBought(cargo));
}

// -------------------------------------------------------------------
// Shield tests: UpgradeAlreadyBought ("was it bought?")
// -------------------------------------------------------------------

// Ship-default shield was NOT bought (upgrade_key is empty).
TEST(UpgradeAlreadyBought, ShipDefaultShieldNotBought) {
    RegisterUpgradeType("shield10", "Shield");

    std::map<std::string, std::string> ship_data = {
        {"shield", "10/10/10"},
        {"shield_facets", "4"},
        {"Shield_Recharge", "1/1/1"}
    };
    UnitCSVFactory::LoadUnit("test_ship_shield_bought", ship_data);

    MockUnit mock_unit("test_ship_shield_bought");
    mock_unit.shield.Load("test_ship_shield_bought");

    Cargo cargo = MakeShieldCargo("shield10");
    EXPECT_FALSE(mock_unit.UpgradeAlreadyBought(cargo));
}

// Purchased shield WAS bought (upgrade_key is set).
TEST(UpgradeAlreadyBought, PurchasedShieldBought) {
    RegisterUpgradeType("shield11", "Shield");

    std::map<std::string, std::string> shield_data = {
        {"shield", "100/100/100"},
        {"shield_facets", "4"},
        {"Shield_Recharge", "10/10/10"}
    };
    UnitCSVFactory::LoadUnit("shield11" + kUpgradesSuffix, shield_data);

    MockUnit mock_unit("test_ship_purchased_shield_bought");
    mock_unit.shield.Upgrade("shield11" + kUpgradesSuffix);

    Cargo cargo = MakeShieldCargo("shield11");
    EXPECT_TRUE(mock_unit.UpgradeAlreadyBought(cargo));
}

// Downgraded shield was NOT bought (not installed).
TEST(UpgradeAlreadyBought, DowngradedShieldNotBought) {
    RegisterUpgradeType("shield12", "Shield");

    std::map<std::string, std::string> shield_data = {
        {"shield", "100/100/100"},
        {"shield_facets", "4"},
        {"Shield_Recharge", "10/10/10"}
    };
    UnitCSVFactory::LoadUnit("shield12" + kUpgradesSuffix, shield_data);

    MockUnit mock_unit("test_ship_downgraded_shield_bought");
    mock_unit.shield.Upgrade("shield12" + kUpgradesSuffix);
    mock_unit.shield.Downgrade();

    Cargo cargo = MakeShieldCargo("shield12");
    EXPECT_FALSE(mock_unit.UpgradeAlreadyBought(cargo));
}

// Ship with no shield was NOT bought.
TEST(UpgradeAlreadyBought, NoShieldNotBought) {
    RegisterUpgradeType("shield13", "Shield");

    std::map<std::string, std::string> no_shield_data = {
        {"Name", "NoShieldShipBought"}
    };
    UnitCSVFactory::LoadUnit("test_no_shield_ship_bought", no_shield_data);

    MockUnit mock_unit("test_no_shield_ship_bought");
    mock_unit.shield.Load("test_no_shield_ship_bought");

    Cargo cargo = MakeShieldCargo("shield13");
    EXPECT_FALSE(mock_unit.UpgradeAlreadyBought(cargo));
}
