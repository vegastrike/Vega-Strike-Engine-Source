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
    Point origin;
    Size size;

    Rect() = default;
    Rect(float x, float y, float width, float height) : origin{x, y}, size{width, height} {
    }
    Rect(const Point &o, const Size &s) : origin(o), size(s) {
    }

    float left() const {
        return origin.x;
    }
    float right() const {
        return origin.x + size.width;
    }
    float top() const {
        return origin.y; // y-down: the top edge is the smaller y
    }
    float bottom() const {
        return origin.y + size.height;
    }
    Point center() const {
        return Point{origin.x + size.width * 0.5f, origin.y + size.height * 0.5f};
    }
    bool inside(const Point &p) const {
        return p.x >= left() && p.x < right() && p.y >= top() && p.y < bottom();
    }
    // Trim `s` off every edge, in place (width/height may go negative; callers clamp).
    void inset(const Size &s) {
        origin.x += s.width;
        origin.y += s.height;
        size.width -= 2.0f * s.width;
        size.height -= 2.0f * s.height;
    }
    Rect copyAndInset(const Size &s) const {
        Rect result = *this;
        result.inset(s);
        return result;
    }
    Rect translated(float dx, float dy) const {
        return Rect{origin.x + dx, origin.y + dy, size.width, size.height};
    }
};

// Map a grid rect to pixels against a viewport.
inline Rect GridToPixelRect(const Rect &grid, const Viewport &viewport) noexcept {
    return Rect{
        GridToPixelX(grid.origin.x, viewport),
        GridToPixelY(grid.origin.y, viewport),
        GridToPixelW(grid.size.width, viewport),
        GridToPixelH(grid.size.height, viewport),
    };
}

// Map a grid point to pixels against a viewport.
inline Point GridToPixelPoint(const Point &grid, const Viewport &viewport) noexcept {
    return Point{GridToPixelX(grid.x, viewport), GridToPixelY(grid.y, viewport)};
}

} // namespace vega_draw

#endif // VEGA_STRIKE_LIBRARIES_VEGA_DRAW_GEOMETRY_H
