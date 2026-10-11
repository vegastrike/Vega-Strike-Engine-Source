/*
 * scroll_bar.h
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
#ifndef VEGA_STRIKE_LIBRARIES_VEGA_DRAW_SCROLL_BAR_H
#define VEGA_STRIKE_LIBRARIES_VEGA_DRAW_SCROLL_BAR_H

#include "vega_draw/scroller.h"
#include "vega_draw/widget.h"

namespace vega_draw {

// A scroll-bar widget: wraps the scroller utility. Its rect is the track; a
// press/drag positions the thumb. The owner reads position() (or polls after
// events).
class ScrollBar : public Widget {
public:
    void setContentExtent(float extent) {
        m_model.content_extent = extent;
    }
    void setViewportExtent(float extent) {
        m_model.viewport_extent = extent;
    }
    float contentExtent() const {
        return m_model.content_extent;
    }
    float viewportExtent() const {
        return m_model.viewport_extent;
    }

    void setPosition(float position) {
        m_model.position = ScrollerClampPosition(m_model, position);
    }
    float position() const {
        return m_model.position;
    }
    float maxPosition() const {
        return ScrollerMaxPosition(m_model);
    }

    void setOrientation(ScrollOrientation orientation) {
        m_style.orientation = orientation;
    }
    ScrollOrientation orientation() const {
        return m_style.orientation;
    }
    void setMinThumbGrid(float min_thumb) {
        m_style.min_thumb_grid = min_thumb;
    }

    void draw(ImDrawList *draw_list, const Viewport &viewport) override;
    bool onMouseDown(const InputEvent &event) override;
    bool onMouseDrag(const InputEvent &event) override;
    bool onMouseUp(const InputEvent &event) override;

private:
    float axisCoord(const Point &p) const;
    void positionToPoint(const Point &p);

    ScrollerModel m_model;
    ScrollerStyle m_style;
    bool m_dragging = false;
};

} // namespace vega_draw

#endif // VEGA_STRIKE_LIBRARIES_VEGA_DRAW_SCROLL_BAR_H
