/*
 * Copyright (C) 2021-2023 Daniel Horn, Stephen G. Tuggy, Benjamen R. Meyer, and
 * other Vega Strike contributors.
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
#ifndef VEGA_STRIKE_ENGINE_VS_LOGGING_H
#define VEGA_STRIKE_ENGINE_VS_LOGGING_H

#include <cstdint>

#include "boost/move/utility_core.hpp"
#include "boost/smart_ptr/shared_ptr.hpp"
#include "boost/format.hpp"
//#include "boost/log/sources/logger.hpp"
#include "boost/log/sources/severity_logger.hpp"
#include "boost/log/sources/record_ostream.hpp"
#include "boost/log/sinks/sync_frontend.hpp"
#include "boost/log/sinks/text_file_backend.hpp"
#include "boost/log/sinks/text_ostream_backend.hpp"
#include "boost/filesystem.hpp"

namespace VegaStrikeLogging {

enum vega_log_level {
    trace,
    debug,
    info,
    important_info,
    warning,
    serious_warning,
    error,
    fatal
};

BOOST_LOG_ATTRIBUTE_KEYWORD(severity, "Severity", vega_log_level)

typedef boost::log::sinks::synchronous_sink<boost::log::sinks::text_ostream_backend> ConsoleLogSink;
typedef boost::log::sinks::synchronous_sink<boost::log::sinks::text_file_backend> FileLogSink;

#define VS_LOG(log_level, log_message)                                                                              \
    do {                                                                                                            \
        VegaStrikeLogging::vega_logger()->Log(VegaStrikeLogging::vega_log_level::log_level, (log_message));         \
    } while (false)
#define VS_LOG_AND_FLUSH(log_level, log_message)                                                                    \
    do {                                                                                                            \
        VegaStrikeLogging::vega_logger()->LogAndFlush(VegaStrikeLogging::vega_log_level::log_level, (log_message)); \
    } while (false)

class VegaStrikeLogger {
private:
    boost::log::core_ptr logging_core_;
    boost::shared_ptr<boost::log::sources::severity_logger_mt<vega_log_level>> slg_;
    boost::shared_ptr<ConsoleLogSink> console_log_sink_;
    boost::shared_ptr<FileLogSink> file_log_sink_;

public:
    VegaStrikeLogger();
    ~VegaStrikeLogger();
    void InitLoggingPart2(const uint8_t debug_level, const boost::filesystem::path &vega_strike_home_dir);
    void FlushLogs();
    void Log(const vega_log_level level, const std::string& message);
    void Log(const vega_log_level level, const char * message);
    void Log(const vega_log_level level, const boost::basic_format<char>& message);
    void LogAndFlush(const vega_log_level level, const std::string& message);
    void LogAndFlush(const vega_log_level level, const char * message);
    void LogAndFlush(const vega_log_level level, const boost::basic_format<char>& message);
};

extern boost::shared_ptr<VegaStrikeLogger> vega_logger();

} // namespace VegaStrikeLogging

#endif //VEGA_STRIKE_ENGINE_VS_LOGGING_H
