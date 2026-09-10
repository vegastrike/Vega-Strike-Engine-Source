/*
 * scroller.cpp
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
#include "vega_draw/scroller.h"

namespace vega_draw {
namespace {

float axis_length(const Rect &track, ScrollOrientation orientation) {
    return (orientation == ScrollOrientation::Vertical) ? track.size.height : track.size.width;
}

float axis_origin(const Rect &track, ScrollOrientation orientation) {
    return (orientation == ScrollOrientation::Vertical) ? track.origin.y : track.origin.x;
}

float cross_origin(const Rect &track, ScrollOrientation orientation) {
    return (orientation == ScrollOrientation::Vertical) ? track.origin.x : track.origin.y;
}

float cross_length(const Rect &track, ScrollOrientation orientation) {
    return (orientation == ScrollOrientation::Vertical) ? track.size.width : track.size.height;
}

float thumb_length(const ScrollerStyle &style, const ScrollerModel &model) {
    const float track_length = axis_length(style.track, style.orientation);
    float visible_fraction = 1.0f;
    if (model.content_extent > 0.0f) {
        visible_fraction = model.viewport_extent / model.content_extent;
    }
    if (visible_fraction < 0.0f) {
        visible_fraction = 0.0f;
    }
    if (visible_fraction > 1.0f) {
        visible_fraction = 1.0f;
    }
    float length = track_length * visible_fraction;
    if (length < style.min_thumb_grid) {
        length = style.min_thumb_grid;
    }
    if (length > track_length) {
        length = track_length;
    }
    return length;
}

} // namespace

float ScrollerMaxPosition(const ScrollerModel &model) {
    const float max_position = model.content_extent - model.viewport_extent;
    return (max_position > 0.0f) ? max_position : 0.0f;
}

float ScrollerClampPosition(const ScrollerModel &model, float position) {
    const float max_position = ScrollerMaxPosition(model);
    if (position < 0.0f) {
        return 0.0f;
    }
    if (position > max_position) {
        return max_position;
    }
    return position;
}

Rect ScrollerThumbRect(const ScrollerStyle &style, const ScrollerModel &model) {
    const float track_length = axis_length(style.track, style.orientation);
    const float origin = axis_origin(style.track, style.orientation);
    const float c_origin = cross_origin(style.track, style.orientation);
    const float c_length = cross_length(style.track, style.orientation);
    const float length = thumb_length(style, model);
    const float max_position = ScrollerMaxPosition(model);

    float offset = 0.0f;
    if (max_position > 0.0f && track_length > length) {
        offset = (track_length - length) * (ScrollerClampPosition(model, model.position) / max_position);
    }

    if (style.orientation == ScrollOrientation::Vertical) {
        return Rect{c_origin, origin + offset, c_length, length};
    }
    return Rect{origin + offset, c_origin, length, c_length};
}

float ScrollerPositionForThumbCentre(const ScrollerStyle &style,
                                     const ScrollerModel &model,
                                     float track_point_grid) {
    const float track_length = axis_length(style.track, style.orientation);
    const float origin = axis_origin(style.track, style.orientation);
    const float length = thumb_length(style, model);
    const float max_position = ScrollerMaxPosition(model);
    if (max_position <= 0.0f || track_length <= length) {
        return 0.0f;
    }
    float travel = track_point_grid - origin - length * 0.5f;
    if (travel < 0.0f) {
        travel = 0.0f;
    }
    if (travel > track_length - length) {
        travel = track_length - length;
    }
    return max_position * (travel / (track_length - length));
}

} // namespace vega_draw
