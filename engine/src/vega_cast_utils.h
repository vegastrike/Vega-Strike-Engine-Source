/*
 * vega_cast_utils.h
 *
 * Copyright (C) 2001-2023 Daniel Horn, Stephen G. Tuggy,
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef VEGA_STRIKE_SRC_VEGA_CAST_UTILS_H_
#define VEGA_STRIKE_SRC_VEGA_CAST_UTILS_H_

#include <typeinfo>
#include <cstdlib>
#include "vs_logging.h"
#include "vs_globals.h"
#include "vs_exit.h"

template<class TargetType, class SourceType>
inline TargetType* vega_dynamic_cast_ptr(SourceType* from) {
    TargetType* ret_val = nullptr;
    try {
        ret_val = dynamic_cast<TargetType*>(from);
    } catch (std::bad_cast& e) {
        VS_LOG_AND_FLUSH(fatal, (boost::format("Fatal Error '%1%' casting type %2%* to %3%*") % e.what() % typeid(SourceType).name() % typeid(TargetType).name()));
        VSExit(-422);
    }
    if (ret_val == nullptr) {
        VS_LOG_AND_FLUSH(fatal, (boost::format("Fatal Failure to Cast type %1%* to %2%* -- nullptr encountered") % typeid(SourceType).name() % typeid(TargetType).name()));
        VSExit(-422);
    }
    return ret_val;
}

template<class TargetType, class SourceType>
inline const TargetType* vega_dynamic_const_cast_ptr(const SourceType* from) {
    const TargetType* ret_val = nullptr;
    try {
        ret_val = dynamic_cast<const TargetType*>(from);
    } catch (std::bad_cast& e) {
        VS_LOG_AND_FLUSH(fatal, (boost::format("Fatal Error '%1%' casting type const %2%* to const %3%*") % e.what() % typeid(SourceType).name() % typeid(TargetType).name()));
        VSExit(-422);
    }
    if (ret_val == nullptr) {
        VS_LOG_AND_FLUSH(fatal, (boost::format("Fatal Failure to Cast type const %1%* to const %2%* -- nullptr encountered") % typeid(SourceType).name() % typeid(TargetType).name()));
        VSExit(-422);
    }
    return ret_val;
}

template<class TargetType, class SourceType>
inline vega_types::SharedPtr<TargetType> vega_dynamic_cast_shared_ptr(vega_types::SharedPtr<SourceType> from) {
    return boost::dynamic_pointer_cast<TargetType>(from);
}

template<class TargetType, class SourceType>
inline const vega_types::SharedPtr<TargetType const> vega_dynamic_const_cast_shared_ptr(const vega_types::SharedPtr<SourceType const> from) {
    return boost::dynamic_pointer_cast<TargetType const>(from);
}

#endif //VEGA_STRIKE_SRC_VEGA_CAST_UTILS_H_
