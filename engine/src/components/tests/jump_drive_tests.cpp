/*
 * jump_drive_tests.cpp
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

#include "jump_drive.h"
#include "resource/random_utils.h"
#include "unit_csv_factory.h"

static const std::string upgrades_suffix_string = "__upgrades";
static const std::string jump_drive_string = "jump_drive";

static const std::map<std::string,std::string> jump_drive_map = {
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



// Used to quickly figure out why the code wasn't working properly
TEST(JumpDrive, Damage) {
    UnitCSVFactory::LoadUnit(jump_drive_string + upgrades_suffix_string, jump_drive_map);

    JumpDrive jump_drive;

    jump_drive.Load(jump_drive_string + upgrades_suffix_string);

    jump_drive.DamageByPercent(0.1);

    // Check operational drive shouldn't get damage
    double chance_to_damage = randomDouble() - 0.01;

    std::cout << chance_to_damage << std::endl;
    std::cout << jump_drive.PercentOperational() << std::endl;
    
    //EXPECT_FALSE(true);
}