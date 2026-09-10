/*
 * text_display.cpp
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
#include "vega_draw/text_display.h"

#include "imgui/imgui.h"

#include "vega_draw/imgui_backend.h"

namespace vega_draw {

void TextDisplay::draw(ImDrawList *draw_list, const Viewport &viewport) {
    if (draw_list == nullptr || m_hidden) {
        return;
    }
    m_box.region_x = m_rect.origin.x;
    m_box.region_y = m_rect.origin.y;
    m_box.region_w = m_rect.size.width;
    m_box.region_h = m_rect.size.height;
    const ImU32 color = IM_COL32(m_text_color.r, m_text_color.g, m_text_color.b, m_text_color.a);
    DrawTextBox(draw_list, m_box, m_text, viewport, color);
}

} // namespace vega_draw
