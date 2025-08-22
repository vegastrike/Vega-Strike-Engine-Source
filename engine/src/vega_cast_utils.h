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

template<class TargetType, class SourceType>
inline TargetType* vega_dynamic_cast_ptr(SourceType* from) {
    TargetType* ret_val = nullptr;
    try {
        ret_val = dynamic_cast<TargetType*>(from);
    } catch (std::bad_cast& e) {
        VS_LOG_AND_FLUSH(fatal, (boost::format("Fatal Error '%1%' casting type %2%* to %3%*") % e.what() % typeid(SourceType).name() % typeid(TargetType).name()));
        VSExit(-422);
    } catch (std::exception& e) {
        VS_LOG_AND_FLUSH(fatal, (boost::format("Fatal Error '%1%' casting type %2%* to %3%*") % e.what() % typeid(SourceType).name() % typeid(TargetType).name()));
        VSExit(-423);
    }
    if (ret_val == nullptr) {
        VS_LOG_AND_FLUSH(fatal, (boost::format("Fatal Failure to Cast type %1%* to %2%* -- nullptr encountered") % typeid(SourceType).name() % typeid(TargetType).name()));
        VSExit(-422);
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
    } catch (std::exception& e) {
        VS_LOG_AND_FLUSH(fatal, (boost::format("Fatal Error '%1%' casting type %2%* to %3%*") % e.what() % typeid(SourceType).name() % typeid(TargetType).name()));
        VSExit(-423);
    }
    if (ret_val == nullptr) {
        VS_LOG_AND_FLUSH(fatal, (boost::format("Fatal Failure to Cast type %1%* to %2%* -- nullptr encountered") % typeid(SourceType).name() % typeid(TargetType).name()));
        VSExit(-422);
    }
    return ret_val;
}

#endif //VEGA_STRIKE_ENGINE_VEGA_CAST_UTILS_H
