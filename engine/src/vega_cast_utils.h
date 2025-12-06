/*
 * vega_cast_utils.h
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
#ifndef VEGA_STRIKE_ENGINE_VEGA_CAST_UTILS_H
#define VEGA_STRIKE_ENGINE_VEGA_CAST_UTILS_H

#include <typeinfo>
#include <cstdlib>
#include "src/vs_logging.h"
#include "root_generic/vs_globals.h"
#include "src/vs_exit.h"

extern std::locale our_numeric_locale;

inline double locale_aware_stod(const std::string& s, const double default_value = 0.0, const std::locale& loc = our_numeric_locale) noexcept {
    std::istringstream iss(s);
    iss.imbue(loc);
    try {
        double tmp = default_value;
        iss >> tmp;
        return tmp;
    } catch(...) {
        VS_LOG(error, "locale_aware_stod encountered an error");
        return default_value;
    }
}

inline float locale_aware_stof(const std::string& s, const float default_value = 0.0F, const std::locale& loc = our_numeric_locale) noexcept {
    std::istringstream iss(s);
    iss.imbue(loc);
    try {
        float tmp = default_value;
        iss >> tmp;
        return tmp;
    } catch(...) {
        VS_LOG(error, "locale_aware_stof encountered an error");
        return default_value;
    }
}

inline int locale_aware_stoi(const std::string& s, const int default_value = 0, const std::locale& loc = our_numeric_locale) noexcept {
    std::istringstream iss(s);
    iss.imbue(loc);
    try {
        int tmp = default_value;
        iss >> tmp;
        return tmp;
    } catch(...) {
        VS_LOG(error, "locale_aware_stoi encountered an error");
        return default_value;
    }
}

inline long locale_aware_stol(const std::string& s, const long default_value = 0L, const std::locale& loc = our_numeric_locale) noexcept {
    std::istringstream iss(s);
    iss.imbue(loc);
    try {
        long tmp = default_value;
        iss >> tmp;
        return tmp;
    } catch(...) {
        VS_LOG(error, "locale_aware_stol encountered an error");
        return default_value;
    }
}

template<class TargetType, class SourceType>
inline TargetType* vega_dynamic_cast_ptr(SourceType* from, const bool allow_null = false) {
    TargetType* ret_val = nullptr;
    try {
        ret_val = dynamic_cast<TargetType*>(from);
    } catch (std::bad_cast& e) {
        VS_LOG_AND_FLUSH(fatal, (boost::format("Fatal Error '%1%' casting type %2%* to %3%*") % e.what() % typeid(SourceType).name() % typeid(TargetType).name()));
        VSExit(-422);
    }
    if (ret_val == nullptr && !allow_null) {
        VS_LOG_AND_FLUSH(fatal, (boost::format("Fatal Failure to Cast type %1%* to %2%* -- nullptr encountered") % typeid(SourceType).name() % typeid(TargetType).name()));
        VSExit(-424);
    }
    return ret_val;
}

template<class TargetType, class SourceType>
inline const TargetType* vega_dynamic_const_cast_ptr(const SourceType* from) {
    TargetType* ret_val = nullptr;
    try {
        ret_val = dynamic_cast<TargetType*>(from);
    } catch (std::bad_cast& e) {
        VS_LOG_AND_FLUSH(fatal, (boost::format("Fatal Error '%1%' casting type %2%* to %3%*") % e.what() % typeid(SourceType).name() % typeid(TargetType).name()));
        VSExit(-422);
    }
    if (ret_val == nullptr) {
        VS_LOG_AND_FLUSH(fatal, (boost::format("Fatal Failure to Cast type %1%* to %2%* -- nullptr encountered") % typeid(SourceType).name() % typeid(TargetType).name()));
        VSExit(-424);
    }
    return ret_val;
}

#endif //VEGA_STRIKE_ENGINE_VEGA_CAST_UTILS_H
