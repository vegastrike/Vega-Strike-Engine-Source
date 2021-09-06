/**
 * vs_logging.h
 *
 * Copyright (C) 2021 Stephen G. Tuggy
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


#ifndef VEGASTRIKE_VS_LOGGING_H
#define VEGASTRIKE_VS_LOGGING_H

#include <cstdint>

#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/format.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/filesystem.hpp>

namespace VegaStrikeLogging {

typedef ::boost::log::sinks::synchronous_sink<::boost::log::sinks::text_ostream_backend> ConsoleLogSink;
typedef ::boost::log::sinks::synchronous_sink<::boost::log::sinks::text_file_backend>    FileLogSink;
// extern boost::shared_ptr<ConsoleLogSink> pConsoleLogSink;
// extern boost::shared_ptr<FileLogSink> pFileLogSink;

#define VS_LOG(log_level, log_message)                      \
    do {                                                    \
        BOOST_LOG_TRIVIAL(log_level) << (log_message);      \
    } while (false)
#define VS_LOG_AND_FLUSH(log_level, log_message)            \
    do {                                                    \
        BOOST_LOG_TRIVIAL(log_level) << (log_message);      \
        ::VegaStrikeLogging::VegaStrikeLogger::FlushLogs(); \
    } while (false)

// void exitProgram(int code);

class VegaStrikeLogger
{
private:
    static ::boost::shared_ptr<ConsoleLogSink> console_log_sink_;
    static ::boost::shared_ptr<FileLogSink>    file_log_sink_;

public:
    static void InitLoggingPart1();
    static void InitLoggingPart2(const uint8_t debug_level, const ::boost::filesystem::path& vega_strike_home_dir);
    static void FlushLogs();
    // inline template<class LogLevel, class LogMessage> static void Log(LogLevel log_level, LogMessage log_message)
    // {
    //     BOOST_LOG_TRIVIAL(log_level) << log_message;
    // }
    // inline template<class LogLevel, class LogMessage> static void LogAndFlush(LogLevel log_level, LogMessage log_message)
    // {
    //     BOOST_LOG_TRIVIAL(log_level) << log_message;
    //     FlushLogs();
    // }
};

} // namespace VegaStrikeLogging

#endif // VEGASTRIKE_VS_LOGGING_H
