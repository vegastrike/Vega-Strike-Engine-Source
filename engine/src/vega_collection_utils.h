/*
 * vega_collection_utils.h
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

#ifndef VEGA_STRIKE_VEGA_COLLECTION_UTILS_H
#define VEGA_STRIKE_VEGA_COLLECTION_UTILS_H

#include <vector>
#include <functional>
#include "owner.h"

template<typename T>
void remove_all_references_to(const T *thus, std::vector<T *> *&vec, bool delete_collection_if_empty,
                              bool &collection_was_deleted) {
    if (vec) {
        if (!vec->empty()) {
            auto first_to_remove = std::stable_partition(vec->begin(), vec->end(),
                                                         [thus](const T *pi) { return pi != thus; });
            vec->erase(first_to_remove, vec->end());
        }
        if (vec->empty() && delete_collection_if_empty) {
            delete vec;
            vec = nullptr;
            collection_was_deleted = true;
        }
    }
}

#endif //VEGA_STRIKE_VEGA_COLLECTION_UTILS_H
