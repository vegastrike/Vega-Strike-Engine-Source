/*
 * vs_logging.h
 * 
 * Copyright (C) 2021-2024 Daniel Horn, Stephen G. Tuggy, Benjamen R. Meyer, and
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */


#ifndef VEGA_STRIKE_ENGINE_VS_LOGGING_H
#define VEGA_STRIKE_ENGINE_VS_LOGGING_H

#include <cstdint>

#include <boost/move/utility_core.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/smart_ptr/make_shared_object.hpp>
#include <boost/format.hpp>
#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sinks/async_frontend.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/filesystem.hpp>

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

BOOST_LOG_GLOBAL_LOGGER(my_logger, boost::log::sources::severity_logger_mt<VegaStrikeLogging::vega_log_level>)

BOOST_LOG_ATTRIBUTE_KEYWORD(severity, "Severity", vega_log_level)

typedef boost::log::sinks::text_ostream_backend ConsoleLogBackEnd;
typedef boost::log::sinks::text_file_backend FileLogBackEnd;
typedef boost::log::sinks::synchronous_sink<ConsoleLogBackEnd> ConsoleLogSink;
typedef boost::log::sinks::synchronous_sink<FileLogBackEnd>    FileLogSink;

#define VS_LOG(log_level, log_message)                                                                                                          \
    do {                                                                                                                                        \
        VegaStrikeLogging::VegaStrikeLogger::instance().Log(VegaStrikeLogging::vega_log_level::log_level, (log_message));                       \
    } while (false)
#define VS_LOG_AND_FLUSH(log_level, log_message)                                                                                                \
    do {                                                                                                                                        \
        VegaStrikeLogging::VegaStrikeLogger::instance().LogAndFlush(VegaStrikeLogging::vega_log_level::log_level, (log_message));               \
    } while (false)
#define VS_LOG_FLUSH_EXIT(log_level, log_message, exit_code)                                                                                    \
    do {                                                                                                                                        \
        VegaStrikeLogging::VegaStrikeLogger::instance().LogFlushExit(VegaStrikeLogging::vega_log_level::log_level, (log_message), exit_code);   \
    } while (false)

class VegaStrikeLogger {
private:
    boost::log::core_ptr logging_core_;
    boost::log::sources::severity_logger_mt<vega_log_level>& slg_;
    boost::shared_ptr<ConsoleLogBackEnd> console_log_back_end_;
    boost::shared_ptr<FileLogBackEnd> file_log_back_end_;
    boost::shared_ptr<ConsoleLogSink> console_log_sink_;
    boost::shared_ptr<FileLogSink> file_log_sink_;

private:
    VegaStrikeLogger();
    ~VegaStrikeLogger();

public:
    static VegaStrikeLogger& instance() {
        static VegaStrikeLogger logger_instance;
        return logger_instance;
    }
        
    void InitLoggingPart2(const uint8_t debug_level, const boost::filesystem::path &vega_strike_home_dir);
    void FlushLogs();
    void FlushLogsProgramExiting();
    void Log(const vega_log_level level, const std::string& message);
    void Log(const vega_log_level level, const char * message);
    void Log(const vega_log_level level, const boost::basic_format<char>& message);
    void LogAndFlush(const vega_log_level level, const std::string& message);
    void LogFlushExit(const vega_log_level level, const std::string& message, const int exit_code);
    void LogAndFlush(const vega_log_level level, const char * message);
    void LogFlushExit(const vega_log_level level, const char* message, const int exit_code);
    void LogAndFlush(const vega_log_level level, const boost::basic_format<char>& message);
    void LogFlushExit(const vega_log_level level, const boost::basic_format<char>& message, const int exit_code);

    VegaStrikeLogger(const VegaStrikeLogger &) = delete;
    VegaStrikeLogger& operator = (const VegaStrikeLogger&) = delete;
};

} // namespace VegaStrikeLogging

#endif //VEGA_STRIKE_ENGINE_VS_LOGGING_H
