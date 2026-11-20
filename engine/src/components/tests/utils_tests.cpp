/*
 * utils_tests.cpp
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

#include "components/component_utils.h"
#include "cmd/unit_csv_factory.h"

std::string dummy_color(double percent) {
    if(percent == 1.0) {
        return "white:";
    } else if(percent == 0.0) {
        return "grey:";
    } else if(percent > 0.66) {
        return "yellow:";
    } else if(percent > 0.33) {
        return "orange:";
    } else {
        return "red:";
    } 
}


TEST(UtilsTests, PrintFormattedComponentInHud) {
    double percents[] = {1.0, 0.0, 0.8, 0.6, 0.4, 0.2, 0.1};

    std::string expected_outputs[] = {"white:Dummy (100%)white:\n",
                                      "grey:Dummy (0%)white:\n",
                                      "yellow:Dummy (80%)white:\n",
                                      "orange:Dummy (60%)white:\n",
                                      "orange:Dummy (40%)white:\n",
                                      "red:Dummy (20%)white:\n",
                                      "red:Dummy (10%)white:\n"};

    for(int i=0;i<7;i++) {
        std::string output = PrintFormattedComponentInHud(percents[i], "Dummy", true, dummy_color);
        EXPECT_EQ(output, expected_outputs[i]);
    }
}

static const std::string kUpgradesSuffix = "__upgrades";

TEST(UtilsTests, GetComponentTypeFromName_WithUpgradeType) {
    // When Upgrade_Type is present, it should be used directly.
    std::map<std::string, std::string> armor_upgrade = {
        {"Upgrade_Type", "Armor"},
        {"armor", "100"}
    };
    UnitCSVFactory::LoadUnit("test_armor_with_type" + kUpgradesSuffix, armor_upgrade);
    EXPECT_EQ(GetComponentTypeFromName("test_armor_with_type"), ComponentType::Armor);
}

TEST(UtilsTests, GetComponentTypeFromName_FallbackArmor) {
    // When Upgrade_Type is missing, the "armor" field should detect Armor type.
    std::map<std::string, std::string> armor_upgrade = {
        {"armor", "100"}
    };
    UnitCSVFactory::LoadUnit("test_armor_fallback" + kUpgradesSuffix, armor_upgrade);
    EXPECT_EQ(GetComponentTypeFromName("test_armor_fallback"), ComponentType::Armor);
}

TEST(UtilsTests, GetComponentTypeFromName_FallbackShield) {
    // When Upgrade_Type is missing, the "shield" field should detect Shield type.
    std::map<std::string, std::string> shield_upgrade = {
        {"shield", "50"}
    };
    UnitCSVFactory::LoadUnit("test_shield_fallback" + kUpgradesSuffix, shield_upgrade);
    EXPECT_EQ(GetComponentTypeFromName("test_shield_fallback"), ComponentType::Shield);
}

TEST(UtilsTests, GetComponentTypeFromName_FallbackReactor) {
    std::map<std::string, std::string> reactor_upgrade = {
        {"Reactor_Recharge", "100"}
    };
    UnitCSVFactory::LoadUnit("test_reactor_fallback" + kUpgradesSuffix, reactor_upgrade);
    EXPECT_EQ(GetComponentTypeFromName("test_reactor_fallback"), ComponentType::Reactor);
}

TEST(UtilsTests, GetComponentTypeFromName_FallbackFuel) {
    std::map<std::string, std::string> fuel_upgrade = {
        {"Fuel_Capacity", "500"}
    };
    UnitCSVFactory::LoadUnit("test_fuel_fallback" + kUpgradesSuffix, fuel_upgrade);
    EXPECT_EQ(GetComponentTypeFromName("test_fuel_fallback"), ComponentType::Fuel);
}

TEST(UtilsTests, GetComponentTypeFromName_FallbackCapacitor) {
    std::map<std::string, std::string> capacitor_upgrade = {
        {"Primary_Capacitor", "200"}
    };
    UnitCSVFactory::LoadUnit("test_capacitor_fallback" + kUpgradesSuffix, capacitor_upgrade);
    EXPECT_EQ(GetComponentTypeFromName("test_capacitor_fallback"), ComponentType::Capacitor);
}

TEST(UtilsTests, GetComponentTypeFromName_FallbackFtlCapacitor) {
    std::map<std::string, std::string> ftl_capacitor_upgrade = {
        {"Warp_Capacitor", "100"}
    };
    UnitCSVFactory::LoadUnit("test_ftl_capacitor_fallback" + kUpgradesSuffix, ftl_capacitor_upgrade);
    EXPECT_EQ(GetComponentTypeFromName("test_ftl_capacitor_fallback"), ComponentType::FtlCapacitor);
}

TEST(UtilsTests, GetComponentTypeFromName_FallbackRadar) {
    std::map<std::string, std::string> radar_upgrade = {
        {"Radar_Range", "500000000"}
    };
    UnitCSVFactory::LoadUnit("test_radar_fallback" + kUpgradesSuffix, radar_upgrade);
    EXPECT_EQ(GetComponentTypeFromName("test_radar_fallback"), ComponentType::Radar);
}

TEST(UtilsTests, GetComponentTypeFromName_FallbackECM) {
    std::map<std::string, std::string> ecm_upgrade = {
        {"ecm", "10/10"}
    };
    UnitCSVFactory::LoadUnit("test_ecm_fallback" + kUpgradesSuffix, ecm_upgrade);
    EXPECT_EQ(GetComponentTypeFromName("test_ecm_fallback"), ComponentType::ECM);
}

TEST(UtilsTests, GetComponentTypeFromName_FallbackRepairBot) {
    std::map<std::string, std::string> repair_upgrade = {
        {"repair", "5"}
    };
    UnitCSVFactory::LoadUnit("test_repair_fallback" + kUpgradesSuffix, repair_upgrade);
    EXPECT_EQ(GetComponentTypeFromName("test_repair_fallback"), ComponentType::RepairBot);
}

TEST(UtilsTests, GetComponentTypeFromName_FallbackJumpDrive) {
    std::map<std::string, std::string> jump_upgrade = {
        {"Jump_Drive_Present", "1"}
    };
    UnitCSVFactory::LoadUnit("test_jump_fallback" + kUpgradesSuffix, jump_upgrade);
    EXPECT_EQ(GetComponentTypeFromName("test_jump_fallback"), ComponentType::JumpDrive);
}

TEST(UtilsTests, GetComponentTypeFromName_FallbackFtlDrive) {
    std::map<std::string, std::string> ftl_upgrade = {
        {"Warp_Usage_Cost", "100"}
    };
    UnitCSVFactory::LoadUnit("test_ftl_fallback" + kUpgradesSuffix, ftl_upgrade);
    EXPECT_EQ(GetComponentTypeFromName("test_ftl_fallback"), ComponentType::FtlDrive);
}

TEST(UtilsTests, GetComponentTypeFromName_FallbackCloak) {
    std::map<std::string, std::string> cloak_upgrade = {
        {"Cloak_Energy", "50"}
    };
    UnitCSVFactory::LoadUnit("test_cloak_fallback" + kUpgradesSuffix, cloak_upgrade);
    EXPECT_EQ(GetComponentTypeFromName("test_cloak_fallback"), ComponentType::Cloak);
}

TEST(UtilsTests, GetComponentTypeFromName_FallbackHull) {
    std::map<std::string, std::string> hull_upgrade = {
        {"Hull", "1000"}
    };
    UnitCSVFactory::LoadUnit("test_hull_fallback" + kUpgradesSuffix, hull_upgrade);
    EXPECT_EQ(GetComponentTypeFromName("test_hull_fallback"), ComponentType::Hull);
}

TEST(UtilsTests, GetComponentTypeFromName_FallbackDrive) {
    // Drive has absolute values and no Upgrade_Improvement Object_Type.
    std::map<std::string, std::string> drive_upgrade = {
        {"Maneuver_Yaw", "10.0"},
        {"Forward_Accel", "5.0"}
    };
    UnitCSVFactory::LoadUnit("test_drive_fallback" + kUpgradesSuffix, drive_upgrade);
    EXPECT_EQ(GetComponentTypeFromName("test_drive_fallback"), ComponentType::Drive);
}

TEST(UtilsTests, GetComponentTypeFromName_FallbackDriveUpgrade) {
    // DriveUpgrade is indicated by Object_Type = Upgrade_Improvement.
    std::map<std::string, std::string> drive_upgrade_map = {
        {"Object_Type", "Upgrade_Improvement"},
        {"Maneuver_Yaw", "1.2"},
        {"Forward_Accel", "1.2"}
    };
    UnitCSVFactory::LoadUnit("test_drive_upgrade_fallback" + kUpgradesSuffix, drive_upgrade_map);
    EXPECT_EQ(GetComponentTypeFromName("test_drive_upgrade_fallback"), ComponentType::DriveUpgrade);
}

TEST(UtilsTests, GetComponentTypeFromName_FallbackAfterburner) {
    // Afterburner has absolute values and no Upgrade_Improvement Object_Type.
    std::map<std::string, std::string> afterburner_upgrade = {
        {"Afterburner_Accel", "10.0"},
        {"Afterburner_Speed_Governor", "100.0"}
    };
    UnitCSVFactory::LoadUnit("test_afterburner_fallback" + kUpgradesSuffix, afterburner_upgrade);
    EXPECT_EQ(GetComponentTypeFromName("test_afterburner_fallback"), ComponentType::Afterburner);
}

TEST(UtilsTests, GetComponentTypeFromName_FallbackAfterburnerUpgrade) {
    // AfterburnerUpgrade is indicated by Object_Type = Upgrade_Improvement.
    // Real-world multiplier values are 1.8, 1.6 etc., not 1.0.
    std::map<std::string, std::string> ab_upgrade_map = {
        {"Object_Type", "Upgrade_Improvement"},
        {"Afterburner_Accel", "1.8"},
        {"Afterburner_Speed_Governor", "1.6"}
    };
    UnitCSVFactory::LoadUnit("test_ab_upgrade_fallback" + kUpgradesSuffix, ab_upgrade_map);
    EXPECT_EQ(GetComponentTypeFromName("test_ab_upgrade_fallback"), ComponentType::AfterburnerUpgrade);
}

TEST(UtilsTests, GetComponentTypeFromName_FallbackDriveUpgradeRealValues) {
    // Test with real-world multiplier values from mult_shady_moreturning__upgrades.
    std::map<std::string, std::string> drive_upgrade_map = {
        {"Object_Type", "Upgrade_Improvement"},
        {"Maneuver_Yaw", "1.2"},
        {"Maneuver_Pitch", "1.2"},
        {"Maneuver_Roll", "1.2"}
    };
    UnitCSVFactory::LoadUnit("test_drive_upgrade_real" + kUpgradesSuffix, drive_upgrade_map);
    EXPECT_EQ(GetComponentTypeFromName("test_drive_upgrade_real"), ComponentType::DriveUpgrade);
}

TEST(UtilsTests, GetComponentTypeFromName_FallbackAfterburnerUpgradeRealValues) {
    // Test with real-world multiplier values from mult_overdrive01__upgrades.
    std::map<std::string, std::string> ab_upgrade_map = {
        {"Object_Type", "Upgrade_Improvement"},
        {"Afterburner_Accel", "1.8"},
        {"Afterburner_Speed_Governor", "1.6"},
        {"Afterburner_Usage_Cost", "1"}
    };
    UnitCSVFactory::LoadUnit("test_ab_upgrade_real" + kUpgradesSuffix, ab_upgrade_map);
    EXPECT_EQ(GetComponentTypeFromName("test_ab_upgrade_real"), ComponentType::AfterburnerUpgrade);
}

TEST(UtilsTests, GetComponentTypeFromName_NoMatch) {
    // When no component-specific fields are present, should return NoComponent.
    std::map<std::string, std::string> empty_upgrade = {
        {"Name", "Unknown"}
    };
    UnitCSVFactory::LoadUnit("test_no_match" + kUpgradesSuffix, empty_upgrade);
    EXPECT_EQ(GetComponentTypeFromName("test_no_match"), ComponentType::NoComponent);
}

TEST(UtilsTests, GetComponentTypeFromName_ExplicitNone) {
    // Upgrade_Type = "None" should explicitly return NoComponent.
    std::map<std::string, std::string> none_upgrade = {
        {"Upgrade_Type", "None"}
    };
    UnitCSVFactory::LoadUnit("test_explicit_none" + kUpgradesSuffix, none_upgrade);
    EXPECT_EQ(GetComponentTypeFromName("test_explicit_none"), ComponentType::NoComponent);
}