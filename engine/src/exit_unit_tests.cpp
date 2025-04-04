/*
 * exit_unit_tests.cpp
 *
 * Implementation of VSExit for unit tests
 *
 * Copyright (C) 2001-2025 Daniel Horn, Stephen G. Tuggy,
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */

#include <cstdlib>
#include "configuration/configuration.h"
#include "src/vs_exit.h"
#include "src/vs_logging.h"

bool STATIC_VARS_DESTROYED = false;
std::shared_ptr<vega_config::Config> vega_config::config = nullptr;

void VSExit(int code) {
    VegaStrikeLogging::VegaStrikeLogger::instance().FlushLogsProgramExiting();
    STATIC_VARS_DESTROYED = true;
    exit(code);
}
