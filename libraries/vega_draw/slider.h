/*
 * slider.h
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
#ifndef VEGA_STRIKE_LIBRARIES_VEGA_DRAW_SLIDER_H
#define VEGA_STRIKE_LIBRARIES_VEGA_DRAW_SLIDER_H

#include "vega_draw/markup.h"
#include "vega_draw/scroller.h" // for ScrollOrientation
#include "vega_draw/widget.h"

namespace vega_draw {

struct SliderStyle {
    ScrollOrientation orientation = ScrollOrientation::Horizontal;
    float thumb_grid = 20.0f; // thumb length along the axis (grid)
    Color track{64, 64, 72, 200, true};
    Color thumb{180, 180, 196, 255, true};
    Color border{0, 0, 0, 0, true};
    float border_px = 1.0f;
};

// A value slider. Its rect is the track; pressing/dragging sets the value within
// [min, max].
class Slider : public Widget {
public:
    void setRange(float min, float max);
    float minValue() const {
        return m_min;
    }
    float maxValue() const {
        return m_max;
    }

    void setValue(float value);
    float value() const {
        return m_value;
    }
    float normalized() const; // 0..1 across the range

    void setOrientation(ScrollOrientation orientation) {
        m_style.orientation = orientation;
    }
    SliderStyle &style() {
        return m_style;
    }

    void draw(ImDrawList *draw_list, const Viewport &viewport) override;
    bool onMouseDown(const InputEvent &event) override;
    bool onMouseDrag(const InputEvent &event) override;
    bool onMouseUp(const InputEvent &event) override;

    // Grid rect of the thumb within the track (for drawing/hit-testing).
    Rect thumbRect() const;

private:
    void valueFromPoint(const Point &p);

    float m_min = 0.0f;
    float m_max = 1.0f;
    float m_value = 0.0f;
    SliderStyle m_style;
    bool m_dragging = false;
};

} // namespace vega_draw

#endif // VEGA_STRIKE_LIBRARIES_VEGA_DRAW_SLIDER_H
