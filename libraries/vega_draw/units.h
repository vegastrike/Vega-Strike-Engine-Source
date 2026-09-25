/*
 * units.h
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2026 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy, Evert Vorster
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
#ifndef VEGA_STRIKE_LIBRARIES_VEGA_DRAW_UNITS_H
#define VEGA_STRIKE_LIBRARIES_VEGA_DRAW_UNITS_H

#include <cmath>

// vega_draw coordinate model: a single 1000x1000 authoring & placement space,
// top-left origin, y increasing downward. Position, region, box size, line
// advance and font size are all authored as integers on this grid. The grid is
// NEVER a render resolution -- it is converted to pixels only at the render
// boundary (the Viewport), and font sizes are rounded to a whole pixel there so
// glyphs stay crisp.
namespace vega_draw {

// The full extent of the authoring grid on each axis.
constexpr float kGridExtent = 1000.0f;

// The pixel size text is rendered into. Supplied by the screen/frame setup.
struct Viewport {
    float width_px = 0.0f;
    float height_px = 0.0f;
};

inline bool operator==(const Viewport &a, const Viewport &b) noexcept {
    return a.width_px == b.width_px && a.height_px == b.height_px;
}

inline bool operator!=(const Viewport &a, const Viewport &b) noexcept {
    return !(a == b);
}

// 1000-grid x -> pixel x (x maps across the width).
inline float GridToPixelX(float grid_x, const Viewport &viewport) noexcept {
    return grid_x / kGridExtent * viewport.width_px;
}

// 1000-grid y -> pixel y (y maps down the height).
inline float GridToPixelY(float grid_y, const Viewport &viewport) noexcept {
    return grid_y / kGridExtent * viewport.height_px;
}

// 1000-grid length -> pixel width.
inline float GridToPixelW(float grid_w, const Viewport &viewport) noexcept {
    return grid_w / kGridExtent * viewport.width_px;
}

// 1000-grid length -> pixel height.
inline float GridToPixelH(float grid_h, const Viewport &viewport) noexcept {
    return grid_h / kGridExtent * viewport.height_px;
}

// Font size is a fraction of the space height, authored per-1000 (2% -> 20).
// Converted to a whole pixel height, so the baked glyph atlas is used at its
// native size (scale 1.0) and text stays crisp.
inline float FontGridToPixel(float font_grid, const Viewport &viewport) noexcept {
    const float px = font_grid / kGridExtent * viewport.height_px;
    return std::round(px);
}

// Standard font-height tiers, authored per-1000 (a fraction of the space height).
// The values are integers, so they can be a real enum -- the old fractional sizes
// (0.04..0.06) could not. These are the single source of truth for the standard
// text sizes used by code and by data (controls.json names resolve to these).
enum class FontHeight : int {
    ExtraSmall = 40, // was 0.04
    Small = 45,      // was 0.045
    Medium = 50,     // was 0.05
    Large = 55,      // was 0.055
    ExtraLarge = 60  // was 0.06
};

constexpr float ToFontGrid(FontHeight height) noexcept {
    return static_cast<float>(static_cast<int>(height));
}

} // namespace vega_draw

#endif // VEGA_STRIKE_LIBRARIES_VEGA_DRAW_UNITS_H
