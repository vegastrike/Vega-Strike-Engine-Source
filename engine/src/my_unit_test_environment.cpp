/*
 * my_unit_test_environment.cpp
 *
 * Copyright (C) 2001-2025 Daniel Horn, pyramid3d, Stephen G. Tuggy,
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
 * along with Vega Strike.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "my_unit_test_environment.h"

#include <configuration/configuration.h>
#include <configuration/game_config.h>

MyUnitTestEnvironment::~MyUnitTestEnvironment()
= default;

void MyUnitTestEnvironment::SetUp() {
    Environment::SetUp();
    const boost::filesystem::path config_file_path{"./config.json"};
    vega_config::config = std::make_shared<vega_config::Config>(config_file_path);
}

void MyUnitTestEnvironment::TearDown() {
    Environment::TearDown();
}
