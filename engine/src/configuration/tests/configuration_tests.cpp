/**
 * configuration_tests.cpp
 *
 * Copyright (C) 2021 Roy Falk and David Wales
 * Copyright (C) 2022 Stephen G. Tuggy
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
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
#include "configuration/game_config.h"

#include <string>

TEST(LoadConfig, Sanity)
{
    // Test without configuration
    const bool default_bool = GameConfig::GetVariable("test", "boolean_variable", false);
    const int default_int = GameConfig::GetVariable("test", "int_variable", 1);
    const float default_float = GameConfig::GetVariable("test", "float_variable", 7.8);
    //const std::string default_string = GameConfig::GetVariable("test", "string_variable", "World");
    EXPECT_FALSE(default_bool);
    EXPECT_TRUE(default_int == 1);
    EXPECT_FLOAT_EQ(default_float, 7.8);
    //EXPECT_TRUE(default_string == "World");

    // Read the configuration from the config file
    const std::string filename = "test_assets/vegastrike.config";
    GameConfig::LoadGameConfig(filename);

    // Test again
    const bool test_bool = GameConfig::GetVariable("test", "boolean_variable", false);
    const int test_int = GameConfig::GetVariable("test", "int_variable", 1);
    const float test_float = GameConfig::GetVariable("test", "float_variable", 7.8);
    EXPECT_TRUE(test_bool);
    EXPECT_TRUE(test_int == 15) << "Expected 15 but got " << test_int;
    EXPECT_FLOAT_EQ(test_float, 4.2) << "Expected 4.2 but got " << test_float;
}
