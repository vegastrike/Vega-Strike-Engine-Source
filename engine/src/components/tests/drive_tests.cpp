/*
 * drive_tests.cpp
 *
 * Copyright (C) 2001-2023 Daniel Horn, Benjamen Meyer, Roy Falk, Stephen G. Tuggy,
 * and other Vega Strike contributors.
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
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */

#include <gtest/gtest.h>
#include <map>
#include <boost/math/constants/constants.hpp>
#include <utility>

#include "drive.h"
#include "drive_upgrade.h"

#include "unit_csv_factory.h"

using boost::math::float_constants::pi;

static const std::string upgrades_suffix_string = "__upgrades";
static const std::string drive_string = "drive";
static const std::string upgrade_string = "upgrade";

static const std::map<std::string,std::string> drive_map = {
    { "Name", "Drive"},
    { "Mass", "5.0"},
    { "Maneuver_Yaw", "10.0" },
    { "Maneuver_Pitch", "10.0" },
    { "Maneuver_Roll", "10.0" },
    { "Forward_Accel", "10.0" },
    { "Retro_Accel", "10.0" },
    { "Left_Accel", "10.0" },
    { "Right_Accel", "10.0" },
    { "Top_Accel", "10.0" },
    { "Bottom_Accel", "10.0" },
    { "Default_Speed_Governor", "10.0" },
    { "Yaw_Governor", "10.0" }, 
    { "Yaw_Governor", "10.0" }, 
    { "Yaw_Governor", "10.0" },
    { "Pitch_Governor", "10.0" }, 
    { "Pitch_Governor_Up", "10.0" }, 
    { "Pitch_Governor_Down", "10.0" },
    { "Roll_Governor", "10.0" }, 
    { "Roll_Governor_Right", "10.0" }, 
    { "Roll_Governor_Left", "10.0" }
};

static const std::map<std::string,std::string> drive_upgrade_map = {
    { "Name", "Upgrade"},
    { "Mass", "5.0"},
    { "Fuel_Consumption", "1.5"},
    { "Maneuver_Yaw", "1.5" },
    { "Maneuver_Pitch", "1.5" },
    { "Maneuver_Roll", "1.5" },
    { "Forward_Accel", "1.5" },
    { "Retro_Accel", "1.5" },
    { "Left_Accel", "1.5" },
    { "Right_Accel", "1.5" },
    { "Top_Accel", "1.5" },
    { "Bottom_Accel", "1.5" },
    { "Default_Speed_Governor", "1.5" },
    { "Yaw_Governor", "1.5" }, 
    { "Yaw_Governor", "1.5" }, 
    { "Yaw_Governor", "1.5" },
    { "Pitch_Governor", "1.5" }, 
    { "Pitch_Governor_Up", "1.5" }, 
    { "Pitch_Governor_Down", "1.5" },
    { "Roll_Governor", "1.5" }, 
    { "Roll_Governor_Right", "1.5" }, 
    { "Roll_Governor_Left", "1.5" }
};

static void DriveExpectEq(const Drive& drive, const double value) {
    EXPECT_FLOAT_EQ(drive.yaw.MaxValue(), value * pi / 180.0);
    EXPECT_FLOAT_EQ(drive.pitch.MaxValue(), value * pi / 180.0);
    EXPECT_FLOAT_EQ(drive.roll.MaxValue(), value * pi / 180.0);
    EXPECT_EQ(drive.forward.MaxValue(), value);
    EXPECT_EQ(drive.retro.MaxValue(), value);
    EXPECT_EQ(drive.lateral.MaxValue(), value);
    EXPECT_EQ(drive.vertical.MaxValue(), value);
    EXPECT_EQ(drive.speed.MaxValue(), value);

    EXPECT_FLOAT_EQ(drive.max_pitch_down.MaxValue(), value * pi / 180.0);
    EXPECT_FLOAT_EQ(drive.max_pitch_up.MaxValue(), value * pi / 180.0);
    EXPECT_FLOAT_EQ(drive.max_roll_left.MaxValue(), value * pi / 180.0);
    EXPECT_FLOAT_EQ(drive.max_roll_right.MaxValue(), value * pi / 180.0);
    EXPECT_FLOAT_EQ(drive.max_yaw_left.MaxValue(), value * pi / 180.0);
    EXPECT_FLOAT_EQ(drive.max_yaw_right.MaxValue(), value * pi / 180.0);
}

static void DriveUpgradeExpectEq(const DriveUpgrade& upgrade, const double value) {
    EXPECT_EQ(upgrade.yaw, value);
    EXPECT_EQ(upgrade.pitch, value);
    EXPECT_EQ(upgrade.roll, value);
    EXPECT_EQ(upgrade.forward, value);
    EXPECT_EQ(upgrade.retro, value);
    EXPECT_EQ(upgrade.lateral, value);
    EXPECT_EQ(upgrade.vertical, value);

    EXPECT_EQ(upgrade.max_pitch_down, value);
    EXPECT_EQ(upgrade.max_pitch_up, value);
    EXPECT_EQ(upgrade.max_roll_left, value);
    EXPECT_EQ(upgrade.max_roll_right, value);
    EXPECT_EQ(upgrade.max_yaw_left, value);
    EXPECT_EQ(upgrade.max_yaw_right, value);
}


TEST(Drive, Sanity) {
    UnitCSVFactory::LoadUnit(drive_string + upgrades_suffix_string, drive_map);
    UnitCSVFactory::LoadUnit(upgrade_string + upgrades_suffix_string, drive_upgrade_map);

    Drive drive;
    DriveUpgrade upgrade(&drive);

    drive.Load("", drive_string + upgrades_suffix_string);
    upgrade.Load(upgrade_string + upgrades_suffix_string);

    // Check Drive Values
    EXPECT_EQ(drive.GetUpgradeName(), "");  // Integrated
    EXPECT_EQ(drive.GetMass(), 0.0);

    DriveExpectEq(drive, 10.0);
    
    EXPECT_EQ(drive.GetConsumption(), 1.0);

    // Check DriveUpgrade Values
    EXPECT_EQ(upgrade.GetUpgradeName(), "Upgrade");
    EXPECT_EQ(upgrade.GetMass(), 5.0);

    DriveUpgradeExpectEq(upgrade, 1.5);
    
    EXPECT_EQ(upgrade.fuel_consumption, 1.5);
}

TEST(Drive, UpgradeDowngrade) {
    UnitCSVFactory::LoadUnit(drive_string + upgrades_suffix_string, drive_map);
    UnitCSVFactory::LoadUnit(upgrade_string + upgrades_suffix_string, drive_upgrade_map);

    Drive drive;
    DriveUpgrade upgrade(&drive);

    drive.Load("", drive_string + upgrades_suffix_string);

// Original
    std::cout << "Original\n--------\n";

    // Drive
    EXPECT_EQ(drive.GetUpgradeName(), "");
    EXPECT_EQ(drive.GetMass(), 0.0);

    DriveExpectEq(drive, 10.0);
    
    EXPECT_EQ(drive.GetConsumption(), 1.0);

    // Upgrade
    EXPECT_EQ(upgrade.GetUpgradeName(), "");
    EXPECT_EQ(upgrade.GetMass(), 0.0);

    DriveUpgradeExpectEq(upgrade, 1.0);
    
    EXPECT_EQ(upgrade.fuel_consumption, 1.0);

// Upgrade
    std::cout << "Upgrade\n-------\n";
    upgrade.Load(upgrade_string + upgrades_suffix_string);
    
    // Drive
    upgrade.Upgrade(upgrade_string);

    EXPECT_EQ(drive.GetUpgradeName(), "");
    EXPECT_EQ(drive.GetMass(), 0.0);

    DriveExpectEq(drive, 15.0);
    
    EXPECT_EQ(drive.GetConsumption(), 1.5);

    // Upgrade
    EXPECT_EQ(upgrade.GetUpgradeName(), "Upgrade");
    EXPECT_EQ(upgrade.GetMass(), 5.0);

    DriveUpgradeExpectEq(upgrade, 1.5);
    
    EXPECT_EQ(upgrade.fuel_consumption, 1.5);

// Downgrade
    std::cout << "Downgrade\n---------\n";
    // Drive
    upgrade.Downgrade();

    EXPECT_EQ(drive.GetUpgradeName(), "");
    EXPECT_EQ(drive.GetMass(), 0.0);

    DriveExpectEq(drive, 10.0);
    
    EXPECT_EQ(drive.GetConsumption(), 1.0);

    // Upgrade
    EXPECT_EQ(upgrade.GetUpgradeName(), "");
    EXPECT_EQ(upgrade.GetMass(), 0.0);

    DriveUpgradeExpectEq(upgrade, 1.0);
    
    EXPECT_EQ(upgrade.fuel_consumption, 1.0);
}