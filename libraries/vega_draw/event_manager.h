/*
 * event_manager.h
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
#ifndef VEGA_STRIKE_LIBRARIES_VEGA_DRAW_EVENT_MANAGER_H
#define VEGA_STRIKE_LIBRARIES_VEGA_DRAW_EVENT_MANAGER_H

#include "vega_draw/widget.h"

namespace vega_draw {

// Routes grid input events into a widget tree. Events go to the modal widget if
// one is set, otherwise to the root.
class EventManager {
public:
    void setRoot(Widget *root) {
        m_root = root;
    }
    Widget *root() const {
        return m_root;
    }

    void setModal(Widget *modal) {
        m_modal = modal;
    }
    Widget *modal() const {
        return m_modal;
    }

    // Returns true if the event was handled.
    bool dispatch(const InputEvent &event);

private:
    Widget *target() const {
        return (m_modal != nullptr) ? m_modal : m_root;
    }

    Widget *m_root = nullptr;
    Widget *m_modal = nullptr;
};

} // namespace vega_draw

#endif // VEGA_STRIKE_LIBRARIES_VEGA_DRAW_EVENT_MANAGER_H
