/*
 * Copyright (C) 2001-2022 Daniel Horn, pyramid3d, Stephen G. Tuggy,
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

#include "atmospheric_fog_mesh.h"

AtmosphericFogMesh::AtmosphericFogMesh()
{
    er = eg = eb = ea = .5;
    dr = dg = db = da = .5;
    meshname = "sphereatm.bfxm";
    scale = 1.05;
    focus = .5;
    concavity = 0;
    tail_mode_start = -1;
    tail_mode_end = -1;
    min_alpha = 0;
    max_alpha = 255;
}
