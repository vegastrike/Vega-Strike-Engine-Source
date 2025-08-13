/*
 * exit_unit_tests.cpp
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

#include <cstdlib>

#include "src/vs_exit.h"
#include "src/vs_logging.h"

bool STATIC_VARS_DESTROYED = false;

#if defined(USE_OPEN_TELEMETRY)
opentelemetry::exporter::otlp::OtlpFileExporterOptions VegaStrikeLogging::opts;
opentelemetry::exporter::otlp::OtlpFileLogRecordExporterOptions VegaStrikeLogging::log_opts;

std::shared_ptr<opentelemetry::sdk::trace::TracerProvider> VegaStrikeLogging::tracer_provider;
std::shared_ptr<opentelemetry::sdk::logs::LoggerProvider> VegaStrikeLogging::logger_provider;
#endif

void VSExit(int code) {
    VegaStrikeLogging::VegaStrikeLogger::instance().FlushLogsProgramExiting();
    STATIC_VARS_DESTROYED = true;
    exit(code);
}
