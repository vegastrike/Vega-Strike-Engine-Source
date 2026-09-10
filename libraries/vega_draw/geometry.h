/*
 * geometry.h
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2026 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
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
#ifndef VEGA_STRIKE_LIBRARIES_VEGA_DRAW_GEOMETRY_H
#define VEGA_STRIKE_LIBRARIES_VEGA_DRAW_GEOMETRY_H

#include "vega_draw/units.h"

// Plain 2D geometry for the vega_draw space: a 1000x1000 grid, top-left origin,
// y increasing downward. The same structs also carry pixel values (the mapper
// below converts grid -> pixels); the units are implied by context. Unlike the
// legacy guidefs Rect, top() is the smaller y.
namespace vega_draw {

struct Point {
    float x = 0.0f;
    float y = 0.0f;
};

struct Size {
    float width = 0.0f;
    float height = 0.0f;
};

struct Rect {
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;

    float left() const {
        return x;
    }
    float right() const {
        return x + width;
    }
    float top() const {
        return y; // y-down: the top edge is the smaller y
    }
    float bottom() const {
        return y + height;
    }
    Point center() const {
        return Point{x + width * 0.5f, y + height * 0.5f};
    }
    bool inside(const Point &p) const {
        return p.x >= x && p.x <= (x + width) && p.y >= y && p.y <= (y + height);
    }
    // A copy with `s` trimmed off every edge. `width`/`height` may go negative if
    // the inset exceeds the rect; callers clamp where it matters.
    Rect inset(const Size &s) const {
        return Rect{x + s.width, y + s.height, width - 2.0f * s.width, height - 2.0f * s.height};
    }
    Rect translated(float dx, float dy) const {
        return Rect{x + dx, y + dy, width, height};
    }
};

// Map a grid rect to pixels against a viewport.
inline Rect GridToPixelRect(const Rect &grid, const Viewport &viewport) noexcept {
    return Rect{
        GridToPixelX(grid.x, viewport),
        GridToPixelY(grid.y, viewport),
        GridToPixelW(grid.width, viewport),
        GridToPixelH(grid.height, viewport),
    };
}

// Map a grid point to pixels against a viewport.
inline Point GridToPixelPoint(const Point &grid, const Viewport &viewport) noexcept {
    return Point{GridToPixelX(grid.x, viewport), GridToPixelY(grid.y, viewport)};
}

} // namespace vega_draw

#endif // VEGA_STRIKE_LIBRARIES_VEGA_DRAW_GEOMETRY_H
