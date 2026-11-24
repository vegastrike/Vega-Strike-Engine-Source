/*
 * ship_functions_tests.cpp
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
#include <string>

#include "components/ship_functions.h"
#include "cmd/unit_csv_factory.h"
#include "resource/random_utils.h"

static const std::string upgrades_suffix_string = "__upgrades";
static const std::string ship_functions_string = "ship_functions";

static const std::map<std::string, std::string> ship_functions_map = {
    {"Key", "ship_functions__upgrades"},
    {"Name", "Ship Functions"},
    {"cockpit", "100/100"},
    {"communications", "100/100"},
    {"fire_control", "100/100"},
    {"life_support", "100/100"},
    {"ftl_interdiction", "0"}
};

// Tests for the Component/ShipFunctions Damage overload resolution fix.
// Component defines Damage() (parameterless), while ShipFunctions adds
// Damage(Function).  The using declaration ensures both are visible.

// ---------------------------------------------------------------------------
// Component base class tests
// ---------------------------------------------------------------------------

TEST(Component, DamageParameterless) {
    // Verify Component::Damage() is callable and affects operational value.
    std::map<std::string, std::string> hull_map = {
        {"Hull", "100/100"}
    };
    UnitCSVFactory::LoadUnit("test_hull" + upgrades_suffix_string, hull_map);

    // ShipFunctions starts fully operational.
    ShipFunctions sf;
    sf.Load("test_hull" + upgrades_suffix_string);

    // The parameterless Damage() inherited from Component should be callable
    // on a ShipFunctions object. Without the using declaration, this call
    // would resolve only to Damage(Function), causing a compilation error.
    sf.Damage();

    // After Damage(), operational should be <= 1.0 (may or may not have
    // actually decreased depending on random, but should never exceed 1.0).
    EXPECT_LE(sf.PercentOperational(), 1.0);
}

TEST(Component, DamageByPercent) {
    std::map<std::string, std::string> hull_map = {
        {"Hull", "100/100"}
    };
    UnitCSVFactory::LoadUnit("test_hull2" + upgrades_suffix_string, hull_map);

    ShipFunctions sf;
    sf.Load("test_hull2" + upgrades_suffix_string);

    // Damage by a known percentage.
    sf.DamageByPercent(0.5);
    EXPECT_DOUBLE_EQ(sf.PercentOperational(), 0.5);
}

TEST(Component, Repair) {
    std::map<std::string, std::string> hull_map = {
        {"Hull", "50/100"}
    };
    UnitCSVFactory::LoadUnit("test_hull3" + upgrades_suffix_string, hull_map);

    ShipFunctions sf;
    sf.Load("test_hull3" + upgrades_suffix_string);

    // Repair should restore operational to 1.0.
    sf.Repair();
    EXPECT_DOUBLE_EQ(sf.PercentOperational(), 1.0);
}

// ---------------------------------------------------------------------------
// ShipFunctions-specific Damage(Function) tests
// ---------------------------------------------------------------------------

TEST(ShipFunctions, DamageCockpit) {
    UnitCSVFactory::LoadUnit(ship_functions_string + upgrades_suffix_string, ship_functions_map);

    ShipFunctions sf;
    sf.Load(ship_functions_string + upgrades_suffix_string);

    double cockpitBefore = sf.Value(Function::cockpit);
    sf.Damage(Function::cockpit);
    double cockpitAfter = sf.Value(Function::cockpit);

    // Cockpit value should not increase after damage.
    EXPECT_LE(cockpitAfter, cockpitBefore);
}

TEST(ShipFunctions, DamageCommunications) {
    UnitCSVFactory::LoadUnit(ship_functions_string + upgrades_suffix_string, ship_functions_map);

    ShipFunctions sf;
    sf.Load(ship_functions_string + upgrades_suffix_string);

    double commsBefore = sf.Value(Function::communications);
    sf.Damage(Function::communications);
    double commsAfter = sf.Value(Function::communications);

    EXPECT_LE(commsAfter, commsBefore);
}

TEST(ShipFunctions, DamageFireControl) {
    UnitCSVFactory::LoadUnit(ship_functions_string + upgrades_suffix_string, ship_functions_map);

    ShipFunctions sf;
    sf.Load(ship_functions_string + upgrades_suffix_string);

    double fcBefore = sf.Value(Function::fire_control);
    sf.Damage(Function::fire_control);
    double fcAfter = sf.Value(Function::fire_control);

    EXPECT_LE(fcAfter, fcBefore);
}

TEST(ShipFunctions, DamageLifeSupport) {
    UnitCSVFactory::LoadUnit(ship_functions_string + upgrades_suffix_string, ship_functions_map);

    ShipFunctions sf;
    sf.Load(ship_functions_string + upgrades_suffix_string);

    double lsBefore = sf.Value(Function::life_support);
    sf.Damage(Function::life_support);
    double lsAfter = sf.Value(Function::life_support);

    EXPECT_LE(lsAfter, lsBefore);
}

TEST(ShipFunctions, DamageFtlInterdictionNoOp) {
    // FTL interdiction does not support damage; calling Damage on it is a no-op.
    UnitCSVFactory::LoadUnit(ship_functions_string + upgrades_suffix_string, ship_functions_map);

    ShipFunctions sf;
    sf.Load(ship_functions_string + upgrades_suffix_string);

    double ftlBefore = sf.Value(Function::ftl_interdiction);
    sf.Damage(Function::ftl_interdiction);
    double ftlAfter = sf.Value(Function::ftl_interdiction);

    EXPECT_DOUBLE_EQ(ftlBefore, ftlAfter);
}

// ---------------------------------------------------------------------------
// Overload visibility test — the key fix verification
// ---------------------------------------------------------------------------

TEST(ShipFunctionsOverload, BothDamageOverloadsVisible) {
    // This test verifies that the `using Component::Damage;` declaration in
    // ShipFunctions makes both overloads visible. Without it, calling
    // sf.Damage() (parameterless) on a ShipFunctions object would fail to
    // compile because Damage(Function) would hide Component::Damage().

    UnitCSVFactory::LoadUnit(ship_functions_string + upgrades_suffix_string, ship_functions_map);

    ShipFunctions sf;
    sf.Load(ship_functions_string + upgrades_suffix_string);

    // Call the parameterless Damage() — inherited from Component via using.
    // This line alone verifies the fix: it compiles because both overloads
    // are visible.
    sf.Damage();

    // Call the parameterized Damage(Function) — defined in ShipFunctions.
    sf.Damage(Function::cockpit);

    // Both calls should succeed without compilation errors.
    SUCCEED();
}
