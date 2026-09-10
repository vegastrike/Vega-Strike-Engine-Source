/*
 * sprite.cpp
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
#include "vega_draw/sprite.h"

#include "imgui/imgui.h"

#include "vega_draw/imgui_backend.h"

namespace vega_draw {

void TexturePanel::draw(ImDrawList *draw_list, const Viewport &viewport) {
    if (draw_list == nullptr || m_hidden) {
        return;
    }
    if (m_background.a > 0) {
        const ImU32 bg = IM_COL32(m_background.r, m_background.g, m_background.b, m_background.a);
        DrawRectFill(draw_list, m_rect, viewport, bg);
    }
    if (m_texture.texture != 0) {
        const ImU32 tint = IM_COL32(m_tint.r, m_tint.g, m_tint.b, m_tint.a);
        DrawSprite(draw_list, m_rect, m_texture, viewport, tint);
    }
    Container::draw(draw_list, viewport);
}

} // namespace vega_draw
