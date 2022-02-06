/*
 * Copyright (C) 2021-2022 Stephen G. Tuggy and other Vega Strike contributors.
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


#include "vs_logging.h"

#include <string>
#include <cstdint>

#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/filesystem.hpp>

namespace VegaStrikeLogging {

boost::shared_ptr<VegaStrikeLogging::FileLogSink>    VegaStrikeLogger::file_log_sink_{};
boost::shared_ptr<VegaStrikeLogging::ConsoleLogSink> VegaStrikeLogger::console_log_sink_{};

// void exitProgram(int code)
// {
//     Music::CleanupMuzak();
//     VegaStrikeLogging::VegaStrikeLogger::FlushLogs();
//     winsys_exit(code);
// }

void VegaStrikeLogger::InitLoggingPart1()
{
    ::boost::log::add_common_attributes();

    console_log_sink_ = ::boost::log::add_console_log
            (
                    std::cerr,
                    ::boost::log::keywords::format =
                            "%Message%",                                                    /*< log record format specific to the console >*/
                    ::boost::log::keywords::auto_flush =
                            true /*false*/                                                  /*< whether to do the equivalent of fflush(stdout) after every msg >*/
            );
}

void VegaStrikeLogger::InitLoggingPart2(const uint8_t debug_level,
                                        const ::boost::filesystem::path &vega_strike_home_dir)
{
    auto logging_core = ::boost::log::core::get();

    const ::boost::filesystem::path &logging_dir = ::boost::filesystem::absolute("logs",
                                                                                 vega_strike_home_dir);         /*< $HOME/.vegastrike/logs, typically >*/
    const std::string &logging_dir_name = logging_dir.string();
    VS_LOG(info, (boost::format("log directory : '%1%'") % logging_dir_name));

    switch (debug_level) {
        case 1:
            logging_core->set_filter(::boost::log::trivial::severity >= ::boost::log::trivial::info);
            break;
        case 2:
            logging_core->set_filter(::boost::log::trivial::severity >= ::boost::log::trivial::debug);
            break;
        case 3:
            logging_core->set_filter(::boost::log::trivial::severity >= ::boost::log::trivial::trace);
            break;
        default:
            logging_core->set_filter(::boost::log::trivial::severity >= ::boost::log::trivial::warning);
            break;
    }

    file_log_sink_ = ::boost::log::add_file_log
            (
                    ::boost::log::keywords::file_name =
                            logging_dir_name + "/" + "vegastrike_%Y-%m-%d_%H_%M_%S.%f.log", /*< file name pattern >*/
                    ::boost::log::keywords::rotation_size = 10 * 1024
                            * 1024,                                               /*< rotate files every 10 MiB... >*/
                    ::boost::log::keywords::time_based_rotation =
                            ::boost::log::sinks::file::rotation_at_time_point(0, 0, 0),     /*< ...or at midnight >*/
                    ::boost::log::keywords::format =
                            "[%TimeStamp%]: %Message%",                                     /*< log record format >*/
                    ::boost::log::keywords::auto_flush =
                            true, /*false,*/                                                /*< whether to auto flush to the file after every line >*/
                    ::boost::log::keywords::min_free_space = 1L * 1024L * 1024L
                            * 1024L                                      /*< stop ::boost::log when there's only 1 GiB free space left >*/
            );

    console_log_sink_->set_filter(::boost::log::trivial::severity >= ::boost::log::trivial::fatal);
}

void VegaStrikeLogger::FlushLogs()
{
    if (console_log_sink_) {
        console_log_sink_->flush();
    }
    if (file_log_sink_) {
        file_log_sink_->flush();
    }
    fflush(stdout);
    fflush(stderr);
}

} // namespace VegaStrikeLogging
