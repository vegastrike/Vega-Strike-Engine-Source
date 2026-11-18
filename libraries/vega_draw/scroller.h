/*
 * scroller.h
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
#ifndef VEGA_STRIKE_LIBRARIES_VEGA_DRAW_SCROLLER_H
#define VEGA_STRIKE_LIBRARIES_VEGA_DRAW_SCROLLER_H

#include "vega_draw/geometry.h"

// Scroller: a scroll-bar widget. Renderer-agnostic core: the scroll model, the
// thumb geometry (on the 1000-grid) and the position math. The ImGui adapter
// draws the track and thumb.
//
// The content/viewport extents are in the same linear unit as the content (e.g.
// pixels); the track is authored on the 1000-grid.
namespace vega_draw {

enum class ScrollOrientation {
    Vertical,
    Horizontal
};

struct ScrollerModel {
    float content_extent = 0.0f;  // total content length
    float viewport_extent = 0.0f; // visible length
    float position = 0.0f;        // scroll offset, within [0, ScrollerMaxPosition]
};

struct ScrollerStyle {
    Rect track;                   // 1000-grid
    ScrollOrientation orientation = ScrollOrientation::Vertical;
    float min_thumb_grid = 20.0f; // minimum thumb length (grid units)
};

// Largest valid scroll position (0 when the content fits).
float ScrollerMaxPosition(const ScrollerModel &model);

// Clamp a position into [0, ScrollerMaxPosition].
float ScrollerClampPosition(const ScrollerModel &model, float position);

// Thumb rectangle within the track, on the 1000-grid.
Rect ScrollerThumbRect(const ScrollerStyle &style, const ScrollerModel &model);

// The scroll position that centres the thumb on `track_point_grid` (a point along
// the track axis, in grid units) -- for click-to-jump and dragging the thumb.
float ScrollerPositionForThumbCentre(const ScrollerStyle &style,
                                     const ScrollerModel &model,
                                     float track_point_grid);

} // namespace vega_draw

#endif // VEGA_STRIKE_LIBRARIES_VEGA_DRAW_SCROLLER_H
