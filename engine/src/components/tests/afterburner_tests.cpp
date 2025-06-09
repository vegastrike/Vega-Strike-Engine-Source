/*
 * afterburner_tests.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
 *
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

#include "components/afterburner.h"
#include "components/afterburner_upgrade.h"

#include "cmd/unit_csv_factory.h"

static const std::string upgrades_suffix_string = "__upgrades";
const std::string afterburner_string = "afterburner";
const std::string upgrade_string = "upgrade";

std::map<std::string,std::string> afterburner_map = {
    { "Name", "Afterburner"},
    { "Mass", "5.0"},
    { "Afterburner_Accel", "10"},
    { "Afterburner_Speed_Governor", "100"},
    { "Afterburner_Usage_Cost", "2.0"}
};

std::map<std::string,std::string> afterburner_upgrade_map = {
    { "Name", "AfterburnerUpgrade"},
    { "Mass", "5.0"},
    { "Afterburner_Accel", "1.5"},
    { "Afterburner_Speed_Governor", "1.5"},
    { "Afterburner_Usage_Cost", "1.5"}
};

TEST(Afterburner, Sanity) {
    UnitCSVFactory::LoadUnit(afterburner_string + upgrades_suffix_string, afterburner_map);
    UnitCSVFactory::LoadUnit(upgrade_string + upgrades_suffix_string, afterburner_upgrade_map);

    Afterburner afterburner;
    AfterburnerUpgrade upgrade(&afterburner);

    afterburner.Load(afterburner_string + upgrades_suffix_string);
    upgrade.Load(upgrade_string + upgrades_suffix_string);

    EXPECT_EQ(afterburner.GetMass(), 0.0);
    EXPECT_EQ(afterburner.thrust.MaxValue(), 10.0);
    EXPECT_EQ(afterburner.speed.MaxValue(), 100.0);
    EXPECT_EQ(afterburner.GetConsumption(), 2.0);

    // Disabled. Afterburner_Upgrade does not get saved to file.
    //EXPECT_EQ(upgrade.GetMass(), 5.0);
    // EXPECT_EQ(upgrade.thrust, 1.5);
    // EXPECT_EQ(upgrade.speed, 1.5);
    // EXPECT_EQ(upgrade.consumption, 1.5);
}

TEST(Afterburner, UpgradeDowngrade) {
    UnitCSVFactory::LoadUnit(afterburner_string + upgrades_suffix_string, afterburner_map);
    UnitCSVFactory::LoadUnit(upgrade_string + upgrades_suffix_string, afterburner_upgrade_map);

    Afterburner afterburner;
    AfterburnerUpgrade upgrade(&afterburner);

    afterburner.Load(afterburner_string + upgrades_suffix_string);

    // Original
    EXPECT_EQ(afterburner.GetMass(), 0.0);
    EXPECT_EQ(afterburner.thrust.MaxValue(), 10.0);
    EXPECT_EQ(afterburner.speed.MaxValue(), 100.0);
    EXPECT_EQ(afterburner.GetConsumption(), 2.0);

    EXPECT_EQ(upgrade.GetMass(), 0.0);
    EXPECT_EQ(upgrade.thrust, 1.0);
    EXPECT_EQ(upgrade.speed, 1.0);
    EXPECT_EQ(upgrade.consumption, 1.0);

    // Upgrade
    upgrade.Upgrade(upgrade_string + upgrades_suffix_string);

    EXPECT_EQ(afterburner.GetMass(), 0.0);
    EXPECT_EQ(afterburner.thrust.MaxValue(), 15.0);
    EXPECT_EQ(afterburner.speed.MaxValue(), 150.0);
    EXPECT_EQ(afterburner.GetConsumption(), 3.0);

    //EXPECT_EQ(upgrade.GetMass(), 5.0);
    EXPECT_EQ(upgrade.thrust, 1.5);
    EXPECT_EQ(upgrade.speed, 1.5);
    EXPECT_EQ(upgrade.consumption, 1.5);

    // Downgrade
    upgrade.Downgrade();

    EXPECT_EQ(afterburner.GetMass(), 0.0);
    EXPECT_EQ(afterburner.thrust.MaxValue(), 10.0);
    EXPECT_EQ(afterburner.speed.MaxValue(), 100.0);
    EXPECT_EQ(afterburner.GetConsumption(), 2.0);

    EXPECT_EQ(upgrade.GetMass(), 0.0);
    EXPECT_EQ(upgrade.thrust, 1.0);
    EXPECT_EQ(upgrade.speed, 1.0);
    EXPECT_EQ(upgrade.consumption, 1.0);
}
