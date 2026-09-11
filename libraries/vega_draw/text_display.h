/*
 * text_display.h
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
#ifndef VEGA_STRIKE_LIBRARIES_VEGA_DRAW_TEXT_DISPLAY_H
#define VEGA_STRIKE_LIBRARIES_VEGA_DRAW_TEXT_DISPLAY_H

#include <string>

#include "vega_draw/text_box.h"
#include "vega_draw/widget.h"

namespace vega_draw {

// A widget that displays mark-up text via the text box. Its rect drives the box
// region on each draw; margins/font/wrap/justification are configured through
// box().
class TextDisplay : public Widget {
public:
    void setText(const std::string &text) {
        m_text = text;
    }
    const std::string &text() const {
        return m_text;
    }

    TextBox &box() {
        return m_box;
    }
    const TextBox &box() const {
        return m_box;
    }

    Color &textColor() {
        return m_text_color;
    }

    void draw(ImDrawList *draw_list, const Viewport &viewport) override;

private:
    std::string m_text;
    TextBox m_box;
    Color m_text_color{255, 255, 255, 255, true};
};

} // namespace vega_draw

#endif // VEGA_STRIKE_LIBRARIES_VEGA_DRAW_TEXT_DISPLAY_H
