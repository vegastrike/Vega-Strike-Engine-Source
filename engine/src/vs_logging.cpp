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
#include <functional>
#include <iostream>
#include <utility>

#include "opentelemetry/exporters/otlp/otlp_file_client_options.h"
#include "opentelemetry/exporters/otlp/otlp_file_exporter_factory.h"
#include "opentelemetry/exporters/otlp/otlp_file_exporter_options.h"
#include "opentelemetry/exporters/otlp/otlp_file_log_record_exporter_factory.h"
#include "opentelemetry/exporters/otlp/otlp_file_log_record_exporter_options.h"
#include "opentelemetry/logs/logger_provider.h"
#include "opentelemetry/nostd/shared_ptr.h"
#include "opentelemetry/sdk/logs/logger_provider.h"
#include "opentelemetry/sdk/logs/logger_provider_factory.h"
#include "opentelemetry/logs/provider.h"
#include "opentelemetry/sdk/logs/simple_log_record_processor_factory.h"
#include "opentelemetry/trace/provider.h"
#include "opentelemetry/sdk/trace/simple_processor_factory.h"
#include "opentelemetry/sdk/trace/tracer_provider.h"
#include "opentelemetry/sdk/trace/tracer_provider_factory.h"
#include "opentelemetry/trace/tracer_provider.h"

namespace trace     = opentelemetry::trace;
namespace nostd     = opentelemetry::nostd;
namespace otlp      = opentelemetry::exporter::otlp;
namespace logs_sdk  = opentelemetry::sdk::logs;
namespace logs      = opentelemetry::logs;
namespace trace_sdk = opentelemetry::sdk::trace;

namespace
{
otlp::OtlpFileExporterOptions opts;
otlp::OtlpFileLogRecordExporterOptions log_opts;

std::shared_ptr<opentelemetry::sdk::trace::TracerProvider> tracer_provider;
std::shared_ptr<opentelemetry::sdk::logs::LoggerProvider> logger_provider;

void InitTracer()
{
  // Create OTLP exporter instance
  std::unique_ptr<opentelemetry::sdk::trace::SpanExporter> exporter   = otlp::OtlpFileExporterFactory::Create(opts);
  std::unique_ptr<opentelemetry::sdk::trace::SpanProcessor> processor  = trace_sdk::SimpleSpanProcessorFactory::Create(std::move(exporter));
  tracer_provider = trace_sdk::TracerProviderFactory::Create(std::move(processor));

  // Set the global trace provider
  std::shared_ptr<trace::TracerProvider> api_provider = tracer_provider;
  trace::Provider::SetTracerProvider(api_provider);
}

void CleanupTracer()
{
  // We call ForceFlush to prevent to cancel running exportings, It's optional.
  if (tracer_provider)
  {
    tracer_provider->ForceFlush();
  }

  tracer_provider.reset();
  std::shared_ptr<trace::TracerProvider> none;
  trace::Provider::SetTracerProvider(none);
}

void InitLogger()
{
  // Create OTLP exporter instance
  auto exporter   = otlp::OtlpFileLogRecordExporterFactory::Create(log_opts);
  auto processor  = logs_sdk::SimpleLogRecordProcessorFactory::Create(std::move(exporter));
  logger_provider = logs_sdk::LoggerProviderFactory::Create(std::move(processor));

  std::shared_ptr<logs::LoggerProvider> api_provider = logger_provider;
  logs::Provider::SetLoggerProvider(api_provider);
}

void CleanupLogger()
{
  // We call ForceFlush to prevent to cancel running exportings, It's optional.
  if (logger_provider)
  {
    logger_provider->ForceFlush();
  }

  logger_provider.reset();
  nostd::shared_ptr<logs::LoggerProvider> none;
  logs::Provider::SetLoggerProvider(none);
}

nostd::shared_ptr<trace::Tracer> get_tracer()
{
    const nostd::shared_ptr<trace::TracerProvider> provider = trace::Provider::GetTracerProvider();
    return provider->GetTracer("py_poc_library");
}

nostd::shared_ptr<logs::Logger> get_logger()
{
    const nostd::shared_ptr<logs::LoggerProvider> provider = logs::Provider::GetLoggerProvider();
    return provider->GetLogger("py_poc_library_logger", "py_poc_library");
}

}  // namespace

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
            logging_core_->set_filter(severity >= info);
            break;
        case 2:
            logging_core_->set_filter(severity >= debug);
            break;
        case 3:
            logging_core_->set_filter(severity >= trace);
            break;
        default:
            logging_core_->set_filter(severity >= important_info);
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

    console_log_sink_->set_filter(severity >= important_info);
}

void VegaStrikeLogger::FlushLogs() {
    if (!STATIC_VARS_DESTROYED) {
        logging_core_->flush();
        #if defined(USE_OPEN_TELEMETRY)
        tracer_provider->ForceFlush();
        #endif
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
        #if defined(USE_OPEN_TELEMETRY)
        tracer_provider->ForceFlush();
        #endif
    }
    std::cout << std::flush;
    std::cerr << std::flush;
    std::clog << std::flush;
    fflush(stdout);
    fflush(stderr);
    STATIC_VARS_DESTROYED = true;
    #if defined(USE_OPEN_TELEMETRY)
    CleanupTracer();
    CleanupLogger();
    #endif
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

    #if defined(USE_OPEN_TELEMETRY)
    otlp::OtlpFileClientFileSystemOptions fs_backend;
    fs_backend.file_pattern = logging_dir_name + "/" + "vegastrike_%Y-%m-%d_%H_%M_%S.%f_trace.jsonl";
    opts.backend_options    = fs_backend;
    otlp::OtlpFileClientFileSystemOptions logs_fs_backend;
    logs_fs_backend.file_pattern = logging_dir_name + "/" + "vegastrike_%Y-%m-%d_%H_%M_%S.%f_log.jsonl";
    log_opts.backend_options     = logs_fs_backend;

    InitTracer();
    InitLogger();
    #endif
}

VegaStrikeLogger::~VegaStrikeLogger() {
    FlushLogsProgramExiting();
    logging_core_->remove_all_sinks();
}

#if defined(USE_OPEN_TELEMETRY)
logs::Severity translate_severity(vega_log_level level) {
    switch (level) {
    case trace:
        return logs::Severity::kTrace;
    case debug:
        return logs::Severity::kDebug;
    case info:
        return logs::Severity::kInfo;
    case important_info:
        return logs::Severity::kInfo2;
    case warning:
        return logs::Severity::kWarning;
    case serious_warning:
        return logs::Severity::kWarning2;
    case error:
        return logs::Severity::kError;
    case fatal:
        return logs::Severity::kFatal;
    }
}
#endif

void VegaStrikeLogger::Log(const vega_log_level level, const std::string &message) {
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

    #if defined(USE_OPEN_TELEMETRY)
    get_logger()->Log(translate_severity(level), message);
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

    #if defined(USE_OPEN_TELEMETRY)
    get_logger()->Log(translate_severity(level), message);
    #endif
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

    #if defined(USE_OPEN_TELEMETRY)
    get_logger()->Log(translate_severity(level), message.str());
    #endif
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
