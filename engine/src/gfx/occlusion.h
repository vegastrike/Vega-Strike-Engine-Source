/*
 * occlusion.h
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
#ifndef VEGA_STRIKE_ENGINE_GFX_OCCLUSION_H
#define VEGA_STRIKE_ENGINE_GFX_OCCLUSION_H

#include "root_generic/vs_globals.h"

#include "src/gfxlib.h"
#include "gfx_generic/vec.h"

namespace Occlusion {

/// Initialize occlusion system for a new frame
void /*GFXDRVAPI*/ start();

/// Finalize occlusion system for a frame, and free up resources
void /*GFXDRVAPI*/ end();

/**
 * Register an occluder
 *
 * @param pos The occluder object's center
 * @param rSize The occluder's radius
 * @param significant If false, the occluder may be ignored
 *          if there are other, more significant occluders.
 *          If true, it will be forcibly considered when
 *          rendering all objects. Ie: for planets.
 */
void /*GFXDRVAPI*/ addOccluder(const QVector &pos, float rSize, bool significant);

/**
 * Test occlusion between a light and an object
 *
 * @param lightPos The light's center
 * @param lightSize The light's radius
 * @param pos The object's center
 * @param rSize The object's radius
 *
 * @returns An occlusion factor, with 0 being fully occluded
 *          and 1 being fully clear.
 */
float /*GFXDRVAPI*/ testOcclusion(const QVector &lightPos, float lightSize, const QVector &pos, float rSize);

}

#endif //VEGA_STRIKE_ENGINE_GFX_OCCLUSION_H
