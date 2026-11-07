/*
 * text_input.cpp
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
#include "vega_draw/text_input.h"

#include "imgui/imgui.h"

#include "vega_draw/imgui_backend.h"

namespace vega_draw {

void TextInput::draw(ImDrawList *draw_list, const Viewport &viewport) {
    if (draw_list == nullptr || m_hidden) {
        return;
    }
    m_box.region_x = m_rect.origin.x;
    m_box.region_y = m_rect.origin.y;
    m_box.region_w = m_rect.size.width;
    m_box.region_h = m_rect.size.height;
    m_box.multiline = false;
    if (m_text.empty() && !m_placeholder.empty()) {
        DrawTextBox(draw_list, m_box, m_placeholder, viewport, IM_COL32(160, 160, 160, 255));
        return;
    }
    const ImU32 color = IM_COL32(m_text_color.r, m_text_color.g, m_text_color.b, m_text_color.a);
    DrawTextBox(draw_list, m_box, m_text, viewport, color);
}

bool TextInput::onKeyDown(const InputEvent &event) {
    const unsigned int code = event.code;
    if (code == 8) { // backspace
        if (!m_text.empty()) {
            m_text.pop_back();
        }
        return true;
    }
    if (code >= 32 && code < 127) {
        const char c = static_cast<char>(code);
        if (m_disallowed.find(c) != std::string::npos) {
            return false;
        }
        if (m_max_length != 0 && m_text.size() >= m_max_length) {
            return false;
        }
        m_text.push_back(c);
        return true;
    }
    return false;
}

} // namespace vega_draw
