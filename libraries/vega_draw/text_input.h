/*
 * text_input.h
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
#ifndef VEGA_STRIKE_LIBRARIES_VEGA_DRAW_TEXT_INPUT_H
#define VEGA_STRIKE_LIBRARIES_VEGA_DRAW_TEXT_INPUT_H

#include <cstddef>
#include <string>

#include "vega_draw/markup.h"
#include "vega_draw/text_box.h"
#include "vega_draw/widget.h"

namespace vega_draw {

// A single-line text input. Key codes are delivered as characters (the engine's
// event layer maps keys to codes): code 8 is backspace, 32..126 are characters.
class TextInput : public Widget {
public:
    void setText(const std::string &text) {
        m_text = text;
    }
    const std::string &text() const {
        return m_text;
    }

    void setPlaceholder(const std::string &placeholder) {
        m_placeholder = placeholder;
    }
    void setDisallowed(const std::string &disallowed) {
        m_disallowed = disallowed;
    }
    void setMaxLength(std::size_t max_length) {
        m_max_length = max_length;
    }

    TextBox &box() {
        return m_box;
    }
    Color &textColor() {
        return m_text_color;
    }

    void draw(ImDrawList *draw_list, const Viewport &viewport) override;
    bool onKeyDown(const InputEvent &event) override;

private:
    std::string m_text;
    std::string m_placeholder;
    std::string m_disallowed;
    std::size_t m_max_length = 0;
    TextBox m_box;
    Color m_text_color{255, 255, 255, 255, true};
};

} // namespace vega_draw

#endif // VEGA_STRIKE_LIBRARIES_VEGA_DRAW_TEXT_INPUT_H
