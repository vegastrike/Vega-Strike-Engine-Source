/*
 * imgui_backend.cpp
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
#include "vega_draw/imgui_backend.h"

#include <cfloat>

namespace vega_draw {

TextMetrics ImGuiTextMeasurer::Measure(const std::string &text, float font_px) const {
    TextMetrics metrics;
    ImFont *font = ImGui::GetFont();
    if (font != nullptr && font->IsLoaded()) {
        // Line height comes from a representative string; empty text measures to
        // zero width, which would also lose the height.
        const char *measure = text.empty() ? "Ag" : text.c_str();
        const ImVec2 size = font->CalcTextSizeA(font_px, FLT_MAX, 0.0f, measure);
        metrics.width = text.empty() ? 0.0f : size.x;
        metrics.height = size.y;
    } else {
        const ImVec2 size = ImGui::CalcTextSize(text.c_str());
        metrics.width = text.empty() ? 0.0f : size.x;
        metrics.height = size.y;
    }
    return metrics;
}

ImU32 ToImU32(const Color &color, ImU32 fallback) {
    if (!color.set) {
        return fallback;
    }
    return IM_COL32(color.r, color.g, color.b, color.a);
}

void DrawTextLayout(ImDrawList *draw_list,
                    const ImVec2 &origin,
                    const TextLayout &layout,
                    float font_px,
                    ImU32 default_color,
                    const ImVec4 *clip_rect) {
    if (draw_list == nullptr) {
        return;
    }
    for (std::size_t li = 0; li < layout.lines.size(); ++li) {
        const LaidOutLine &line = layout.lines[li];
        const float y = origin.y + line.y;
        for (std::size_t ri = 0; ri < line.runs.size(); ++ri) {
            const LaidOutRun &run = line.runs[ri];
            if (run.text.empty()) {
                continue;
            }
            const ImU32 color = ToImU32(run.style.color, default_color);
            const ImVec2 pos(origin.x + run.x, y);
            // Bold is carried in the run style but not yet rendered differently;
            // how <b> maps to a face/weight is a separate decision (single-weight
            // atlas today), and faking it with an offset shadow is deliberately
            // avoided.
            draw_list->AddText(nullptr, font_px, pos, color, run.text.c_str(), nullptr, 0.0f, clip_rect);
        }
    }
}

float DrawText(ImDrawList *draw_list,
               const Viewport &viewport,
               float grid_x,
               float grid_y,
               float font_grid,
               const std::string &markup,
               ImU32 default_color,
               const ImVec4 *clip_rect) {
    const float font_px = FontGridToPixel(font_grid, viewport);
    const ImGuiTextMeasurer measurer;
    const TextLines parsed = ParseMarkup(markup);

    TextStyle style;
    style.font_px = font_px;
    // The primitive does not wrap; explicit <br> breaks still produce lines.

    const TextLayout layout = LayoutText(parsed, style, measurer);
    const ImVec2 origin(GridToPixelX(grid_x, viewport), GridToPixelY(grid_y, viewport));
    DrawTextLayout(draw_list, origin, layout, font_px, default_color, clip_rect);
    return layout.height;
}

} // namespace vega_draw
