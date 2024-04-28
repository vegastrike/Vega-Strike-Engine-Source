/*
 * vega_collection_utils.h
 *
 * Copyright (C) 2001-2024 Daniel Horn, pyramid3d, Stephen G. Tuggy,
 * Benjamen R. Meyer, Roy Falk, and other Vega Strike contributors
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

#ifndef VEGA_STRIKE_VEGA_COLLECTION_UTILS_H
#define VEGA_STRIKE_VEGA_COLLECTION_UTILS_H

#include <vector>
#include <functional>
#include "owner.h"

template<typename T> void remove_all_references_to(const T *thus, owner<std::vector<T *> *> &vec, bool& collection_was_deleted);

#endif //VEGA_STRIKE_VEGA_COLLECTION_UTILS_H
