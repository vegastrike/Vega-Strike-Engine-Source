/*
 * jump_drive_tests.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 ...
 * (license header unchanged)
 */

#include <gtest/gtest.h>
#include <map>
#include <string>
#include <limits>

#include "components/jump_drive.h"
#include "cmd/unit_csv_factory.h"

namespace {
constexpr char kUpgradesSuffix[] = "__upgrades";
constexpr char kJumpDrive[]      = "jump_drive";

const std::map<std::string, std::string> kJumpDriveMap = {
    {"Key", "jump_drive__upgrades"},
    {"Name", "Interstellar Jump Drive"},
    {"Upgrade_Type", "Jump_Drive"},
    {"Object_Type", "Upgrade_Replacement"},
    {"Textual_Description", "\"@upgrades/jump_drive.png@Jump drive for traveling between stars\"\n"},
    {"Mass", "10"},
    {"Moment_Of_Inertia", "10"},
    {"Jump_Drive_Present", "TRUE"},
    {"Jump_Drive_Delay", "1"}
};

inline std::string Key() { return std::string(kJumpDrive) + kUpgradesSuffix; }

inline void ExpectUnitInterval(double v) {
    EXPECT_GE(v, 0.0);
    EXPECT_LE(v, 1.0);
}
} // namespace

// Test fixture: loads the CSV unit definition once for all tests.
class JumpDriveTest : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        UnitCSVFactory::LoadUnit(Key(), kJumpDriveMap);
    }

    void SetUp() override {
        drive.Load(Key());
    }

    JumpDrive drive;
};

TEST_F(JumpDriveTest, LoadsWithOperationalWithinRange) {
    const double op = drive.PercentOperational();
    ExpectUnitInterval(op);
}

TEST_F(JumpDriveTest, ZeroDamageDoesNotChangeOperational) {
    const double before = drive.PercentOperational();
    drive.DamageByPercent(0.0);
    const double after = drive.PercentOperational();
    EXPECT_NEAR(after, before, 1e-12);
    ExpectUnitInterval(after);
}

TEST_F(JumpDriveTest, DamageIsMonotonicNonIncreasing) {
    double prev = drive.PercentOperational();
    const double damages[] = {0.05, 0.10, 0.20, 0.50, 1.00};
    for (double d : damages) {
        SCOPED_TRACE(::testing::Message() << "Damage=" << d);
        drive.DamageByPercent(d);
        const double now = drive.PercentOperational();
        EXPECT_LE(now, prev + 1e-12);
        ExpectUnitInterval(now);
        prev = now;
    }
}

TEST_F(JumpDriveTest, OutOfRangeDamageInputsAreSafe) {
    // Negative damage should not increase operational %
    double prev = drive.PercentOperational();
    drive.DamageByPercent(-1.0);
    double now = drive.PercentOperational();
    EXPECT_LE(now, prev + 1e-12);
    ExpectUnitInterval(now);

    // Excessive damage should not produce NaN/Inf or >1/<0 values
    prev = now;
    drive.DamageByPercent(2.0);
    now = drive.PercentOperational();
    EXPECT_LE(now, prev + 1e-12);
    ExpectUnitInterval(now);
}

TEST_F(JumpDriveTest, RepeatedHeavyDamageStaysWithinBounds) {
    for (int i = 0; i < 20; ++i) {
        drive.DamageByPercent(1.0);
        ExpectUnitInterval(drive.PercentOperational());
    }
}
