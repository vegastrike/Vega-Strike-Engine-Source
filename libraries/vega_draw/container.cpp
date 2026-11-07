/*
 * container.cpp
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
#include "vega_draw/widget.h"

namespace vega_draw {

void Container::addChild(std::unique_ptr<Widget> child) {
    child->m_parent = this;
    m_children.push_back(std::move(child));
}

Widget *Container::findById(const std::string &id) const {
    for (std::size_t i = 0; i < m_children.size(); ++i) {
        if (m_children[i]->id() == id) {
            return m_children[i].get();
        }
        Container *nested = dynamic_cast<Container *>(m_children[i].get());
        if (nested != nullptr) {
            Widget *found = nested->findById(id);
            if (found != nullptr) {
                return found;
            }
        }
    }
    return nullptr;
}

Widget *Container::childAt(const Point &p) const {
    // Topmost first (children added later draw on top).
    for (std::size_t i = m_children.size(); i > 0; --i) {
        Widget *child = m_children[i - 1].get();
        if (child->hitTest(p)) {
            return child;
        }
    }
    return nullptr;
}

void Container::draw(ImDrawList *draw_list, const Viewport &viewport) {
    for (std::size_t i = 0; i < m_children.size(); ++i) {
        if (!m_children[i]->hidden()) {
            m_children[i]->draw(draw_list, viewport);
        }
    }
}

bool Container::onMouseDown(const InputEvent &event) {
    Widget *child = childAt(event.loc);
    if (child != nullptr && child->onMouseDown(event)) {
        m_mouse_capture = child;
        return true;
    }
    return false;
}

bool Container::onMouseUp(const InputEvent &event) {
    Widget *target = (m_mouse_capture != nullptr) ? m_mouse_capture : childAt(event.loc);
    const bool handled = target != nullptr && target->onMouseUp(event);
    m_mouse_capture = nullptr;
    return handled;
}

bool Container::onMouseMove(const InputEvent &event) {
    Widget *child = childAt(event.loc);
    return child != nullptr && child->onMouseMove(event);
}

bool Container::onMouseDrag(const InputEvent &event) {
    Widget *target = (m_mouse_capture != nullptr) ? m_mouse_capture : childAt(event.loc);
    return target != nullptr && target->onMouseDrag(event);
}

} // namespace vega_draw
