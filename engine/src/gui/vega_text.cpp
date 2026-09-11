/*
 * vega_text.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2026 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file.
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
#include "gui/vega_text.h"

#include <utility>

#include "configuration/configuration.h"
#include "gfx/hud.h"
#include "gldrv/mouse_cursor.h"
#include "gui/guidefs.h"

#include "imgui.h"

#include "vega_draw/imgui_backend.h"
#include "vega_draw/legacy_text.h"
#include "vega_draw/text_layout.h"

namespace vega_text {
namespace {

bool IsTransparent(ImU32 color) {
    return ((color >> IM_COL32_A_SHIFT) & 0xFF) == 0;
}

} // namespace

void DrawTextPlane(TextPlane &plane, const std::string &text, bool transparent) {
    if (text.empty()) {
        return;
    }

    float pos_x = 0.0f;
    float pos_y = 0.0f;
    plane.GetPos(pos_y, pos_x);
    float size_w = 0.0f;
    float size_h = 0.0f;
    plane.GetSize(size_w, size_h);
    float res_w = 0.0f;
    float res_h = 0.0f;
    plane.layoutResolution(res_w, res_h);
    float off_x = 0.0f;
    float off_y = 0.0f;
    plane.layoutOffset(off_x, off_y);

    float anchor_x = 0.0f;
    float anchor_y = 0.0f;
    float wrap_px = 0.0f;
    if (res_w > 0.0f && res_h > 0.0f) {
        // Lay out against the base resolution and shift into the letterboxed
        // window, so the width and the position use the same space.
        anchor_x = Coordinates::normToPixelX(pos_x, res_w) + off_x;
        anchor_y = Coordinates::normToPixelY(pos_y, res_h) + off_y;
        wrap_px = Coordinates::normToPixelW(size_w, res_w) * 1.05f;
    } else {
        const std::pair<int, int> native = CalculateAbsoluteXY(pos_x, pos_y);
        anchor_x = static_cast<float>(native.first);
        anchor_y = static_cast<float>(native.second);
        wrap_px = Coordinates::normToPixelW(size_w) * 1.05f;
    }

    // The engine draws all text at the user-set Text Height.
    const float font_px = configuration().graphics.font_point_flt;

    // The base text grows one character at a time while a character speaks, so
    // drop any not-yet-complete colour token rather than flashing it literally.
    const vega_draw::TextLines lines =
            vega_draw::ParseLegacyVegaText(text, vega_draw::LegacyTextDialect::TextPlane, true);

    vega_draw::TextStyle style;
    style.font_px = font_px;
    style.wrap = true;
    style.wrap_width_px = wrap_px;

    const vega_draw::ImGuiTextMeasurer measurer;
    const vega_draw::TextLayout layout = vega_draw::LayoutText(lines, style, measurer);

    const ImU32 background = (transparent || IsTransparent(plane.background_color))
            ? 0u
            : plane.background_color;

    vega_draw::DrawTextLayout(ImGui::GetBackgroundDrawList(),
                              ImVec2(anchor_x, anchor_y),
                              layout,
                              font_px,
                              plane.color,
                              nullptr,
                              0,
                              background);
}

} // namespace vega_text
