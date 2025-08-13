/*
 * vs_logging.cpp
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


#include "src/vs_logging.h"
#include "src/vs_exit.h"
#include "root_generic/vs_globals.h"

#include <string>
#include <cstdint>

#include <boost/smart_ptr/make_shared_object.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/filesystem.hpp>

#if defined(USE_OPEN_TELEMETRY)
#include "opentelemetry/nostd/shared_ptr.h"
#endif

namespace VegaStrikeLogging {

void VegaStrikeLogger::InitLoggingPart2(const uint8_t debug_level,
        const boost::filesystem::path &vega_strike_home_dir) {

    const boost::filesystem::path &logging_dir = boost::filesystem::absolute("logs",
            vega_strike_home_dir);         /*< $HOME/.vegastrike/logs, typically >*/
    const std::string &logging_dir_name = logging_dir.string();
    VS_LOG(info, (boost::format("log directory : '%1%'") % logging_dir_name));

    switch (debug_level) {
        case 1:
            logging_core_->set_filter(severity >= vega_log_level::info);
            break;
        case 2:
            logging_core_->set_filter(severity >= vega_log_level::debug);
            break;
        case 3:
            logging_core_->set_filter(severity >= vega_log_level::trace);
            break;
        default:
            logging_core_->set_filter(severity >= vega_log_level::important_info);
            break;
    }

    file_log_sink_ = boost::log::add_file_log
            (
                    boost::log::keywords::file_name =
                            logging_dir_name + "/" + "vegastrike_%Y-%m-%d_%H_%M_%S.%f.log", /*< file name pattern >*/
                    boost::log::keywords::rotation_size = 10 * 1024
                            * 1024,                                               /*< rotate files every 10 MiB... >*/
                    boost::log::keywords::time_based_rotation =
                            boost::log::sinks::file::rotation_at_time_point(0, 0, 0),     /*< ...or at midnight >*/
                    boost::log::keywords::format =
                            "[%TimeStamp%]: %Message%",                                     /*< log record format >*/
                    boost::log::keywords::auto_flush =
                            true, /*false,*/                                                /*< whether to auto flush to the file after every line >*/
                    boost::log::keywords::min_free_space = 5UL * 1024UL * 1024UL
                            * 1024UL                                      /*< stop boost::log when there's only 5 GiB free space left >*/
            );

    console_log_sink_->set_filter(severity >= vega_log_level::important_info);

#if defined(USE_OPEN_TELEMETRY)
    opentelemetry::exporter::otlp::OtlpFileClientFileSystemOptions fs_backend;
    fs_backend.file_pattern = logging_dir_name + "/" + "vegastrike_otel.trace";
    opts.backend_options    = fs_backend;

    InitTracer();

    opentelemetry::exporter::otlp::OtlpFileClientFileSystemOptions logs_fs_backend;
    logs_fs_backend.file_pattern = logging_dir_name + "/" + "vegastrike_otel.log";
    log_opts.backend_options     = logs_fs_backend;

    InitOtelLogger();
#endif
}

void VegaStrikeLogger::FlushLogs() {
    if (!STATIC_VARS_DESTROYED) {
        logging_core_->flush();
    }
    std::cout << std::flush;
    std::cerr << std::flush;
    std::clog << std::flush;
    fflush(stdout);
    fflush(stderr);
}

void VegaStrikeLogger::FlushLogsProgramExiting() {
    if (!STATIC_VARS_DESTROYED) {
        logging_core_->flush();
    }
    std::cout << std::flush;
    std::cerr << std::flush;
    std::clog << std::flush;
    fflush(stdout);
    fflush(stderr);
    #if defined(USE_OPEN_TELEMETRY)
    CleanupOtelLogger();
    CleanupTracer();
    #endif
    STATIC_VARS_DESTROYED = true;
}

BOOST_LOG_GLOBAL_LOGGER_INIT(my_logger, severity_logger_mt<vega_log_level>) {
    boost::log::sources::severity_logger_mt<vega_log_level> lg;
    boost::log::add_common_attributes();

    return lg;
}

VegaStrikeLogger::VegaStrikeLogger() : slg_(my_logger::get()), file_log_back_end_(nullptr), file_log_sink_(nullptr) {
    boost::filesystem::path::imbue(std::locale(""));
    logging_core_ = boost::log::core::get();
    console_log_sink_ = boost::log::add_console_log
            (
                    std::cerr,
                    boost::log::keywords::format =
                            "%Message%",                                                    /*< log record format specific to the console >*/
                    boost::log::keywords::auto_flush =
                            true /*false*/                                                  /*< whether to do the equivalent of fflush(stdout) after every msg >*/
            );
}

VegaStrikeLogger::~VegaStrikeLogger() {
    FlushLogsProgramExiting();
    logging_core_->remove_all_sinks();
}

#if defined(USE_OPEN_TELEMETRY)
nostd::shared_ptr<logs::Logger> get_otel_logger()
{
    auto provider = logs::Provider::GetLoggerProvider();
    return provider->GetLogger("vega_strike_logger", "vega_strike");
}
#endif

void VegaStrikeLogger::Log(const vega_log_level level, const std::string &message) {
    if (STATIC_VARS_DESTROYED) {
        return;
    }
    boost::log::record rec = slg_.open_record(boost::log::keywords::severity = level);
    if (rec) {
        boost::log::record_ostream strm(rec);
        strm << message;
        strm.flush();
        slg_.push_record(boost::move(rec));
    }

#if defined(USE_OPEN_TELEMETRY)
    auto span = get_tracer()->StartSpan("logging span");
    auto ctx = span->GetContext();
    auto otel_logger = get_otel_logger();

    switch (level) {
        case vega_log_level::info:
        case vega_log_level::important_info:
            otel_logger->Info(message, ctx.trace_id(), ctx.span_id(), ctx.trace_flags());
            break;
        case vega_log_level::debug:
            otel_logger->Debug(message, ctx.trace_id(), ctx.span_id(), ctx.trace_flags());
            break;
        case vega_log_level::trace:
            otel_logger->Trace(message, ctx.trace_id(), ctx.span_id(), ctx.trace_flags());
            break;
        case vega_log_level::warning:
        case vega_log_level::serious_warning:
            otel_logger->Warn(message, ctx.trace_id(), ctx.span_id(), ctx.trace_flags());
            break;
        case vega_log_level::error:
            otel_logger->Error(message, ctx.trace_id(), ctx.span_id(), ctx.trace_flags());
            break;
        case vega_log_level::fatal:
            otel_logger->Fatal(message, ctx.trace_id(), ctx.span_id(), ctx.trace_flags());
            break;
        default:
            otel_logger->Error("unrecognized log level (shouldn't happen)", ctx.trace_id(), ctx.span_id(),
                               ctx.trace_flags());
            break;
    }
#endif
}

void VegaStrikeLogger::LogAndFlush(const vega_log_level level, const std::string &message) {
    Log(level, message);
    FlushLogs();
}

void VegaStrikeLogger::LogFlushExit(const vega_log_level level, const std::string& message, const int exit_code = -50) {
    Log(level, message);
    FlushLogsProgramExiting();
    VSExit(exit_code);
}

void VegaStrikeLogger::Log(const vega_log_level level, const char *message) {
    if (STATIC_VARS_DESTROYED) {
        return;
    }
    boost::log::record rec = slg_.open_record(boost::log::keywords::severity = level);
    if (rec)
    {
        boost::log::record_ostream strm(rec);
        strm << message;
        strm.flush();
        slg_.push_record(boost::move(rec));
    }
}

void VegaStrikeLogger::LogAndFlush(const vega_log_level level, const char *message) {
    Log(level, message);
    FlushLogs();
}

void VegaStrikeLogger::LogFlushExit(const vega_log_level level, const char* message, const int exit_code = -50) {
    Log(level, message);
    FlushLogsProgramExiting();
    VSExit(exit_code);
}

void VegaStrikeLogger::Log(const vega_log_level level, const boost::basic_format<char> &message) {
    if (STATIC_VARS_DESTROYED) {
        return;
    }
    boost::log::record rec = slg_.open_record(boost::log::keywords::severity = level);
    if (rec)
    {
        boost::log::record_ostream strm(rec);
        strm << message;
        strm.flush();
        slg_.push_record(boost::move(rec));
    }
}

void VegaStrikeLogger::LogAndFlush(const vega_log_level level, const boost::basic_format<char> &message) {
    Log(level, message);
    FlushLogs();
}

void VegaStrikeLogger::LogFlushExit(const vega_log_level level, const boost::basic_format<char>& message, const int exit_code = -50) {
    Log(level, message);
    FlushLogsProgramExiting();
    VSExit(exit_code);
}

} // namespace VegaStrikeLogging
