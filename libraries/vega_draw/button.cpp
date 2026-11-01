/*
 * button.cpp
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
#include "vega_draw/button.h"

#include "imgui/imgui.h"

#include "vega_draw/imgui_backend.h"
#include "vega_draw/text_box.h"

namespace vega_draw {
namespace {

ImU32 ToU32(const Color &c) {
    return IM_COL32(c.r, c.g, c.b, c.a);
}

} // namespace

void PushButton::draw(ImDrawList *draw_list, const Viewport &viewport) {
    if (draw_list == nullptr || m_hidden) {
        return;
    }
    Color face = m_style.face;
    Color text = m_style.text;
    if (!m_enabled) {
        face.a = static_cast<std::uint8_t>(face.a / 2);
        text.a = static_cast<std::uint8_t>(text.a / 2);
    } else if (m_down) {
        face = m_style.down;
        text = m_style.down_text;
    } else if (m_highlight) {
        face = m_style.highlight;
        text = m_style.highlight_text;
    }

    DrawRectFill(draw_list, m_rect, viewport, ToU32(face));
    if (m_style.border.a > 0) {
        DrawRectOutline(draw_list, m_rect, viewport, ToU32(m_style.border), m_style.border_px);
    }
    if (m_enabled) {
        if (m_down) {
            DrawUpLeftShadow(draw_list, m_rect, viewport, ToU32(m_style.shadow_dark), m_style.shadow_px);
            DrawLowRightShadow(draw_list, m_rect, viewport, ToU32(m_style.shadow_light), m_style.shadow_px);
        } else {
            DrawLowRightShadow(draw_list, m_rect, viewport, ToU32(m_style.shadow_dark), m_style.shadow_px);
            DrawUpLeftShadow(draw_list, m_rect, viewport, ToU32(m_style.shadow_light), m_style.shadow_px);
        }
    }

    // Centred label.
    TextBox box;
    box.region_x = m_rect.origin.x;
    box.region_y = m_rect.origin.y;
    box.region_w = m_rect.size.width;
    box.region_h = m_rect.size.height;
    box.font_grid = m_style.font_grid;
    box.multiline = false;
    box.justification = Justification::Center;
    DrawTextBox(draw_list, box, m_label, viewport, ToU32(text));
}

bool PushButton::onMouseDown(const InputEvent &event) {
    if (!m_enabled || !hitTest(event.loc)) {
        return false;
    }
    m_down = true;
    return true;
}

bool PushButton::onMouseUp(const InputEvent &event) {
    if (!m_down) {
        return false;
    }
    const bool inside = hitTest(event.loc);
    m_down = false;
    if (inside && m_enabled) {
        fireCommand();
    }
    return true;
}

bool PushButton::onMouseMove(const InputEvent &event) {
    m_highlight = m_enabled && hitTest(event.loc);
    return false;
}

void PushButton::fireCommand() {
    if (m_target != nullptr) {
        m_target->onCommand(m_command, this);
        return;
    }
    if (m_parent != nullptr) {
        m_parent->onCommand(m_command, this);
    }
}

} // namespace vega_draw
