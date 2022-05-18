/*
 * configuration_tests.cpp
 *
 * Copyright (C) 2021-2022 Daniel Horn, Roy Falk, David Wales,
 * Stephen G. Tuggy, and other Vega Strike contributors
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

TEST(LoadConfig, Sanity) {
    // Test without configuration
    const bool default_bool = vega_config::GetGameConfig().GetVariable("test.boolean_variable", false);
    const int default_int = vega_config::GetGameConfig().GetVariable("test.int_variable", 1);
    const float default_float = vega_config::GetGameConfig().GetVariable("test.float_variable", 7.8F);
    const std::string default_string = vega_config::GetGameConfig().GetString("test.string_variable", "World");
    const std::string default_escaped_string = vega_config::GetGameConfig().GetEscapedString("test.default_escaped_string", "#00FF00Hi\\\r\\\nthere!#000000");
    EXPECT_FALSE(default_bool);
    EXPECT_EQ(default_int, 1);
    EXPECT_FLOAT_EQ(default_float, 7.8F);
    EXPECT_EQ(default_string, "World");
    EXPECT_EQ(default_escaped_string, "#00FF00Hi\r\nthere!#000000");

    // Read the configuration from the config file
    const std::string filename = "test_assets/vegastrike.config";
    vega_config::GetGameConfig().LoadGameConfig(filename);

    // Test again
    const bool test_bool = vega_config::GetGameConfig().GetVariable("test.boolean_variable", false);
    const int test_int = vega_config::GetGameConfig().GetVariable("test.int_variable", 1);
    const float test_float = vega_config::GetGameConfig().GetVariable("test.float_variable", 7.8F);
    const std::string test_string = vega_config::GetGameConfig().GetString("test.string_variable", "World");
    const std::string escaped_string = vega_config::GetGameConfig().GetEscapedString("test.escaped_string_variable", "#00FF00Hi\\\r\\\nthere!#000000");
    EXPECT_TRUE(test_bool);
    EXPECT_EQ(test_int, 15) << "Expected 15 but got " << test_int;
    EXPECT_FLOAT_EQ(test_float, 4.2F) << "Expected 4.2 but got " << test_float;
    EXPECT_EQ(test_string, "hello");
    EXPECT_EQ(escaped_string, "#FF0000Hello\r\nthere!#000000");

    // Test subsection functionality
    const bool subsection_bool = vega_config::GetGameConfig().GetVariable("test.subsection.subsection_boolean_variable", false);
    const int subsection_int = vega_config::GetGameConfig().GetVariable("test.subsection.subsection_int_variable", 2);
    const float subsection_float = vega_config::GetGameConfig().GetVariable("test.subsection.subsection_float_variable", 8.9F);
    const std::string subsection_string = vega_config::GetGameConfig().GetString("test.subsection.subsection_string_variable", "World");
    const std::string subsection_escaped_string = vega_config::GetGameConfig().GetEscapedString("test.subsection.subsection_escaped_string_variable", "#00FF00Hi\\\r\\\nthere!#000000");
    EXPECT_TRUE(subsection_bool);
    EXPECT_EQ(subsection_int, 15) << "Expected 15 but got " << subsection_int;
    EXPECT_FLOAT_EQ(subsection_float, 4.2F) << "Expected 4.2 but got " << subsection_float;
    EXPECT_EQ(subsection_string, "hello");
    EXPECT_EQ(subsection_escaped_string, "#FF0000Hello\r\nthere!#000000");

    // Another main section
    const bool test_bool2 = vega_config::GetGameConfig().GetVariable("test2.boolean_variable2", true);
    const int test_int2 = vega_config::GetGameConfig().GetVariable("test2.int_variable2", 3);
    const float test_float2 = vega_config::GetGameConfig().GetVariable("test2.float_variable2", 10.0F);
    const std::string test_string2 = vega_config::GetGameConfig().GetString("test2.string_variable2", "World");
    const std::string escaped_string2 = vega_config::GetGameConfig().GetEscapedString("test2.escaped_string_variable2", "#00FF00Hi\\\r\\\nthere!#000000");
    EXPECT_TRUE(test_bool2);
    EXPECT_EQ(test_int2, 15) << "Expected 15 but got " << test_int2;
    EXPECT_FLOAT_EQ(test_float2, 4.2F) << "Expected 4.2 but got " << test_float2;
    EXPECT_EQ(test_string2, "hello");
    EXPECT_EQ(escaped_string2, "#FF0000Hello\r\nthere!#000000");

}
