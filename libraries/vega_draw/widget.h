/*
 * widget.h
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
#ifndef VEGA_STRIKE_LIBRARIES_VEGA_DRAW_WIDGET_H
#define VEGA_STRIKE_LIBRARIES_VEGA_DRAW_WIDGET_H

#include <memory>
#include <string>
#include <vector>

#include "vega_draw/geometry.h"

struct ImDrawList; // forward declaration (drawing is adapter-implemented)

// vega_draw widget framework: a 1000-grid, top-left/y-down UI tree with grid
// input events. The layout/hit-test/event core is renderer-agnostic; drawing is
// implemented against ImDrawList by the adapter.
namespace vega_draw {

enum class InputType {
    MouseDown,
    MouseUp,
    MouseMove,
    MouseDrag,
    KeyDown,
    KeyUp
};

// A grid-space input event (mouse loc is on the 1000-grid).
struct InputEvent {
    InputType type = InputType::MouseMove;
    Point loc{};
    unsigned int code = 0; // mouse button or key
    unsigned int mask = 0; // modifier mask
};

using EventCommand = std::string;
class Widget;

class Widget {
public:
    virtual ~Widget() = default;

    Rect rect() const {
        return m_rect;
    }
    void setRect(const Rect &r) {
        m_rect = r;
    }

    const std::string &id() const {
        return m_id;
    }
    void setId(const std::string &id) {
        m_id = id;
    }

    bool hidden() const {
        return m_hidden;
    }
    void setHidden(bool hidden = true) {
        m_hidden = hidden;
    }

    // Whether a grid point is inside this (visible) widget.
    bool hitTest(const Point &p) const {
        return !m_hidden && m_rect.inside(p);
    }

    // Draw (adapter-implemented against the ImGui draw list).
    virtual void draw(ImDrawList *draw_list, const Viewport &viewport) = 0;

    // Event handlers. Return true if the event was handled (stops routing).
    virtual bool onMouseDown(const InputEvent &) {
        return false;
    }
    virtual bool onMouseUp(const InputEvent &) {
        return false;
    }
    virtual bool onMouseMove(const InputEvent &) {
        return false;
    }
    virtual bool onMouseDrag(const InputEvent &) {
        return false;
    }
    virtual bool onKeyDown(const InputEvent &) {
        return false;
    }
    virtual bool onKeyUp(const InputEvent &) {
        return false;
    }

    // A command from a child (e.g. a button press). Return true if handled.
    virtual bool onCommand(const EventCommand &, Widget *) {
        return false;
    }

protected:
    Rect m_rect;
    std::string m_id;
    bool m_hidden = false;
};

// A container holds children and routes events to the topmost hit child.
class Container : public Widget {
public:
    void addChild(std::unique_ptr<Widget> child);
    Widget *findById(const std::string &id) const;

    void draw(ImDrawList *draw_list, const Viewport &viewport) override;

    bool onMouseDown(const InputEvent &event) override;
    bool onMouseUp(const InputEvent &event) override;
    bool onMouseMove(const InputEvent &event) override;
    bool onMouseDrag(const InputEvent &event) override;

    const std::vector<std::unique_ptr<Widget>> &children() const {
        return m_children;
    }

private:
    // The topmost visible child containing `p`, or nullptr.
    Widget *childAt(const Point &p) const;

    std::vector<std::unique_ptr<Widget>> m_children;
};

} // namespace vega_draw

#endif // VEGA_STRIKE_LIBRARIES_VEGA_DRAW_WIDGET_H
