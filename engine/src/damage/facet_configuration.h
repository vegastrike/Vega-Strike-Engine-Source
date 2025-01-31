/*
 * Copyright (C) 2021-2023 Roy Falk, Stephen G. Tuggy, Benjamen R. Meyer,
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
#ifndef VEGA_STRIKE_ENGINE_DAMAGE_FACET_CONFIGURATION_H
#define VEGA_STRIKE_ENGINE_DAMAGE_FACET_CONFIGURATION_H

#include "core_vector.h"
#include <float.h>
#include <string>

/**
 * @brief The FacetConfiguration enum represents how many facets a layer has and it what configuration.
 * @details The use of enum class enforces the correctness of the parameter without checking.
 * You can't pass 17 for example.
 * The game originally supported 8 facets. 
 * However, I disabled all facets except 1,2 and 4 due to complexity.
 */
enum class FacetConfiguration {
    one = 1,    // A single facet covering all directions
    two = 2,    // Front and back
    four = 4,   // Front, back, left and right
};


#endif //VEGA_STRIKE_ENGINE_DAMAGE_FACET_CONFIGURATION_H
